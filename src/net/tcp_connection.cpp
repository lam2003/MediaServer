#define SMS_CLASS "TcpConnection"

#include <net/tcp_connection.h>
#include <net/socket_utils.h>
#include <net/connection_manager.h>
#include <dep/dep_libuv.h>
#include <common/logger.h>

namespace sms
{
    inline static void on_close(uv_handle_t *handle)
    {
        handle->data = nullptr;
        delete handle;
    }

    inline static void on_shutdown(uv_shutdown_t *req, int status)
    {
        uv_stream_t *handle = req->handle;
        delete req;
        uv_close(reinterpret_cast<uv_handle_t *>(handle), static_cast<uv_close_cb>(on_close));
    }

    inline static void on_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
    {
        TcpConnection *conn = reinterpret_cast<TcpConnection *>(handle->data);
        if (conn)
        {
            if (!ConnectionManager::Instance().IsExist(conn))
            {
                return;
            }
            conn->OnUvRead(nread, buf);
        }
    }

    inline static void on_write(uv_write_t *req, int status)
    {
        uv_stream_t *handle = req->handle;
        TcpConnection *conn = reinterpret_cast<TcpConnection *>(handle->data);
        TcpConnection::UvWriteData *write_data = reinterpret_cast<TcpConnection::UvWriteData *>(req->data);

        if (conn)
        {
            if (!ConnectionManager::Instance().IsExist(conn))
            {
                delete write_data;
                return;
            }
            conn->OnUvWrite(status, std::move(write_data->cb));
        }

        delete write_data;
    }

    inline static void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
    {
        TcpConnection *conn = reinterpret_cast<TcpConnection *>(handle->data);
        if (conn)
        {
            if (!ConnectionManager::Instance().IsExist(conn))
            {
                return;
            }
            conn->OnUvAlloc(suggested_size, buf);
        }
    }

    TcpConnection::TcpConnection(size_t buffer_size)
    {
        buffer_.SetCapacity(buffer_size + 1);
        uv_handle_ = new uv_tcp_t;
        uv_handle_->data = static_cast<void *>(this);

        SetReadCB(nullptr);
        SetClosedCB(nullptr);
        SetErrorCB(nullptr);

        ConnectionManager::Instance().Register(this);
    }

    TcpConnection::~TcpConnection()
    {
        Close(false);
    }

    int TcpConnection::Setup(struct sockaddr_storage *local_addr,
                             const std::string &local_ip,
                             uint16_t local_port,
                             ClosedCB &&closed_cb)
    {
        if (closed_)
        {
            return -1;
        }

        int ret = uv_tcp_init(DepLibUV::GetLoop(), uv_handle_);
        if (ret != 0)
        {
            LOG_E << "uv_tcp_init failed: " << uv_strerror(ret);
            return -1;
        }

        local_addr_ = local_addr;
        local_ip_ = local_ip;
        local_port_ = local_port;
        SetClosedCB(std::move(closed_cb));

        return 0;
    }

    int TcpConnection::Start()
    {
        if (closed_)
        {
            return -1;
        }

        if (!uv_handle_->loop)
        {
            LOG_E << "error start tcp connection before setup";
            return -1;
        }

        if (uv_handle_->read_cb)
        {
            return -1;
        }

        int ret = uv_read_start(
            reinterpret_cast<uv_stream_t *>(uv_handle_),
            static_cast<uv_alloc_cb>(on_alloc),
            static_cast<uv_read_cb>(on_read));
        if (ret != 0)
        {
            LOG_E << "uv_read_start() failed: " << uv_strerror(ret);
            return -1;
        }

        if (!set_peer_addr())
        {
            LOG_E << "error setting peer IP and port";
            return -1;
        }

        return 0;
    }

    void TcpConnection::Write(const Buffer::Ptr &buf, WriteCB &&cb)
    {
        if (closed_)
        {
            if (cb)
            {
                cb(false);
            }
            return;
        }

        if (!uv_handle_->loop)
        {
            LOG_E << "error send before setup";
            if (cb)
            {
                cb(false);
            }
            return;
        }

        if (!buf->Size())
        {
            if (cb)
            {
                cb(false);
            }
            return;
        }

        uv_buf_t buffer = uv_buf_init(buf->Data(), buf->Size());
        int written = uv_try_write(reinterpret_cast<uv_stream_t *>(uv_handle_),
                                   &buffer,
                                   1);

        if (written < 0)
        {
            if (written == UV_EAGAIN || written == UV_EBUSY)
            {
                written = 0;
            }
            else
            {
                LOG_E << "uv_try_write() failed: " << uv_strerror(written);
                if (cb)
                {
                    cb(false);
                }
                return;
            }
        }
        else
        {
            if (written == static_cast<int>(buf->Size()))
            {
                sent_bytes_ += written;
                if (cb)
                {
                    cb(true);
                }
                return;
            }
        }

        UvWriteData *write_data = new UvWriteData(buf);

        write_data->req.data = write_data;
        write_data->cb = std::move(cb);

        int ret = uv_write(&write_data->req,
                           reinterpret_cast<uv_stream_t *>(uv_handle_),
                           &buffer,
                           1,
                           static_cast<uv_write_cb>(on_write));
        if (ret != 0)
        {
            LOG_E << "uv_write() failed: " << uv_strerror(ret);
            if (cb)
            {
                cb(false);
            }
            delete write_data;
        }
        else
        {
            sent_bytes_ += buf->Size();
        }
    }

    void TcpConnection::Write(const BufferList::Ptr &list, WriteCB &&cb)
    {
        if (closed_)
        {
            if (cb)
            {
                cb(false);
            }
            return;
        }

        if (!uv_handle_->loop)
        {
            LOG_E << "error send before setup";
            if (cb)
            {
                cb(false);
            }
            return;
        }

        if (list->Empty())
        {
            if (cb)
            {
                cb(false);
            }
            return;
        }

        int written = uv_try_write(reinterpret_cast<uv_stream_t *>(uv_handle_),
                                   list->GetUvBuf(),
                                   list->GetUvBufNum());

        if (written < 0)
        {
            if (written == UV_EAGAIN || written == UV_EBUSY)
            {
                written = 0;
            }
            else
            {
                LOG_E << "uv_try_write() failed: " << uv_strerror(written);
                if (cb)
                {
                    cb(false);
                }
                return;
            }
        }
        else
        {
            list->ReOffset(written);
            sent_bytes_ += written;

            if (list->Empty())
            {
                if (cb)
                {
                    cb(true);
                }
                return;
            }
        }

        UvWriteData *write_data = new UvWriteData(list);

        write_data->req.data = write_data;
        write_data->cb = std::move(cb);

        int ret = uv_write(&write_data->req,
                           reinterpret_cast<uv_stream_t *>(uv_handle_),
                           list->GetUvBuf(),
                           list->GetUvBufNum(),
                           static_cast<uv_write_cb>(on_write));
        if (ret != 0)
        {
            LOG_E << "uv_write() failed: " << uv_strerror(ret);
            if (cb)
            {
                cb(false);
            }
            delete write_data;
        }
        else
        {
            sent_bytes_ += list->GetRemainSize();
        }
    }

    void TcpConnection::SetReadCB(ReadCB &&read_cb)
    {
        if (read_cb)
        {
            read_cb_ = std::move(read_cb);
        }
        else
        {
            read_cb_ = [](TcpConnection *conn, const char *data, size_t len) {
                LOG_W << "on read callback not set! drop " << len << " bytes data";
                return len;
            };
        }
    }

    void TcpConnection::SetClosedCB(ClosedCB &&closed_cb)
    {
        if (closed_cb)
        {
            closed_cb_ = std::move(closed_cb);
        }
        else
        {
            closed_cb_ = [](TcpConnection *conn) {
                LOG_W << "on closed callback not set!";
            };
        }
    }

    void TcpConnection::SetErrorCB(ErrorCB &&error_cb)
    {
        if (error_cb)
        {
            error_cb_ = std::move(error_cb);
        }
        else
        {
            error_cb_ = [](const SockException &err) {
                LOG_E << "on error callback not set: " << err.what();
            };
        }
    }

    void TcpConnection::Dump() const
    {
        LOG_I << SMS_LOG_SEPARATOR_CHAR_STD
              << "<TcpConnection>" << SMS_LOG_SEPARATOR_CHAR_STD
              << "  localIp    : " << local_ip_ << SMS_LOG_SEPARATOR_CHAR_STD
              << "  localPort  : " << local_port_ << SMS_LOG_SEPARATOR_CHAR_STD
              << "  remoteIp   : " << peer_ip_ << SMS_LOG_SEPARATOR_CHAR_STD
              << "  remotePort : " << peer_port_ << SMS_LOG_SEPARATOR_CHAR_STD
              << "  closed     : " << closed_ << SMS_LOG_SEPARATOR_CHAR_STD
              << "</TcpConnection>" << std::endl;
    }

    const struct sockaddr *TcpConnection::GetLocalAddr() const
    {
        return reinterpret_cast<struct sockaddr *>(local_addr_);
    }

    int TcpConnection::GetLocalFamily() const
    {
        return reinterpret_cast<const struct sockaddr *>(this->local_addr_)->sa_family;
    }

    uint16_t TcpConnection::GetLocalPort() const
    {
        return local_port_;
    }
    const struct sockaddr *TcpConnection::GetPeerAddr() const
    {
        return reinterpret_cast<const struct sockaddr *>(&peer_addr_);
    }

    uint16_t TcpConnection::GetPeerPort() const
    {
        return peer_port_;
    }
    bool TcpConnection::IsClosed() const
    {
        return closed_;
    }
    uv_tcp_t *TcpConnection::GetUvHandle() const
    {
        return uv_handle_;
    }

    size_t TcpConnection::GetSentBytes() const
    {
        return sent_bytes_;
    }

    size_t TcpConnection::GetRecvBytes() const
    {
        return recv_bytes_;
    }

    inline void TcpConnection::OnUvRead(ssize_t nread, const uv_buf_t *buf)
    {
        if (nread > 0)
        {
            buffer_data_len_ += static_cast<size_t>(nread);
            recv_bytes_ += static_cast<size_t>(nread);

            user_on_tcp_connection_read();
            return;
        }
        else if (nread == UV_EOF || nread == UV_ECONNRESET)
        {
            closed_by_peer_ = true;
        }
        else
        {
            error_ = true;
        }

        error_cb_(SockException(uv_strerror(nread), nread));
        Close();
    }

    inline void TcpConnection::OnUvAlloc(size_t suggested_size, uv_buf_t *buf)
    {
        buf->base = buffer_.Data() + buffer_data_len_;

        if (buffer_.Capacity() > buffer_data_len_)
        {
            buf->len = buffer_.Capacity() - buffer_data_len_;
            buf->base[buf->len] = '\0';
        }
        else
        {
            buf->len = 0;
            LOG_W << "no available space in the buffer";
        }
    }

    inline void TcpConnection::OnUvWrite(int status, WriteCB &&cb)
    {

        if (status == 0)
        {

            if (cb)
            {
                cb(true);
            }
        }
        else
        {
            if (status == UV_EPIPE && status == UV_ENOTCONN)
            {
                closed_by_peer_ = true;
            }
            else
            {
                error_ = true;
            }

            if (cb)
            {
                cb(false);
            }

            error_cb_(SockException(uv_strerror(status), status));
            Close();
        }
    }

    bool TcpConnection::set_peer_addr()
    {
        int len = sizeof(peer_addr_);

        int ret = uv_tcp_getpeername(uv_handle_, reinterpret_cast<sockaddr *>(&peer_addr_), &len);
        if (ret != 0)
        {
            LOG_E << "uv_tcp_getpeername() failed: " << uv_strerror(ret);
            return false;
        }

        int family;
        SocketUtils::GetAddrInfo(reinterpret_cast<sockaddr *>(&peer_addr_), family, peer_ip_, peer_port_);
        return true;
    }

    void TcpConnection::user_on_tcp_connection_read()
    {
        if (closed_)
        {
            return;
        }

        size_t data_len = buffer_data_len_ - read_pos_;

        size_t consumed = 0;
        consumed = read_cb_(this,
                            buffer_.Data() + read_pos_,
                            data_len);

        if (consumed >= data_len)
        {
            // 清空buffer
            buffer_data_len_ = 0;
            read_pos_ = 0;
        }
        else
        {
            read_pos_ += consumed;
        }

        // 检查缓存是否满了
        if (buffer_data_len_ == buffer_.Capacity())
        {
            if (read_pos_ != 0)
            {
                std::memmove(
                    buffer_.Data(), buffer_.Data() + read_pos_, buffer_data_len_ - read_pos_);
                buffer_data_len_ = buffer_.Capacity() - read_pos_;
                read_pos_ = 0;
            }
            else
            {
                error_cb_(SockException("recv buffer overflow", UV_EAI_OVERFLOW));
                Close();
            }
        }
    }

    void TcpConnection::Close(bool invoke_cb)
    {
        if (closed_)
        {
            return;
        }

        closed_ = true;

        ConnectionManager::Instance().Remove(this);

        if (uv_handle_->loop)
        {
            int ret = uv_read_stop(reinterpret_cast<uv_stream_t *>(uv_handle_));
            if (ret != 0)
            {
                SMS_ABORT("uv_read_stop() failed: %s", uv_strerror(ret));
            }

            uv_shutdown_t *req = new uv_shutdown_t;
            req->data = static_cast<void *>(this);

            if (!error_ && !closed_by_peer_)
            {
                ret = uv_shutdown(req, reinterpret_cast<uv_stream_t *>(uv_handle_), static_cast<uv_shutdown_cb>(on_shutdown));
                if (ret != 0)
                {
                    uv_close(reinterpret_cast<uv_handle_t *>(uv_handle_), static_cast<uv_close_cb>(on_close));
                }
            }
            else
            {
                uv_close(reinterpret_cast<uv_handle_t *>(uv_handle_), static_cast<uv_close_cb>(on_close));
            }
        }
        else
        {
            on_close(reinterpret_cast<uv_handle_t *>(uv_handle_));
        }

        if (invoke_cb)
        {
            closed_cb_(this);
        }
    }

} // namespace sms
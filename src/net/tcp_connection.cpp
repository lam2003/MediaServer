#define SMS_CLASS "TcpConnection"

#include <net/tcp_connection.h>
#include <net/socket_utils.h>
#include <common/dep_libuv.h>
#include <common/logger.h>

namespace sms
{
    inline static void on_close(uv_handle_t *handle)
    {
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
            conn->OnUvRead(nread, buf);
        }
    }

    inline static void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
    {
        TcpConnection *conn = reinterpret_cast<TcpConnection *>(handle->data);
        if (conn)
        {
            conn->OnUvAlloc(suggested_size, buf);
        }
    }

    TcpConnection::TcpConnection(size_t buffer_size)
    {
        buffer_size_ = buffer_size;
        buffer_ = new uint8_t[buffer_size];
        uv_handle_ = new uv_tcp_t;
        uv_handle_->data = static_cast<void *>(this);
    }

    TcpConnection::~TcpConnection()
    {
        Close();
        delete[] buffer_;
    }

    void TcpConnection::Close()
    {
        if (closed_)
        {
            return;
        }

        closed_ = true;
        uv_handle_->data = nullptr;

        if (uv_handle_->loop != nullptr)
        {
            int ret = uv_read_stop(reinterpret_cast<uv_stream_t *>(uv_handle_));
            if (ret != 0)
            {
                SMS_ABORT("uv_read_stop() failed: %s", uv_strerror(ret));
            }

            uv_shutdown_t *req = new uv_shutdown_t;
            req->data = static_cast<void *>(this);

            ret = uv_shutdown(req, reinterpret_cast<uv_stream_t *>(uv_handle_), static_cast<uv_shutdown_cb>(on_shutdown));
            if (ret != 0)
            {
                uv_close(reinterpret_cast<uv_handle_t *>(uv_handle_), static_cast<uv_close_cb>(on_close));
            }
        }
        else
        {
            on_close(reinterpret_cast<uv_handle_t *>(uv_handle_));
        }
    }

    int TcpConnection::Setup(Listener *listener,
                             struct sockaddr_storage *local_addr,
                             const std::string &local_ip,
                             uint16_t local_port)
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

        listener_ = listener;
        local_addr_ = local_addr;
        local_ip_ = local_ip;
        local_port_ = local_port;

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

    void TcpConnection::Dump() const
    {
        LOG_I << "<TcpConnection>" << std::endl
              << "  localIp    : " << local_ip_ << std::endl
              << "  localPort  : " << local_port_ << std::endl
              << "  remoteIp   : " << peer_ip_ << std::endl
              << "  remotePort : " << peer_port_ << std::endl
              << "  closed     : " << closed_ << std::endl
              << "</TcpConnection>" << std::endl;
    }

    // void TcpConnection::Start()

    inline void TcpConnection::OnUvRead(ssize_t nread, const uv_buf_t *buf)
    {
        if (nread > 0)
        {
            buffer_len_ += static_cast<size_t>(nread);
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

        Close();
        listener_->OnTcpConnectionClosed(this);
    }

    inline void TcpConnection::OnUvAlloc(size_t suggested_size, uv_buf_t *buf)
    {
        buf->base = reinterpret_cast<char *>(buffer_ + buffer_len_);

        if (buffer_size_ > buffer_len_)
        {
            buf->len = buffer_size_ - buffer_len_;
        }
        else
        {
            buf->len = 0;
            LOG_W << "no available space in the buffer";
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

        size_t data_len = buffer_len_ - read_pos_;

        size_t consumed = 0;
        consumed = listener_->OnTcpConnectionPacketReceived(this,
                                                            buffer_ + read_pos_,
                                                            data_len);

        if (consumed >= data_len)
        {
            // 清空buffer
            buffer_len_ = 0;
            read_pos_ = 0;
        }
        else
        {
            read_pos_ += consumed;
        }

        // 检查缓存是否满了
        if (buffer_len_ == buffer_size_)
        {
            if (read_pos_ != 0)
            {
                std::memmove(
                    buffer_, buffer_ + read_pos_, buffer_len_ - read_pos_);
                buffer_len_ = buffer_size_ - read_pos_;
                read_pos_ = 0;
            }
            else
            {
                Close();
                listener_->OnTcpConnectionClosed(this);
            }
        }
    }

} // namespace sms
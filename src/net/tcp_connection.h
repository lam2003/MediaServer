#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <net/buffer.h>
#include <net/socket_exception.h>

namespace sms
{

    class TcpConnection : public NonCopyable
    {
    public:
        using WriteCB = std::function<void(bool)>;
        using ClosedCB = std::function<void(TcpConnection *)>;
        using ReadCB = std::function<size_t(TcpConnection *, const uint8_t *data, size_t len)>;
        using ErrorCB = std::function<void(const SockException &)>;

        class UvWriteData : public NonCopyable
        {
        public:
            explicit UvWriteData(const std::shared_ptr<BufferList> &list)
            {
                this->list = list;
            }

            explicit UvWriteData(const std::shared_ptr<Buffer> &buf)
            {
                this->buf = buf;
            }

            ~UvWriteData() = default;

            uv_write_t req;
            std::shared_ptr<BufferList> list{nullptr};
            std::shared_ptr<Buffer> buf{nullptr};
            TcpConnection::WriteCB cb{nullptr};
        };

    public:
        explicit TcpConnection(size_t buffer_size);
        virtual ~TcpConnection();

    public:
        void Close(bool invoke_cb = true);
        int Setup(struct sockaddr_storage *local_addr,
                  const std::string &local_ip,
                  uint16_t local_port, ClosedCB &&on_closed);
        int Start();
        void Write(const std::shared_ptr<Buffer> &buf, WriteCB &&cb);
        void Write(const std::shared_ptr<BufferList> &list, WriteCB &&cb);
        void SetReadCB(ReadCB &&read_cb);
        void SetClosedCB(ClosedCB &&closed_cb);
        void SetErrorCB(ErrorCB &&error_cb);

        void Dump() const;
        const struct sockaddr *GetLocalAddr() const;
        int GetLocalFamily() const;
        uint16_t GetLocalPort() const;
        const struct sockaddr *GetPeerAddr() const;
        uint16_t GetPeerPort() const;
        bool IsClosed() const;
        uv_tcp_t *GetUvHandle() const;
        size_t GetSentBytes() const;
        size_t GetRecvBytes() const;

    public:
        void OnUvRead(ssize_t nread, const uv_buf_t *buf);
        void OnUvAlloc(size_t suggested_size, uv_buf_t *buf);
        void OnUvWrite(int status, WriteCB &&cb);

    private:
        bool set_peer_addr();
        void user_on_tcp_connection_read();

    private:
        BufferRaw buffer_;
        size_t buffer_data_len_{0u};
        size_t read_pos_{0u};

        std::string local_ip_;
        uint16_t local_port_{0u};
        struct sockaddr_storage *local_addr_{nullptr};
        std::string peer_ip_;
        uint16_t peer_port_{0u};
        struct sockaddr_storage peer_addr_;

        uv_tcp_t *uv_handle_{nullptr};
        size_t recv_bytes_{0u};
        size_t sent_bytes_{0u};

        ReadCB read_cb_{nullptr};
        ClosedCB closed_cb_{nullptr};
        ErrorCB error_cb_{nullptr};
        bool closed_{false};
        bool closed_by_peer_{false};
        bool error_{false};
    };

} // namespace sms

#endif
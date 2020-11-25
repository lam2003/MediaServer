#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <common/noncopyable.h>
#include <common/global_inc.h>

#include <uv.h>

namespace sms
{

    class TcpConnection : public NonCopyable
    {
    public:
        typedef std::function<void(bool)> OnSendCB;

    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;

        public:
            virtual void OnTcpConnectionClosed(TcpConnection *connection) = 0;
            virtual size_t OnTcpConnectionPacketReceived(TcpConnection *connection, const uint8_t *data, size_t len) = 0;
        };

        class UvWriteData : public NonCopyable
        {
        public:
            explicit UvWriteData(size_t store_size)
            {
                this->store = new uint8_t[store_size];
            }

            ~UvWriteData()
            {
                delete[] store;
            }

            uv_write_t req;
            uint8_t *store{nullptr};
            TcpConnection::OnSendCB cb{nullptr};
        };

    public:
        explicit TcpConnection(size_t buffer_size);
        virtual ~TcpConnection();

    public:
        void Close();
        virtual void Dump() const;
        int Setup(Listener *listener,
                  struct sockaddr_storage *local_addr,
                  const std::string &local_ip,
                  uint16_t local_port);
        int Start();
        void Write(const uint8_t *data, size_t len, OnSendCB &&cb);
        const struct sockaddr *GetLocalAddr() const;
        int GetLocalFamily() const;
        uint16_t GetLocalPort() const;
        const struct sockaddr *GetPeerAddr() const;
        uint16_t GetPeerPort() const;
        bool IsClosed() const;
        uv_tcp_t *GetUVHandle() const;
        size_t GetSentBytes() const;
        size_t GetRecvBytes() const;

    public:
        void OnUvRead(ssize_t nread, const uv_buf_t *buf);
        void OnUvAlloc(size_t suggested_size, uv_buf_t *buf);
        void OnUvWrite(int status, OnSendCB &&cb);

    private:
        bool set_peer_addr();
        void user_on_tcp_connection_read();

    private:
        size_t buffer_size_{0u};
        uint8_t *buffer_{nullptr};
        size_t buffer_len_{0u};
        size_t read_pos_{0u};

        std::string local_ip_;
        uint16_t local_port_{0u};
        struct sockaddr_storage *local_addr_{nullptr};
        std::string peer_ip_;
        uint16_t peer_port_{0u};
        struct sockaddr_storage peer_addr_;

        Listener *listener_{nullptr};
        uv_tcp_t *uv_handle_{nullptr};
        size_t recv_bytes_{0u};
        size_t sent_bytes_{0u};

        bool closed_{false};
        bool closed_by_peer_{false};
        bool error_{false};
    };

} // namespace sms

#endif
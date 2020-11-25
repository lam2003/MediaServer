#ifndef SMS_TCP_SERVER_H
#define SMS_TCP_SERVER_H

#include <net/tcp_connection.h>

namespace sms
{

    class TcpServer : public TcpConnection::Listener
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() {}
            virtual void OnTcpConnectionClosed(TcpConnection *conn) = 0;
            virtual size_t OnTcpConnectionPacketReceived(TcpConnection *conn, const uint8_t *data, size_t len) = 0;
        };

    public:
        explicit TcpServer();
        virtual ~TcpServer();

    public:
        void Close();
        int Start(Listener *listener, uv_tcp_t *handle, int backlog);

        void Dump() const;
        size_t GetNumConnections() const;
        int GetLocalFamily() const;
        const struct sockaddr *GetLocalAddr() const;
        const std::string GetLocalIP() const;
        uint16_t GetLocalPort() const;

    public:
        void OnTcpConnectionClosed(TcpConnection *conn) override;
        size_t OnTcpConnectionPacketReceived(TcpConnection *conn, const uint8_t *data, size_t len) override;
        void OnUvConnection(int status);

    private:
        bool set_local_addr();

    private:
        struct sockaddr_storage local_addr_;
        std::string local_ip_;
        uint16_t local_port_{0u};
        uv_tcp_t *uv_handle_{nullptr};
        Listener *listener_{nullptr};
        std::unordered_set<TcpConnection *> conns_;
        bool closed_{false};
    };
} // namespace sms
#endif
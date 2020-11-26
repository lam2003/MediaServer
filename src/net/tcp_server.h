#ifndef SMS_TCP_SERVER_H
#define SMS_TCP_SERVER_H

#include <net/tcp_connection.h>

namespace sms
{

    class TcpServer
    {
    public:
        using AcceptCB = std::function<void(TcpConnection *conn)>;
        using ClosedCB = std::function<void(TcpConnection *conn)>;

    public:
        explicit TcpServer();
        virtual ~TcpServer();

    public:
        void Close();
        int Start(uv_tcp_t *handle, int backlog);
        void SetAcceptCB(AcceptCB &&accept_cb);
        void SetClosedCB(ClosedCB &&closed_cb);

        void Dump() const;
        size_t GetNumConnections() const;
        int GetLocalFamily() const;
        const struct sockaddr *GetLocalAddr() const;
        const std::string GetLocalIP() const;
        uint16_t GetLocalPort() const;

    public:
        void OnUvConnection(int status);

    private:
        bool set_local_addr();

    private:
        struct sockaddr_storage local_addr_;
        std::string local_ip_;
        uint16_t local_port_{0u};
        uv_tcp_t *uv_handle_{nullptr};
        std::unordered_set<TcpConnection *> conns_;

        AcceptCB accept_cb_{nullptr};
        ClosedCB closed_cb_{nullptr};
        bool closed_{false};
    };
} // namespace sms
#endif
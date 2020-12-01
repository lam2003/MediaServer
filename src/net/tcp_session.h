#ifndef SMS_TCP_SESSION_H
#define SMS_TCP_SESSION_H

#include <net/tcp_server.h>

namespace sms
{

    class TcpSession
    {
    public:
        TcpSession(TcpConnection *conn);
        virtual ~TcpSession();

    public:
        virtual void OnError(const SockException &err) = 0;
        virtual size_t OnRecv(const uint8_t *data, size_t len) = 0;
        virtual void OnManage() = 0;

    protected:
        virtual void send(const std::shared_ptr<BufferList> &list);
        virtual void send(const std::shared_ptr<Buffer> &buf);
        virtual void shutdown();

    private:
        TcpConnection *conn_;
    };

} // namespace sms

#endif
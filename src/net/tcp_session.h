#ifndef SMS_TCP_SESSION_H
#define SMS_TCP_SESSION_H

#include <net/tcp_server.h>

namespace sms
{

    class TcpSession
    {
    public:
        TcpSession(TcpConnection *conn)
        {
            conn_ = conn;
        }

        virtual ~TcpSession() = default;

    public:
        virtual size_t OnRecv(const uint8_t *data, size_t len) = 0;

        virtual void OnManager(){};

        virtual void AttachServer(TcpServer *server){};

        virtual TcpConnection *GetConnection() const
        {
            return conn_;
        }

    protected:
        virtual void send(const std::shared_ptr<BufferList> &list)
        {
            conn_->Write(list, [](bool) {
                // ignore
            });
        }

        virtual void send(const std::shared_ptr<Buffer> &buf)
        {
            conn_->Write(buf, [](bool ) {
                // ignore
            });
        }

        virtual void shutdown()
        {
            conn_->Close();
        }

    private:
        TcpConnection *conn_;
    };

} // namespace sms

#endif
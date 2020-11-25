#ifndef SMS_TCP_SERVER_H
#define SMS_TCP_SERVER_H

#include <net/tcp_connection.h>

namespace sms
{

    class TcpServer : public TcpConnection::Listener
    {
    public:
    
    public:
        void OnTcpConnectionClosed(TcpConnection *connection) override;
        size_t OnTcpConnectionPacketReceived(TcpConnection *connection, const uint8_t *data, size_t len) override;
    };
} // namespace sms
#endif
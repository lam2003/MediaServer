#ifndef SMS_TCP_SESSION_H
#define SMS_TCP_SESSION_H

#include <net/tcp_connection.h>

namespace sms
{

    class TcpSession
    {
    public:
        TcpSession(TcpConnection *conn);
    };

} // namespace sms

#endif
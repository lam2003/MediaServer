#ifndef SMS_SOCKET_UTILS_H
#define SMS_SOCKET_UTILS_H

#include <common/global_inc.h>
#include <common/logger.h>

#include <uv.h>

namespace sms
{
    class SocketUtils
    {
    public:
        static void GetAddrInfo(const struct sockaddr *addr, int &family, std::string &ip, uint16_t &port);
    };

} // namespace sms

#endif
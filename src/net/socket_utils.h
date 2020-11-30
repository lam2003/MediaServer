#ifndef SMS_SOCKET_UTILS_H
#define SMS_SOCKET_UTILS_H

#include <common/global_inc.h>

#include <uv.h>

namespace sms
{
    class SocketUtils
    {
    public:
        enum SOCK_TYPE
        {
            SOCK_TCP = 1,
            SOCK_UDP
        };

    public:
        static void
        GetAddrInfo(const struct sockaddr *addr, int &family, std::string &ip, uint16_t &port);

        static int GetFamily(const std::string &ip);

        static bool NormalizeIp(std::string &ip);

        static uv_handle_t *Bind(SOCK_TYPE sock_type, const std::string &ip, uint16_t port);
    };

} // namespace sms

#endif
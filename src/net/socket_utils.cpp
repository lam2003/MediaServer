#include <net/socket_utils.h>
#include <common/logger.h>

namespace sms
{
    void SocketUtils::GetAddrInfo(const struct sockaddr *addr, int &family, std::string &ip, uint16_t &port)
    {
        int ret;
        char buf[INET6_ADDRSTRLEN] = {0};

        switch (addr->sa_family)
        {
        case AF_INET:
        {
            ret = uv_inet_ntop(AF_INET,
                               std::addressof(reinterpret_cast<const struct sockaddr_in *>(addr)->sin_addr),
                               buf,
                               sizeof(buf));
            if (ret != 0)
            {
                SMS_ABORT("uv_inet_ntop() failed: %s", uv_strerror(ret));
            }

            port = static_cast<uint16_t>(ntohs(reinterpret_cast<const struct sockaddr_in *>(addr)->sin_port));

            break;
        }
        case AF_INET6:
        {
            ret = uv_inet_ntop(AF_INET6,
                               std::addressof(reinterpret_cast<const struct sockaddr_in6 *>(addr)->sin6_addr),
                               buf,
                               sizeof(buf));
            if (ret != 0)
            {
                SMS_ABORT("uv_inet_ntop() failed: %s", uv_strerror(ret));
            }

            port = static_cast<uint16_t>(ntohs(reinterpret_cast<const struct sockaddr_in6 *>(addr)->sin6_port));

            break;
        }
        default:
        {
            SMS_ABORT("unknow network family: %d", static_cast<int>(addr->sa_family));
        }
        }

        family = addr->sa_family;
        ip.assign(buf);
    }

} // namespace sms
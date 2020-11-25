#include <net/socket_utils.h>
#include <net/dep_libuv.h>
#include <common/logger.h>

namespace sms
{
    inline static void on_fake_connection(uv_stream_t *handle, int status)
    {
    }

    inline static void on_close(uv_handle_t *handle)
    {
        delete handle;
    }

    void SocketUtils::GetAddrInfo(const struct sockaddr *addr, int &family, std::string &ip, uint16_t &port)
    {
        int ret{0};
        char buf[INET6_ADDRSTRLEN]{0};

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

    int SocketUtils::GetFamily(const std::string &ip)
    {
        if (ip.size() > INET6_ADDRSTRLEN)
        {
            return AF_UNSPEC;
        }

        char buf[INET6_ADDRSTRLEN]{0};

        if (uv_inet_pton(AF_INET, ip.c_str(), buf) == 0)
        {
            return AF_INET;
        }
        else if (uv_inet_pton(AF_INET6, ip.c_str(), buf) == 0)
        {
            return AF_INET6;
        }
        else
        {
            return AF_UNSPEC;
        }
    }

    bool SocketUtils::NormalizeIp(std::string &ip)
    {
        static sockaddr_storage s_addr{0};
        char buf[INET6_ADDRSTRLEN]{0};
        int ret{0};
        int family = GetFamily(ip);

        switch (family)
        {
        case AF_INET:
        {
            ret = uv_ip4_addr(ip.c_str(), 0, reinterpret_cast<struct sockaddr_in *>(&s_addr));
            if (ret != 0)
            {
                LOG_E << "uv_ip4_addr() failed [ip:" << ip << "]: " << uv_strerror(ret);
                return false;
            }

            ret = uv_ip4_name(reinterpret_cast<const struct sockaddr_in *>(std::addressof(s_addr)),
                              buf,
                              sizeof(buf));
            if (ret != 0)
            {
                LOG_E << "uv_ip4_name() failed [ip:" << ip << "]: " << uv_strerror(ret);
                return false;
            }
            break;
        }
        case AF_INET6:
        {
            ret = uv_ip6_addr(ip.c_str(), 0, reinterpret_cast<struct sockaddr_in6 *>(&s_addr));
            if (ret != 0)
            {
                LOG_E << "uv_ip6_addr() failed [ip:" << ip << "]: " << uv_strerror(ret);
                return false;
            }

            ret = uv_ip6_name(reinterpret_cast<const struct sockaddr_in6 *>(std::addressof(s_addr)),
                              buf,
                              sizeof(buf));
            if (ret != 0)
            {
                LOG_E << "uv_ip6_name() failed [ip:" << ip << "]: " << uv_strerror(ret);
                return false;
            }
            break;
        }
        default:
        {
            LOG_E << "invalid IP '" << ip << "'";
            return false;
        }
        }
        ip.assign(buf);
        return true;
    }

    uv_handle_t *SocketUtils::Bind(SOCK_TYPE sock_type, std::string &ip, uint16_t port)
    {
        if (!NormalizeIp(ip))
        {
            return nullptr;
        }

        int ret{0};
        int flags{0};
        sockaddr_storage addr{0};
        int family = GetFamily(ip);

        switch (family)
        {
        case AF_INET:
        {
            ret = uv_ip4_addr(ip.c_str(), port, reinterpret_cast<struct sockaddr_in *>(&addr));
            if (ret != 0)
            {
                LOG_E << "uv_ip4_addr failed: " << uv_strerror(ret);
                return nullptr;
            }
            break;
        }
        case AF_INET6:
        {
            ret = uv_ip6_addr(ip.c_str(), port, reinterpret_cast<struct sockaddr_in6 *>(&addr));
            if (ret != 0)
            {
                LOG_E << "uv_ip6_addr failed: " << uv_strerror(ret);
                return nullptr;
            }
            flags |= UV_UDP_IPV6ONLY;
            break;
        }
        default:
        {
            LOG_E << "unknow ip family";
            return nullptr;
        }
        }

        uv_handle_t *handle{nullptr};
        switch (sock_type)
        {
        case SOCK_TCP:
        {
            handle = reinterpret_cast<uv_handle_t *>(new uv_tcp_t);
            ret = uv_tcp_init(DepLibUV::GetLoop(),
                              reinterpret_cast<uv_tcp_t *>(handle));
            if (ret != 0)
            {
                LOG_E << "uv_tcp_init() failed: " << uv_strerror(ret);
                delete handle;
                return nullptr;
            }

            ret = uv_tcp_bind(reinterpret_cast<uv_tcp_t *>(handle),
                              reinterpret_cast<sockaddr *>(&addr),
                              flags);
            if (ret != 0)
            {
                LOG_E << "uv_tcp_bind failed [ip:" << ip << ", port:" << port << "]: " << uv_strerror(ret);
            }
            else if (ret == 0)
            {
                // uv_tcp_bind()返回成功，可能在uv_listen()时失败，所以这里进行二次检查
                ret = uv_listen(reinterpret_cast<uv_stream_t *>(handle),
                                1024,
                                static_cast<uv_connection_cb>(on_fake_connection));
                if (ret != 0)
                {
                    LOG_E << "uv_listen failed [ip:" << ip << ", port:" << port << "]: " << uv_strerror(ret);
                }
            }

            break;
        }
        case SOCK_UDP:
        {
            handle = reinterpret_cast<uv_handle_t *>(new uv_udp_t);
            ret = uv_udp_init_ex(DepLibUV::GetLoop(),
                                 reinterpret_cast<uv_udp_t *>(handle),
                                 UV_UDP_RECVMMSG);
            if (ret != 0)
            {
                LOG_E << "uv_udp_init_ex() failed: " << uv_strerror(ret);
                delete handle;
                return nullptr;
            }
                        
            ret = uv_udp_bind(reinterpret_cast<uv_udp_t *>(handle),
                              reinterpret_cast<sockaddr *>(&addr),
                              flags);
            if (ret != 0)
            {
                LOG_E << "uv_udp_bind failed [ip:" << ip << ", port:" << port << "]: " << uv_strerror(ret);
            }
            break;
        }
        default:
        {
            LOG_E << "unknow SOCK_TYPE";
            return nullptr;
        }
        }

        if (ret != 0)
        {
            uv_close(handle, static_cast<uv_close_cb>(on_close));
            return nullptr;
        }

        return handle;
    }
} // namespace sms
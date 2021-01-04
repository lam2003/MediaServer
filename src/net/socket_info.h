#ifndef SMS_SOCKET_INFO_H
#define SMS_SOCKET_INFO_H

#include <common/global_inc.h>

namespace sms
{

    class SocketInfo
    {
    public:
        using Ptr = std::shared_ptr<SocketInfo>;

        SocketInfo() = default;

        virtual ~SocketInfo() = default;

    public:
        virtual const std::string &GetLocalIP() const = 0;

        virtual const std::string &GetPeerIP() const = 0;

        virtual uint16_t GetLocalPort() const = 0;

        virtual uint16_t GetPeerPort() const = 0;

        virtual const struct sockaddr *GetLocalAddr() const = 0;

        virtual const struct sockaddr *GetPeerAddr() const = 0;

        virtual int GetLocalFamily() const = 0;
    };
} // namespace sms

#endif
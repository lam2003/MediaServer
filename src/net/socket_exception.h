#ifndef SMS_SOCKET_EXCEPTION_H
#define SMS_SOCKET_EXCEPTION_H

#include <common/global_inc.h>

namespace sms
{

    class SockException : public std::exception
    {
    public:
        SockException(const std::string &msg, int code)
        {
            msg_ = msg;
            code_ = code;
        }

        ~SockException() = default;

        const char *what() const noexcept override
        {
            return msg_.c_str();
        }

        operator bool() const
        {
            return code_ != 0;
        }

    private:
        std::string msg_;
        int code_;
    };

} // namespace sms

#endif
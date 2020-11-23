#ifndef SMS_ONCE_TOKEN_H
#define SMS_ONCE_TOKEN_H

#include <common/noncopyable.h>

#include <functional>

namespace sms
{

    class OnceToken final : public NonCopyable
    {
    public:
        typedef std::function<void()> Task;
        OnceToken(const Task &on_constructed, const Task &on_destructed = nullptr)
        {
            if (on_constructed)
            {
                on_constructed();
            }
            on_destructed_ = on_destructed;
        }

        OnceToken(Task &&on_constructed, Task &&on_destructed = nullptr)
        {
            if (on_constructed)
            {
                on_constructed();
            }
            on_destructed_ = std::move(on_destructed);
        }

        ~OnceToken()
        {
            if (on_destructed_)
            {
                on_destructed_();
            }
        }

    private:
        Task on_destructed_;
    };
} // namespace sms

#endif
#ifndef SMS_TIMER_H
#define SMS_TIMER_H

#include <common/global_inc.h>
#include <common/noncopyable.h>
#include <uv.h>

namespace sms
{

    class Timer final : public NonCopyable
    {
    public:
        using Ptr = std::shared_ptr<Timer>;

        Timer();

        ~Timer();

    private:
        uv_timer_t *uv_handler_{nullptr};
    };

} // namespace sms

#endif
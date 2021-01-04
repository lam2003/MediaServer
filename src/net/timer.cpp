#include <net/timer.h>
#include <dep/dep_libuv.h>

namespace sms
{

    Timer::Timer()
    {
        uv_handler_ = new uv_timer_t;
    }

    Timer::~Timer()
    {
        delete uv_handler_;
        uv_handler_ = nullptr;
    }

} // namespace sms
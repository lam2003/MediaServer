#ifndef SMS_SEMAPHORE_H
#define SMS_SEMAPHORE_H

#include <common/global_inc.h>

namespace sms
{

    class Semaphore final
    {
    public:
        Semaphore() = default;
        ~Semaphore() = default;

    public:
        void Post(uint32_t n = 1)
        {
            std::unique_lock<std::mutex> lock(mux_);
            count_ += n;
            if (n == 1)
            {
                cond_.notify_one();
            }
            else
            {
                cond_.notify_all();
            }
        }

        void Wait()
        {
            std::unique_lock<std::mutex> lock(mux_);
            // 防止虚假唤醒
            while (count_ == 0)
            {
                cond_.wait(lock);
            }
            --count_;
        }

    private:
        uint32_t count_ = 0;
        std::mutex mux_;
        std::condition_variable_any cond_;
    };

} // namespace sms

#endif
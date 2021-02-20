#ifndef SMS_BYTES_SPEED_H
#define SMS_BYTES_SPEED_H

#include <common/ticker.h>

namespace sms
{
    class BytesSpeed
    {
    public:
        BytesSpeed() = default;
        ~BytesSpeed() = default;

        BytesSpeed &operator+=(uint64_t bytes)
        {
            bytes_ += bytes;

            if (bytes_ > 1024 * 1024)
            {
                compute();
            }

            return *this;
        }

        int GetSpeed() const
        {
            if (ticker_.GetElapsedTimeMS() < 1000)
            {
                return speed_;
            }
            return compute();
        }

    private:
        uint64_t compute() const
        {
            uint64_t elapsed = ticker_.GetElapsedTimeMS();
            if (!elapsed)
            {
                return speed_;
            }

            speed_ = bytes_ * 1000 / elapsed;
            ticker_.Reset();
            bytes_ = 0;
            return speed_;
        }

    private:
        mutable uint64_t bytes_{0};
        mutable int speed_{0};
        mutable Ticker ticker_;
    };

}

#endif
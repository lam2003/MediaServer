#ifndef SMS_BYTES_SPEED_H
#define SMS_BYTES_SPEED_H

#include <common/global_inc.h>

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

    private:
        void compute();

    private:
        uint64_t bytes_{0};
    };

}

#endif
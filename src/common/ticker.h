#ifndef SMS_TICKER_H
#define SMS_TICKER_H

#include <common/utils.h>

namespace sms
{

    class Ticker
    {
    public:
        Ticker()
        {
            last_stamp_ = create_stamp_ = get_current_milliseconds();
        }

        ~Ticker() = default;

        uint64_t GetCreatedTimeMS()
        {
            return get_current_milliseconds() - create_stamp_;
        }

        uint64_t GetElapsedTimeMS()
        {
            return get_current_milliseconds() - last_stamp_;
        }

        void Reset()
        {
            last_stamp_ = get_current_milliseconds();
        }

    private:
        uint64_t create_stamp_;
        uint64_t last_stamp_;
    };

}

#endif
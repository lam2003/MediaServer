#include <common/track.h>

namespace sms
{

    Track::Track(const Track &that)
    {
        bit_rate_ = that.bit_rate_;
    }

    int Track::GetBitRate() const
    {
        return bit_rate_;
    }

    void Track::SetBitRate(int bit_rate)
    {
        bit_rate_ = bit_rate;
    }
} // namespace sms
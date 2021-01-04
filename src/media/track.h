#ifndef SMS_TRACK_H
#define SMS_TRACK_H

#include <media/frame.h>
#include <media/sdp_parser.h>

namespace sms
{
    class Track : public CodecInfo, public FrameDispatcher
    {
    public:
        using Ptr = std::shared_ptr<Track>;

        Track() = default;

        Track(const Track &that);

        virtual ~Track() = default;

    public:
        virtual bool Ready() = 0;

        virtual Track::Ptr Clone() = 0;

        virtual SdpInfo::Ptr GetSdp() = 0;

        virtual int GetBitRate() const;

        virtual void SetBitRate(int bit_rate);

    private:
        int bit_rate_{0};
    };

} // namespace sms

#endif
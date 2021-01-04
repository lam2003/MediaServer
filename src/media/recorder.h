#ifndef SMS_RECORDER_H
#define SMS_RECORDER_H

namespace sms
{
    class Recorder
    {
    public:
        enum Type
        {
            UNSET = 0,
            HLS = 1,
            MP4 = 2
        };
    };
} // namespace sms

#endif
#ifndef SMS_TRACK_H
#define SMS_TRACK_H

#include <common/global_inc.h>
namespace sms
{

    enum class TrackType
    {
        UNSET = 0,
        AUDIO = 1,
        VIDEO = 2
    };

    enum class CodecType
    {
        UNSET = 0,
        H264,
        AAC_MP4_GEN
    };

} // namespace sms

#endif
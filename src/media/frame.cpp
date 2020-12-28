#include <media/frame.h>

namespace sms
{

#define SWITCH_CASE(codec_id) \
    case codec_id:            \
        return #codec_id

    static const char *get_codec_name(CodecId codec_id)
    {
        switch (codec_id)
        {
            SWITCH_CASE(CodecId::H264);
            SWITCH_CASE(CodecId::H265);
            SWITCH_CASE(CodecId::AAC);
        default:
            SWITCH_CASE(CodecId::UNSET);
        }
    }

    const char *CodecInfo::GetCodecName() const
    {
        return get_codec_name(GetCodecId());
    }

    static TrackType get_track_type(CodecId codec_id)
    {
        switch (codec_id)
        {
        case CodecId::H264:
        case CodecId::H265:
            return TrackType::VIDEO;
        case CodecId::AAC:
            return TrackType::AUDIO;
        default:
            return TrackType::UNSET;
        }
    }

    TrackType CodecInfo::GetTrackType() const
    {
        return get_track_type(GetCodecId());
    }

} // namespace sms
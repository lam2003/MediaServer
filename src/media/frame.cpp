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

    bool Frame::IsCacheAble() const
    {
        return true;
    }

    Frame::Ptr Frame::GetCacheAbleFrame(const Frame::Ptr &frame)
    {
        if (frame->IsCacheAble())
        {
            return frame;
        }
        // return std::make_shared<Frame>(frame);
        return nullptr;
    }

    char *FrameImpl::Data() const
    {
        return buffer_.Data();
    }

    size_t FrameImpl::Size() const
    {
        return buffer_.Size();
    }

    uint64_t FrameImpl::GetDTS() const
    {
        return dts_;
    }

    uint64_t FrameImpl::GetPTS() const
    {
        return pts_ ? pts_ : GetDTS();
    }

    size_t FrameImpl::PrefixSize() const
    {
        return prefix_size_;
    }

    bool FrameImpl::IsKeyFrame() const
    {
        return false;
    }

    bool FrameImpl::IsConfigFrame() const
    {
        return false;
    }

    CodecId FrameImpl::GetCodecId() const
    {
        return codec_id_;
    }

    FrameFromPtr::FrameFromPtr(CodecId codec_id,
                               const char *ptr,
                               size_t size,
                               uint64_t dts,
                               uint64_t pts,
                               size_t prefix_size)
        : FrameFromPtr(ptr,
                       size,
                       dts,
                       pts,
                       prefix_size)
    {
        codec_id_ = codec_id;
    }

    FrameFromPtr::FrameFromPtr(const char *ptr,
                               size_t size,
                               uint64_t dts,
                               uint64_t pts,
                               size_t prefix_size)
    {
        ptr_ = const_cast<char *>(ptr);
        size_ = size;
        dts_ = dts;
        pts_ = pts;
        prefix_size_ = prefix_size;
    }

    char *FrameFromPtr::Data() const
    {
        return ptr_;
    }

    size_t FrameFromPtr::Size() const
    {
        return size_;
    }

    uint64_t FrameFromPtr::GetDTS() const
    {
        return dts_;
    }

    uint64_t FrameFromPtr::GetPTS() const
    {
        return pts_ ? pts_ : GetDTS();
    }

    size_t FrameFromPtr::PrefixSize() const
    {
        return prefix_size_;
    }

    bool FrameFromPtr::IsKeyFrame() const
    {
        return false;
    }

    bool FrameFromPtr::IsConfigFrame() const
    {
        return false;
    }

    CodecId FrameFromPtr::GetCodecId() const
    {
        return codec_id_;
    }

} // namespace sms
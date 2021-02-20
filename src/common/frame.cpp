#include <common/frame.h>

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

        return std::make_shared<FrameCacheable>(frame);
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

    FrameCacheable::FrameCacheable(const Frame::Ptr &frame)
    {
        if (frame->IsCacheAble())
        {
            frame_ = frame;
            ptr_ = frame_->Data();
        }
        else
        {
            buffer_ = std::make_shared<BufferRaw>();
            buffer_->Assign(frame->Data(), frame->Size());
            ptr_ = buffer_->Data();
        }

        size_ = frame->Size();
        dts_ = frame->GetDTS();
        pts_ = frame->GetPTS();
        prefix_size_ = frame->PrefixSize();
        codec_id_ = frame->GetCodecId();
        key_ = frame->IsKeyFrame();
        config_ = frame->IsConfigFrame();
    }

    bool FrameCacheable::IsCacheAble() const
    {
        return true;
    }

    bool FrameCacheable::IsKeyFrame() const
    {
        return key_;
    }

    bool FrameCacheable::IsConfigFrame() const
    {
        return config_;
    }

    FrameWriterInterfaceHelper::FrameWriterInterfaceHelper(WriteFrameCB &&cb)
    {
        cb_ = std::forward<decltype(cb)>(cb);
    }

    void FrameWriterInterfaceHelper::InputFrame(const Frame::Ptr &frame)
    {
        cb_(frame);
    }

    void FrameDispatcher::AddDelegate(const FrameWriterInterface::Ptr &delegate)
    {
        delegates_.emplace(delegate.get(), delegate);
    }

    void FrameDispatcher::DelDelegate(const FrameWriterInterface::Ptr &delegate)
    {
        delegates_.erase(delegate.get());
    }

    size_t FrameDispatcher::Size() const
    {
        return delegates_.size();
    }

    void FrameDispatcher::InputFrame(const Frame::Ptr &frame)
    {
        for (auto &ptr : delegates_)
        {
            ptr.second->InputFrame(frame);
        }
    }
} // namespace sms
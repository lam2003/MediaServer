#ifndef SMS_FRAME_H
#define SMS_FRAME_H

#include <common/global_inc.h>
#include <net/buffer.h>

namespace sms
{
    enum class CodecId
    {
        UNSET = -1,
        H264,
        H265,
        AAC,
    };

    enum class TrackType
    {
        UNSET = -1,
        AUDIO,
        VIDEO,
    };

    class CodecInfo
    {
    public:
        using Ptr = std::shared_ptr<CodecInfo>;

        CodecInfo() = default;

        virtual ~CodecInfo() = default;

    public:
        virtual CodecId GetCodecId() const = 0;

        const char *GetCodecName() const;

        TrackType GetTrackType() const;
    };

    class Frame : public Buffer, public CodecInfo
    {
    public:
        using Ptr = std::shared_ptr<Frame>;

        Frame() = default;

        virtual ~Frame() = default;

    public:
        /**
         * 返回解码时间戳(decode timestamp), 单位ms
         */
        virtual uint64_t GetDTS() const = 0;

        /**
         * 返回显示时间戳(play timestamp), 单位ms
         */
        virtual uint64_t GetPTS() const = 0;

        /**
         * 前缀长度, 譬如h264前缀为0x00 0x00 0x00 0x00 0x01,那么返回前缀长度是4字节
         * aac前缀则为7字节
         */
        virtual size_t PrefixSize() const = 0;

        /**
         * 是否关键帧
         */
        virtual bool IsKeyFrame() const = 0;

        /**
         * 是否配置帧,譬如sps pps vps
         */
        virtual bool IsConfigFrame() const = 0;

        /**
         * 是否可以缓存
         */
        virtual bool IsCacheAble() const;

        /**
         * 返回可缓存的frame
         */
        static Ptr GetCacheAbleFrame(const Ptr &frame);
    };

    class FrameImpl : public Frame
    {
    public:
        using Ptr = std::shared_ptr<FrameImpl>;

        FrameImpl() = default;

        ~FrameImpl() = default;

    public:
        // class Buffer
        char *Data() const override;

        size_t Size() const override;

    public:
        // class Frame
        uint64_t GetDTS() const override;

        uint64_t GetPTS() const override;

        size_t PrefixSize() const override;

        bool IsKeyFrame() const override;

        bool IsConfigFrame() const override;

    public:
        // class CodecInfo
        CodecId GetCodecId() const override;

    private:
        uint64_t pts_{0};
        uint64_t dts_{0};
        size_t prefix_size_{0};
        CodecId codec_id_{CodecId::UNSET};
        BufferLikeString buffer_;
    };

    class FrameFromPtr : public Frame
    {
    public:
        using Ptr = std::shared_ptr<FrameFromPtr>;

        FrameFromPtr(CodecId codec_id,
                     const char *ptr,
                     size_t size,
                     uint64_t dts,
                     uint64_t pts = 0,
                     size_t prefix_size = 0);

        FrameFromPtr(const char *ptr,
                     size_t size,
                     uint64_t dts,
                     uint64_t pts = 0,
                     size_t prefix_size = 0);

        virtual ~FrameFromPtr() override = default;

    public:
        // class Buffer
        virtual char *Data() const override;

        virtual size_t Size() const override;

    public:
        // class Frame
        virtual uint64_t GetDTS() const override;

        virtual uint64_t GetPTS() const override;

        virtual size_t PrefixSize() const override;

        virtual bool IsKeyFrame() const override;

        virtual bool IsConfigFrame() const override;

    public:
        // class CodecInfo
        virtual CodecId GetCodecId() const override;

    protected:
        FrameFromPtr() = default;

    protected:
        char *ptr_{nullptr};
        size_t size_{0};
        uint64_t dts_{0};
        uint64_t pts_{0};
        size_t prefix_size_{0};
        CodecId codec_id_{CodecId::UNSET};
    };

    class FrameCacheable final : public FrameFromPtr
    {
    public:
        using Ptr = std::shared_ptr<FrameCacheable>;

        FrameCacheable(const Frame::Ptr &frame);

        ~FrameCacheable() override = default;

    public:
        // class Frame
        bool IsCacheAble() const override;

        bool IsKeyFrame() const override;

        bool IsConfigFrame() const override;

    private:
        Frame::Ptr frame_{nullptr};
        BufferRaw::Ptr buffer_{nullptr};
        bool key_{false};
        bool config_{false};
    };

    class FrameWriterInterface
    {
    public:
        using Ptr = std::shared_ptr<FrameWriterInterface>;

        FrameWriterInterface() = default;
        virtual ~FrameWriterInterface() = default;

        // public:
        // virtual void InputFrame(const)
    };
} // namespace sms
#endif
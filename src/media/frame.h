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
        virtual bool CacheAble() const
        {
            return true;
        }

        /**
         * 返回可缓存的frame
         */
        static Ptr GetCacheAbleFrame(const Ptr &frame);
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
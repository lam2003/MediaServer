#ifndef SMS_SDP_PARSER_H
#define SMS_SDP_PARSER_H

#include <common/global_inc.h>

namespace sms
{
    enum class SdpTrackType
    {
        UNSET = 0,
        AUDIO = 1,
        VIDEO = 2
    };

    enum class SdpCodecType
    {
        UNSET = 0,
        H264,
        AAC_MP4_GEN
    };
    
    class SdpMediaTrack
    {
    public:
        SdpMediaTrack() = default;
        SdpMediaTrack(const std::string &sdp);
        ~SdpMediaTrack() = default;

    public:
        bool ProcessML(const std::string &line);
        bool ProcessAL(const std::string &line);
        bool Process();
        void Clear();
        void Dump();

    public:
        const std::string &GetCodecName() const;
        SdpCodecType GetCodecType() const;
        uint32_t GetSamplerate() const;
        uint32_t GetChannels() const;
        uint32_t GetPayload() const;
        uint32_t GetRTXPayload() const;
        SdpTrackType GetTrackType() const;
        const std::string &GetTransport() const;
        float GetStartTime() const;
        float GetEndTime() const;
        float GetDuration() const;
        uint16_t GetPort() const;
        const std::string &GetFmtp() const;
        const std::string &GetControl() const;
        const std::unordered_map<std::string, std::string> &GetAttributeMap() const;

    public:
        static std::unordered_map<std::string, SdpTrackType> string2type;
        static std::map<SdpTrackType, std::string> type2string;
        static std::unordered_map<std::string, SdpCodecType> string2subtype;
        static std::map<SdpCodecType, std::string> subtype2string;

    private:
        std::string codec_name_;
        SdpCodecType codec_type_{SdpCodecType::UNSET};
        uint32_t samplerate_{0};
        uint32_t channels_{0};
        uint32_t payload_{0};
        uint32_t rtx_payload_{0};
        SdpTrackType track_type_{SdpTrackType::UNSET};
        std::string transport_;
        float start_{0.f};
        float end_{0.f};
        float duration_{0.f};
        uint16_t port_{0};
        std::string fmtp_;
        std::string control_;
        std::unordered_map<std::string, std::string> attribute_map_;
        bool got_ml_{false};
    };

    struct SdpInfo
    {
        std::string v;                                                // version 版本
        std::string o;                                                // origin 来源（会话发起者）
        std::string s;                                                // session name 会话名称
        std::string t;                                                // timing 会话开始和结束时间，如果两个值都为0， 意味着会话是永久的
        std::string b;                                                // bandwidth 建议带宽
        std::string c;                                                // connection data 连接信息，在sip协议下十分重要
        std::vector<std::shared_ptr<SdpMediaTrack>> media_track_vec_; // media line 媒体行
    };

    class SdpParser
    {
    public:
        SdpParser() = default;
        SdpParser(const std::string &sdp);
        ~SdpParser() = default;

    public:
        void Process(const std::string &sdp);
        std::vector<std::shared_ptr<SdpMediaTrack>> GetMediaTracks() const;

    private:
        std::shared_ptr<SdpInfo> info_{nullptr};
    };
} // namespace sms

#endif
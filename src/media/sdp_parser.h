#ifndef SMS_SDP_PARSER_H
#define SMS_SDP_PARSER_H

#include <common/global_inc.h>
#include <media/track.h>

namespace sms
{

    class SdpMediaLine
    {
    public:
        SdpMediaLine() = default;
        ~SdpMediaLine() = default;

    public:
        bool ProcessML(const std::string &line);
        bool ProcessAL(const std::string &line);
        bool Process();
        void Clear();
        void Dump();

    public:
        static std::unordered_map<std::string, TrackType> string2type;
        static std::map<TrackType, std::string> type2string;
        static std::unordered_map<std::string, CodecType> string2subtype;
        static std::map<CodecType, std::string> subtype2string;

    private:
        std::string codec_name_;
        CodecType codec_type_{CodecType::UNSET};
        uint32_t samplerate_{0};
        uint32_t channels_{0};
        uint32_t payload_{0};
        uint32_t rtx_payload_{0};
        TrackType track_type_{TrackType::UNSET};
        std::string transport_;
        float start_{0.f};
        float end_{0.f};
        float duration_{0.f};
        uint16_t port_{0};
        bool got_ml_{false};
        std::string fmtp_;
        std::string control_;
        std::unordered_map<std::string, std::string> attribute_map_;
    };

    struct SdpInfo
    {
        std::string v;                                     // version 版本
        std::string o;                                     // origin 来源（会话发起者）
        std::string s;                                     // session name 会话名称
        std::string t;                                     // timing 会话开始和结束时间，如果两个值都为0， 意味着会话是永久的
        std::string b;                                     // bandwidth 建议带宽
        std::string c;                                     // connection data 连接信息，在sip协议下十分重要
        std::vector<std::shared_ptr<SdpMediaLine>> ml_vec; // media line 媒体行
    };

    class SdpParser
    {
    public:
        SdpParser() = default;
        ~SdpParser() = default;
        SdpParser(const std::string &sdp)
        {
            Process(sdp);
        }

    public:
        void Process(const std::string &sdp);

    private:
        std::vector<std::shared_ptr<SdpInfo>> info_vec_;
    };
} // namespace sms

#endif
#ifndef SMS_SDP_PARSER_H
#define SMS_SDP_PARSER_H

#include <common/global_inc.h>

namespace sms
{

    class SdpMediaLine
    {
    public:
        enum class Type : uint8_t
        {
            UNSET = 0,
            AUDIO = 1,
            VIDEO = 2
        };

        enum class SubType : uint16_t
        {
            UNSET = 0,
            AAC_MP4_GEN = 100,
            H264,
        };

        enum class TransportType : uint8_t
        {
            UNSET = 0,
            RTP,
            AVP
        };

    public:
        SdpMediaLine() = default;
        ~SdpMediaLine() = default;

    public:
        bool ProcessML(const std::string &line);
        bool ProcessAL(const std::string &line);

    public:
        static std::unordered_map<std::string, Type> string2type;
        static std::map<Type, std::string> type2string;
        static std::unordered_map<std::string, SubType> string2subtype;
        static std::map<SubType, std::string> subtype2string;
        static std::unordered_map<std::string, TransportType> string2tsptype;
        static std::map<TransportType, std::string> tsptype2string;

        Type type{Type::UNSET};
        SubType subtype{SubType::UNSET};
        uint16_t port;
        std::vector<TransportType> transport_vec;
        std::vector<uint32_t> payload_vec;
        std::unordered_map<std::string, std::string> attribute_map;
    };

    class SdpInfo
    {
    public:
        std::string v; // version 版本
        std::string o; // origin 来源（会话发起者）
        std::string s; // session name 会话名称
        std::string t; // timing 会话开始和结束时间，如果两个值都为0， 意味着会话是永久的
        std::string b; // bandwidth 建议带宽
        std::string c; // connection data 连接信息，在sip协议下十分重要
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
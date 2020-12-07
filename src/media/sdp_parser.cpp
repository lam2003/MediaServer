#include <media/sdp_parser.h>
#include <common/config.h>
#include <common/utils.h>
#include <common/logger.h>

namespace sms
{

    std::unordered_map<std::string, SdpMediaLine::Type> SdpMediaLine::string2type = {
        {"video", SdpMediaLine::Type::VIDEO},
        {"audio", SdpMediaLine::Type::AUDIO},
    };

    std::map<SdpMediaLine::Type, std::string> SdpMediaLine::type2string = {
        {SdpMediaLine::Type::VIDEO, "video"},
        {SdpMediaLine::Type::AUDIO, "audio"},
    };

    std::unordered_map<std::string, SdpMediaLine::SubType> SdpMediaLine::string2subtype = {
        {"mpeg4-generic", SdpMediaLine::SubType::AAC_MP4_GEN},
        {"h264", SdpMediaLine::SubType::H264},
    };

    std::map<SdpMediaLine::SubType, std::string> SdpMediaLine::subtype2string = {
        {SdpMediaLine::SubType::AAC_MP4_GEN, "MPEG4-GENERIC"},
        {SdpMediaLine::SubType::H264, "H264"},
    };

    std::unordered_map<std::string, SdpMediaLine::TransportType> SdpMediaLine::string2tsptype = {
        {"rtp", SdpMediaLine::TransportType::RTP},
        {"avp", SdpMediaLine::TransportType::AVP},
    };

    std::map<SdpMediaLine::TransportType, std::string> SdpMediaLine::tsptype2string = {
        {SdpMediaLine::TransportType::RTP, "RTP"},
        {SdpMediaLine::TransportType::AVP, "AVP"},
    };

    bool SdpMediaLine::ProcessML(const std::string &line)
    {
        std::vector<std::string> split_vec;
        split_string(line, split_vec, " ");

        if (split_vec.size() < 4)
        {
            // 分割出来至少有四个元素
            // 例如: rtpmap:96 H264/90000
            return false;
        }

        for (std::string &str : split_vec)
        {
            trim(str);
        }

        std::string &type_str = split_vec[0];
        std::string &port_str = split_vec[1];
        std::string &transport_str = split_vec[2];

        {
            // 处理type
            to_lower_case(type_str);
            auto it = string2type.find(type_str);
            if (it == string2type.end())
            {
                return false;
            }
            type = it->second;
        }
        {
            // 处理port
            try
            {
                port = std::stoi(port_str);
            }
            catch (std::exception &e)
            {
                return false;
            }
        }
        {
            // 处理传输类型
            std::vector<std::string> transport_str_vec;
            split_string(transport_str, transport_str_vec, "/");
            for (std::string str : transport_str_vec)
            {
                trim(str);
                to_lower_case(str);
                auto it = string2tsptype.find(str);
                if (it == string2tsptype.end())
                {
                    return false;
                }
                transport_vec.emplace_back(it->second);
            }
        }

        {
            for (size_t i = 3; i < split_vec.size(); i++)
            {
                try
                {
                    uint32_t payload = std::stoul(split_vec[i]);
                    payload_vec.emplace_back(payload);
                }
                catch (std::exception &e)
                {
                    return false;
                }
            }
        }

        return true;
    }

    void SdpParser::Process(const std::string &sdp)
    {
        info_vec_.clear();

        std::shared_ptr<SdpInfo> info = std::make_shared<SdpInfo>();

        std::vector<std::string> line_vec;
        split_string(sdp, line_vec, "\n");

        for (std::string &line : line_vec)
        {
            trim(line);
            if (line.size() < 2 || line[1] != '=')
            {
                continue;
            }

            char opt = line[0];
            switch (opt)
            {
            case 'v':
            {
                info->v = line.substr(2);
                break;
            }
            case 'o':
            {
                info->o = line.substr(2);
                break;
            }
            case 's':
            {
                info->s = line.substr(2);
                break;
            }
            case 't':
            {
                info->t = line.substr(2);
                break;
            }
            case 'b':
            {
                info->b = line.substr(2);
                break;
            }
            case 'm':
            {
                std::vector<std::string> media_line_vec;
                split_string(line.substr(2), media_line_vec, " ");
                //m=video 9 RTP/SAVP 96 97
                if (media_line_vec.size() < 3)
                {
                    LOG_E << "parse media line failed. dump:" << line;
                    break;
                }
                for (std::string &str : media_line_vec)
                {
                    trim(str);
                }

                // info->
            }
            }
        }
    }

} // namespace sms

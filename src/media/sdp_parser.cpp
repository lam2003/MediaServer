#include <media/sdp_parser.h>
#include <common/config.h>
#include <common/utils.h>
#include <common/logger.h>

namespace sms
{

    std::unordered_map<std::string, TrackType> SdpMediaLine::string2type = {
        {"video", TrackType::VIDEO},
        {"audio", TrackType::AUDIO},
    };

    std::map<TrackType, std::string> SdpMediaLine::type2string = {
        {TrackType::VIDEO, "video"},
        {TrackType::AUDIO, "audio"},
    };

    std::unordered_map<std::string, CodecType> SdpMediaLine::string2subtype = {
        {"mpeg4-generic", CodecType::AAC_MP4_GEN},
        {"h264", CodecType::H264},
    };

    std::map<CodecType, std::string> SdpMediaLine::subtype2string = {
        {CodecType::AAC_MP4_GEN, "MPEG4-GENERIC"},
        {CodecType::H264, "H264"},
    };

    bool SdpMediaLine::ProcessML(const std::string &line)
    {
        Clear();

        char type[16]{0};
        uint16_t port{0};
        char transport[16]{0};
        uint32_t payload{0};
        // supoort webrtc sdp parse
        uint32_t rtx_payload{0};

        int ret = sscanf(line.c_str(), "%15[^ ] %hu %15[^ ] %u %u", type, &port, transport, &payload, &rtx_payload);
        if (ret < 4)
        {
            LOG_E << "Error ###1";
            return false;
        }

        auto it = string2type.find(type);
        if (it != string2type.end())
        {
            track_type_ = it->second;
        }

        port_ = port;
        transport_ = transport;
        payload_ = payload;
        rtx_payload_ = rtx_payload;
        got_ml_ = true;

        return true;
    }

    bool SdpMediaLine::ProcessAL(const std::string &line)
    {
        if (!got_ml_)
        {
            return false;
        }

        std::string key, value;
        size_t pos = line.find_first_of(':');
        if (pos == std::string::npos)
        {
            key = line;
        }
        else
        {
            key = line.substr(0, pos);
            value = line.substr(pos + 1);
        }

        attribute_map_.emplace(key, value);
        return true;
    }

    bool SdpMediaLine::Process()
    {
        if (!got_ml_)
        {
            return false;
        }

        int ret{0};
        auto it = attribute_map_.find("range");
        if (it != attribute_map_.end())
        {
            char name[16]{0}, start[16]{0}, end[16]{0};
            ret = sscanf(it->second.c_str(), "%15[^=]=%15[^-]-%15s", name, start, end);
            if (ret > 2)
            {
                if (strcmp(start, "now") == 0)
                {
                    strcpy(start, "0");
                }
                try
                {
                    start_ = std::stof(start);
                    end_ = std::stof(end);
                    duration_ = end_ - start_;
                }
                catch (std::exception &e)
                {
                    LOG_E << "Error ###2";
                    return false;
                }
            }
        }

        it = attribute_map_.find("rtpmap");
        if (it != attribute_map_.end())
        {
            uint32_t payload{0};
            char codec[16]{0};
            uint32_t samplerate{0};
            uint32_t channel{0};

            ret = sscanf(it->second.c_str(), "%u %15[^/]/%u/%u", &payload, codec, &samplerate, &channel);
            if (ret > 2)
            {
                if (payload == payload_)
                {
                    codec_name_ = codec;
                    to_lower_case(codec_name_);
                    auto itc = string2subtype.find(codec_name_);
                    if (itc != string2subtype.end())
                    {
                        codec_type_ = itc->second;
                    }
                    samplerate_ = samplerate;
                    channels_ = channel;
                }
                else
                {
                    // Is rtx. we not need to parse it;
                }
            }
        }

        it = attribute_map_.find("fmtp");
        if (it != attribute_map_.end())
        {
            uint32_t payload{0};
            ret = sscanf(it->second.c_str(), "%u[^ ]", &payload);
            if (ret != 1)
            {
                LOG_E << "Error ###3";
                return false;
            }
            if (payload == payload_)
            {
                fmtp_ = it->second.substr(it->second.find(' ') + 1);
            }
        }

        it = attribute_map_.find("control");
        if (it != attribute_map_.end())
        {
            std::string tmp = "/" + it->second;
            control_ = tmp.substr(1 + tmp.rfind('/'));
        }
        return true;
    }

    void SdpMediaLine::Clear()
    {
        codec_name_ = "";
        codec_type_ = CodecType::UNSET;
        samplerate_ = 0;
        channels_ = 0;
        payload_ = 0;
        rtx_payload_ = 0;
        track_type_ = TrackType::UNSET;
        transport_ = "";
        start_ = 0.f;
        end_ = 0.f;
        duration_ = 0.f;
        port_ = 0;
        got_ml_ = false;
        fmtp_ = "";
        control_ = "";
        attribute_map_.clear();
    }

    void SdpMediaLine::Dump()
    {
        std::ostringstream oss;
        for (std::pair<const std::string, std::string> &p : attribute_map_)
        {
            oss << "[" << p.first << ", " << trim(p.second) << "2], ";
        }

        std::string tmp = oss.str();
        if (tmp.length() > 1)
        {
            if (tmp[tmp.length() - 2] == ',' && tmp[tmp.length() - 1] == ' ')
            {
                tmp.erase(tmp.end() - 2, tmp.end());
            }
        }

        LOG_D << "\n"
              << "codec_name=" << codec_name_ << "\n"
              << "codec_type=" << (int)codec_type_ << "\n"
              << "samplerate=" << samplerate_ << "\n"
              << "channels=" << channels_ << "\n"
              << "payload=" << payload_ << "\n"
              << "rtx_payload=" << rtx_payload_ << "\n"
              << "track_type=" << (int)track_type_ << "\n"
              << "transport=" << transport_ << "\n"
              << "start=" << start_ << "\n"
              << "end=" << end_ << "\n"
              << "duration=" << duration_ << "\n"
              << "port=" << port_ << "\n"
              << "fmtp=" << fmtp_ << "\n"
              << "control=" << control_ << "\n"
              << "attribute_map=" << tmp;
    }

    void SdpParser::Process(const std::string &sdp)
    {
        info_vec_.clear();

        std::shared_ptr<SdpInfo> info = std::make_shared<SdpInfo>();
        std::shared_ptr<SdpMediaLine> ml = nullptr;

        std::vector<std::string> line_vec;
        split_string(sdp, line_vec, "\n");

        for (std::string &line : line_vec)
        {
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
                if (!ml)
                {
                    ml = std::make_shared<SdpMediaLine>();
                    if (!ml->ProcessML(line.substr(2)))
                    {
                        LOG_E << "ERR ###1";
                    }
                }
                else
                {
                    ml->Process();
                    ml->Dump();
                    info->ml_vec.emplace_back(ml);
                    ml = std::make_shared<SdpMediaLine>();
                    if (!ml->ProcessML(line.substr(2)))
                    {
                        LOG_E << "ERR ###2";
                    }
                }
                break;
            }
            case 'a':
            {
                if (ml)
                {
                    if (!ml->ProcessAL(line.substr(2)))
                    {
                        LOG_E << "ERR ###3";
                    }
                }
                break;
            }
            }
        }

        if (ml)
        {
            ml->Process();
            ml->Dump();
            info->ml_vec.emplace_back(ml);
        }
        info_vec_.emplace_back(info);
    }

} // namespace sms

#include <common/config.h>
#include <common/utils.h>
#include <common/logger.h>
#include <common/sdp_parser.h>

namespace sms
{

    std::unordered_map<std::string, SdpTrackType> SdpMediaTrack::string2type = {
        {"video", SdpTrackType::VIDEO},
        {"audio", SdpTrackType::AUDIO},
    };

    std::map<SdpTrackType, std::string> SdpMediaTrack::type2string = {
        {SdpTrackType::VIDEO, "video"},
        {SdpTrackType::AUDIO, "audio"},
    };

    std::unordered_map<std::string, SdpCodecType> SdpMediaTrack::string2subtype = {
        {"mpeg4-generic", SdpCodecType::AAC_MP4_GEN},
        {"h264", SdpCodecType::H264},
    };

    std::map<SdpCodecType, std::string> SdpMediaTrack::subtype2string = {
        {SdpCodecType::AAC_MP4_GEN, "MPEG4-GENERIC"},
        {SdpCodecType::H264, "H264"},
    };

    bool SdpMediaTrack::ProcessML(const std::string &line)
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

    bool SdpMediaTrack::ProcessAL(const std::string &line)
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

    bool SdpMediaTrack::Process()
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

    void SdpMediaTrack::Clear()
    {
        codec_name_ = "";
        codec_type_ = SdpCodecType::UNSET;
        samplerate_ = 0;
        channels_ = 0;
        payload_ = 0;
        rtx_payload_ = 0;
        track_type_ = SdpTrackType::UNSET;
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

    void SdpMediaTrack::Dump()
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

    const std::string &SdpMediaTrack::GetCodecName() const
    {
        return codec_name_;
    }
    SdpCodecType SdpMediaTrack::GetCodecType() const
    {
        return codec_type_;
    }
    uint32_t SdpMediaTrack::GetSamplerate() const
    {
        return samplerate_;
    }
    uint32_t SdpMediaTrack::GetChannels() const
    {
        return channels_;
    }
    uint32_t SdpMediaTrack::GetPayload() const
    {
        return payload_;
    }
    uint32_t SdpMediaTrack::GetRTXPayload() const
    {
        return rtx_payload_;
    }
    SdpTrackType SdpMediaTrack::GetTrackType() const
    {
        return track_type_;
    }
    const std::string &SdpMediaTrack::GetTransport() const
    {
        return transport_;
    }
    float SdpMediaTrack::GetStartTime() const
    {
        return start_;
    }
    float SdpMediaTrack::GetEndTime() const
    {
        return end_;
    }
    float SdpMediaTrack::GetDuration() const
    {
        return duration_;
    }
    uint16_t SdpMediaTrack::GetPort() const
    {
        return port_;
    }
    const std::string &SdpMediaTrack::GetFmtp() const
    {
        return fmtp_;
    }
    const std::string &SdpMediaTrack::GetControl() const
    {
        return control_;
    }
    const std::unordered_map<std::string, std::string> &SdpMediaTrack::GetAttributeMap() const
    {
        return attribute_map_;
    }

    SdpParser::SdpParser(const std::string &sdp)
    {
        Process(sdp);
    }

    void SdpParser::Process(const std::string &sdp)
    {
        SdpInfo::Ptr info = std::make_shared<SdpInfo>();
        SdpMediaTrack::Ptr ml = nullptr;

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
                    ml = std::make_shared<SdpMediaTrack>();
                    if (!ml->ProcessML(line.substr(2)))
                    {
                        LOG_E << "ERR ###1";
                    }
                }
                else
                {
                    ml->Process();
                    ml->Dump();
                    info->media_track_vec_.emplace_back(ml);
                    ml = std::make_shared<SdpMediaTrack>();
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
            info->media_track_vec_.emplace_back(ml);
        }
        info_ = info;
    }

    std::vector<SdpMediaTrack::Ptr> SdpParser::GetMediaTracks() const
    {
        std::vector<SdpMediaTrack::Ptr> res;
        if (!info_)
        {
            return res;
        }

        res.reserve(2);
        bool got_audio = false;
        bool got_video = false;

        for (auto &track : info_->media_track_vec_)
        {
            if (!got_audio && track->GetTrackType() == SdpTrackType::AUDIO)
            {
                got_audio = true;
                res.emplace_back(track);
                continue;
            }

            if (!got_video && track->GetTrackType() == SdpTrackType::VIDEO)
            {
                got_video = true;
                res.emplace_back(track);
                continue;
            }
        }

        return res;
    }

} // namespace sms

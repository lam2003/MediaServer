#include <media/media_source.h>
#include <common/utils.h>
#include <common/config.h>
#include <http/http_parser.h>

namespace sms
{
    ///////////////////////////////////MediaInfo////////////////////////////////////
    MediaInfo::MediaInfo(const std::string &url)
    {
        Process(url);
    }

    void MediaInfo::Process(const std::string &url_in)
    {
        std::string url = url_in;

        full_url_ = url;

        size_t pos = url.find("?");
        if (pos != std::string::npos)
        {
            param_strs_ = url.substr(pos + 1);
            url.erase(pos);
        }

        size_t schema_pos = url.find("://");
        if (schema_pos != std::string::npos)
        {
            schema_ = url.substr(0, schema_pos);
        }
        else
        {
            schema_pos = -3;
        }

        std::vector<std::string> split_vec;
        split_string(url.substr(schema_pos + 3), split_vec, "/");

        if (split_vec.size() > 0)
        {
            auto vhost = split_vec[0];
            size_t pos = vhost.find(":");
            if (pos != std::string::npos)
            {
                host_ = vhost_ = vhost.substr(0, pos);
                port_ = vhost.substr(pos + 1);
            }
            else
            {
                host_ = vhost_ = vhost;
            }
            if (vhost_ == "localhost" || INADDR_NONE != inet_addr(vhost_.data()))
            {
                //如果访问的是localhost或ip，那么则为默认虚拟主机
                vhost_ = SMS_DEFAULT_VHOST;
            }
        }
        if (split_vec.size() > 1)
        {
            app_ = split_vec[1];
        }
        if (split_vec.size() > 2)
        {
            std::string stream_id;
            for (size_t i = 2; i < split_vec.size(); ++i)
            {
                stream_id.append(split_vec[i] + "/");
            }
            if (stream_id.back() == '/')
            {
                stream_id.pop_back();
            }
            stream_id_ = stream_id;
        }

        StrCaseMap params = HttpParser::ParseArgs(param_strs_);
        if (params.find(SMS_VHOST_KEY) != params.end())
        {
            vhost_ = params[SMS_VHOST_KEY];
        }

        bool enableVhost = true;
        // GET_CONFIG(bool, enableVhost, General::kEnableVhost);
        if (!enableVhost || vhost_.empty())
        {
            //如果关闭虚拟主机或者虚拟主机为空，则设置虚拟主机为默认
            vhost_ = SMS_DEFAULT_VHOST;
        }
    }

    const std::string &MediaInfo::FullUrl()
    {
        return full_url_;
    }
    const std::string &MediaInfo::Schema()
    {
        return schema_;
    }
    const std::string &MediaInfo::Host()
    {
        return host_;
    }
    const std::string &MediaInfo::Port()
    {
        return port_;
    }
    const std::string &MediaInfo::Vhost()
    {
        return vhost_;
    }

    const std::string &MediaInfo::App()
    {
        return app_;
    }
    const std::string &MediaInfo::StreamID()
    {
        return stream_id_;
    }

    const std::string &MediaInfo::ParamStrs()
    {
        return param_strs_;
    }

    void MediaInfo::SetSchema(const std::string &schema)
    {
        schema_ = schema;
    }

    ///////////////////////////////////MediaSourceEventInterceptor////////////////////////////////////
    void MediaSourceEventInterceptor::SetDelegate(const std::weak_ptr<MediaSourceEventListener> &listener)
    {
        if (listener_.lock().get() == this)
        {
            throw std::invalid_argument("can't set self to delegate");
        }
        listener_ = listener;
    }

    MediaSourceEventListener::Ptr MediaSourceEventInterceptor::GetDelegate() const
    {
        return listener_.lock();
    }

    MediaOriginType MediaSourceEventInterceptor::GetOriginType(const MediaSource &sender) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return MediaOriginType::UNSET;
        }
        return listener->GetOriginType(sender);
    }

    std::string MediaSourceEventInterceptor::GetOriginUrl(const MediaSource &sender) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return "";
        }
        return listener->GetOriginUrl(sender);
    }

    SocketInfo::Ptr MediaSourceEventInterceptor::GetOriginSocket(const MediaSource &sender) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return nullptr;
        }
        return listener->GetOriginSocket(sender);
    }

    bool MediaSourceEventInterceptor::SeekTo(MediaSource &sender, uint64_t ts)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->SeekTo(sender, ts);
    }

    bool MediaSourceEventInterceptor::Close(MediaSource &sender, bool force)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->Close(sender, force);
    }

    int MediaSourceEventInterceptor::TotalReadercount(const MediaSource &sender) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return sender.ReaderCount();
        }
        return listener->TotalReadercount(sender);
    }

    void MediaSourceEventInterceptor::OnReaderChanged(const MediaSource &sender, int size)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            listener->OnReaderChanged(sender, size);
        }
    }

    void MediaSourceEventInterceptor::OnRegist(MediaSource &sender, bool regist)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            listener->OnRegist(sender, regist);
        }
    }

    bool MediaSourceEventInterceptor::SetupRecord(MediaSource &sender,
                                                  Recorder::Type type,
                                                  bool start,
                                                  const std::string &path)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->SetupRecord(sender, type, start, path);
    }

    bool MediaSourceEventInterceptor::IsRecording(const MediaSource &sender, Recorder::Type type) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->IsRecording(sender, type);
    }

    std::vector<Track::Ptr> MediaSourceEventInterceptor::GetTracks(const MediaSource &sender, bool ready) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return std::vector<Track::Ptr>();
        }
        return listener->GetTracks(sender, ready);
    }

    void MediaSourceEventInterceptor::StartSendRtp(
        MediaSource &sender,
        const std::string &dst_url,
        uint16_t dst_port,
        const std::string &ssrc,
        bool is_udp,
        std::function<void(const SockException &)> &&cb)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            return listener->StartSendRtp(sender,
                                          dst_url,
                                          dst_port,
                                          ssrc,
                                          is_udp,
                                          std::forward<decltype(cb)>(cb));
        }
    }

    void MediaSourceEventInterceptor::StopSendRtp(MediaSource &sender)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            listener->StopSendRtp(sender);
        }
    }

    ///////////////////////////////////MediaSource////////////////////////////////////
    MediaSource::MediaSource(const std::string &schema,
                             const std::string &vhost,
                             const std::string &app,
                             const std::string &stream_id)
    {
        schema_ = schema;
        vhost_ = vhost;
        app_ = app;
        stream_id_ = stream_id;
    }

    MediaSource::~MediaSource()
    {
    }

    const std::string &MediaSource::GetSchema() const
    {
        return schema_;
    }

    const std::string &MediaSource::GetVhost() const
    {
        return vhost_;
    }

    const std::string &MediaSource::GetApp() const
    {
        return app_;
    }

    const std::string &MediaSource::GetStreamId() const
    {
        return stream_id_;
    }

    int MediaSource::GetBytesSpeed(TrackType type) const
    {
        if (type == TrackType::UNSET)
        {
            return speed_[(int)TrackType::AUDIO].GetSpeed() +
                   speed_[(int)TrackType::VIDEO].GetSpeed();
        }
        return speed_[(int)type].GetSpeed();
    }

    uint64_t MediaSource::GetCreateStamp() const
    {
        return create_stamp_;
    }

    uint64_t MediaSource::GetAliveSecond() const
    {
        return ticker_.GetCreatedTimeMS() / 1000;
    }

    void MediaSource::SetListener(const std::weak_ptr<MediaSourceEventListener> &listener)
    {
        listener_ = listener;
    }

    std::weak_ptr<MediaSourceEventListener> MediaSource::GetListener(bool next) const
    {
        if (!next)
        {
            return listener_;
        }
        MediaSourceEventInterceptor::Ptr interceptor =
            std::dynamic_pointer_cast<MediaSourceEventInterceptor>(listener_.lock());
        if (!interceptor)
        {
            return listener_;
        }
        MediaSourceEventListener::Ptr next_obj = interceptor->GetDelegate();
        return next_obj ? next_obj : listener_;
    }

    int MediaSource::TotalReaderCount() const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return ReaderCount();
        }
        return listener->TotalReadercount(*this);
    }

    MediaOriginType MediaSource::GetOriginType() const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return MediaOriginType::UNSET;
        }
        return listener->GetOriginType(*this);
    }

    std::string MediaSource::GetOriginUrl() const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return "";
        }
        return listener->GetOriginUrl(*this);
    }

    SocketInfo::Ptr MediaSource::GetOriginSocket() const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return nullptr;
        }
        return listener->GetOriginSocket(*this);
    }

    bool MediaSource::SeekTo(uint64_t ts)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->SeekTo(*this, ts);
    }

    bool MediaSource::Close(bool force)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->Close(const_cast<MediaSource &>(*this), force);
    }

    void MediaSource::OnRegist(bool regist)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            return listener->OnRegist(*this, regist);
        }
    }

    void MediaSource::OnReaderChanged(int size)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            return listener->OnReaderChanged(*this, size);
        }
    }

    bool MediaSource::SetupRecord(Recorder::Type type,
                                  bool start,
                                  const std::string &path)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->SetupRecord(*this, type, start, path);
    }

    bool MediaSource::IsRecording(Recorder::Type type) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return false;
        }
        return listener->IsRecording(*this, type);
    }

    void MediaSource::StartSendRtp(const std::string &dst_url,
                                   uint16_t dst_port,
                                   const std::string &ssrc,
                                   bool is_udp,
                                   std::function<void(const SockException &)> &&cb)
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            return listener->StartSendRtp(*this,
                                          dst_url,
                                          dst_port,
                                          ssrc,
                                          is_udp,
                                          std::forward<decltype(cb)>(cb));
        }
    }

    void MediaSource::StopSendRtp()
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (listener)
        {
            listener->StopSendRtp(*this);
        }
    }

    std::vector<Track::Ptr> MediaSource::GetTracks(bool ready) const
    {
        MediaSourceEventListener::Ptr listener = listener_.lock();
        if (!listener)
        {
            return std::vector<Track::Ptr>();
        }

        return listener->GetTracks(*this, ready);
    }

} // namespace sms
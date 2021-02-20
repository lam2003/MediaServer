#ifndef SMS_MEDIA_SOURCE_H
#define SMS_MEDIA_SOURCE_H

#include <common/track.h>
#include <common/recorder.h>
#include <common/bytes_speed.h>
#include <net/socket_exception.h>
#include <net/socket_info.h>

namespace sms
{

    enum class MediaOriginType : uint8_t
    {
        UNSET = 0,
        RTSP = 1,
    };

    class MediaInfo
    {
    public:
        MediaInfo() = default;
        MediaInfo(const std::string &url);
        ~MediaInfo() = default;

    public:
        void Process(const std::string &url);

        const std::string &FullUrl();
        const std::string &Schema();
        const std::string &Host();
        const std::string &Port();
        const std::string &Vhost();
        const std::string &App();
        const std::string &StreamID();
        const std::string &ParamStrs();
        void SetSchema(const std::string &schema);

    private:
        std::string full_url_;
        std::string schema_;
        std::string host_;
        std::string port_;
        std::string vhost_;
        std::string app_;
        std::string stream_id_;
        std::string param_strs_;
    };

    class TrackSource
    {
    public:
        TrackSource() = default;

        virtual ~TrackSource() = default;

    public:
        virtual std::vector<Track::Ptr> GetTracks(bool ready = true) const = 0;

        Track::Ptr GetTrack(TrackType type, bool ready = true) const
        {
            const std::vector<Track::Ptr> &tracks = GetTracks(ready);
            for (auto &ptr : tracks)
            {
                if (ptr->GetTrackType() == type)
                {
                    return ptr;
                }
            }
            return nullptr;
        }
    };

    class MediaSource;
    class MediaSourceEventListener
    {
    public:
        using Ptr = std::shared_ptr<MediaSourceEventListener>;

        friend class MediaSource;

        MediaSourceEventListener() = default;

        virtual ~MediaSourceEventListener() = default;

    public:
        virtual MediaOriginType GetOriginType(const MediaSource &sender) const
        {
            return MediaOriginType::UNSET;
        }

        virtual std::string GetOriginUrl(const MediaSource &sender) const
        {
            return "";
        }

        virtual SocketInfo::Ptr GetOriginSocket(const MediaSource &sender) const
        {
            return nullptr;
        }

        virtual bool SeekTo(MediaSource &sender, uint64_t ts)
        {
            return false;
        }

        virtual bool Close(MediaSource &sender, bool force)
        {
            return false;
        }
        virtual int TotalReadercount(const MediaSource &sender) const = 0;

        virtual void OnReaderChanged(const MediaSource &sender, int size)
        {
        }

        virtual void OnRegist(MediaSource &sender, bool regist)
        {
        }

        virtual bool SetupRecord(MediaSource &sender,
                                 Recorder::Type type,
                                 bool start,
                                 const std::string &path)
        {
            return false;
        }

        virtual bool IsRecording(const MediaSource &sender, Recorder::Type type) const
        {
            return false;
        }

        virtual std::vector<Track::Ptr> GetTracks(const MediaSource &sender, bool ready = true) const
        {
            return std::vector<Track::Ptr>();
        }

        virtual void StartSendRtp(
            MediaSource &sender,
            const std::string &dst_url,
            uint16_t dst_port,
            const std::string &ssrc,
            bool is_udp,
            std::function<void(const SockException &)> &&cb)
        {
        }

        virtual void StopSendRtp(MediaSource &sender)
        {
        }
    };

    class MediaSourceEventInterceptor : public MediaSourceEventListener
    {
    public:
        using Ptr = std::shared_ptr<MediaSourceEventInterceptor>;

        MediaSourceEventInterceptor() = default;

        ~MediaSourceEventInterceptor() = default;

    public:
        void SetDelegate(const std::weak_ptr<MediaSourceEventListener> &listener);

        std::shared_ptr<MediaSourceEventListener> GetDelegate() const;

        //##########################Implement MediaSourceEventListener##########################
        virtual MediaOriginType GetOriginType(const MediaSource &sender) const override final;

        virtual std::string GetOriginUrl(const MediaSource &sender) const override final;

        virtual SocketInfo::Ptr GetOriginSocket(const MediaSource &sender) const override final;

        virtual bool SeekTo(MediaSource &sender, uint64_t ts) override final;

        virtual bool Close(MediaSource &sender, bool force) override final;

        virtual int TotalReadercount(const MediaSource &sender) const override final;

        virtual void OnReaderChanged(const MediaSource &sender, int size) override final;

        virtual void OnRegist(MediaSource &sender, bool regist) override final;

        virtual bool SetupRecord(MediaSource &sender,
                                 Recorder::Type type,
                                 bool start,
                                 const std::string &path) override final;

        virtual bool
        IsRecording(const MediaSource &sender, Recorder::Type type) const override final;

        virtual std::vector<Track::Ptr>
        GetTracks(const MediaSource &sender, bool ready = true) const override final;

        virtual void StartSendRtp(
            MediaSource &sender,
            const std::string &dst_url,
            uint16_t dst_port,
            const std::string &ssrc,
            bool is_udp,
            std::function<void(const SockException &)> &&cb) override final;

        virtual void StopSendRtp(MediaSource &sender) override final;

    private:
        std::weak_ptr<MediaSourceEventListener> listener_;
    };

    class MediaSource : public TrackSource,
                        public std::enable_shared_from_this<MediaSource>
    {
    public:
        using Ptr = std::shared_ptr<MediaSource>;

        MediaSource(const std::string &schema,
                    const std::string &vhost,
                    const std::string &app,
                    const std::string &stream_id);

        virtual ~MediaSource();

    public:
        virtual uint64_t GetTimeStamp(TrackType type) const
        {
            return 0;
        }

        virtual void SetTimeStamp(uint64_t ts)
        {
        }

        const std::string &GetSchema() const;

        const std::string &GetVhost() const;

        const std::string &GetApp() const;

        const std::string &GetStreamId() const;

        int GetBytesSpeed(TrackType type = TrackType::UNSET) const;

        uint64_t GetCreateStamp() const;

        uint64_t GetAliveSecond() const;

    public:
        //##########################MediaSourceEvent相关接口实现##########################
        virtual void SetListener(const std::weak_ptr<MediaSourceEventListener> &listener);

        std::weak_ptr<MediaSourceEventListener> GetListener(bool next = false) const;

        virtual int ReaderCount() const = 0;

        virtual int TotalReaderCount() const;

        MediaOriginType GetOriginType() const;

        std::string GetOriginUrl() const;

        SocketInfo::Ptr GetOriginSocket() const;

        bool SeekTo(uint64_t ts);

        bool Close(bool force);

        void OnRegist(bool regist);

        void OnReaderChanged(int size);

        bool SetupRecord(Recorder::Type type,
                         bool start,
                         const std::string &path);

        bool IsRecording(Recorder::Type type) const;

        void StartSendRtp(const std::string &dst_url,
                          uint16_t dst_port,
                          const std::string &ssrc,
                          bool is_udp,
                          std::function<void(const SockException &)> &&cb);

        void StopSendRtp();

    public:
        // class TrackSource
        std::vector<Track::Ptr> GetTracks(bool ready = true) const override;

    protected:
        BytesSpeed speed_[(int)TrackType::MAX];

    private:
        time_t create_stamp_;
        Ticker ticker_;
        std::string schema_;
        std::string vhost_;
        std::string app_;
        std::string stream_id_;
        std::weak_ptr<MediaSourceEventListener> listener_;
    };

} // namespace sms

#endif
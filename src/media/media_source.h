#ifndef SMS_MEDIA_SOURCE_H
#define SMS_MEDIA_SOURCE_H

#include <media/track.h>
#include <media/recorder.h>
#include <net/socket_exception.h>
#include <net/socket_info.h>

namespace sms
{

    enum class MediaOriginType : uint8_t
    {
        UNSET = 0,
        RTSP = 1,
    };

    class TrackSource
    {
    public:
        TrackSource() = default;

        virtual ~TrackSource() = default;

    public:
        virtual std::vector<Track::Ptr> GetTracks(bool ready = true) const = 0;

        Track::Ptr GetTrack(TrackType type, bool ready = true) const;
    };

    class MediaSource;
    class MedisSourceEventListener
    {
    public:
        MedisSourceEventListener() = default;

        virtual ~MedisSourceEventListener() = default;

    public:
        virtual MediaOriginType GetOriginType(const MediaSource &sender) const;

        virtual std::string GetOriginUrl(const MediaSource &sender) const;

        virtual SocketInfo::Ptr GetOriginSocket(const MediaSource &sender) const;

        virtual bool SeekTo(MediaSource &sender, uint64_t ts);

        virtual bool Close(MediaSource &sender, bool force);

        virtual void OnRegist(MediaSource &sender, bool regist);

        virtual int TotalReadercount(const MediaSource &sender) const = 0;

        virtual bool SetupRecord(MediaSource &sender,
                                 Recorder::Type type,
                                 bool start,
                                 const std::string &path);

        virtual bool IsRecording(const MediaSource &sender, Recorder::Type type) const;

        virtual std::vector<Track::Ptr> GetTracks(const MediaSource &sender, bool ready = true) const;

        virtual void StartSendRtp(
            MediaSource &sender,
            const std::string &dst_url,
            uint16_t dst_port,
            const std::string &ssrc,
            bool is_udp,
            std::function<void(const SockException &)> &&cb);

        virtual void StopSendRtp(MediaSource &sender);
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
        virtual uint64_t GetTimeStamp(TrackType type) const;

        virtual uint64_t SetTimeStamp(uint64_t ts);

        const std::string &GetSchema() const;

        const std::string &GetVhost() const;

        const std::string &GetApp() const;

        const std::string &GetStreamId() const;

        int GetBytesSpeed(TrackType type = TrackType::UNSET) const;

        uint64_t GetCreateStamp() const;

        uint64_t GetAliveSecond() const;

    public:
        // class TrackSource
        std::vector<Track::Ptr> GetTracks(bool ready = true) const override;
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
} // namespace sms

#endif
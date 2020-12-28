#ifndef SMS_MEDIA_SOURCE_H
#define SMS_MEDIA_SOURCE_H

#include <common/global_inc.h>
namespace sms
{

    class TrackSource
    {
    public:
        TrackSource() = default;
        virtual ~TrackSource() = default;
    
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
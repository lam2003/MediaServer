#ifndef SMS_RTSP_SPLITTER_H
#define SMS_RTSP_SPLITTER_H

#include <http/http_request_splitter.h>
#include <common/media_source.h>

namespace sms
{
    class RtspSplitter : public HttpRequestSplitter
    {
    public:
        RtspSplitter() = default;
        virtual ~RtspSplitter() = default;

    protected:
        // override HttpRequestSplitter
        virtual const char *on_search_packet_tail(const char *data, size_t len) override;
        virtual size_t on_recv_header(const char *data, size_t len) override;
        virtual void on_recv_content(const char *data, size_t len) override;

    protected:
        virtual void on_whole_rtsp_packet(HttpParser &parser) = 0;

    protected:
        bool is_rtp_packet_{false};
    };
} // namespace sms

#endif
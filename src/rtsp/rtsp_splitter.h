#ifndef SMS_RTSP_SPLITTER_H
#define SMS_RTSP_SPLITTER_H

#include <http/http_request_splitter.h>
#include <media/media_source.h>

namespace sms
{
    class RtspSplitter : public HttpRequestSplitter
    {
    public:
        RtspSplitter() = default;
        virtual ~RtspSplitter() = default;

    protected:
        // override HttpRequestSplitter
        virtual const uint8_t *on_search_packet_tail(const uint8_t *data, size_t len) override;
        virtual size_t on_recv_header(const uint8_t *data, size_t len) override;
        virtual void on_recv_content(const uint8_t *data, size_t len) override;

    protected:
        virtual void on_whole_rtsp_packet(HttpParser &parser) = 0;

    protected:
        bool is_rtp_packet_{false};
    };
} // namespace sms

#endif
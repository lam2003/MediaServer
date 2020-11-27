#ifndef SMS_RTSP_SPLITTER_H
#define SMS_RTSP_SPLITTER_H

#include <http/http_request_splitter.h>

namespace sms
{
    class RtspSplitter : public HttpRequestSplitter
    {
    public:
        RtspSplitter() = default;
        virtual ~RtspSplitter() = default;

    protected:
        virtual const uint8_t *on_search_packet_tail(const uint8_t *data, size_t len) override;
        virtual size_t on_recv_header(const uint8_t *data, size_t len) override;

    private:
        bool is_rtp_packet_{false};
    };
} // namespace sms

#endif
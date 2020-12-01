#ifndef SMS_HTTP_REQUEST_SPLITTER_H
#define SMS_HTTP_REQUEST_SPLITTER_H

#include <http/http_parser.h>

namespace sms
{
    class HttpRequestSplitter
    {
    public:
        HttpRequestSplitter();
        virtual ~HttpRequestSplitter();

    public:
        size_t Input(const uint8_t *data, size_t len);

    protected:
        virtual const uint8_t *on_search_packet_tail(const uint8_t *data, size_t len);

        virtual size_t on_recv_header(const uint8_t *data, size_t len) = 0;

        virtual void on_recv_content(const uint8_t *data, uint64_t len) = 0;

    protected:
        HttpParser parser_;
        size_t content_len_{0};
    };

} // namespace sms

#endif
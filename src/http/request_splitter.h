#ifndef SMS_HTTP_REQUEST_SPLITTER_H
#define SMS_HTTP_REQUEST_SPLITTER_H

#include <http/parser.h>

namespace sms
{
    class HttpRequestSplitter
    {
    public:
        HttpRequestSplitter();
        virtual ~HttpRequestSplitter();

    public:
        size_t Input(const uint8_t *data, size_t len);

    private:
        const uint8_t *on_search_packet_tail(const uint8_t *data, size_t len);
        size_t on_recv_header(const uint8_t *data, size_t len);

    private:
        HttpParser parser_;
    };

} // namespace sms

#endif
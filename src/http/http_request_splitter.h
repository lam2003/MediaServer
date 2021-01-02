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
        size_t Input(const char *data, size_t len);

    protected:
        virtual const char *on_search_packet_tail(const char *data, size_t len);

        virtual size_t on_recv_header(const char *data, size_t len) = 0;

        virtual void on_recv_content(const char *data, uint64_t len) = 0;

    protected:
        HttpParser parser_;
        size_t content_len_{0};
    };

} // namespace sms

#endif
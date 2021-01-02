#include <http/http_request_splitter.h>
#include <common/config.h>
#include <common/logger.h>

namespace sms
{

    HttpRequestSplitter::HttpRequestSplitter()
    {
    }

    HttpRequestSplitter::~HttpRequestSplitter()
    {
    }

    size_t HttpRequestSplitter::Input(const char *data, size_t len)
    {
        size_t rest_len = len;
        size_t last_head_len = 0;
        const char *head_front = data;
        const char *head_tail = data;

    again:
        while (content_len_ == 0 && rest_len != 0 && (head_tail = on_search_packet_tail(head_front, rest_len)))
        {
            last_head_len = head_tail - head_front;
            rest_len -= last_head_len;
            content_len_ = on_recv_header(head_front, last_head_len);
        }

        if (rest_len == 0)
        {
            LOG_D << "HttpRequestSplitter::Input() Debug ###1";
            return len;
        }

        if (content_len_ == 0)
        {
            if (head_tail == head_front)
            {
                // http head还未收完整
                LOG_D << "HttpRequestSplitter::Input() Debug ###2";
                return 0;
            }
            else
            {
                // 也许这个请求未包含content
                head_front = head_tail;
                goto again;
            }
        }

        // http content还未收完整
        if (rest_len < content_len_)
        {
            LOG_D << "HttpRequestSplitter::Input() Debug ###3";
            return len - rest_len;
        }

        on_recv_content(head_tail, content_len_);
        rest_len -= content_len_;
        head_tail += content_len_;
        content_len_ = 0;

        if (rest_len != 0)
        {
            LOG_D << "HttpRequestSplitter::Input() Debug ###4";
            head_front = head_tail;
            goto again;
        }

        LOG_D << "HttpRequestSplitter::Input() Debug ###5";

        return len;
    }

    const char *HttpRequestSplitter::on_search_packet_tail(const char *data, size_t len)
    {
        const char *pos = strstr(data, SMS_HTTP_HEADER_END);
        if (!pos)
        {
            return nullptr;
        }
        return pos + SMS_HTTP_HEADER_END_LEN;
    }

} // namespace sms
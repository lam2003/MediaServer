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

    size_t HttpRequestSplitter::Input(const uint8_t *data, size_t len)
    {
        size_t rest_len = len;
        size_t last_head_len = 0;
        const uint8_t *head_front = data;
        const uint8_t *head_tail = data;

    again:
        while (content_len_ == 0 && rest_len != 0 && (head_tail = on_search_packet_tail(head_front, rest_len)))
        {
            last_head_len = head_tail - head_front;
            rest_len -= last_head_len;
            content_len_ = on_recv_header(head_front, last_head_len);
            head_front = head_tail;
        }

        if (rest_len == 0)
        {
            LOG_D << "HttpRequestSplitter::Input() Debug ###0";
            return len;
        }

        // http head还未收完整
        if (content_len_ == 0)
        {
            LOG_D << "HttpRequestSplitter::Input() Debug ###1";
            return 0;
        }

        // http content还未收完整
        if (rest_len < content_len_)
        {
            LOG_D << "HttpRequestSplitter::Input() Debug ###2";
            return len - rest_len;
        }

        on_recv_content(head_tail, content_len_);
        rest_len -= content_len_;
        head_front += content_len_;
        content_len_ = 0;

        if (rest_len != 0)
        {
            LOG_D << "HttpRequestSplitter::Input() Debug ###3";
            goto again;
        }

        LOG_D << "HttpRequestSplitter::Input() Debug ###4";

        return len;
    }

    const uint8_t *HttpRequestSplitter::on_search_packet_tail(const uint8_t *data, size_t len)
    {
        char *pos = strstr(const_cast<char *>(reinterpret_cast<const char *>(data)), SMS_HTTP_HEADER_END);
        if (!pos)
        {
            return nullptr;
        }
        return reinterpret_cast<uint8_t *>(pos + SMS_HTTP_HEADER_END_LEN);
    }

} // namespace sms
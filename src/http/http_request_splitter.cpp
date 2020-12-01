#include <http/http_request_splitter.h>

#define SMS_HTTP_HEADER_END_LEN 4
#define SMS_HTTP_HEADER_END "\r\n\r\n"
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
        int rest_len = len;
        const uint8_t *read_pos = data;
        int content_len = 0;

        const uint8_t *head_tail = nullptr;
        while (content_len == 0 && rest_len > 0 && (head_tail = on_search_packet_tail(read_pos, rest_len)))
        {
            const uint8_t *head_front = read_pos;
            size_t head_size = head_tail - head_front;

            read_pos = head_tail;
            rest_len -= head_size;
            content_len = on_recv_header(head_front, head_size);
        }

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
#include <http/http_parser.h>
#include <common/logger.h>
#include <common/utils.h>

#define SMS_HTTP_CRLF_CHAR_LEN 2
#define SMS_HTTP_CRLF_CHAR "\r\n"
#define SMS_HTTP_SPACE_CHAR " "
#define SMS_HTTP_SEPARATOR_CHAR ": "
#define SMS_HTTP_GET_LINE(s, l) find_field((s), (l), nullptr, SMS_HTTP_CRLF_CHAR)
#define SMS_HTTP_GET_METHOD(s, l) find_field((s), (l), nullptr, SMS_HTTP_SPACE_CHAR)
#define SMS_HTTP_GET_FULLURL(s, l) find_field((s), (l), SMS_HTTP_SPACE_CHAR, SMS_HTTP_SPACE_CHAR)
#define SMS_HTTP_GET_KEY(s, l) find_field((s), (l), nullptr, SMS_HTTP_SEPARATOR_CHAR)
#define SMS_HTTP_GET_VAL(s, l) find_field((s), (l), SMS_HTTP_SEPARATOR_CHAR, nullptr)
namespace sms
{

    void HttpParser::Process(const std::string &str)
    {
        Clear();
        bool first_line = true;
        const char *pos = str.c_str();
        size_t rest_len = str.length();
        size_t line_len = 0;

        while (true)
        {
            std::string line = SMS_HTTP_GET_LINE(pos, rest_len);
            if (line.empty())
            {
                break;
            }

            line_len = line.length() + SMS_HTTP_CRLF_CHAR_LEN;
            pos += line_len;
            rest_len -= line_len;

            if (first_line)
            {
                first_line = false;
                method_ = SMS_HTTP_GET_METHOD(line.c_str(), line.length());
                full_url_ = SMS_HTTP_GET_FULLURL(line.c_str(), line.length());
                parse_args(full_url_);
            }
            else
            {
                headers_.emplace(SMS_HTTP_GET_KEY(line.c_str(), line.length()),
                                 SMS_HTTP_GET_VAL(line.c_str(), line.length()));
            }
        }
    }

    void HttpParser::Clear()
    {
        method_.clear();
        url_.clear();
        full_url_.clear();
        params_.clear();
        headers_.clear();
        url_args_.clear();
    }

    std::string HttpParser::find_field(const char *data,
                                       size_t len,
                                       const char *start,
                                       const char *end)
    {
        const char *start_pos = data;
        const char *end_pos = data + len;

        if (start)
        {
            start_pos = strstr(data, start);
            if (!start_pos)
            {
                return "";
            }
            start_pos += strlen(start);
        }

        if (end)
        {
            end_pos = strstr(start_pos, end);
            if (!end_pos)
            {
                return "";
            }
        }

        return std::string(start_pos, end_pos - start_pos);
    }

    void HttpParser::parse_args(const std::string &str)
    {
        size_t pos = str.find('?');
        if (pos == std::string::npos)
        {
            url_ = str;
            return;
        }

        url_ = str.substr(0, pos);
        // 去除问号
        params_ = str.substr(pos + 1);

        std::vector<std::string> kv_str_vec;
        split_string(params_, kv_str_vec, "&");

        std::vector<std::string> kv;
        for (const std::string &kv_str : kv_str_vec)
        {
            split_string(kv_str, kv, "=");
            if (kv.size() != 2)
            {
                continue;
            }
            url_args_.emplace(kv[0], kv[1]);
        }
    }

} // namespace sms
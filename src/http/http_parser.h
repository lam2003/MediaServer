#ifndef SMS_HTTP_PARSE_H
#define SMS_HTTP_PARSE_H

#include <common/global_inc.h>

namespace sms
{

    struct StrCaseCompare
    {
        bool operator()(const std::string &lhs, const std::string &rhs) const
        {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };

    class StrCaseMap : public std::multimap<std::string, std::string, StrCaseCompare>
    {
    public:
        using Super = std::multimap<std::string, std::string, StrCaseCompare>;
        StrCaseMap() = default;
        ~StrCaseMap() = default;

        std::string &operator[](const std::string &k)
        {
            Super::iterator it = Super::find(k);
            if (it == Super::end())
            {
                it = Super::emplace(k, "");
            }
            return it->second;
        }

        template <typename T>
        void emplace(const std::string &k, T &&v)
        {
            Super::iterator it = Super::find(k);
            if (it != end())
            {
                return;
            }

            Super::emplace(k, std::forward<T>(v));
        }

        template <typename T>
        void emplace_force(const std::string &k, T &&v)
        {
            Super::emplace(k, std::forward<T>(v));
        }
    };

    class HttpParser
    {
    public:
        HttpParser() = default;
        ~HttpParser() = default;

    public:
        void Process(const std::string &str);
        void Clear();

        const std::string &operator[](const char *name) const;

        const std::string &Method() const;

        const std::string &Url() const;

        const std::string &FullUrl() const;

        void SetContent(const std::string content);

        static StrCaseMap ParseArgs(const std::string &str, const char *pair_delim = "&", const char *key_delim = "=");

    private:
        static std::string find_field(const char *data,
                                      size_t len,
                                      const char *start,
                                      const char *end);
        void parse_args(const std::string &str);

    private:
        std::string method_;
        std::string url_;
        std::string tail_;
        std::string full_url_;
        std::string params_;
        StrCaseMap headers_;
        StrCaseMap url_args_;
        std::string null_;
        std::string content_;
    };

} // namespace sms

#endif
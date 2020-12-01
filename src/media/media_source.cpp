#include <media/media_source.h>
#include <common/utils.h>
#include <common/config.h>
#include <http/http_parser.h>

namespace sms
{

    void MediaInfo::Process(const std::string &url_in)
    {
        std::string url = url_in;

        full_url_ = url;

        size_t pos = url.find("?");
        if (pos != std::string::npos)
        {
            param_strs_ = url.substr(pos + 1);
            url.erase(pos);
        }

        size_t schema_pos = url.find("://");
        if (schema_pos != std::string::npos)
        {
            schema_ = url.substr(0, schema_pos);
        }
        else
        {
            schema_pos = -3;
        }

        std::vector<std::string> split_vec;
        split_string(url.substr(schema_pos + 3), split_vec, "/");

        if (split_vec.size() > 0)
        {
            auto vhost = split_vec[0];
            size_t pos = vhost.find(":");
            if (pos != std::string::npos)
            {
                host_ = vhost_ = vhost.substr(0, pos);
                port_ = vhost.substr(pos + 1);
            }
            else
            {
                host_ = vhost_ = vhost;
            }
            if (vhost_ == "localhost" || INADDR_NONE != inet_addr(vhost_.data()))
            {
                //如果访问的是localhost或ip，那么则为默认虚拟主机
                vhost_ = DEFAULT_VHOST;
            }
        }
        if (split_vec.size() > 1)
        {
            app_ = split_vec[1];
        }
        if (split_vec.size() > 2)
        {
            std::string stream_id;
            for (size_t i = 2; i < split_vec.size(); ++i)
            {
                stream_id.append(split_vec[i] + "/");
            }
            if (stream_id.back() == '/')
            {
                stream_id.pop_back();
            }
            stream_id_ = stream_id;
        }

        StrCaseMap params = HttpParser::ParseArgs(param_strs_);
        if (params.find(VHOST_KEY) != params.end())
        {
            vhost_ = params[VHOST_KEY];
        }

        bool enableVhost = true;
        // GET_CONFIG(bool, enableVhost, General::kEnableVhost);
        if (!enableVhost || vhost_.empty())
        {
            //如果关闭虚拟主机或者虚拟主机为空，则设置虚拟主机为默认
            vhost_ = DEFAULT_VHOST;
        }
    }

    const std::string &MediaInfo::FullUrl()
    {
        return full_url_;
    }
    const std::string &MediaInfo::Schema()
    {
        return schema_;
    }
    const std::string &MediaInfo::Host()
    {
        return host_;
    }
    const std::string &MediaInfo::Port()
    {
        return port_;
    }
    const std::string &MediaInfo::Vhost()
    {
        return vhost_;
    }

    const std::string &MediaInfo::App()
    {
        return app_;
    }
    const std::string &MediaInfo::StreamID()
    {
        return stream_id_;
    }

    const std::string &MediaInfo::ParamStrs()
    {
        return param_strs_;
    }

    void MediaInfo::SetSchema(const std::string &schema)
    {
        schema_ = schema;
    }
} // namespace sms
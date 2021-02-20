#include <rtsp/rtsp_session.h>
#include <common/utils.h>
#include <common/config.h>
#include <common/once_token.h>
#include <common/logger.h>
#include <common/sdp_parser.h>

#define SMS_RTSP_SCHEMA "rtsp"

namespace sms
{
    RtspSession::RtspSession(TcpConnection *conn) : TcpSession(conn)
    {
    }

    RtspSession::~RtspSession()
    {
    }

    void RtspSession::OnError(const SockException &err)
    {
    }

    size_t RtspSession::OnRecv(const char *data, size_t len)
    {
        // invoke HttpRequestSplitter::Input()
        size_t consumed = Input(data, len);
        bytes_usage_ += consumed;

        return consumed;
    }

    void RtspSession::OnManage()
    {
    }

    void RtspSession::handle_options(const HttpParser &parser)
    {
        send_rtsp_response("200 OK", {"Public", "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, ANNOUNCE, RECORD, SET_PARAMETER, GET_PARAMETER"});
    }

    void RtspSession::handle_announce(const HttpParser &http_parser)
    {
        if (media_info_.App().empty() || media_info_.StreamID().empty())
        {
            send_rtsp_response("403 Forbidden", {"Content-Type", "text/plain"},
                               "illegal url. stream url format should like rtsp://$host/$app/$stream_id");
            throw SockException("illegal url", SockException::SHUTDOWN);
        }
        SdpParser sdp_parser;
        sdp_parser.Process(http_parser.Content());

        sdp_media_track_vec_ = sdp_parser.GetMediaTracks();
        send_rtsp_response("200 OK", {"Content-Base", content_base_ + "/"});
        // send_rtsp_response("200 OK", {"Public", "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, ANNOUNCE, RECORD, SET_PARAMETER, GET_PARAMETER"});
    }

    static std::string rtsp_date_str()
    {
        char buf[64];
        time_t tt = time(NULL);
        strftime(buf, sizeof buf, "%a, %b %d %Y %H:%M:%S GMT", gmtime(&tt));
        return buf;
    }

    void RtspSession::send_rtsp_response(const std::string &res_code,
                                         const StrCaseMap &header_const,
                                         const std::string &sdp,
                                         const char *protocol)
    {
        StrCaseMap header = header_const;

        header.emplace("CSeq", StringPrinter << cseq_);
        if (!session_id_.empty())
        {
            header.emplace("Session", session_id_);
        }

        header.emplace("Server", SMS_SERVER_NAME);
        header.emplace("Data", rtsp_date_str());

        if (!sdp.empty())
        {
            header.emplace("Content-Length", StringPrinter << sdp.size());
            header.emplace("Content-Type", "application/sdp");
        }

        __StringPrinter printer;
        printer << protocol << SMS_HTTP_SPACE_CHAR << res_code << SMS_HTTP_CRLF_CHAR;
        for (auto &p : header)
        {
            printer << p.first << SMS_HTTP_SEPARATOR_CHAR << p.second << SMS_HTTP_CRLF_CHAR;
        }

        printer << SMS_HTTP_CRLF_CHAR;

        if (!sdp.empty())
        {
            printer << sdp;
        }

        send(std::make_shared<BufferString>(std::move(printer)));
    }

    void RtspSession::send_rtsp_response(const std::string &res_code,
                                         const std::initializer_list<std::string> &header,
                                         const std::string &sdp,
                                         const char *protocol)
    {
        std::string key;
        StrCaseMap header_map;

        int i = 0;
        for (const std::string &val : header)
        {
            if (++i % 2 == 0)
            {
                header_map.emplace(key, val);
            }
            else
            {
                key = val;
            }
        }

        send_rtsp_response(res_code, header_map, sdp, protocol);
    }

    void RtspSession::send(const BufferList::Ptr &list)
    {
        bytes_usage_ += list->GetRemainSize();
        TcpSession::send(list);
    }

    void RtspSession::send(const Buffer::Ptr &buf)
    {
        bytes_usage_ += buf->Size();
        TcpSession::send(buf);
    }

    void RtspSession::on_whole_rtsp_packet(HttpParser &parser)
    {
        std::string method = parser.Method();
        cseq_ = atoi(parser["CSeq"].data());

        if (content_base_.empty() && method != "GET")
        {
            content_base_ = parser.Url();
            media_info_.Process(parser.FullUrl());
            media_info_.SetSchema(SMS_RTSP_SCHEMA);
        }

        typedef void (RtspSession::*rtsp_request_handler)(const HttpParser &parser);
        static std::unordered_map<std::string, rtsp_request_handler> s_cmd_functions;
        static OnceToken token([]() {
            s_cmd_functions.emplace("OPTIONS", &RtspSession::handle_options);
            s_cmd_functions.emplace("ANNOUNCE", &RtspSession::handle_announce);
        });

        auto it = s_cmd_functions.find(method);
        if (it == s_cmd_functions.end())
        {
            send_rtsp_response("403 Forbidden");
            shutdown();
            return;
        }

        auto &func = it->second;
        try
        {
            (this->*func)(parser);
        }
        catch (std::exception &e)
        {
            LOG_E << e.what();
            shutdown();
        }

        parser.Clear();
    }

} // namespace sms
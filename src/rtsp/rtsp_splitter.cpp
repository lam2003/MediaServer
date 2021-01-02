#include <rtsp/rtsp_splitter.h>
#include <rtsp/rtsp_session.h>
#include <common/config.h>
#include <common/logger.h>

#define SMS_RTSP_RTP_BEGIN_CHAR '$'

namespace sms
{

    const char *RtspSplitter::on_search_packet_tail(const char *data, size_t len)
    {
        if (len < 1)
        {
            return nullptr;
        }

        if (data[0] == SMS_RTSP_RTP_BEGIN_CHAR)
        {
            // is rtp packet
            is_rtp_packet_ = true;
            return nullptr;
        }
        else
        {
            // is rtsp singling
            is_rtp_packet_ = false;
            return HttpRequestSplitter::on_search_packet_tail(data, len);
        }
    }

    size_t RtspSplitter::on_recv_header(const char *data, size_t len)
    {
        if (is_rtp_packet_)
        {
            return 0;
        }

        std::string str(data, len);
        LOG_I << "################ header: " << str;

        parser_.Process(std::string(data, len));

        int content_len = atoi(parser_["Content-Length"].data());
        if (!content_len)
        {
            on_whole_rtsp_packet(parser_);
        }
        return content_len;
    }

    void RtspSplitter::on_recv_content(const char *data, size_t len)
    {
        std::string str(data, len);
        LOG_I << "################ content: " << str;

        parser_.SetContent(std::string(data, len));
        on_whole_rtsp_packet(parser_);
    }

} // namespace sms
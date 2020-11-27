#include <rtsp/rtsp_splitter.h>

#define SMS_RTSP_RTP_BEGIN_CHAR '$'

namespace sms
{

    const uint8_t *RtspSplitter::on_search_packet_tail(const uint8_t *data, size_t len)
    {
        if (len < 1)
        {
            return nullptr;
        }

        if (data[0] == SMS_RTSP_RTP_BEGIN_CHAR)
        {
            // is rtp packet
            is_rtp_packet_ = true;
        }
        else
        {
            // is rtsp singling
            is_rtp_packet_ = false;
            return HttpRequestSplitter::on_search_packet_tail(data, len);
        }
    }

    size_t RtspSplitter::on_recv_header(const uint8_t *data, size_t len)
    {
        if (is_rtp_packet_)
        {
            return 0;
        }
        else
        {
            
        }
    }

} // namespace sms
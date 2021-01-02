#include <net/tcp_session.h>
#include <rtsp/rtsp_splitter.h>
#include <media/sdp_parser.h>

namespace sms
{

    class RtspSession : public RtspSplitter, public TcpSession
    {
    public:
        RtspSession(TcpConnection *conn);
        ~RtspSession();

    public:
        // override TcpSession
        void OnError(const SockException &err) override;
        size_t OnRecv(const char *data, size_t len) override;
        void OnManage() override;

    protected:
        // override TcpSession
        void send(const BufferList::Ptr &list) override;
        void send(const Buffer::Ptr &buf) override;

        // override RtspSplitter
        void on_whole_rtsp_packet(HttpParser &parser) override;

    private:
        void handle_options(const HttpParser &parser);
        void handle_announce(const HttpParser &parser);

        void send_rtsp_response(const std::string &res_code,
                                const std::initializer_list<std::string> &header,
                                const std::string &sdp = "",
                                const char *protocol = "RTSP/1.0");

        void send_rtsp_response(const std::string &res_code,
                                const StrCaseMap &header = StrCaseMap(),
                                const std::string &sdp = "",
                                const char *protocol = "RTSP/1.0");

    private:
        size_t bytes_usage_{0};
        int cseq_{0};
        std::string session_id_;
        std::string content_base_;
        MediaInfo media_info_;
        std::vector<SdpMediaTrack::Ptr> sdp_media_track_vec_;
    };

} // namespace sms

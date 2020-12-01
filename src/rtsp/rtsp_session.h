#include <net/tcp_session.h>
#include <rtsp/rtsp_splitter.h>

namespace sms
{

    class RtspSession : public RtspSplitter, public TcpSession
    {
    public:
        RtspSession(TcpConnection *conn) : TcpSession(conn) {}

        ~RtspSession() = default;

    public:
        size_t OnRecv(const uint8_t *data, size_t len)
        {
            return Input(data, len);
        }

    private:
        void handle_options(const HttpParser &parser);

        void send_rtsp_response(const std::string &res_code,
                                const std::initializer_list<std::string> &header,
                                const std::string &sdp = "",
                                const char *protocol = "RTSP/1.0");

        void send_rtsp_response(const std::string &res_code,
                                const StrCaseMap &header = StrCaseMap(),
                                const std::string &sdp = "",
                                const char *protocol = "RTSP/1.0");

    protected:
        // override TcpSession
        void send(const std::shared_ptr<BufferList> &list) override;

        void send(const std::shared_ptr<Buffer> &buf) override;

        // override RtspSplitter
        void on_whole_rtsp_packet(const HttpParser &parser) override;

    private:
        size_t bytes_usage_{0};
    };

} // namespace sms

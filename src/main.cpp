#include <common/logger.h>
#include <net/tcp_connection.h>
#include <net/tcp_server.h>
#include <dep/dep_libuv.h>
#include <net/signals_handler.h>
#include <net/socket_utils.h>
#include <http/http_parser.h>
#include <common/utils.h>
#include "common/media_source.h"
#include "rtsp/rtsp_session.h"
#include "http/http_request_splitter.h"
using namespace sms;

void signal_handler(int signo) {}

int main(int argc, char **argv)
{
    Logger::Instance().AddChannel(std::make_shared<ConsoleChannel>());
    Logger::Instance().SetWriter(
        std::make_shared<AsyncLogWriter>(Logger::Instance()));

    DepLibUV::ClassInit();
    DepLibUV::PrintVersion();

    TcpServer tcp_server;
    SignalsHandler g_sig_hdl([&tcp_server](SignalsHandler *handler, int signo) {
        // printf("###################### %d\n",signo);
        if (signo == SIGPIPE)
        {
            return;
        }

        tcp_server.Close();
        handler->Close();
    });
    g_sig_hdl.AddSignal(SIGTERM);
    g_sig_hdl.AddSignal(SIGINT);
    g_sig_hdl.AddSignal(SIGPIPE);
    char buff[2 * 1024 * 1024] = {1};
    std::shared_ptr<BufferRaw> buf = std::make_shared<BufferRaw>();

    MediaInfo info;
    info.Process("rtsp://linmin.xyz:443/live/1/test?fuck=true&haha=9&vhost=fuck.com");
    // LOG_D<<info.schema_<<" "<<info.app_ << " "<<info.stream_id_<<" "<< info.param_strs_ <<" "<<info.vhost_;

    buf->Assign(buff, 2 * 1024 * 1024);
    tcp_server.Start(reinterpret_cast<uv_tcp_t *>(SocketUtils::Bind(SocketUtils::SOCK_TCP, "0.0.0.0", 8088)), 1024);
    tcp_server.SetAcceptCB([buf](TcpConnection *cc) {
        // std::list<std::shared_ptr<Buffer>> ll;
        // ll.push_back(buf);
        // ll.push_back(buf);
        // ll.push_back(buf);
        // ll.push_back(buf);
        // ll.push_back(buf);
        // LOG_I << cc->GetPeerPort() << " connected";

        RtspSession *ss = new RtspSession(cc);
        cc->SetReadCB([ss](TcpConnection *cc, const char *data, size_t len) {
            return ss->OnRecv(data, len);
        });

        // cc->Write(std::make_shared<BufferList>(ll), [cc](bool v) {
        //     LOG_E << "####################################### 1 " << v << "   " << cc->GetPeerPort();
        // });
    });
    tcp_server.SetClosedCB([](TcpConnection *cc) {
        LOG_D << cc->GetPeerPort() << " closed";
    });

    DepLibUV::RunLoop();
    DepLibUV::ClassDestroy();
    return 0;
}
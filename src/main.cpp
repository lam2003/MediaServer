#include <common/logger.h>
#include <net/tcp_connection.h>
#include <net/tcp_server.h>
#include <net/dep_libuv.h>
#include <net/signals_handler.h>
#include <net/socket_utils.h>
#include <http/http_parser.h>
#include <common/utils.h>
#include "http/http_request_splitter.h"
using namespace sms;

void signal_handler(int signo){}

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
    char buf[2 * 1024 * 1024] = {1};
    tcp_server.Start(reinterpret_cast<uv_tcp_t *>(SocketUtils::Bind(SocketUtils::SOCK_TCP, "0.0.0.0", 80)), 1024);
    tcp_server.SetAcceptCB([buf](TcpConnection *cc) {
        // LOG_I << cc->GetPeerPort() << " connected";
        cc->SetReadCB([](TcpConnection *cc, const uint8_t *data, size_t len) {
            cc->Close();
            // HttpRequestSplitter ss;
            // HttpParser parser;

            // return ss.Input(data, len);
            return len;
            // parser.Process(std::string(reinterpret_cast<const char *>(data), len));
            // std::string str(reinterpret_cast<const char *>(data), len);
            // LOG_D << str;
            // return len;
        });

        cc->Write(reinterpret_cast<const uint8_t *>(buf), 2 * 1024 * 1024, [](bool v) {
            // LOG_E << "####################################### " << v;
        });
    });
    tcp_server.SetClosedCB([](TcpConnection *cc) {
        // LOG_D << cc->GetPeerPort() << " closed";
    });

    DepLibUV::RunLoop();
    DepLibUV::ClassDestroy();
    return 0;
}
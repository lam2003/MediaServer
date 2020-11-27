#include <common/logger.h>
#include <net/tcp_connection.h>
#include <net/tcp_server.h>
#include <net/dep_libuv.h>
#include <net/signals_handler.h>
#include <net/socket_utils.h>
#include <http/parser.h>
#include <common/utils.h>
#include "http/request_splitter.h"
using namespace sms;

int main(int argc, char **argv)
{
    Logger::Instance().AddChannel(std::make_shared<ConsoleChannel>());
    Logger::Instance().SetWriter(
        std::make_shared<AsyncLogWriter>(Logger::Instance()));

    DepLibUV::ClassInit();
    DepLibUV::PrintVersion();

    TcpServer tcp_server;
    SignalsHandler g_sig_hdl([&tcp_server](SignalsHandler *handler, int signo) {
        tcp_server.Close();
        handler->Close();
    });

    g_sig_hdl.AddSignal(SIGTERM);
    g_sig_hdl.AddSignal(SIGINT);

    tcp_server.Start(reinterpret_cast<uv_tcp_t *>(SocketUtils::Bind(SocketUtils::SOCK_TCP, "0.0.0.0", 80)), 1024);
    tcp_server.SetAcceptCB([](TcpConnection *cc) {
        LOG_I << cc->GetPeerPort() << " connected";
        cc->SetReadCB([](TcpConnection *cc, const uint8_t *data, size_t len) {
            HttpRequestSplitter ss;
            // HttpParser parser;


            return ss.Input(data, len);
            // parser.Process(std::string(reinterpret_cast<const char *>(data), len));
            // std::string str(reinterpret_cast<const char *>(data), len);
            // LOG_D << str;
            // return len;
        });
    });
    tcp_server.SetClosedCB([](TcpConnection *cc) {
        LOG_D << cc->GetPeerPort() << " closed";
    });

    DepLibUV::RunLoop();
    DepLibUV::ClassDestroy();
    return 0;
}
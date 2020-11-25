#include <common/logger.h>
#include <net/tcp_connection.h>
#include <net/tcp_server.h>
#include <net/dep_libuv.h>
#include <net/signals_handler.h>
#include <net/socket_utils.h>
using namespace sms;

SignalsHandler *pg_sig_hdl;
TcpServer *pg_server;
class AA : public SignalsHandler::Listener, public TcpServer::Listener
{
public:
    ~AA() {}
    void OnSignal(SignalsHandler *hdl, int signo)
    {
        if (signo == SIGTERM || signo == SIGINT)
        {
            pg_server->Close();
            pg_sig_hdl->Close();
        }
    }
    size_t OnTcpConnectionPacketReceived(TcpConnection *conn, const uint8_t *data, size_t len)
    {
        return len;
    }
};

AA a;
TcpServer g_server;
SignalsHandler g_sig_hdl(&a);

int main(int argc, char **argv)
{
    pg_sig_hdl = &g_sig_hdl;
    pg_server = &g_server;
    Logger::Instance().AddChannel(std::make_shared<ConsoleChannel>());
    Logger::Instance().SetWriter(
        std::make_shared<AsyncLogWriter>(Logger::Instance()));

    DepLibUV::ClassInit();
    DepLibUV::PrintVersion();

    g_sig_hdl.AddSignal(SIGTERM);
    g_sig_hdl.AddSignal(SIGINT);

    g_server.Start(&a, reinterpret_cast<uv_tcp_t *>(SocketUtils::Bind(SocketUtils::SOCK_TCP, "0.0.0.0", 80)), 1024);

    DepLibUV::RunLoop();
    DepLibUV::ClassDestroy();
    return 0;
}
#include <common/logger.h>
#include <net/tcp_connection.h>
#include <net/tcp_server.h>
#include <net/dep_libuv.h>
#include <net/socket_utils.h>
using namespace sms;

int main(int argc, char **argv)
{

    Logger::Instance().AddChannel(std::make_shared<ConsoleChannel>());
    Logger::Instance().SetWriter(
        std::make_shared<AsyncLogWriter>(Logger::Instance()));
    LOG_D << "TEST LOG";

    DepLibUV::ClassInit();
    DepLibUV::PrintVersion();

    TcpServer server;
    server.Start(reinterpret_cast<uv_tcp_t *>(SocketUtils::Bind(SocketUtils::SOCK_TCP, "0.0.0.0", 80)), 1024);

    DepLibUV::RunLoop();

    return 0;
}
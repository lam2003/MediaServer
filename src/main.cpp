#include <common/logger.h>
#include <net/tcp_connection.h>
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

    // TcpConnection *p =  new TcpConnection(1024 * 1024);
    // p->Setup(nullptr, nullptr, "", 1);

    // p->Start();

    // p->Close();
    // p->Dump();
    // delete p;

    std::string ip = "0.0.0.0";
    SocketUtils::Bind(SocketUtils::SOCK_TCP, ip, 80);
    // p->Close();
    DepLibUV::RunLoop();

    return 0;
}
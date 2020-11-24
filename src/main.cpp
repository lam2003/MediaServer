#include <common/logger.h>
#include <net/tcp_connection.h>
#include <common/dep_libuv.h>
using namespace sms;

int main(int argc, char **argv)
{

    Logger::Instance().AddChannel(std::make_shared<ConsoleChannel>());
    Logger::Instance().SetWriter(
        std::make_shared<AsyncLogWriter>(Logger::Instance()));
    LOG_D << "TEST LOG";

    DepLibUV::ClassInit();
    DepLibUV::PrintVersion();

    TcpConnection c(1024 * 1024);
    c.Setup(nullptr, nullptr, "", 1);

    // c.Start();

    c.Close();

    DepLibUV::RunLoop();

    return 0;
}
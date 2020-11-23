#include <common/logger.h>

using namespace sms;

int main(int argc, char **argv)
{

    Logger::Instance().AddChannel(std::make_shared<ConsoleChannel>());
    Logger::Instance().SetWriter(
        std::make_shared<AsyncLogWriter>(Logger::Instance()));
    LOG_D << "TEST LOG";
    return 0;
}
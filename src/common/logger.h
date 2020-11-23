#ifndef SMS_LOGGER_H
#define SMS_LOGGER_H

#include <thread/semaphore.h>
#include <common/noncopyable.h>

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <thread>
#include <list>

namespace sms
{

    class LogContext;
    class LogWriter;
    class LogChannel;
    typedef enum
    {
        LTRACE,
        LDEBUG,
        LINFO,
        LWARN,
        LERROR
    } LogLevel;

    class Logger final : public NonCopyable
    {
    public:
        friend class LogWriter;

        ~Logger() = default;

        static Logger &Instance();

    public:
        void AddChannel(const std::shared_ptr<LogChannel> &channel);

        void RemoveChannel(const std::string &name);

        std::shared_ptr<LogChannel> GetChannel(const std::string &name);

        const std::string &GetLoggerName() const;

        void SetWriter(std::shared_ptr<LogWriter> writer);

        void SetLevel(LogLevel level);

        void Write(const std::shared_ptr<LogContext> &pctx);

    private:
        Logger(const std::string &logger_name);

    private:
        void write_channels(const std::shared_ptr<LogContext> pctx);

    private:
        std::map<std::string, std::shared_ptr<LogChannel>> channels_;
        std::string logger_name_;
        std::shared_ptr<LogWriter> writer_;
    };

    class LogContext final : public std::ostringstream
    {
    public:
        LogContext(LogLevel level,
                   const char *file,
                   pid_t tid,
                   const char *function,
                   int line);

        ~LogContext() = default;

    public:
        LogLevel level;
        int line;
        std::string file;
        pid_t tid;
        std::string function;
        timeval tv;
    };

    class LogWriter : public NonCopyable
    {
    public:
        LogWriter(Logger &logger);
        virtual ~LogWriter() = default;

    public:
        virtual void Write(const std::shared_ptr<LogContext> &pctx) = 0;

    protected:
        void write_to_channels(const std::shared_ptr<LogContext> &pctx);

    private:
        Logger &logger_;
    };

    class AsyncLogWriter final : public LogWriter
    {
    public:
        AsyncLogWriter(Logger &logger);
        ~AsyncLogWriter();

    public:
        void Write(const std::shared_ptr<LogContext> &pctx) override;

    private:
        void run();
        void flush_all();

    private:
        volatile bool running_;
        std::thread *thread_;
        Semaphore sem_;
        std::list<std::shared_ptr<LogContext>> pctxs_;
        std::mutex mux_;
    };

    class LogChannel : public NonCopyable
    {
    public:
        LogChannel(const std::string &name, LogLevel level = LTRACE);

        virtual ~LogChannel() = default;

    public:
        virtual void Write(const Logger &logger,
                           const std::shared_ptr<LogContext> &pctx) = 0;

        const std::string &Name() const;

        void SetLevel(LogLevel level);

    protected:
        virtual void format(const Logger &logger,
                            std::ostream &ostr,
                            const std::shared_ptr<LogContext> &pctx,
                            bool enable_color = true,
                            bool enable_detail = true);

    protected:
        std::string name_;
        LogLevel level_;
    };

    class ConsoleChannel final : public LogChannel
    {
    public:
        ConsoleChannel(const std::string &name = "ConsoleChannel",
                       LogLevel level = LTRACE);

        ~ConsoleChannel() = default;

    public:
        void Write(const Logger &logger,
                   const std::shared_ptr<LogContext> &pctx) override;
    };

    class LogContextCapturer final
    {
    public:
        LogContextCapturer(Logger &logger,
                           LogLevel level,
                           const char *file,
                           const char *function,
                           int line);

        LogContextCapturer(const LogContextCapturer &that);

        ~LogContextCapturer();

    public:
        // 输入std::endl时直接输出结果
        LogContextCapturer &operator<<(std::ostream &(*f)(std::ostream &));

        template <typename T>
        LogContextCapturer &operator<<(T &&data)
        {
            if (!pctx_)
            {
                return *this;
            }
            (*pctx_) << std::forward<T>(data);
            return *this;
        }

        void Clear();

    private:
        std::shared_ptr<LogContext> pctx_;
        Logger &logger_;
    };

#define LOG_T                                                              \
    LogContextCapturer(Logger::Instance(), LTRACE, __FILE__, __FUNCTION__, \
                       __LINE__)
#define LOG_D                                                              \
    LogContextCapturer(Logger::Instance(), LDEBUG, __FILE__, __FUNCTION__, \
                       __LINE__)
#define LOG_I                                                             \
    LogContextCapturer(Logger::Instance(), LINFO, __FILE__, __FUNCTION__, \
                       __LINE__)
#define LOG_W                                                             \
    LogContextCapturer(Logger::Instance(), LWARN, __FILE__, __FUNCTION__, \
                       __LINE__)
#define LOG_E                                                              \
    LogContextCapturer(Logger::Instance(), LERROR, __FILE__, __FUNCTION__, \
                       __LINE__)

} // namespace sms

#endif
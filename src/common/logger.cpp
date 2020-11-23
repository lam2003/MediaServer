#include <common/logger.h>
#include <common/utils.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#ifndef gettid
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

namespace sms
{

    static const char *s_log_const_table[][3] = {
        {"\033[44;37m", "\033[34m", "T"},  // 蓝底灰字，黑底蓝字
        {"\033[42;37m", "\033[32m", "D"},  // 绿底灰字，黑底绿字
        {"\033[46;37m", "\033[36m", "I"},  // 天蓝底灰字，黑底天蓝字
        {"\033[43;37m", "\033[33m", "W"},  // 黄底灰字，黑底黄字
        {"\033[41;37m", "\033[31m", "E"}}; // 红底灰字，黑底红字

#define CLEAR_COLOR "\033[0m"

    INSTANCE_IMPL(Logger, get_exe_name());

    Logger::Logger(const std::string &logger_name)
    {
        logger_name_ = logger_name;
        writer_ = nullptr;
    }

    void Logger::AddChannel(const std::shared_ptr<LogChannel> &channel)
    {
        channels_[channel->Name()] = channel;
    }

    void Logger::RemoveChannel(const std::string &name)
    {
        channels_.erase(name);
    }

    std::shared_ptr<LogChannel> Logger::GetChannel(const std::string &name)
    {
        std::map<std::string, std::shared_ptr<LogChannel>>::iterator it =
            channels_.find(name);
        if (it != channels_.end())
        {
            return it->second;
        }

        return nullptr;
    }

    const std::string &Logger::GetLoggerName() const
    {
        return logger_name_;
    }

    void Logger::SetWriter(std::shared_ptr<LogWriter> writer)
    {
        writer_ = writer;
    }

    void Logger::SetLevel(LogLevel level)
    {
        for (std::pair<const std::string, std::shared_ptr<LogChannel>> &it :
             channels_)
        {
            it.second->SetLevel(level);
        }
    }

    void Logger::Write(const std::shared_ptr<LogContext> &pctx)
    {
        if (!writer_)
        {
            return;
        }

        writer_->Write(pctx);
    }

    void Logger::write_channels(const std::shared_ptr<LogContext> pctx)
    {
        for (std::pair<const std::string, std::shared_ptr<LogChannel>> &it :
             channels_)
        {
            it.second->Write(*this, pctx);
        }
    }

    LogContext::LogContext(LogLevel level,
                           const char *file,
                           pid_t tid,
                           const char *function,
                           int line)
    {
        this->level = level;
        this->file = file;
        this->tid = tid;
        this->function = function;
        this->line = line;
        gettimeofday(&tv, nullptr);
    }

    LogWriter::LogWriter(Logger &logger) : logger_(logger) {}

    void LogWriter::write_to_channels(const std::shared_ptr<LogContext> &pctx)
    {
        logger_.write_channels(pctx);
    }

    AsyncLogWriter::AsyncLogWriter(Logger &logger) : LogWriter(logger)
    {
        running_ = true;
        thread_ = new std::thread(std::bind(&AsyncLogWriter::run, this));
    }

    void AsyncLogWriter::Write(const std::shared_ptr<LogContext> &pctx)
    {
        {
            std::lock_guard<std::mutex> lock(mux_);
            pctxs_.emplace_back(pctx);
        }
        sem_.Post();
    }

    void AsyncLogWriter::run()
    {
        set_thread_name("logger");
        while (running_)
        {
            sem_.Wait();
            flush_all();
        }
    }

    void AsyncLogWriter::flush_all()
    {
        std::list<std::shared_ptr<LogContext>> tmp_pctxs;
        {
            std::lock_guard<std::mutex> lock(mux_);
            tmp_pctxs.swap(pctxs_);
        }

        for (std::shared_ptr<LogContext> pctx : tmp_pctxs)
        {
            write_to_channels(pctx);
        }
    }

    AsyncLogWriter::~AsyncLogWriter()
    {
        running_ = false;
        sem_.Post();
        thread_->join();
        delete thread_;
        flush_all();
    }

    LogChannel::LogChannel(const std::string &name, LogLevel level)
    {
        name_ = name;
        level_ = level;
    }

    const std::string &LogChannel::Name() const
    {
        return name_;
    }

    void LogChannel::SetLevel(LogLevel level)
    {
        level_ = level;
    }

    void LogChannel::format(const Logger &logger,
                            std::ostream &ost,
                            const std::shared_ptr<LogContext> &pctx,
                            bool enable_color,
                            bool enable_detail)
    {
        if (!enable_detail && pctx->str().empty())
        {
            return;
        }

        if (enable_color)
        {
            ost << s_log_const_table[pctx->level][1];
        }

        ost << print_time(pctx->tv) << " " << s_log_const_table[pctx->level][2]
            << " ";

        if (enable_detail)
        {
            ost << logger.GetLoggerName() << "[" << getpid() << "]"
                << "[" << pctx->tid << "]" << pctx->file << ":" << pctx->line << " "
                << pctx->function << " | ";
        }

        ost << pctx->str();

        if (enable_color)
        {
            ost << CLEAR_COLOR;
        }

        ost << std::endl;
    }

    ConsoleChannel::ConsoleChannel(const std::string &name, LogLevel level)
        : LogChannel(name, level)
    {
    }

    void ConsoleChannel::Write(const Logger &logger,
                               const std::shared_ptr<LogContext> &pctx)
    {
        if (level_ > pctx->level)
        {
            return;
        }

        format(logger, std::cout, pctx, true, true);
    }

    LogContextCapturer::LogContextCapturer(Logger &logger,
                                           LogLevel level,
                                           const char *file,
                                           const char *function,
                                           int line)
        : logger_(logger)
    {
        pctx_ = std::make_shared<LogContext>(level, file, gettid(), function, line);
    }

    LogContextCapturer::LogContextCapturer(const LogContextCapturer &that)
        : logger_(that.logger_)
    {
        pctx_ = that.pctx_;
        const_cast<std::shared_ptr<LogContext> &>(that.pctx_).reset();
    }

    LogContextCapturer::~LogContextCapturer()
    {
        *this << std::endl;
    }

    LogContextCapturer &
    LogContextCapturer::operator<<(std::ostream &(*f)(std::ostream &))
    {
        if (!pctx_)
        {
            return *this;
        }
        logger_.Write(pctx_);
        pctx_.reset();
        return *this;
    }

    void LogContextCapturer::Clear()
    {
        pctx_.reset();
    }

} // namespace sms
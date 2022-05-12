// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
#pragma once
#include "util.h"
#include "util_locale.h"
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <functional>

#if (!defined(ALBC_HAVE_THREADS) || defined(ALBC_CONFIG_ASAN)) && !defined(ALBC_DISABLE_THREADED_LOGGING)
#define ALBC_DISABLE_THREADED_LOGGING // AddressSanitizer causes segfault on concurrent queue
#endif

#ifndef ALBC_ENABLE_THREADED_LOGGING
#define ALBC_DISABLE_THREADED_LOGGING
#endif

#ifndef ALBC_DISABLE_THREADED_LOGGING
#include <condition_variable>
#include "blockingconcurrentqueue.h"
#endif

#define ALBC_LOG_DO_S_LOG(id, target, ...)                                                                                  \
    do                                                                                                                      \
    {                                                                                                                       \
        albc::util::VariantPutLn(target, __VA_ARGS__);                                                                      \
    } while (false)

#define S_LOG(target, ...) ALBC_LOG_DO_S_LOG(__COUNTER__, target, __VA_ARGS__)

#define LOG_TRACED(level, ...) S_LOG(albc::util::Logger()(level), __FILENAME__, ":", __func__, ":", STRINGIFY(__LINE__), "|", __VA_ARGS__)
#define LOG_D(...) LOG_TRACED(albc::util::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_I(...) LOG_TRACED(albc::util::LogLevel::INFO, __VA_ARGS__)
#define LOG_W(...) LOG_TRACED(albc::util::LogLevel::WARN, __VA_ARGS__)
#define LOG_E(...) LOG_TRACED(albc::util::LogLevel::ERROR, __VA_ARGS__)
#define LOG_TRACED_DETAIL(level, ...) S_LOG(albc::util::Logger()(level), __FILENAME__, ":", __PRETTY_FUNCTION__, ":", STRINGIFY(__LINE__), "|", __VA_ARGS__)
#define LOG_D_DETAIL(...) LOG_TRACED_DETAIL(albc::util::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_I_DETAIL(...) LOG_TRACED_DETAIL(albc::util::LogLevel::INFO, __VA_ARGS__)
#define LOG_W_DETAIL(...) LOG_TRACED_DETAIL(albc::util::LogLevel::WARN, __VA_ARGS__)
#define LOG_E_DETAIL(...) LOG_TRACED_DETAIL(albc::util::LogLevel::ERROR, __VA_ARGS__)

namespace albc::util
{

template <typename Out, typename... Args>
auto VariantPut(Out &os, Args &&...args) -> decltype((os << ... << args))
{
    return (os << ... << args);
}

template <typename Out, typename... Args>
void VariantPutLn(Out &os, Args &&...args)
{
    (os << ... << args) << std::endl;
}

enum class LogLevel
{
    ALL = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    NONE = 5
};

class GlobalLogConfig
{
public:
    using LogCallback = bool (*)(unsigned long logger_id, const char *msg, void *user_data);
    using FlushLogCallback = bool (*)(unsigned long logger_id, void *user_data);

    struct LogHandler
    {
    public:
        LogHandler() = default;

        LogHandler(LogCallback cb, void *data)
            : cb(cb), data(data)
        {
        }

        bool operator()(unsigned long logger_id, const char *msg) const
        {
            return cb != nullptr && cb(logger_id, msg, data);
        }

    private:
        LogCallback cb;
        void *data;
    };

    struct FlushLogHandler
    {
    public:
        FlushLogHandler() = default;

        FlushLogHandler(FlushLogCallback cb, void *data)
            : cb(cb), data(data)
        {
        }

        bool operator()(unsigned long logger_id) const
        {
            return cb != nullptr && cb(logger_id, data);
        }

    private:
        FlushLogCallback cb;
        void *data;
    };

    static LogLevel GetLogLevel()
    {
        return log_level_;
    }

    static void SetLogLevel(LogLevel level)
    {
        std::lock_guard lock(mutex_);
        log_level_ = level;
    }

    static bool CanLog(LogLevel level)
    {
        return log_level_ <= level;
    }

    static void SetLogCallback(LogCallback callback, void *data)
    {
        std::lock_guard lock(mutex_);
        log_handler_ = LogHandler(callback, data);
    }

    static const LogHandler &GetLogCallback()
    {
        return log_handler_;
    }

    static void SetFlushLogCallback(FlushLogCallback callback, void *data)
    {
        std::lock_guard lock(mutex_);
        flush_log_handler_ = FlushLogHandler(callback, data);
    }

    static const FlushLogHandler &GetFlushLogCallback()
    {
        return flush_log_handler_;
    }

private:
    inline static LogLevel log_level_;
    inline static std::mutex mutex_;
    inline static LogHandler log_handler_;
    inline static FlushLogHandler flush_log_handler_;
};

struct DefaultLogPrinter
{
    static void Print(unsigned long logger_id, const std::string &content)
    {
        if (GlobalLogConfig::GetLogCallback()(logger_id, content.c_str()))
            return;

        std::cout << content;
        if (content.back() != '\n')
        {
            std::cout << '\n';
        }
    }

    static void Flush(unsigned long logger_id)
    {
        if (GlobalLogConfig::GetFlushLogCallback()(logger_id))
            return;

        std::cout.flush();
    }
};

template <typename TPrinter = DefaultLogPrinter> class ThreadedConsoleLogger
{
public:
    ~ThreadedConsoleLogger()
    {
        Flush();
#ifndef ALBC_DISABLE_THREADED_LOGGING
        running_ = false;
        thread_.join();
#endif
    }

    void Log(const LogLevel level, std::string &&str)
    {
        if (GlobalLogConfig::CanLog(level))
        {
#ifndef ALBC_DISABLE_THREADED_LOGGING
            if (!queue_.enqueue(std::move(str)))
            {
                std::cerr << "Failed to enqueue log message!" << std::endl;
                return;
            }
#else
            // log directly
            Print(str);
#endif
        }
    }

    void Log(const LogLevel level, const std::string &str)
    {
        Log(level, std::string(str));
    }

    void Flush()
    {
#ifndef ALBC_DISABLE_THREADED_LOGGING
        std::string str;
        while (queue_.try_dequeue(str))
        {
            Print(str);
        }
#endif
        TPrinter::Flush(kDefaultLoggerId);
    }

private:
    static constexpr unsigned long kDefaultLoggerId = 0;
#ifndef ALBC_DISABLE_THREADED_LOGGING
    moodycamel::BlockingConcurrentQueue<std::string> queue_{4096};
    std::thread thread_{&ThreadedConsoleLogger::MainLoop, this};
#endif
    bool running_ = true;

#ifndef ALBC_DISABLE_THREADED_LOGGING
    void MainLoop()
    {
        while (true)
        {
            constexpr size_t bulk_reserve_size = 256;
            std::string msg_list[bulk_reserve_size];
            const size_t bulk_size =
                queue_.wait_dequeue_bulk_timed(msg_list, bulk_reserve_size, std::chrono::milliseconds(10));

            for (size_t i = 0; i < bulk_size; ++i)
            {
                const auto &msg = msg_list[i];
                Print(msg);
            }

            if (!running_)
            {
                TPrinter::Flush(kDefaultLoggerId);
                break;
            }
        }
    }
#endif

    static void Print(const std::string &str)
    {
        TPrinter::Print(kDefaultLoggerId, ToTargetLocale(str));
    }
};

using SingletonLogger = LazySingleton<ThreadedConsoleLogger<DefaultLogPrinter>>;

class Logger
{
public:
    typedef std::ostream &(*ManipFn)(std::ostream &);
    typedef std::ios_base &(*FlagsFn)(std::ios_base &);

    Logger() = default;

    template <class T> // int, double, strings, etc
    [[nodiscard]] Logger &operator<<(const T &output)
    {
        m_stream << output;
        return *this;
    }

    Logger &operator<<(ManipFn manip) /// endl, Flush, setw, setfill, etc.
    {
        manip(m_stream);

        if (manip == static_cast<ManipFn>(std::flush) || manip == static_cast<ManipFn>(std::endl))
            this->Flush();

        return *this;
    }

    Logger &operator<<(FlagsFn manip) /// setiosflags, resetiosflags
    {
        manip(m_stream);
        return *this;
    }

    Logger &operator()(LogLevel level)
    {
        m_logLevel = level;
        return *this;
    }

    void Flush()
    {
        /*
          m_stream.str() has your full message here.
          Good place to prepend time, log-level.
          Send to console, file, socket, or whatever you like here.
        */
        if (GlobalLogConfig::CanLog(m_logLevel))
        {
            SingletonLogger::instance()->Log(m_logLevel, std::move(std::string("ALBC|")
                                                                   .append(GetReadableTime())
                                                                   .append("|")
                                                                   .append(get_current_thread_id())
                                                                   .append(GetLogLevelTag(m_logLevel))
                                                                   .append(m_stream.str())));
        }

        m_stream.str(std::string());
        m_stream.clear();
    }

private:
    std::stringstream m_stream;
    LogLevel m_logLevel = LogLevel::INFO;

    // get log level as std::string
    static constexpr std::string_view GetLogLevelTag(const LogLevel e)
    {
        switch (e)
        {
        case LogLevel::ALL:
            return "|ALL  |";
        case LogLevel::DEBUG:
            return "|DEBUG|";
        case LogLevel::INFO:
            return "|INFO |";
        case LogLevel::WARN:
            return "|WARN |";
        case LogLevel::ERROR:
            return "|ERROR|";
        case LogLevel::NONE:
            return "|NONE |";
        }
        ALBC_UNREACHABLE();
    }
};
} // namespace albc::diagnostics

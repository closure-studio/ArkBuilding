// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
#pragma once
#include "util.h"
#include "locale_util.h"
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <functional>

#if (!defined(ALBC_HAVE_THREADS) || defined(ALBC_CONFIG_ASAN)) && !defined(ALBC_DISABLE_THREADED_LOGGING)
#define ALBC_DISABLE_THREADED_LOGGING // AddressSanitizer causes segfault on concurrent queue
#endif

#ifndef ALBC_DISABLE_THREADED_LOGGING
#include <condition_variable>
#include "blockingconcurrentqueue.h"
#endif

#define ALBC_LOG_UNIQUE2(name, id) $##name##$##id
#define ALBC_LOG_UNIQUE(name, id) ALBC_LOG_UNIQUE2(name, id)

#define ALBC_LOG_CONSTEXPR_STRING_VIEW(n, s) static constexpr std::string_view n = s;

#define ALBC_LOG_COMPILE_TIME_BUILD_TAG(name, id)                                                                      \
    ALBC_LOG_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(filename, id), __FILENAME__)                                        \
    ALBC_LOG_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(funcname, id), __func__)                                        \
    ALBC_LOG_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(line, id), STRINGIFY(__LINE__))                                     \
    static constexpr std::string_view ALBC_LOG_UNIQUE(name, id) =                                                      \
        albc::util::join_v<ALBC_LOG_UNIQUE(filename, id), albc::util::kColonSep, ALBC_LOG_UNIQUE(funcname, id), \
                           albc::util::kColonSep, ALBC_LOG_UNIQUE(line, id), albc::util::kBarSep>;

#define ALBC_LOG_DO_S_LOG(id, target, ...)                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        ALBC_LOG_COMPILE_TIME_BUILD_TAG(loc, id)                                                                       \
        albc::util::VariantPut(target, ALBC_LOG_UNIQUE(loc, id), __VA_ARGS__) << std::endl;                     \
    } while (false)

#define S_LOG(target, ...) ALBC_LOG_DO_S_LOG(__COUNTER__, target, __VA_ARGS__)

#define LOG(level, ...) S_LOG(albc::util::Logger()(level), __VA_ARGS__)
#define LOG_D(...) LOG(albc::util::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_I(...) LOG(albc::util::LogLevel::INFO, __VA_ARGS__)
#define LOG_W(...) LOG(albc::util::LogLevel::WARN, __VA_ARGS__)
#define LOG_E(...) LOG(albc::util::LogLevel::ERROR, __VA_ARGS__)

namespace albc::util
{
static constexpr std::string_view kBarSep = "|";
static constexpr std::string_view kColonSep = ":";

template <typename Out, typename... Args> auto VariantPut(Out &os, Args &&...args) -> decltype((os << ... << args))
{
    return (os << ... << args);
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
    using LogCallback = void (*)(const char *);
    using FlushLogCallback = void (*)();

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

    static void SetLogCallback(LogCallback callback)
    {
        std::lock_guard lock(mutex_);
        log_callback_ = callback;
    }

    static LogCallback GetLogCallback()
    {
        return log_callback_;
    }

    static void SetFlushLogCallback(FlushLogCallback callback)
    {
        std::lock_guard lock(mutex_);
        flush_log_callback_ = callback;
    }

    static FlushLogCallback GetFlushLogCallback()
    {
        return flush_log_callback_;
    }

  private:
    inline static LogLevel log_level_;
    inline static std::mutex mutex_;
    inline static LogCallback log_callback_;
    inline static FlushLogCallback flush_log_callback_;
};

struct DefaultLogPrinter
{
    static void Print(std::string content)
    {
        auto global_log_cb = GlobalLogConfig::GetLogCallback();
        if (global_log_cb)
        {
            global_log_cb(content.c_str());
            return;
        }

        std::cout << content;
        if (content.back() != '\n')
        {
            std::cout << '\n';
        }
    }

    static void Flush()
    {
        auto global_flush_log_cb = GlobalLogConfig::GetFlushLogCallback();
        if (global_flush_log_cb)
        {
            global_flush_log_cb();
            return;
        }

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
        TPrinter::Flush();
    }

  private:
#ifndef ALBC_DISABLE_THREADED_LOGGING
    moodycamel::BlockingConcurrentQueue<std::string> queue_ { 4096 };
    std::thread thread_ { std::bind(&ThreadedConsoleLogger::MainLoop, this) };
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
                TPrinter::Flush();
                break;
            }
        }
    }
#endif

    static void Print(const std::string &str)
    {
        TPrinter::Print(ToTargetLocale(str));
    }
};

using SingletonLogger = util::LazySingleton<ThreadedConsoleLogger<DefaultLogPrinter>>;

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
                                                                       .append(util::GetReadableTime())
                                                                       .append("|")
                                                                       .append(util::get_current_thread_id())
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
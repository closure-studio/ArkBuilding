// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
#pragma once
#include "ConcurrentQueue/blockingconcurrentqueue.h"
#include "util.h"
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>

#define ALBC_LOG_UNIQUE2(name, id) $##name##$##id
#define ALBC_LOG_UNIQUE(name, id) ALBC_LOG_UNIQUE2(name, id)

#define ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(n, s) static constexpr std::string_view n = s;

#define ALBC_LOG_COMPILE_TIME_BUILD_TAG(name, id)                                                                      \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(filename, id), __FILENAME__)                    \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(funcname, id), __FUNCTION__)             \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(line, id), STRINGIFY(__LINE__))                 \
    static constexpr std::string_view ALBC_LOG_UNIQUE(name, id) =                                                      \
        albc::util::join_v<ALBC_LOG_UNIQUE(filename, id), albc::diagnostics::kColonSep, ALBC_LOG_UNIQUE(funcname, id), \
                           albc::diagnostics::kColonSep, ALBC_LOG_UNIQUE(line, id), albc::diagnostics::kBarSep>;

#define ALBC_LOG_DO_S_LOG_PUT_TAG(target, content) target << content

#define ALBC_LOG_DO_S_LOG(id, target)                                                                                  \
    ALBC_LOG_COMPILE_TIME_BUILD_TAG(loc, id)                                                                           \
    ALBC_LOG_DO_S_LOG_PUT_TAG(target, ALBC_LOG_UNIQUE(loc, id))

#define S_LOG(target) ALBC_LOG_DO_S_LOG(__COUNTER__, target)

#define CREATE_LOGGER(id)                                                                                              \
    albc::diagnostics::Logger ALBC_LOG_UNIQUE(logr, id);                                                               \
    ALBC_LOG_UNIQUE(logr, id)

#define GET_LOGGER_WITH_LEVEL(level) CREATE_LOGGER(__COUNTER__)(level)
#define LOG_D S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::DEBUG))
#define LOG_I S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::INFO))
#define LOG_W S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::WARN))
#define LOG_E S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::ERROR))
#define LOG(level) S_LOG(GET_LOGGER_WITH_LEVEL(level))

namespace albc::diagnostics
{
static constexpr std::string_view kBarSep = "|";
static constexpr std::string_view kColonSep = ":";

enum class LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class GlobalLogConfig
{
  public:
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
        return GetLogLevel() <= level;
    }

  private:
    inline static LogLevel log_level_;
    inline static std::mutex mutex_;
};

class ThreadedConsoleLogger
{
  public:
    ThreadedConsoleLogger() : thread_([this]() { MainLoop(); }), queue_(4096)
    {
    }

    ~ThreadedConsoleLogger()
    {
        running_ = false;
        cv_.notify_one();
        thread_.join();
    }

    void Log(const LogLevel level, string &&str)
    {
        if (GlobalLogConfig::CanLog(level))
        {
#ifndef ALBC_CONFIG_ASAN // AddressSanitizer causes segfault on concurrent queue
            if (!queue_.enqueue(std::move(str)))
            {
                std::cerr << "Failed to enqueue log message!" << std::endl;
                return;
            }
            {
                std::unique_lock lock(cv_mutex_);
                has_new_ = true;
            }
            cv_.notify_one();
#else
            // log directly
            std::cout << str;
            if (str.back() != '\n')
            {
                std::cout << '\n';
            }
#endif
        }
    }
    
    void Log(const LogLevel level, const string &str)
    {
        Log(level, string(str));
    }

  private:
    std::condition_variable cv_;
    std::mutex cv_mutex_;
    std::thread thread_;
    bool running_ = true;
    bool has_new_ = false;
    moodycamel::ConcurrentQueue<string> queue_;

    void MainLoop()
    {
#ifdef ALBC_CONFIG_ASAN
        std::cout << "Warning: AddressSanitizer detected, threaded logging disabled." << std::endl;
        return;
#endif
        
        while (true)
        {
            constexpr size_t bulk_reserve_size = 1024;

            std::unique_lock lock(cv_mutex_);
            cv_.wait(lock, [this] { return has_new_ || !running_; });
            has_new_ = false;

            string msg_list[bulk_reserve_size];
            const size_t bulk_size = queue_.try_dequeue_bulk(msg_list, bulk_reserve_size);

            for (int i = 0; i < bulk_size; ++i)
            {
                const auto &msg = msg_list[i];
                std::cout << msg;
                // auto add new line
                if (const auto &back = msg.back(); back != '\n')
                {
                    std::cout << '\n';
                }
            }

            if (!running_)
            {
                std::cout << std::flush;
                break;
            }
        }
    }
};

class Logger
{
  public:
    typedef std::ostream &(*ManipFn)(std::ostream &);
    typedef std::ios_base &(*FlagsFn)(std::ios_base &);

    Logger() : m_logLevel(LogLevel::INFO)
    {
    }

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
            LazySingleton<ThreadedConsoleLogger>::instance()->Log(
                m_logLevel, std::move(GetReadableTime()
                    .append("|")
                    .append(get_current_thread_id())
                    .append(GetLogLevelTag(m_logLevel))
                    .append(m_stream.str())));
        }

        m_stream.str(std::string());
        m_stream.clear();
    }

    virtual ~Logger() {
        Flush();
    }

  private:
    std::stringstream m_stream;
    LogLevel m_logLevel;

    // get log level as string
    static constexpr string_view GetLogLevelTag(const LogLevel e)
    {
        switch (e)
        {
        case LogLevel::INFO:
            return "|INFO |";
        case LogLevel::WARN:
            return "|WARN |";
        case LogLevel::ERROR:
            return "|ERROR|";
        case LogLevel::DEBUG:
            return "|DEBUG|";
        }
        UNREACHABLE();
    }
};

using std::cout;
using std::endl;
using std::flush;
using std::resetiosflags;
using std::setfill;
using std::setiosflags;
using std::setw;
} // namespace albc::diagnostics

using namespace albc::diagnostics;
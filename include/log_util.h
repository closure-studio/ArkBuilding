// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
#pragma once
#include "time_util.h"
#include "util.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <mutex>
#include <queue>

#define ALBC_LOG_UNIQUE2(name, id) $##name##$##id
#define ALBC_LOG_UNIQUE(name, id) ALBC_LOG_UNIQUE2(name, id)

#define ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(n, s) static constexpr std::string_view n = s;

#define ALBC_LOG_COMPILE_TIME_BUILD_TAG(name, id)                                                                      \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(filename, id), __FILENAME__)                    \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(ALBC_LOG_UNIQUE(funcname, id), __PRETTY_FUNCTION__)             \
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
#define LOG_I S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::INFO))
#define LOG_W S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::WARN))
#define LOG_D S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::DEBUG))
#define LOG_E S_LOG(GET_LOGGER_WITH_LEVEL(albc::diagnostics::LogLevel::ERROR))

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
        std::lock_guard<std::mutex> lock(mutex_);
        return log_level_;
    }

    static void SetLogLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(mutex_);
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

    Logger &operator<<(ManipFn manip) /// endl, flush, setw, setfill, etc.
    {
        manip(m_stream);

        if (manip == static_cast<ManipFn>(std::flush) || manip == static_cast<ManipFn>(std::endl))
            this->flush();

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

    void flush()
    {
        /*
          m_stream.str() has your full message here.
          Good place to prepend time, log-level.
          Send to console, file, socket, or whatever you like here.
        */
        // put the message to std::cerr if log level is error
        if (GlobalLogConfig::CanLog(m_logLevel))
        {
            (m_logLevel == LogLevel::ERROR ? std::cerr : std::cout) << GetReadableTime()
                                                                       .append("|")
                                                                       .append(GetLogLevelString(m_logLevel))
                                                                       .append("|")
                                                                       .append(m_stream.str());

            m_logLevel = LogLevel::INFO;
        }

        m_stream.str(std::string());
        m_stream.clear();
    }

  private:
    std::stringstream m_stream;
    LogLevel m_logLevel;

    // get log level as string
    static string GetLogLevelString(LogLevel e)
    {
        switch (e)
        {
        case LogLevel::INFO:
            return "INFO ";
        case LogLevel::WARN:
            return "WARN ";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::DEBUG:
            return "DEBUG";
        default:
            return "UNKNOWN";
        }
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
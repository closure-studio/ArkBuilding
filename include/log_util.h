// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
#pragma once
#include "time_util.h"
#include "util.h"
#include <iomanip>
#include <iostream>
#include <sstream>

#define ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(n, s) static constexpr std::string_view n = s;

#define ALBC_LOG_COMPILE_TIME_DEFINE_LOC_STRING_VIEW(id)                                                               \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW($filename$##id, __FILENAME__)                                   \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW($funcname$##id, __PRETTY_FUNCTION__)                            \
    ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW($line$##id, STRINGIFY(__LINE__))

#define ALBC_LOG_COMPILE_TIME_BUILD_LOC_STRING(id)                                                                     \
    ALBC_LOG_COMPILE_TIME_DEFINE_LOC_STRING_VIEW(id)                                                                   \
    static constexpr auto $loc$##id =                                                                                  \
        albc::util::join_v<$filename$##id, albc::diagnostics::kColonSep, $funcname$##id, albc::diagnostics::kColonSep, \
                           $line$##id, albc::diagnostics::kBarSep>;

#define ALBC_LOG_PUT_STREAM(target, var, id) target << var##id

#define ALBC_LOG_DO_STREAMED_LOG(id, target)                                                                           \
    ALBC_LOG_COMPILE_TIME_BUILD_LOC_STRING(id)                                                                         \
    ALBC_LOG_PUT_STREAM(target, $loc$, id)

#define GET_LOGGER_WITH_LEVEL(level) albc::diagnostics::Logger::instance()->operator()(level)
#define LOG_I ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::INFO))
#define LOG_W ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::WARN))
#define LOG_D ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::DEBUG))
#define LOG_E ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::ERR))

namespace albc::diagnostics
{
static constexpr std::string_view kBarSep = "|";
static constexpr std::string_view kColonSep = ":";

class Logger : public LazySingleton<Logger>
{
  public:
    typedef std::ostream &(*ManipFn)(std::ostream &);
    typedef std::ios_base &(*FlagsFn)(std::ios_base &);

    enum LogLevel
    {
        DEBUG,
        INFO,
        WARN,
        ERR,
    };

    Logger() : m_logLevel(INFO)
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

    Logger &operator()(int e)
    {
        m_logLevel = e;
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
        (m_logLevel == ERR ? std::cerr : std::cout)
            << GetReadableTime().append("|").append(GetLogLevelString(m_logLevel)).append("|").append(m_stream.str());

        m_logLevel = INFO;

        m_stream.str(std::string());
        m_stream.clear();
    }

  private:
    std::stringstream m_stream;
    int m_logLevel;

    // get log level as string
    static string GetLogLevelString(int e)
    {
        switch (e)
        {
        case INFO:
            return "INFO ";
        case WARN:
            return "WARN ";
        case ERR:
            return "ERROR";
        case DEBUG:
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
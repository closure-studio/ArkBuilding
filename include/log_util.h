// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
#pragma once
#include <sstream>
#include <iostream>
#include <iomanip>
#include "time_util.h"
#include "util.h"

#define _ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW(n, s) \
    static constexpr std::string_view n = s;

#define _ALBC_LOG_COMPILE_TIME_DEFINE_LOC_STRING_VIEW(id) \
    _ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW($filename$##id, __FILENAME__) \
    _ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW($funcname$##id, __PRETTY_FUNCTION__) \
    _ALBC_LOG_COMPILE_TIME_DEFINE_CONSTEXPR_STRING_VIEW($line$##id, STRINGIFY(__LINE__))

#define _ALBC_LOG_COMPILE_TIME_BUILD_LOC_STRING(id) \
    _ALBC_LOG_COMPILE_TIME_DEFINE_LOC_STRING_VIEW(id) \
    static constexpr auto $loc$##id =          \
    albc::util::join_v<$filename$##id,               \
    albc::diagnostics::colon_sep, $funcname$##id, \
    albc::diagnostics::colon_sep, $line$##id, albc::diagnostics::bar_sep>;

#define  _ALBC_LOG_PUT_STREAM(target, var, id) \
    target << var##id

#define  _ALBC_LOG_DO_STREAMED_LOG(id, target) \
    _ALBC_LOG_COMPILE_TIME_BUILD_LOC_STRING(id) \
    _ALBC_LOG_PUT_STREAM(target, $loc$, id)

#define GET_LOGGER_WITH_LEVEL(level) albc::diagnostics::Logger::GetInstance()(level)
#define LOG_I _ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::INFO))
#define LOG_W _ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::WARN))
#define LOG_D _ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::DEBUG))
#define LOG_E _ALBC_LOG_DO_STREAMED_LOG(__COUNTER__, GET_LOGGER_WITH_LEVEL(albc::diagnostics::Logger::ERR))

namespace albc::diagnostics
{
    static constexpr std::string_view bar_sep = "|";
    static constexpr std::string_view colon_sep = ":";
    //static constexpr std::string_view func_bracket = "()";

    template<typename TLogger>
    class LazySingleton
    {
    public:
        static TLogger& GetInstance()
        {
            static TLogger instance;
            return instance;
        }
    };

    class Logger : public LazySingleton<Logger>
    {
    public:
        typedef std::ostream&  (*ManipFn)(std::ostream&);
        typedef std::ios_base& (*FlagsFn)(std::ios_base&);

        enum LogLevel
        {
            DEBUG,
            INFO,
            WARN,
            ERR,
        };

        Logger() : m_logLevel(INFO) {}

        template<class T>  // int, double, strings, etc
        [[nodiscard]] Logger& operator<<(const T& output)
        {
            m_stream << output;
            return *this;
        }

        Logger& operator<<(ManipFn manip) /// endl, flush, setw, setfill, etc.
        {
            manip(m_stream);

            if (manip == static_cast<ManipFn>(std::flush)
                || manip == static_cast<ManipFn>(std::endl ) )
                this->flush();

            return *this;
        }

        Logger& operator<<(FlagsFn manip) /// setiosflags, resetiosflags
        {
            manip(m_stream);
            return *this;
        }

        Logger& operator()(int e)
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
            (m_logLevel == ERR
                ? std::cerr
                : std::cout) << GetReadableTime()
                    .append("|")
                    .append(GetLogLevelString(m_logLevel))
                    .append("|")
                    .append(m_stream.str());

            m_logLevel = INFO;

            m_stream.str( std::string() );
            m_stream.clear();
        }

    private:
        std::stringstream  m_stream;
        int           m_logLevel;

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
    using std::setw;
    using std::setfill;
    using std::setiosflags;
    using std::resetiosflags;

}

using namespace albc::diagnostics;
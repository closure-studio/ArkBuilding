//
// Created by User on 2022-02-04.
//
#pragma once
#include <iostream>
#include <string>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <functional>
#include <type_traits>
#include "primitive_types.h"
#include "log_util.h"
#include "util.h"

// this marco is used to measure the execution time of a scope, and prints filename, function name, line number, and duration
#define SCOPE_TIMER_WITH_TRACE(name) albc::diagnostics::ScopeTimer(                 \
    string("ALBC|").append(albc::util::GetReadableTime()).append("|").append(albc::util::get_current_thread_id())     \
    .append("|TIMER|").append(__FILENAME__).append(":")                                        \
    .append(__FUNCTION__).append(STRINGIFY(:__LINE__|[ScopeTimer])).append((name)))

namespace albc::diagnostics
{
    using PerfClock = std::conditional<                // this type is used to measure high resolution time
        std::chrono::high_resolution_clock::is_steady, // check if std::chrono::high_resolution_clock is steady
        std::chrono::high_resolution_clock,            // if yes, use it
        std::chrono::steady_clock>::type;              // if not, use std::chrono::steady_clock

    using FloatingSeconds = std::chrono::duration<double>; // floating point seconds type

    // measures the time of a function call, does not require a lambda expression
    template <typename Func, typename... Args>
    FloatingSeconds MeasureTime(Func &&func, Args &&...args)
    {
        const auto t0 = PerfClock::now();
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
        return FloatingSeconds{PerfClock::now() - t0};
    }

    // measures the execution time of a scope, using RAII
    class [[nodiscard]] ScopeTimer
    {
    public:
        explicit ScopeTimer(string name, LogLevel log_level = LogLevel::INFO)
            : m_name(std::move(name)), // store name
              m_start(PerfClock::now()), // store start time
                log_level_(log_level)
        {
        }

        ~ScopeTimer()
        {
            // print name and duration
            double sec = FloatingSeconds(PerfClock::now() - m_start).count();
            SingletonLogger::instance()->Log(
                log_level_, std::move(m_name.append(": Done in ").append(std::to_string(sec)).append("s")));
        }

    private:
        string m_name;
        const PerfClock::time_point m_start;
        const LogLevel log_level_;
    };
}
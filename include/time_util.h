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
#include "./primitive_types.h"

// this marco is used to measure the execution time of a scope, and prints filename, function name, line number, and duration
#define SCOPE_TIMER_WITH_TRACE(name) albc::diagnostics::ScopeTimer(          \
    albc::diagnostics::GetReadableTime() + "|TIMER|" + string(__FILENAME__) + ":" +\
    string(__PRETTY_FUNCTION__) + STRINGIFY(:__LINE__:[ScopeTimer]) + name)

namespace albc::diagnostics
{
    using PerfClock = std::conditional<                // this type is used to measure high resolution time
        std::chrono::high_resolution_clock::is_steady, // check if std::chrono::high_resolution_clock is steady
        std::chrono::high_resolution_clock,            // if yes, use it
        std::chrono::steady_clock>::type;              // if not, use std::chrono::steady_clock

    using FloatingSeconds = std::chrono::duration<double>; // floating point seconds type

    // get ops_for_partial_comb time_t in MM-DD HH:MM:SS
    // uses chrono
    static string GetReadableTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&in_time_t);
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%m-%d.%H:%M:%S", &tm);
        return string{buffer};
    }

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
        explicit ScopeTimer(string name)
            : m_name(std::move(name)), // store name
              m_start(PerfClock::now()) // store start time
        {
        }

        ~ScopeTimer()
        {
            // print name and duration
            double sec = FloatingSeconds(PerfClock::now() - m_start).count();
            std::cout << m_name << ": Done in " << sec << "s" << std::endl;
        }

    private:
        const string m_name;
        const PerfClock::time_point m_start;
    };
}
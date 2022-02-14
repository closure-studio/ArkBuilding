#pragma once
#include "primitive_types.h"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>

// get filename, without path
#ifndef __FILENAME__
#define __FILENAME__ detail::strip_path(__FILE__) // NOLINT(bugprone-reserved-identifier)
#endif

// stringify a macro
#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 256
#include "magic_enum.hpp"

#ifndef MAGIC_ENUM_SUPPORTED
// raise error that contains info about ops_for_partial_comb compiler
static_assert(false, STRINGIFY(magic_enum is not supported by compiler.gcc\
                               : __GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__.clang\
                               : __clang_major__.__clang_minor__.__clang_patchlevel__.msvc\
                               : _MSC_VER.));
#endif

// this marco unrolls the loop for the given number of times, using compiler-specific unrolling
// place this macro before the loop you want to unroll

#ifdef __GNUC__ // in gcc, the marco is "#pragma GCC unroll(n)"
#define UNROLL_LOOP(n) _Pragma(STRINGIFY(GCC unroll(n)))

#elif defined(__clang__) // in clang, the marco is "#pragma clang loop unroll_count(n)"
#define UNROLL_LOOP(n) _Pragma(STRINGIFY(clang loop unroll_count(n)))

#else // if no compiler-specific unrolling is available, the marco is empty
#define UNROLL_LOOP(n)
#endif

namespace albc::util
{
    constexpr double kDoubleCmpTolerance = 1e-7; // default tolerance for double comparisons
    constexpr int kSignificantDigits = 7;        // default number of significant digits for double comparisons

    template <typename TVal> constexpr bool fp_eq(const TVal lhs, const TVal rhs)
    { // Floating point equality comparison, tolerance is 1e-7
        return std::fabs(lhs - rhs) < static_cast<TVal>(kDoubleCmpTolerance);
    }

    template <typename TVal, typename Tn = int> constexpr TVal fp_round_n(const TVal val, const Tn n)
    { // Floating point rounding to n significant digits
        const double mul = std::pow(10, n);
        return std::round(val * mul) / mul;
    }

    template <typename TVal> constexpr TVal fp_round_n(const TVal val)
    { // Floating point rounding to n significant digits, default n = 7
        return std::round(val * 1e7) * 1e-7;
    }

    // C(n, k)
    constexpr UInt64 n_choose_k(const UInt32 n, UInt32 k)
    {
        if (k > n)
            return 0;
        if (k * 2 > n)
            k = n - k;
        if (k == 0)
            return 1;

        UInt64 result = n;
        for (UInt32 i = 2; i <= k; ++i)
        {
            result *= (n - i + 1);
            result /= i;
        }
        return result;
    }

    // indicates whether the build is a debug build
    constexpr bool is_debug_build()
    {
#ifdef NDEBUG
        return false;
#else
        return true;
#endif
    }

    // get system architecture
    constexpr const char *system_architecture()
    {
#if defined(__x86_64__) || defined(_M_X64)
        return "x64";
#elif defined(__i386) || defined(_M_IX86)
        return "x86";
#elif defined(__arm__) || defined(_M_ARM)
        return "arm";
#elif defined(__aarch64__) || defined(_M_ARM64)
        return "arm64";
#else
        return "unknown";
#endif
    }

    // get system name
    constexpr const char *system_name()
    {
#if defined(__linux__)
        return "linux";
#elif defined(__APPLE__)
        return "macos";
#elif defined(_WIN32)
        return "windows";
#else
        return "unknown";
#endif
    }

    constexpr const char *file_name(const char *path)
    {
        const char *file = path;
        while (*path)
        {
            if (*path++ == '/' || *path == '\\')
            {
                file = path;
            }
        }
        return file;
    } // this function can be evaluated at compile time

    // convert a enum value to a string, using magic enum
    // use std::enable_if and std::is_enum_v<T> to check if T is an enum
    template <typename T>
    constexpr string_view to_string(const T value, typename std::enable_if<std::is_enum_v<T>>::type * = nullptr)
    {
        return magic_enum::enum_name<T>(value);
    }

    // extract the total number of elements in an enum
    template <typename T> class enum_size
    {
      public: // we manually added a entry in the enum class named "E_NUM", so we can use it here
              // "E_NUM" is the last element in the enum, so its value is the total number of elements
        static constexpr size_t value = static_cast<size_t>(T::E_NUM);
    };

    // join static strings at compile time
    template <std::string_view const &...Strs> struct join
    {
        // Join all strings into a single std::array of chars
        static constexpr auto impl() noexcept
        {
            constexpr std::size_t len = (Strs.size() + ... + 0);
            std::array<char, len + 1> arr{};
            auto append = [i = 0, &arr](auto const &s) mutable {
                for (auto c : s)
                    arr[i++] = c;
            };
            (append(Strs), ...);
            arr[len] = 0;
            return arr;
        }
        // Give the joined string static storage
        static constexpr auto arr = impl();
        // View as a std::string_view
        static constexpr std::string_view value{arr.data(), arr.size() - 1};
    };
    // Helper to get the value out
    template <std::string_view const &...Strs> static constexpr auto join_v = join<Strs...>::value;

    template <typename TGet, typename TStore = TGet> class LazySingleton
    {
      public:
        static std::shared_ptr<TGet> instance()
        {
            static std::shared_ptr<TStore> instance{new TStore()};
            return instance;
        }

        ~LazySingleton()
        {
            std::cout << "LazySingleton destructor called: " << __PRETTY_FUNCTION__ << std::endl;
        }
    };
} // namespace util

namespace detail
{
constexpr bool is_path_sep(char c)
{
    return c == '/' || c == '\\';
}

constexpr const char *strip_path(const char *path)
{
    auto lastname = path;
    for (auto p = path; *p; ++p)
    {
        if (is_path_sep(*p) && *(p + 1))
            lastname = p + 1;
    }
    return lastname;
}
} // namespace detail

using namespace albc::util;
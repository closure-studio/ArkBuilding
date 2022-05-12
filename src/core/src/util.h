#pragma once
#include "albc_types.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <cstdarg>
#include <sstream>
#include <thread>
#include <cstring>

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

// get filename, without path
#ifndef __FILENAME__
#define __FILENAME__ ::detail::strip_path(__FILE__) // NOLINT(bugprone-reserved-identifier)
#endif

#ifdef __GNUC__
#define UNROLL_LOOP(n) _Pragma(STRINGIFY(GCC unroll(n)))

#elif defined(__clang__)
#define UNROLL_LOOP(n) _Pragma(STRINGIFY(clang loop unroll_count(n)))

#elif defined(ALBC_CONFIG_MSVC)
#define UNROLL_LOOP(n) 

#else
#define UNROLL_LOOP(n)
#endif

#define ALBC_REQUIRES(...) typename std::enable_if_t<(__VA_ARGS__), bool> = true
#define ALBC_DELETE_COPY(class) \
    class(const class&) = delete; \
    class& operator=(const class&) = delete;

#define ALBC_DELETE_MOVE(class) \
    class(class&&) = delete; \
    class& operator=(class&&) = delete;

#define ALBC_DELETE_COPY_AND_MOVE(class) ALBC_DELETE_COPY(class) ALBC_DELETE_MOVE(class)

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
    constexpr UInt32 n_choose_k(const UInt32 n, UInt32 k)
    {
        if (k > n)
            return 0;
        if (k * 2 > n)
            k = n - k;
        if (k == 0)
            return 1;

        UInt32 result = n;
        for (UInt32 i = 2; i <= k; ++i)
        {
            result *= (n - i + 1);
            result /= i;
        }
        return result;
    }

    // convert an enum value to a string, using magic enum
    // use std::enable_if and std::is_enum_v<T> to check if T is an enum
    template <typename T>
    constexpr std::string_view enum_to_string(const T value, typename std::enable_if<std::is_enum_v<T>>::type * = nullptr)
    {
        return magic_enum::enum_name<T>(value);
    }

    // extract the total number of elements in an enum
    template <typename T> class enum_size
    {
      public:
        static constexpr std::size_t value = magic_enum::enum_count<T>();
    };

    template <typename T> constexpr std::size_t enum_size_v = enum_size<T>::value;

    template <typename TGet, typename TStore = TGet> class LazySingleton
    {
      public:
        inline static std::shared_ptr<TGet> instance()
        {
            static std::shared_ptr<TStore> instance{new TStore()};
            return instance;
        }

        ~LazySingleton()
        {
            std::cout << "LazySingleton destructor called: " << __PRETTY_FUNCTION__ << std::endl;
        }
    };

    template <typename TU> 
    static constexpr bool is_pow_of_two(TU n)
    {
        return n && (!(n & (n - 1))); // std::has_single_bit(n);
    }

    [[maybe_unused]] static int ctz(UInt32 x)
    {
#if defined(__GNUC__) || defined(__clang__)
        return __builtin_ctz(x);
#elif defined(_MSC_VER)
        unsigned long r = 0;
        _BitScanForward(&r, x);
        return r;
#elif defined(ffs)
        return ffs(x) - 1;
#else
        if (x == 0) return(32);
        int n = 1;
        if ((x & 0x0000FFFF) == 0) {n = n +16; x = x >>16;}
        if ((x & 0x000000FF) == 0) {n = n + 8; x = x >> 8;}
        if ((x & 0x0000000F) == 0) {n = n + 4; x = x >> 4;}
        if ((x & 0x00000003) == 0) {n = n + 2; x = x >> 2;}
        return n - (x & 1);
#endif
    }

    template <typename TU> static std::string to_bin_string(const TU x)
    {
        std::stringstream ss;
        ss << std::bitset<8 * sizeof(TU)>(x);
        return ss.str();
    }

    template <typename TEnum> static constexpr TEnum parse_enum_string(const std::string& str, TEnum default_value)
    {
        const auto val_or_null = magic_enum::enum_cast<TEnum>(str);
        return val_or_null.has_value() ? val_or_null.value() : default_value;
    }


    // get ops_for_partial_comb time_t in MM-DD HH:MM:SS
    // uses chrono
    [[maybe_unused]] static std::string GetReadableTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&in_time_t);
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%m-%d.%H:%M:%S", &tm);
        return std::string{buffer};
    }

    [[maybe_unused]] static int append_snprintf(char *&buffer, std::size_t &buffer_size, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int ret = std::vsnprintf(buffer, buffer_size, fmt, args);
        va_end(args);
        if (ret < 0)
        {
            std::cerr << "vsnprintf failed: " << std::strerror(errno) << " when trying to write: " << fmt << std::endl;
            return ret;
        }
        buffer += ret;
        buffer_size -= ret;
        return ret;
    }

    [[maybe_unused]]  static std::string get_current_thread_id()
    {
        char buf[20];
        std::sprintf(buf, "%08X", static_cast<UInt32>(std::hash<std::thread::id>{}(std::this_thread::get_id())));
        return std::string{buf};
    }

    template <typename TFunc>
    struct defer
    {
        explicit defer(TFunc &&func) : func(std::forward<TFunc>(func)) {}
        defer(const defer &) = delete;
        defer &operator=(const defer &) = delete;

        ~defer() { func(); }
        TFunc func;
    };

    template <typename TFunc>
    defer<TFunc> make_defer(TFunc &&func)
    {
        return defer<TFunc>(std::forward<TFunc>(func));
    }

    template <typename T>
    std::string TypeName()
    {
#ifdef ALBC_HAS_RTTI
#   if __has_include(<cxxabi.h>)
        int status = 114514;
        std::unique_ptr<char, decltype(&std::free)> result(
            abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status), std::free);

        return (status == 0) ? result.get() : typeid(T).name();
#   else
        return typeid(T).name();
#   endif
#else
        return "void";
#endif
    }

    template <typename TIter>
    std::string Join(const TIter &begin, const TIter &end, const std::string &sep)
    {
        std::string result;
        for (auto it = begin; it != end; ++it)
        {
            if (!result.empty())
                result.append(sep);
            result.append(*it);
        }
        return result;
    }

    template <typename TElem, typename TIter, typename TPred>
    void ReplaceFirstIf(TIter iter, const TIter& end, const TPred &pred, const TElem &new_value, bool err_on_failure = true)
    {
        (void)err_on_failure;
        auto it = std::find_if(iter, end, pred);
        if (it != end)
            *it = new_value;
        else
            assert(!err_on_failure && "Failed to replace first element");
    }

    template <typename TElem, typename TIter>
    void ReplaceFirst(TIter iter, const TIter & end, const TElem &old_value, const TElem &new_value, bool err_on_failure = true)
    {
        (void)err_on_failure;
        auto it = std::find(iter, end, old_value);
        if (it != end)
            *it = new_value;
        else
            assert(!err_on_failure && "Failed to replace first element");
    }
} // namespace albc::util

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
#define _CRT_SECURE_NO_WARNINGS

#if !defined(__GNUC__) && !defined(__clang__) && defined(_MSC_VER)
#define ALBC_CONFIG_MSVC
#define __attribute__(x) /* Do nothing */
#endif

#if defined(ALBC_CONFIG_MSVC) && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 256
#include "magic_enum.hpp"

#ifndef MAGIC_ENUM_SUPPORTED
#ifdef __GNUC__
#error "magic_enum requires GCC >= 9"
#elif defined(_MSC_VER)
#error "magic_enum requires MSVC >= 1910"
#elif defined(__clang__)
#error "magic_enum requires clang >= 5"
#else
#error "unsupported compiler"
#endif
#endif

// determine whether ASan is enabled, some of the code may not work when ASan is enabled
#if defined(__has_feature)
#   if __has_feature(address_sanitizer)
#       define ALBC_CONFIG_ASAN
#   endif
#endif

#ifdef __SANITIZE_ADDRESS__
#define ALBC_CONFIG_ASAN
#endif

#ifndef ALBC_CONFIG_MSVC
#define ALBC_HIDDEN __attribute__((visibility("hidden")))
#define ALBC_INTERNAL __attribute__((visibility("internal")))
#else
#define ALBC_HIDDEN
#define ALBC_INTERNAL
#endif
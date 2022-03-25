#define _CRT_SECURE_NO_WARNINGS

#if !defined(__GNUC__) && !defined(__clang__) && defined(_MSC_VER)
#   define ALBC_CONFIG_MSVC
#endif

#if defined(ALBC_CONFIG_MSVC) && !defined(__PRETTY_FUNCTION__)
#   define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 256
#include "magic_enum.hpp"

#ifndef MAGIC_ENUM_SUPPORTED
#   ifdef __GNUC__
#       error "magic_enum requires GCC >= 9"
#   elif defined(_MSC_VER)
#       error "magic_enum requires MSVC >= 1910"
#   elif defined(__clang__)
#       error "magic_enum requires clang >= 5"
#   else
#       error "unsupported compiler"
#   endif
#endif

// determine whether ASan is enabled, some of the code may not work when ASan is enabled
#if defined(__has_feature)
#   if __has_feature(address_sanitizer)
#       define ALBC_CONFIG_ASAN
#   endif
#endif

#ifdef __SANITIZE_ADDRESS__
#   define ALBC_CONFIG_ASAN
#endif

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#ifndef ALBC_CONFIG_MSVC
#   define ALBC_ATTRIBUTE(attr) __attribute__((attr))
#   define ALBC_HIDDEN __attribute__((visibility("hidden")))
#   define ALBC_INTERNAL __attribute__((visibility("internal")))
#   define ALBC_PUBLIC __attribute__((visibility("default")))
#   define ALBC_INLINE __attribute__((always_inline))
#   define ALBC_NO_INLINE __attribute__((noinline))
#   define ALBC_DEPRECATED __attribute__((deprecated))
#   define ALBC_NORETURN __attribute__((noreturn))
#   define ALBC_UNUSED __attribute__((unused))
#   define ALBC_FLATTEN __attribute__((flatten))
#   define ALBC_ALIGNED(n) __attribute__((aligned(n)))
#   define ALBC_PACKED __attribute__((packed))
#   define ALBC_UNREACHABLE() __builtin_unreachable()
#else
#   define __attribute__(x)
#   define ALBC_ATTRIBUTE(attr)
#   define ALBC_HIDDEN
#   define ALBC_INTERNAL
#   define ALBC_PUBLIC __declspec(dllexport)
#   define ALBC_INLINE __forceinline
#   define ALBC_NO_INLINE __declspec(noinline)
#   define ALBC_DEPRECATED __declspec(deprecated)
#   define ALBC_NORETURN __declspec(noreturn)
#   define ALBC_UNUSED
#   define ALBC_FLATTEN
#   define ALBC_ALIGNED(n) __declspec(align(n))
#   define ALBC_PACKED
#   define ALBC_UNREACHABLE() __assume(0)
#endif
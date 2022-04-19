#pragma once
#include "albc_common.h"

#define CALBC_HANDLE_DECL(name) \
    struct name;\
    typedef struct name name;

#ifndef CALBC_E_PTR
#   define CALBC_E_PTR AlbcException** e_ptr
#endif

#ifdef __GNUC__
#   define CALBC_MAYBE_UNUSED __attribute__((unused))
#else
#   define CALBC_MAYBE_UNUSED
#endif

#ifdef _WIN32
#   define CALBC_API(ret) CALBC_MAYBE_UNUSED ret __cdecl ALBC_EXPORT
#else
#   define CALBC_API(ret) CALBC_MAYBE_UNUSED ret
#endif
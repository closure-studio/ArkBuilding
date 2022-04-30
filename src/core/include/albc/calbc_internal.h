#pragma once
#ifndef CALBC_INTERNAL_H
#define CALBC_INTERNAL_H
#include "albc_common.h"

#define CALBC_HANDLE_DECL(name) \
    typedef struct name name;\

#ifndef CALBC_E_PTR
#   define CALBC_E_PTR AlbcException** e_ptr
#endif

#ifdef __GNUC__
#   define CALBC_MAYBE_UNUSED __attribute__((unused))
#else
#   define CALBC_MAYBE_UNUSED
#endif

#ifdef ALBC_EXPORTS
#   define CALBC_API CALBC_MAYBE_UNUSED ALBC_EXPORT
#else
#   define CALBC_API CALBC_MAYBE_UNUSED ALBC_IMPORT
#endif
#endif // CALBC_INTERNAL_H
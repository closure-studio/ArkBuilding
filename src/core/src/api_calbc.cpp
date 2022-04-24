#define ALBC_IS_INTERNAL
#include "albc/calbc.h"
#include "albc/albc.h"
#include "api_util.h"
#include "api_impl.h"

#undef CALBC_API
#define CALBC_API [[maybe_unused]] ALBC_PUBLIC

#include "json/json.h"
#include "api_util.h"
#include "util_mem.h"

#include <memory>

CALBC_HANDLE_IMPL(AlbcString, albc::String)
 
CALBC_API void AlbcTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, AlbcException**e_ptr)
{
    albc::RunTest(game_data_json, player_data_json, config, e_ptr);
}


CALBC_API void AlbcSetLogLevel(AlbcLogLevel level, AlbcException** e_ptr)
{
    albc::SetLogLevel(level, e_ptr);
}

 
CALBC_API void AlbcFlushLog(AlbcException** e_ptr)
{
    albc::FlushLog(e_ptr);
}

 
CALBC_API AlbcLogLevel AlbcParseLogLevel(const char *level, AlbcLogLevel default_level, AlbcException** e_ptr)
{
    return albc::ParseLogLevel(level, default_level, e_ptr);
}

 
CALBC_API AlbcTestMode AlbcParseTestMode(const char *mode, AlbcTestMode default_mode, AlbcException** e_ptr)
{
    return albc::ParseTestMode(mode, default_mode, e_ptr);
}

 
CALBC_API void AlbcFreeException(AlbcException *exception)
{
    albc::FreeException(exception);
}

 
CALBC_API const char *AlbcGetExceptionMessage(AlbcException *exception)
{
    return exception->what;
}
 
CALBC_API void AlbcStringDel(AlbcString *string)
{
    try
    {
        delete string;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}

 
CALBC_API void AlbcDoLog(AlbcLogLevel level, const char *msg, AlbcException **e_ptr)
{
    albc::DoLog(level, msg, e_ptr);
}

 
CALBC_API void AlbcSetLogHandler(AlbcLogHandler handler, AlbcException **e_ptr)
{
    albc::SetLogHandler(handler, e_ptr);
}

 
CALBC_API void AlbcSetFlushLogHandler(AlbcFlushLogHandler handler, AlbcException **e_ptr)
{
    albc::SetFlushLogHandler(handler, e_ptr);
}

 
CALBC_API size_t AlbcStringGetLength(AlbcString *string)
{
    return string->impl->size();
}

 
CALBC_API void AlbcStringCopyTo(AlbcString *string, char *buffer, size_t buffer_size)
{
    ::strncpy(buffer, string->impl->c_str(), buffer_size);
}

 
CALBC_API const char *AlbcStringGetContent(AlbcString *string)
{
    return string->impl->c_str();
}

CALBC_API void AlbcLoadGameDataJson(AlbcGameDataDbType type, const char *json, AlbcException **e_ptr)
{
    albc::LoadGameDataJson(type, json, e_ptr);
}

CALBC_API void AlbcLoadGameDataFile(AlbcGameDataDbType type, const char *path, AlbcException **e_ptr)
{
    albc::LoadGameDataFile(type, path, e_ptr);
}

CALBC_API AlbcString *AlbcRunWithJsonParams(const char *json, AlbcException **e_ptr)
{
    return new AlbcString(new albc::String(albc::RunWithJsonParams(json, e_ptr)));
}

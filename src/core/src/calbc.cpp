#include "albc/calbc.h"
#include "albc/albc.h"

#include "worker.h"
#include "json/json.h"
#include "api_utils.h"

#include <memory>

CALBC_HANDLE_IMPL(AlbcModel)
{
    std::unique_ptr<albc::AlbcModel> model;
};

CALBC_HANDLE_IMPL(AlbcWorkerParams)
{
    std::unique_ptr<albc::worker::WorkerParams> params;
};

CALBC_HANDLE_IMPL(AlbcWorker)
{

};

CALBC_HANDLE_IMPL(AlbcResult)
{

};

static std::unique_ptr<albc::bm::BuildingData> global_building_data;
[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, AlbcException**e_ptr)
{
    albc::RunTest(game_data_json, player_data_json, config, e_ptr);
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcSetGlobalBuildingData(const char *json, AlbcException**e_ptr)
{
    try
    {
        const auto& val = read_json_from_char_array(json);
        global_building_data = std::make_unique<albc::bm::BuildingData>(val);
        LOG_I("Successfully set global building data.");
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcSetLogLevel(AlbcLogLevel level, AlbcException** e_ptr)
{
    albc::SetLogLevel(level, e_ptr);
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcFlushLog(AlbcException** e_ptr)
{
    albc::FlushLog(e_ptr);
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(AlbcLogLevel) AlbcParseLogLevel(const char *level, AlbcLogLevel default_level, AlbcException** e_ptr)
{
    return albc::ParseLogLevel(level, default_level, e_ptr);
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(AlbcTestMode) AlbcParseTestMode(const char *mode, AlbcTestMode default_mode, AlbcException** e_ptr)
{
    return albc::ParseTestMode(mode, default_mode, e_ptr);
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcFreeException(AlbcException *exception)
{
    albc::FreeException(exception);
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(const) char *AlbcGetExceptionMessage(AlbcException *exception)
{
    return exception->what;
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcModelDestroy(AlbcModel *model, AlbcException **e_ptr)
{
    try
    {
        delete model;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcWorkerDestroy(AlbcWorker *worker, AlbcException **e_ptr)
{
    try
    {
        delete worker;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcWorkerParamsDestroy(AlbcWorkerParams *params, AlbcException **e_ptr)
{
    try
    {
        delete params;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

[[maybe_unused]]
ALBC_PUBLIC
CALBC_API(void) AlbcResultDestroy(AlbcResult *result, AlbcException **e_ptr)
{
    try
    {
        delete result;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

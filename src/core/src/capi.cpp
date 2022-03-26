#include "worker.h"
#include "json/json.h"
#include "albc/capi.h"

#include <memory>
#include "external/byte_array_buffer.h"

static std::unique_ptr<albc::bm::BuildingData> global_building_data;

static void ResetOutParamJson(AlbcRunResult *out, const string& json)
{
    delete[] out->json;

    out->json = new char[json.size() + 1];
    json.copy(out->json, json.length() + 1);
}

ALBC_PUBLIC
int AlbcRun(const AlbcRunParams *in, AlbcRunResult **out)
{
    try
    {
        if (!in || !in->json)
        {
            LOG_E << "Invalid AlbcRunParams" << std::endl;
            return -1;
        }

        std::unique_ptr<const AlbcRunParams> in_params(in);
        std::unique_ptr<const char[]> in_json(in->json);
        auto& out_ptr = *out;
        out_ptr = new AlbcRunResult;
        ResetOutParamJson(out_ptr, "{}");
        
        if (!global_building_data)
        {
            LOG_E << "No global building data" << std::endl;
            return -1;
        }

        const Json::Value val(in_json.get());
        albc::PlayerDataModel player_data(val);
        const albc::worker::WorkerParams params(player_data, *global_building_data);

        return 0;
    }
    catch (const std::exception& e)
    {
        LOG_E << "Error running building arrangement: " << e.what() << std::endl;
        return -1;
    }
}

ALBC_PUBLIC
void AlbcFree(const AlbcRunResult *out)
{
    if (out)
    {
        delete[] out->json;
    }

    delete out;
}

ALBC_PUBLIC
void AlbcTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config)
{
    try
    {
        const auto& player_data = read_json_from_char_array(player_data_json);
        const auto& game_data = read_json_from_char_array(game_data_json);
        
        auto log_level = static_cast<albc::diagnostics::LogLevel>(config->base_parameters.level);

        LOG_I << "Running test with log level: " << to_string(log_level) << std::endl;
        {
            const auto sc = SCOPE_TIMER_WITH_TRACE("albc::worker::work");
            albc::worker::launch_test(player_data, game_data, *config);
        }
        LOG_I << "Test completed" << std::endl;
    }
    catch (const std::exception &e)
    {
        LOG_E << "Exception: " << e.what() << std::endl;
    }
}

ALBC_PUBLIC
void AlbcSetGlobalBuildingData(const char *json)
{
    try
    {
        const auto& val = read_json_from_char_array(json);
        global_building_data = std::make_unique<albc::bm::BuildingData>(val);
        LOG_I << "Successfully set global building data." << std::endl;
    }
    catch (const std::exception& e)
    {
        LOG_E << "Cannot set global building data: " << e.what() << std::endl;
    }
}

ALBC_PUBLIC
void AlbcSetLogLevel(AlbcLogLevel level)
{
    try
    {
        albc::diagnostics::GlobalLogConfig::SetLogLevel(static_cast<albc::diagnostics::LogLevel>(level));
    }
    catch(const std::exception& e)
    {
        LOG_E << "Cannot set log level: " << e.what() << std::endl;
    }
    
}

ALBC_PUBLIC
void AlbcFlushLog()
{
    try
    {
        albc::diagnostics::SingletonLogger::instance()->Flush();
    }
    catch(const std::exception& e)
    {
        LOG_E << "Cannot flush log: " << e.what() << std::endl;
    }
}

ALBC_PUBLIC
AlbcLogLevel AlbcParseLogLevel(const char *level, AlbcLogLevel default_level)
{
    try
    {
        return static_cast<AlbcLogLevel>(albc::util::parse_enum_string(level, static_cast<albc::diagnostics::LogLevel>(default_level)));
    }
    catch(const std::exception& e)
    {
        LOG_E << "Cannot parse log level: " << e.what() << std::endl;
    }
    return default_level;
}
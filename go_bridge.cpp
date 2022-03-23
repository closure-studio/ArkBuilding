//
// Created by User on 2022-02-08.
//
#include "worker.h"
#include "go_bridge.h"

static std::unique_ptr<albc::bm::BuildingData> global_building_data;

static void ResetOutParamJson(OutParams *out, const string& json)
{
    delete[] out->json;

    out->json = new char[json.size() + 1];
    json.copy(out->json, json.length() + 1);
}

int AlbcRun(const InParams *in, OutParams **out)
{
    try
    {
        if (!in || !in->json)
        {
            LOG_E << "Invalid InParams" << std::endl;
            return -1;
        }

        std::unique_ptr<const InParams> in_params(in);
        std::unique_ptr<const char[]> in_json(in->json);
        auto& out_ptr = *out;
        out_ptr = new OutParams;
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

void AlbcFree(const OutParams *out)
{
    if (out)
    {
        delete[] out->json;
    }

    delete out;
}

void AlbcTest(const char *game_data_path, const char *player_data_path)
{
    try
    {
        const string &game_data = string(game_data_path);
        const string &player_data = string(player_data_path);

        LOG_I << "Running test, with GameData: " << game_data << ", PlayerData: " << player_data << std::endl;
        {
            const auto sc = SCOPE_TIMER_WITH_TRACE("albc::worker::work");
            albc::worker::run_test(player_data, game_data, albc::diagnostics::LogLevel::INFO);
        }
        LOG_I << "Test completed" << std::endl;
    }
    catch (const std::exception &e)
    {
        LOG_E << "Exception: " << e.what() << std::endl;
    }
}

void AlbcSetGlobalBuildingData(const char *json)
{
    try
    {
        const Json::Value val(json);
        global_building_data.reset(new albc::bm::BuildingData(val));
        LOG_I << "Successfully set global building data." << std::endl;
    }
    catch (const std::exception& e)
    {
        LOG_E << "Cannot set global building data: " << e.what() << std::endl;
    }
}
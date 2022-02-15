//
// Created by User on 2022-02-08.
//
#include "worker.h"

extern "C"
{
#include "go_bridge.h"
}

int AlbcRun(InParams *in, OutParams **out)
{

    *out = new OutParams;
    return 0;
}

void AlbcFree(OutParams *out)
{
    delete out->json;
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
            const auto &sc = SCOPE_TIMER_WITH_TRACE("albc::worker::work");
            albc::worker::run_test(player_data, game_data, albc::diagnostics::LogLevel::INFO);
        }
        LOG_I << "Test completed" << std::endl;
    }
    catch (const std::exception &e)
    {
        LOG_E << "Exception: " << e.what() << std::endl;
    }
}

// ReSharper disable StringLiteralTypo
#include <future>
#include "worker.h"
#include "algorithm.h"
#include "buff_def.h"
#include "locale_util.h"
#include "player_data_model.h"
#include "time_util.h"
namespace albc::worker
{
void run_test(const string &player_data_path, const string &game_data_path, LogLevel logLevel)
{
    LOG_I << "Initializing internal buff models" << std::endl;
    LOG_I << "Loaded " << BuffMap::instance()->size() << " internal building buff models" << std::endl;

    std::shared_ptr<bm::BuildingData> building_data;
    std::shared_ptr<PlayerDataModel> player_data;

    LOG_I << "Parsing game building data file: " << game_data_path << std::endl;
    try
    {
        const Json::Value &building_data_json = read_json_from_file(game_data_path);
        building_data = std::make_shared<bm::BuildingData>(building_data_json);
        LOG_I << "Loaded " << building_data->chars.size() << " building character definitions." << std::endl;
        LOG_I << "Loaded " << building_data->buffs.size() << " building buff definitions." << std::endl;
    }
    catch (const std::exception &e)
    {
        LOG_E << "Error: Unable to parse game building data file: " << e.what() << std::endl;
        throw;
    }

    int unsupported_buff_cnt = 0;
    for (const auto &[buff_id, buff] : building_data->buffs)
    {
        if (BuffMap::instance()->count(buff_id) <= 0)
        {
            if (show_all_ops)
            {
                std::cout << "\"" << buff->buff_id << "\": " << toOSCharset(buff->buff_name) << ": "
                          << toOSCharset(xml::strip_xml_tags(buff->description)) << std::endl;
            }
            ++unsupported_buff_cnt;
        }
    }
    if (!show_all_ops)
    {
        LOG_D << unsupported_buff_cnt
              << R"( unsupported buff found in building data buff definitions. Add "--all-ops" param to check all.)"
              << std::endl;
    }

    LOG_I << "Parsing player data file: " << player_data_path << std::endl;
    try
    {
        const Json::Value &player_data_json = read_json_from_file(player_data_path);
        player_data = std::make_shared<PlayerDataModel>(player_data_json);
        LOG_I << "Added " << player_data->troop.chars.size() << " existing character instance" << std::endl;
        LOG_I << "Added " << player_data->building.player_building_room.manufacture.size() << " factories."
              << std::endl;
        LOG_I << "Added " << player_data->building.player_building_room.trading.size() << " trading posts."
              << std::endl;
        LOG_I << "Player building data parsing completed." << std::endl;
    }
    catch (std::exception &e)
    {
        LOG_E << "Error: Unable to parse player data file: " << e.what() << std::endl;
        throw;
    }

    LOG_I << "Data feeding completed." << std::endl;

    WorkerParams params(*player_data, *building_data);
    const auto& sc = SCOPE_TIMER_WITH_TRACE("Running greedy algorithm");
    MultiRoomGreedy alg_manu(params.GetRoomsOfType(bm::RoomType::MANUFACTURE), params.GetOperators());
    alg_manu.Run();

    MultiRoomGreedy alg_trade(params.GetRoomsOfType(bm::RoomType::TRADING), params.GetOperators());
    alg_trade.Run();
}

void run_parallel_test(const string &player_data_path, const string &game_data_path, LogLevel logLevel, int parallel_cnt)
{
    const auto& sc = SCOPE_TIMER_WITH_TRACE("Parallel test");

	LOG_I << "Running parallel test for " << parallel_cnt << " concurrency" << std::endl;
	
	Vector<std::future<void>> futures;
	for (int i = 0; i < parallel_cnt; ++i)
	{
		futures.push_back(std::async(std::launch::async, run_test, player_data_path, game_data_path, logLevel));
	}

	// wait for all threads to finish
	for (auto &f : futures)
	{
		f.wait();
	}

	LOG_I << "Parallel test completed." << std::endl;
}

void run_sequential_test(const string &player_data_path, const string &game_data_path, LogLevel logLevel, int sequential_cnt)
{
    LOG_I << "Running sequential test for " << sequential_cnt << " iterations" << std::endl;

    for (int i = 0; i < sequential_cnt; ++i)
    {
        run_test(player_data_path, game_data_path, logLevel);
    }

    LOG_I << "Sequential test completed." << std::endl;
}
} // namespace albc::worker

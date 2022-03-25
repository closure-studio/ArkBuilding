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
void run_test(const Json::Value &player_data_json, const Json::Value &game_data_json, LogLevel logLevel)
{
    std::shared_ptr<bm::BuildingData> building_data;
    std::shared_ptr<PlayerDataModel> player_data;

    const auto orig_log_level = GlobalLogConfig::GetLogLevel();
    make_defer([orig_log_level]() { GlobalLogConfig::SetLogLevel(orig_log_level); });
    GlobalLogConfig::SetLogLevel(logLevel);

    {
        auto sc = SCOPE_TIMER_WITH_TRACE("Data feeding");
        LOG_I << "Parsing building json object." << std::endl;
        try
        {
            building_data = std::make_shared<bm::BuildingData>(game_data_json);
            LOG_I << "Loaded " << building_data->chars.size() << " building character definitions." << std::endl;
            LOG_I << "Loaded " << building_data->buffs.size() << " building buff definitions." << std::endl;
        }
        catch (const std::exception &e)
        {
            LOG_E << "Error: Unable to parse game building data json object: " << e.what() << std::endl;
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

        LOG_I << "Parsing player json object." << std::endl;
        try
        {
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
            LOG_E << "Error: Unable to parse player data json object: " << e.what() << std::endl;
            throw;
        }
    }

    GenTestModePlayerData(*player_data, *building_data);
    WorkerParams params(*player_data, *building_data);
    const auto sc = SCOPE_TIMER_WITH_TRACE("Running greedy algorithm");
    Vector<RoomModel *> all_rooms;
    const auto& manu_rooms = get_raw_ptr_vector(params.GetRoomsOfType(bm::RoomType::MANUFACTURE));
    const auto& trade_tooms = get_raw_ptr_vector(params.GetRoomsOfType(bm::RoomType::TRADING));
    all_rooms.insert(all_rooms.end(), manu_rooms.begin(), manu_rooms.end());
    all_rooms.insert(all_rooms.end(), trade_tooms.begin(), trade_tooms.end());

    MultiRoomIntegerProgramming alg_all(all_rooms, params.GetOperators());
    alg_all.Run();

    // MultiRoomGreedy alg_trade(params.GetRoomsOfType(bm::RoomType::TRADING), params.GetOperators());
    // alg_trade.Run();
}

void run_parallel_test(const Json::Value &player_data_json, const Json::Value &game_data_json, LogLevel logLevel, int parallel_cnt)
{
    const auto sc = SCOPE_TIMER_WITH_TRACE("Parallel test");

	LOG_I << "Running parallel test for " << parallel_cnt << " concurrency" << std::endl;
	
	Vector<std::future<void>> futures;
	for (int i = 0; i < parallel_cnt; ++i)
	{
		futures.push_back(std::async(std::launch::async, run_test, player_data_json, game_data_json, logLevel));
	}

	// wait for all threads to finish
	for (auto &f : futures)
	{
		f.wait();
	}

	LOG_I << "Parallel test completed." << std::endl;
}

void run_sequential_test(const Json::Value &player_data_json, const Json::Value &game_data_json, LogLevel logLevel, int sequential_cnt)
{
    LOG_I << "Running sequential test for " << sequential_cnt << " iterations" << std::endl;

    for (int i = 0; i < sequential_cnt; ++i)
    {
        run_test(player_data_json, game_data_json, logLevel);
    }

    LOG_I << "Sequential test completed." << std::endl;
}
} // namespace albc::worker

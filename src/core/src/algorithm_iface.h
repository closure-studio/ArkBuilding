#pragma once
#include "albc/calbc.h"
#include "model_buff.h"
#include "data_building.h"
#include "util_flag.h"
#include "model_operator.h"
#include "albc_types.h"
#include "model_simulator.h"
#include "util_xml.h"

namespace albc::algorithm::iface
{
enum class TestMode
{
    ONCE = 0,
    SEQUENTIAL = 1,
    PARALLEL = 2
};

void launch_test(const Json::Value &player_data_json, const Json::Value &game_data_json,
    const AlbcTestConfig& test_config);

void test_once(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    const AlbcTestConfig& test_config);

void run_parallel_test(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    const AlbcTestConfig& test_config);

void run_sequential_test(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    const AlbcTestConfig& test_config);
} // namespace albc::algorithm::iface

#pragma once
#include "albc/capi.h"
#include "buff_model.h"
#include "building_data_model.h"
#include "flag_util.h"
#include "operator_model.h"
#include "primitive_types.h"
#include "simulator.h"
#include "worker_params.h"
#include "xml_util.h"

namespace albc::worker
{   
void launch_test(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    const AlbcTestConfig& test_config);

void test_once(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    const AlbcTestConfig& test_config);

void run_parallel_test(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    const AlbcTestConfig& test_config);

void run_sequential_test(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    const AlbcTestConfig& test_config);
} // namespace albc::worker

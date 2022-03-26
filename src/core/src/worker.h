#pragma once
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
inline static bool show_all_ops = false;
    
void run_test(const Json::Value &player_data_json, const Json::Value &game_data_json, LogLevel logLevel);

void run_parallel_test(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    LogLevel logLevel, int parallel_cnt);

void run_sequential_test(const Json::Value &player_data_json, const Json::Value &game_data_json, 
    LogLevel logLevel, int sequential_cnt);
} // namespace albc::worker
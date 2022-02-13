#pragma once
#include "flag_util.h"
#include "primitive_types.h"
#include "buff_model.h"
#include "building_data_model.h"
#include "simulator.h"
#include "operator_model.h"
#include "xml_util.h"

namespace albc::worker
{
	void work(const string& player_data_path, const string& game_data_path, int logLevel);
}

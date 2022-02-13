#pragma once
#include <string>

#include "json/json.h"
#include "primitive_types.h"
#include "mem_util.h"
#include "buff_primitives.h"

namespace albc::bm
{
	enum class PlayerRoomState
	{
		STOP = 0,
		RUN = 1,
	};

	struct BuildingBuffDisplay
	{
		explicit BuildingBuffDisplay(const Json::Value &json);

		int base_buff;
		int buff;
	};

	struct PlayerBuildingChar
	{
		explicit PlayerBuildingChar(const Json::Value &json);

		string char_id;
		string room_slot_id;
		time_t last_ap_add_time;
		int ap;
		int index;
		int change_scale;
		int work_time;
	};

	struct PlayerBuildingManufacture
	{
		explicit PlayerBuildingManufacture(const Json::Value &json);

		PlayerRoomState state;
		string formula_id;
		int remain_sln_cnt;
		int output_sln_cnt;
		int capacity;
		int ap_cost;
		double process_point;

		time_t last_update_time;
		time_t complete_work_time;
	};


	struct TradingOrderBuff
	{
		explicit TradingOrderBuff(const Json::Value &json);

		string from;
		int param;
	};

	struct TradingBuff
	{
		explicit TradingBuff(const Json::Value &json);

		double speed;
		int limit;
	};
	
	struct PlayerBuildingTrading
	{
		explicit PlayerBuildingTrading(const Json::Value &json);

		TradingBuff buff;
		PlayerRoomState state;
		OrderType order_type = OrderType::UNDEFINED;
		int stock_limit;
		BuildingBuffDisplay display;
	};

	struct PlayerBuildingLabor
	{
		explicit PlayerBuildingLabor(const Json::Value &json);

		double buff_speed;
		int value;
		int max_value;
		double process_point;
	};

	class PlayerBuildingRoom
	{
	public:
		Dictionary<string, PlayerBuildingManufacture> manufacture;
		Dictionary<string, PlayerBuildingTrading> trading;

		explicit PlayerBuildingRoom(const Json::Value &json);
	};

	class PlayerBuilding
	{
		public:
		explicit PlayerBuilding(const Json::Value &json);

		PlayerBuildingLabor status_labor;
		PlayerBuildingRoom player_building_room;
		PtrDictionary<string, PlayerBuildingChar> chars;
		List<int> assist;
	};
}
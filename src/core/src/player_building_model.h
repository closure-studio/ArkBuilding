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

    enum class PlayerRoomSlotState
    {
        EMPTY = 0,
        UPGRADING = 1,
        BUILD = 2
    };

    struct PlayerBuildingRoomSlot
    {
        explicit PlayerBuildingRoomSlot(const Json::Value& json);

        int level;
        PlayerRoomSlotState state;
        RoomType room_id;
        Vector<int> char_inst_ids;
    };

	struct BuildingBuffDisplay
	{
		explicit BuildingBuffDisplay(const Json::Value &json);

		int base_buff;
		int buff;
	};

	struct PlayerBuildingChar
	{
		PlayerBuildingChar() = default;
	    explicit PlayerBuildingChar(const Json::Value &json);

		string char_id;
		string room_slot_id;
		time_t last_ap_add_time;
		int ap;
		int index;
		int change_scale;
		int work_time;
	};

    enum class ManufactureFormulaId
    {
        RECORD_1 = 1,
        RECORD_2 = 2,
        RECORD_3 = 3,
        GOLD = 4,
        CHIP_1 = 5,
        CHIP_2 = 6,
        CHIP_3 = 7,
        CHIP_4 = 8,
        CHIP_5 = 9,
        CHIP_6 = 10,
        CHIP_7 = 11,
        CHIP_8 = 12,
        ORIGINIUM_SHARD_ORIROCK = 13,
        ORIGINIUM_SHARD_DEVICE = 14,
    };

	struct PlayerBuildingManufacture
	{
		explicit PlayerBuildingManufacture(const Json::Value &json);

		PlayerRoomState state;
		ManufactureFormulaId formula_id;
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

    struct PlayerBuildingTradingOrder
    {
        explicit PlayerBuildingTradingOrder(const Json::Value&)
        { }
    };
	
	struct PlayerBuildingTrading
	{
		explicit PlayerBuildingTrading(const Json::Value &json);

		TradingBuff buff;
		PlayerRoomState state;
		OrderType order_type = OrderType::UNDEFINED;
		int stock_limit;
        Vector<PlayerBuildingTradingOrder> stock;
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
        Dictionary<string, PlayerBuildingRoomSlot> room_slots;
		PlayerBuildingRoom player_building_room;
		PtrDictionary<string, PlayerBuildingChar> chars;
		List<int> assist;
	};
}
#pragma once
#include <string>

#include "model_buff_primitives.h"
#include "util_json.h"
#include "util_mem.h"
#include "albc_types.h"
#include "json/json.h"

namespace albc::data::player
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
    explicit PlayerBuildingRoomSlot(const Json::Value &json);

    int level;
    PlayerRoomSlotState state;
    building::RoomType room_id;
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

    std::string char_id;
    std::string room_slot_id;
    int ap = 0;
    int index = 0;
    int change_scale = 1;
    int work_time = 0;
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
};

struct TradingOrderBuff
{
    explicit TradingOrderBuff(const Json::Value &json);

    std::string from;
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
    explicit PlayerBuildingTradingOrder(const Json::Value &)
    {
    }
};

struct PlayerBuildingTrading
{
    explicit PlayerBuildingTrading(const Json::Value &json);

    TradingBuff buff;
    PlayerRoomState state;
    model::buff::OrderType order_type = model::buff::OrderType::UNDEFINED;
    int stock_limit;
    Vector<PlayerBuildingTradingOrder> stock;
    BuildingBuffDisplay display;
};

struct PlayerBuildingLabor
{
    PlayerBuildingLabor() = default;
    explicit PlayerBuildingLabor(const Json::Value &json);

    double buff_speed;
    int value;
    int max_value;
    double process_point;
};

class PlayerBuildingRoom
{
  public:
    Dictionary<std::string, PlayerBuildingManufacture> manufacture;
    Dictionary<std::string, PlayerBuildingTrading> trading;

    PlayerBuildingRoom() = default;
    explicit PlayerBuildingRoom(const Json::Value &json);
};

class PlayerBuilding
{
  public:
    PlayerBuilding() = default;
    explicit PlayerBuilding(const Json::Value &json);

    PlayerBuildingLabor status_labor{};
    Dictionary<std::string, PlayerBuildingRoomSlot> room_slots;
    PlayerBuildingRoom player_building_room;
    mem::PtrDictionary<std::string, PlayerBuildingChar> chars;
    List<int> assist;
};
} // namespace albc::data::player
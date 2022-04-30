//
// Created by Nonary on 2022/4/24.
//
#include "data_player_building.h"

namespace albc::data::player
{
PlayerBuildingRoomSlot::PlayerBuildingRoomSlot(const Json::Value &json)
    : level(json["level"].asInt()),
      state(util::json_val_as_enum<PlayerRoomSlotState>(json["state"])),
      room_id(util::json_string_as_enum(json["roomId"], building::RoomType::NONE)),
      char_inst_ids(util::json_val_as_vector<int>(json["charInstIds"], util::json_cast<int>))
{}
BuildingBuffDisplay::BuildingBuffDisplay(const Json::Value &json)
    : base_buff(json["base"].asInt()),
      buff(json["buff"].asInt())
{}
PlayerBuildingChar::PlayerBuildingChar(const Json::Value &json)
    : char_id(json["charId"].asString()),
      room_slot_id(json["roomSlotId"].asString()),
      ap(json["ap"].asInt()),
      index(json["index"].asInt()),
      change_scale(json["changeScale"].asInt()),
      work_time(json["workTime"].asInt())
{}
PlayerBuildingManufacture::PlayerBuildingManufacture(const Json::Value &json)
    : state(util::json_val_as_enum<PlayerRoomState>(json["state"])),
      formula_id(static_cast<ManufactureFormulaId>(strtol(json["formulaId"].asString().c_str(), nullptr, 0))),
      remain_sln_cnt(json["remainSolutionCnt"].asInt()),
      output_sln_cnt(json["outputSolutionCnt"].asInt()),
      capacity(json["capacity"].asInt()),
      ap_cost(json["apCost"].asInt()),
      process_point(json["processPoint"].asDouble())
{
    if (!magic_enum::enum_contains<ManufactureFormulaId>(formula_id))
    {
        LOG_E("Invalid formula id: ", static_cast<int>(formula_id));
    }
}
TradingOrderBuff::TradingOrderBuff(const Json::Value &json)
    : from(json["from"].asString()),
      param(json["param"].asInt())
{
}
TradingBuff::TradingBuff(const Json::Value &json) : speed(json["speed"].asDouble()),
                                                    limit(json["limit"].asInt())
{
}
PlayerBuildingTrading::PlayerBuildingTrading(const Json::Value &json)
    : buff(json["buff"]), state(util::json_val_as_enum<PlayerRoomState>(json["state"])),
      stock_limit(json["stockLimit"].asInt()),
      stock(util::json_val_as_vector<PlayerBuildingTradingOrder>(json["stock"])), display(json["display"])
{
    const auto o_type = json["strategy"].asString();
    if (o_type == "O_GOLD")
    {
        order_type = model::buff::OrderType::GOLD;
    }
    else if (o_type == "O_DIAMOND")
    {
        order_type = model::buff::OrderType::ORUNDUM;
    }
    else
    {
        assert(false);
    }
}
PlayerBuildingLabor::PlayerBuildingLabor(const Json::Value &json)
    : buff_speed(json["buffSpeed"].asDouble()),
      value(json["value"].asInt()),
      max_value(json["maxValue"].asInt()),
      process_point(json["ProcessPoint"].asDouble())
{
}
PlayerBuildingRoom::PlayerBuildingRoom(const Json::Value &json)
    : manufacture(util::json_val_as_dictionary<PlayerBuildingManufacture>(json["MANUFACTURE"])),
      trading(util::json_val_as_dictionary<PlayerBuildingTrading>(json["TRADING"]))
{
}
PlayerBuilding::PlayerBuilding(const Json::Value &json)
    : status_labor(json["status"]["labor"]),
      room_slots(util::json_val_as_dictionary<PlayerBuildingRoomSlot>(json["roomSlots"])),
      player_building_room(json["rooms"]), chars(util::json_val_as_ptr_dictionary<PlayerBuildingChar>(json["chars"]))
{
}
}
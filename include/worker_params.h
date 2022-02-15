//
// Created by Nonary on 2022/2/15.
//
#pragma once
#include "buff_primitives.h"
#include "player_building_model.h"
#include "building_data_model.h"
#include "player_data_model.h"
#include <bitset>

namespace albc::worker
{
using PlayerBuildingRoomMap = Array<PtrVector<RoomModel>, bm::kRoomTypeCount>;

static GlobalAttributeFields GlobalAttributeFactory(const bm::PlayerBuilding &building)
{
    UInt32 power_plant_cnt = 0;
    UInt32 trading_post_cnt = 0;
    UInt32 dorm_operator_cnt = 0;
    UInt32 gold_prod_line_cnt = 0;
    UInt32 dorm_sum_level = 0;

    for (const auto &[id, slot] : building.room_slots)
    {
        switch (slot.room_id)
        {
        case bm::RoomType::POWER:
            power_plant_cnt++;
            break;

        case bm::RoomType::DORMITORY:
            dorm_operator_cnt += slot.char_inst_ids.size();
            dorm_sum_level += slot.level;
            break;

        case bm::RoomType::TRADING:
            trading_post_cnt++;
            break;

        default:
            break;
        }
    }

    for (const auto &[id, manu_room] : building.player_building_room.manufacture)
    {
        if (manu_room.formula_id == bm::ManufactureFormulaId::GOLD)
            gold_prod_line_cnt++;
    }

    GlobalAttributeFields result;
    write_attribute(result, GlobalAttributeType::POWER_PLANT_CNT, power_plant_cnt);
    write_attribute(result, GlobalAttributeType::TRADING_POST_CNT, trading_post_cnt);
    write_attribute(result, GlobalAttributeType::DORM_OPERATOR_CNT, dorm_operator_cnt);
    write_attribute(result, GlobalAttributeType::GOLD_PROD_LINE_CNT, gold_prod_line_cnt);
    write_attribute(result, GlobalAttributeType::DORM_SUM_LEVEL, dorm_sum_level);

    return result;
}

class WorkerParams
{
  public:
    WorkerParams(const PlayerDataModel)
    {

    }

    [[nodiscard]] const PtrVector<RoomModel> &GetRoomsOfType(bm::RoomType type) const
    {
        if (UInt32 type_val = static_cast<UInt32>(type), idx = ctz(type_val);
            is_pow_of_two(type_val) && idx > 0 && idx < static_cast<int>(rooms_map_.size()))
        {
            return rooms_map_[idx];
        }
        else
        {
            LOG_E << "Invalid room type: 0b" << to_bin_string(type_val) << std::endl;
            return PtrVector<RoomModel>::Default();
        }
    }

  private:
    PlayerBuildingRoomMap rooms_map_;
};
} // namespace albc::worker
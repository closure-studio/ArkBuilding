//
// Created by Nonary on 2022/2/15.
//
#pragma once
#include "buff_primitives.h"
#include "building_data_model.h"
#include "player_building_model.h"
#include "player_data_model.h"
#include <bitset>
#include <unordered_set>

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

    GlobalAttributeFields attr;
    write_attribute(attr, GlobalAttributeType::POWER_PLANT_CNT, power_plant_cnt);
    write_attribute(attr, GlobalAttributeType::TRADING_POST_CNT, trading_post_cnt);
    write_attribute(attr, GlobalAttributeType::DORM_OPERATOR_CNT, dorm_operator_cnt);
    write_attribute(attr, GlobalAttributeType::GOLD_PROD_LINE_CNT, gold_prod_line_cnt);
    write_attribute(attr, GlobalAttributeType::DORM_SUM_LEVEL, dorm_sum_level);

    return attr;
}

static std::unique_ptr<RoomModel> RoomFactory(const string &id, const bm::PlayerBuildingManufacture &manufacture_room,
                                              int level)
{
    auto room = std::make_unique<RoomModel>();
    room->type = bm::RoomType::MANUFACTURE;
    room->id = id;
    room->max_slot_count = level;
    room->room_attributes.base_char_cost = 1;
    room->room_attributes.base_prod_eff = 1 + 0.01 * level;
    room->room_attributes.base_prod_cap = manufacture_room.capacity;

    switch (manufacture_room.formula_id)
    {
    case bm::ManufactureFormulaId::RECORD_1:
    case bm::ManufactureFormulaId::RECORD_2:
    case bm::ManufactureFormulaId::RECORD_3:
        room->room_attributes.prod_type = ProdType::RECORD;
        break;

    case bm::ManufactureFormulaId::GOLD:
        room->room_attributes.prod_type = ProdType::GOLD;
        break;

    case bm::ManufactureFormulaId::ORIGINIUM_SHARD_ORIROCK:
    case bm::ManufactureFormulaId::ORIGINIUM_SHARD_DEVICE:
        room->room_attributes.prod_type = ProdType::ORIGINIUM_SHARD;
        break;

    default:
        room->room_attributes.prod_type = ProdType::CHIP;
        break;
    }

    return room;
}

static std::unique_ptr<RoomModel> RoomFactory(const string &id, const bm::PlayerBuildingTrading &trading_room,
                                              int level)
{
    auto room = std::make_unique<RoomModel>();
    room->type = bm::RoomType::TRADING;
    room->id = id;
    room->max_slot_count = level;
    room->room_attributes.base_char_cost = 1;
    room->room_attributes.base_prod_eff = 1 + 0.01 * level;
    room->room_attributes.order_type = trading_room.order_type;
    room->room_attributes.base_prod_cap = 4 + 2 * level;
    room->room_attributes.prod_cnt = static_cast<int>(trading_room.stock.size());

    return room;
}

[[maybe_unused]] static void GenTestModePlayerData(PlayerDataModel& player_data, const bm::BuildingData &building_data)
{
    LOG_W << "Worker is running under testing mode." << endl;
    int inst_id_counter = player_data.troop.chars.size();
    std::unordered_set<string> char_ids;
    std::transform(player_data.troop.chars.begin(), player_data.troop.chars.end(), 
        std::inserter(char_ids, char_ids.end()),
        [] (const auto& pair) { return pair.second->char_id; });

    for (const auto& [id, building_char] : building_data.chars)
    {
        if (!char_ids.count(id))
        {
            auto dummy_char = std::make_unique<PlayerCharacter>();
            dummy_char->char_id = id;
            dummy_char->inst_id = ++inst_id_counter;
            player_data.troop.chars.insert( {id, std::move(dummy_char)} );

            auto dummy_building_char = std::make_unique<bm::PlayerBuildingChar>();
            dummy_building_char->char_id = id;
            player_data.building.chars.insert({ id, std::move(dummy_building_char) });
        }
    }

    for (auto& [id, player_char] : player_data.troop.chars)
    {
        player_char->evolve_phase = EvolvePhase::PHASE_2;
        player_char->level = 90;
    }

    for (auto& [id, building_char] : player_data.building.chars)
    {
        building_char->ap = 86400;
    }
}

class WorkerParams
{
  public:
    WorkerParams(const PlayerDataModel &player_data, const bm::BuildingData &building_data)
    {
        PlayerTroopLookup lookup(player_data.troop);

        for (const auto &[inst_id, player_char] : player_data.troop.chars)
        {
            if (player_data.building.chars.count(inst_id) <= 0)
            {
                LOG_E << "Unable to find troop character in building characters! : " << player_char->char_id << endl;
                continue;
            }
            if (building_data.chars.count(player_char->char_id) <= 0)
            {
                LOG_E << "Unable to find building data definition for character with given ID! Is building data "
                         "outdated? : "
                      << player_char->char_id << endl;
                continue;
            }

            const auto op = new OperatorModel(*player_char, *player_data.building.chars.at(inst_id));
            op->Empower(lookup, *player_char, building_data);
            operators_.emplace_back(op);
        }

        Dictionary<string, int> room_level_map;
        GlobalAttributeFields global_attr = GlobalAttributeFactory(player_data.building);

        for (const auto &[id, slot] : player_data.building.room_slots)
        {
            room_level_map[id] = slot.level;
        }

        for (const auto &[id, manu_room] : player_data.building.player_building_room.manufacture)
        {
            auto room = RoomFactory(id, manu_room, room_level_map[id]);
            room->global_attributes = global_attr;
            AddRoom(bm::RoomType::MANUFACTURE, std::move(room));
        }

        for (const auto &[id, trade_room] : player_data.building.player_building_room.trading)
        {
            auto room = RoomFactory(id, trade_room, room_level_map[id]);
            room->global_attributes = global_attr;
            AddRoom(bm::RoomType::TRADING, std::move(room));
        }
    }

    void UpdateGlobalAttributes(const GlobalAttributeFields& global_attr) const
    {
        for (const auto& rooms : rooms_map_)
            for (const auto& room : rooms)
                room->global_attributes = global_attr;
    }

    [[nodiscard]] const PtrVector<RoomModel>& GetRoomsOfType(bm::RoomType type) const
    {
        if (UInt32 type_val = static_cast<UInt32>(type), idx = ctz(type_val);
            is_pow_of_two(type_val) && idx > 0 && idx < static_cast<UInt32>(rooms_map_.size()))
        {
            return rooms_map_[idx];
        }
        else
        {
            LOG_E << "Invalid room type: 0b" << to_bin_string(type_val) << std::endl;
            return PtrVector<RoomModel>::Default();
        }
    }

    [[nodiscard]] const PtrVector<OperatorModel>& GetOperators() const
    {
        return operators_;
    }

  private:
    PlayerBuildingRoomMap rooms_map_;
    PtrVector<OperatorModel> operators_;

    [[nodiscard]] static int GetRoomTypeIndex(const bm::RoomType type)
    {
        return ctz(static_cast<UInt32>(type));
    }

    void AddRoom(const bm::RoomType type, std::unique_ptr<RoomModel>&& room)
    {
        rooms_map_[GetRoomTypeIndex(type)].emplace_back(std::move(room));
    }
};
} // namespace albc::worker
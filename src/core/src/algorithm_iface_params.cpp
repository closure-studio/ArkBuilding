//
// Created by Nonary on 2022/4/24.
//
#include "algorithm_iface_params.h"

namespace albc::algorithm::iface
{

model::buff::GlobalAttributeFields GlobalAttributeFactory(const data::player::PlayerBuilding &building)
{
    using namespace model::buff;
    using namespace util;

    UInt32 power_plant_cnt = 0;
    UInt32 trading_post_cnt = 0;
    UInt32 dorm_operator_cnt = 0;
    UInt32 gold_prod_line_cnt = 0;
    UInt32 dorm_sum_level = 0;

    for (const auto &[id, slot] : building.room_slots)
    {
        switch (slot.room_id)
        {
        case data::building::RoomType::POWER:
            power_plant_cnt++;
            break;

        case data::building::RoomType::DORMITORY:
            dorm_operator_cnt += slot.char_inst_ids.size();
            dorm_sum_level += slot.level;
            break;

        case data::building::RoomType::TRADING:
            trading_post_cnt++;
            break;

        default:
            break;
        }
    }

    for (const auto &[id, manu_room] : building.player_building_room.manufacture)
    {
        if (manu_room.formula_id == data::player::ManufactureFormulaId::GOLD)
            gold_prod_line_cnt++;
    }

    model::buff::GlobalAttributeFields attr;
    write_attribute(attr, GlobalAttributeType::POWER_PLANT_CNT, power_plant_cnt);
    write_attribute(attr, GlobalAttributeType::TRADING_POST_CNT, trading_post_cnt);
    write_attribute(attr, GlobalAttributeType::DORM_OPERATOR_CNT, dorm_operator_cnt);
    write_attribute(attr, GlobalAttributeType::GOLD_PROD_LINE_CNT, gold_prod_line_cnt);
    write_attribute(attr, GlobalAttributeType::DORM_SUM_LEVEL, dorm_sum_level);

    return attr;
}
std::unique_ptr<model::buff::RoomModel> RoomFactory(const std::string &id,
                                                    const data::player::PlayerBuildingManufacture &manufacture_room,
                                                    int level)
{
    auto room = std::make_unique<model::buff::RoomModel>();
    room->type = data::building::RoomType::MANUFACTURE;
    room->id = id;
    room->max_slot_count = level;
    room->room_attributes.base_char_cost = 1;
    room->room_attributes.base_prod_eff = 1 + 0.01 * level;
    room->room_attributes.base_prod_cap = manufacture_room.capacity;

    switch (manufacture_room.formula_id)
    {
        case data::player::ManufactureFormulaId::RECORD_1:
        case data::player::ManufactureFormulaId::RECORD_2:
        case data::player::ManufactureFormulaId::RECORD_3:
            room->room_attributes.prod_type = model::buff::ProdType::RECORD;
            break;

        case data::player::ManufactureFormulaId::GOLD:
            room->room_attributes.prod_type = model::buff::ProdType::GOLD;
            break;

        case data::player::ManufactureFormulaId::ORIGINIUM_SHARD_ORIROCK:
        case data::player::ManufactureFormulaId::ORIGINIUM_SHARD_DEVICE:
            room->room_attributes.prod_type = model::buff::ProdType::ORIGINIUM_SHARD;
            break;

        default:
            room->room_attributes.prod_type = model::buff::ProdType::CHIP;
            break;
    }

    return room;
}
std::unique_ptr<model::buff::RoomModel> RoomFactory(const std::string &id,
                                                    const data::player::PlayerBuildingTrading &trading_room, int level)
{
    auto room = std::make_unique<model::buff::RoomModel>();
    room->type = data::building::RoomType::TRADING;
    room->id = id;
    room->max_slot_count = level;
    room->room_attributes.base_char_cost = 1;
    room->room_attributes.base_prod_eff = 1 + 0.01 * level;
    room->room_attributes.order_type = trading_room.order_type;
    room->room_attributes.base_prod_cap = 4 + 2 * level;
    room->room_attributes.prod_cnt = static_cast<int>(trading_room.stock.size());

    return room;
}
std::unique_ptr<model::buff::RoomModel> RoomFactory(const CustomRoomData &room_data)
{
    auto room = std::make_unique<model::buff::RoomModel>();
    room->type = room_data.type;
    room->id = room_data.identifier;
    room->max_slot_count = room_data.max_slot_cnt;
    room->room_attributes = room_data.room_attributes;

    return room;
}
void GenTestModePlayerData(data::player::PlayerDataModel &player_data,
                           const data::building::BuildingData &building_data)
{
    LOG_W("Model is running under testing mode. All data will be generated to max level.");
    int inst_id_counter = (int)player_data.troop.chars.size();
    std::unordered_set<std::string> char_ids;
    std::transform(player_data.troop.chars.begin(), player_data.troop.chars.end(),
                   std::inserter(char_ids, char_ids.end()), [](const auto &pair) { return pair.second->char_id; });

    for (const auto &[id, building_char] : building_data.chars)
    {
        if (!char_ids.count(id))
        {
            auto dummy_char = std::make_unique<data::player::PlayerCharacter>();
            dummy_char->char_id = id;
            dummy_char->inst_id = ++inst_id_counter;
            player_data.troop.chars.insert({id, std::move(dummy_char)});

            auto dummy_building_char = std::make_unique<data::player::PlayerBuildingChar>();
            dummy_building_char->char_id = id;
            player_data.building.chars.insert({id, std::move(dummy_building_char)});
        }
    }

    for (auto &[id, player_char] : player_data.troop.chars)
    {
        player_char->evolve_phase = data::EvolvePhase::PHASE_2;
        player_char->level = 90;
    }

    for (auto &[id, building_char] : player_data.building.chars)
    {
        building_char->ap = 86400;
    }
}
AlgorithmParams::AlgorithmParams(const data::player::PlayerDataModel &player_data,
                                 const data::building::BuildingData &building_data)
{
    data::player::PlayerTroopLookup lookup(player_data.troop);

    for (const auto &[inst_id, player_char] : player_data.troop.chars)
    {
        if (player_data.building.chars.count(inst_id) <= 0)
        {
            LOG_E("Unable to find troop character in building characters! : ", player_char->char_id);
            continue;
        }
        if (building_data.chars.count(player_char->char_id) <= 0)
        {
            LOG_E("Unable to find building data definition for character with given ID! Is building data "
                  "outdated? : ",
                  player_char->char_id);
            continue;
        }

        const auto op = new model::OperatorModel(*player_char, *player_data.building.chars.at(inst_id));
        op->identifier = player_char->char_id;
        op->Empower(lookup, *player_char, building_data);
        operators_.emplace_back(op);
    }

    Dictionary<std::string, int> room_level_map;
    model::buff::GlobalAttributeFields global_attr = GlobalAttributeFactory(player_data.building);

    for (const auto &[id, slot] : player_data.building.room_slots)
    {
        room_level_map[id] = slot.level;
    }

    for (const auto &[id, manu_room] : player_data.building.player_building_room.manufacture)
    {
        auto room = RoomFactory(id, manu_room, room_level_map[id]);
        room->global_attributes = global_attr;
        AddRoom(data::building::RoomType::MANUFACTURE, std::move(room));
    }

    for (const auto &[id, trade_room] : player_data.building.player_building_room.trading)
    {
        auto room = RoomFactory(id, trade_room, room_level_map[id]);
        room->global_attributes = global_attr;
        AddRoom(data::building::RoomType::TRADING, std::move(room));
    }
}
AlgorithmParams::AlgorithmParams(const CustomPackedInput &custom_input,
                                 const data::building::BuildingData &building_data)
{
    Vector<std::pair<int, std::string>> char_ids;
    // 只添加基本信息和PlayerTroopLookup所需信息
    {
        UInt32 inst_id_counter = 0;
        for (const auto &custom_char : custom_input.characters)
        {
            inst_id_counter++;
            if (!custom_char.resolved_char_id.empty())
                char_ids.emplace_back(inst_id_counter, custom_char.resolved_char_id);
            else
                char_ids.emplace_back(inst_id_counter, "CUSTOM_CHAR_" + std::to_string(inst_id_counter));

            auto op = std::make_unique<model::OperatorModel>(inst_id_counter, custom_char.resolved_char_id,
                                                             3600. * custom_char.morale);
            op->identifier = custom_char.identifier;
            op->sp_char_group = custom_char.sp_char_group;
            operators_.emplace_back(std::move(op));
        }
    }

    data::player::PlayerTroopLookup lookup(char_ids);

    // 添加干员Buff
    for (UInt32 i = 0; i < custom_input.characters.size(); ++i)
    {
        const auto &custom_char = custom_input.characters[i];
        const auto &op = operators_[i];
        const auto &[inst_id, char_id] = char_ids[i];
        if (custom_char.is_regular_character)
        {
            data::player::PlayerCharacter player_char;
            player_char.inst_id = inst_id;
            player_char.char_id = char_id;
            player_char.level = custom_char.level;
            player_char.evolve_phase = custom_char.phase;

            op->Empower(lookup, player_char, building_data);
        }
        else
        {
            for (const auto &buff_id : custom_char.resolved_skill_ids)
            {
                if (!op->AddBuff(lookup, building_data, buff_id))
                    LOG_W("Unable to add buff to operator! : ", buff_id, " Operator ID : ", custom_char.identifier);
            }
            op->ResolvePatches();
        }

        if (op->buffs.empty())
        {
            LOG_W("Operator has no buffs! : ", custom_char.identifier);
        }
    }

    for (const auto &custom_room : custom_input.rooms)
    {
        auto room = RoomFactory(custom_room);
        room->global_attributes = custom_input.global_data.global_attributes;
        AddRoom(custom_room.type, std::move(room));
    }
}
void AlgorithmParams::UpdateGlobalAttributes(const model::buff::GlobalAttributeFields &global_attr) const
{
    for (const auto &rooms : rooms_map_)
        for (const auto &room : rooms)
            room->global_attributes = global_attr;
}
const mem::PtrVector<model::buff::RoomModel> &AlgorithmParams::GetRoomsOfType(data::building::RoomType type) const
{
    if (UInt32 type_val = static_cast<UInt32>(type), idx = util::ctz(type_val);
        util::is_pow_of_two(type_val) && idx > 0 && idx < static_cast<UInt32>(rooms_map_.size()))
    {
        return rooms_map_[idx];
    }
    else
    {
        LOG_E("Invalid room type: 0b", util::to_bin_string(type_val));
        return rooms_map_[0];
    }
}
int AlgorithmParams::GetRoomTypeIndex(const data::building::RoomType type)
{
    return util::ctz(static_cast<UInt32>(type));
}
void AlgorithmParams::AddRoom(const data::building::RoomType type, std::unique_ptr<model::buff::RoomModel> &&room)
{
    rooms_map_[GetRoomTypeIndex(type)].emplace_back(std::move(room));
}
}

//
// Created by Nonary on 2022/4/24.
//
#include "model_operator.h"
#include "model_buff_map.h"
#include "util_xml.h"
#include "util_flag.h"

namespace albc::model
{
OperatorModel::OperatorModel(const data::player::PlayerCharacter &player_char,
                             const data::player::PlayerBuildingChar &building_char)
    : inst_id(player_char.inst_id),
      char_id(player_char.char_id),
      room_type_mask(data::building::RoomType::NONE),
      duration(building_char.ap)
{
}
OperatorModel::OperatorModel(int inst_id, std::string char_id, UInt32 duration)
    : inst_id(inst_id),
      char_id(std::move(char_id)),
      room_type_mask(data::building::RoomType::NONE),
      duration(duration)
{
}
OperatorModel::~OperatorModel()
{
    mem::free_ptr_vector(this->buffs);
}
void OperatorModel::Empower(const data::player::PlayerTroopLookup &lookup,
                            const data::player::PlayerCharacter &player_char,
                            const data::building::BuildingData &building_data, const bool error_on_buff_not_found,
                            const bool ignore_unlock_cond)
{
    const auto evolve_phase = player_char.evolve_phase;
    const auto level = player_char.level;
    const auto buff_map = buff::BuffMap::instance();

    for (const auto &buff_char : building_data.chars.at(char_id)->buff_char)
    { // BuffChar : 角色可以同时拥有的不同buff槽位
        for (auto it = buff_char->buff_data.rbegin(); it != buff_char->buff_data.rend(); ++it)
        { // BuffData : 每个Buff槽位中对应角色当前等级数据的Buff
            const auto &buff_data = *it;

            if (!buff_map->count(buff_data.buff_id))
            {
                if (error_on_buff_not_found)
                {
                    LOG_E("Unable to find buff with given ID! Is buff data outdated? : ", buff_data.buff_id);
                }
                continue;
            }

            if (ignore_unlock_cond || buff_data.cond.Check(evolve_phase, level))
            {
                AddBuff(lookup, building_data, buff_data.buff_id);
                break;
            }
        }
    }

    ResolvePatches();
}
bool OperatorModel::AddBuff(const data::player::PlayerTroopLookup &lookup,
                            const data::building::BuildingData &building_data, const std::string &buff_id)
{
    if (std::any_of(this->buffs.begin(), this->buffs.end(), [&](const auto &buff) { return buff->buff_id == buff_id; }))
    {
        LOG_W("Buff already exists! : ", buff_id, " on ", char_id);
        return false;
    }

    const auto buff_map = buff::BuffMap::instance();
    if (!buff_map->count(buff_id))
    {
        return false;
    }
    auto buff = buff_map->at(buff_id)->Clone();
    assert(!buff->buff_id.empty());

    buff->owner_inst_id = inst_id;
    buff->owner_char_id = char_id;
    buff->name = building_data.buffs.at(buff->buff_id)->buff_name;
    buff->description = xml::strip_xml_tags(building_data.buffs.at(buff->buff_id)->description);
    buff->duration = duration;
    buff->sort_id = building_data.buffs.at(buff->buff_id)->sort_id;
    buff->UpdateLookup(lookup);
    this->buffs.push_back(buff);

    room_type_mask = util::merge_flag(room_type_mask, buff->room_type);
    return true;
}
void OperatorModel::ResolvePatches()
{
    Set<std::string> patch_target;

    for (const auto buff : this->buffs)
    {
        for (const auto &patch : buff->patch_targets)
        {
            patch_target.insert(patch); // 暂不考虑复杂情况
        }
    }

    buffs.erase(std::remove_if(buffs.begin(), buffs.end(),
                               [patch_target](const buff::RoomBuff *buff) -> bool {
                                 bool remove = patch_target.count(buff->buff_id);
                                 if (remove)
                                 {
                                     LOG_D("Patching buff ", buff->buff_id,
                                           " of operator ", buff->owner_char_id);

                                     delete buff;
                                 }
                                 return remove;
                               }),
                buffs.end());
}
}

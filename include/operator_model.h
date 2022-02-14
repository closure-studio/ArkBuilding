#pragma once
#include "buff_def.h"
#include "buff_model.h"
#include "building_data_model.h"
#include "flag_util.h"
#include "player_data_model.h"
#include "primitive_types.h"
#include "xml_util.h"

namespace albc
{
class OperatorModel // 表明一个干员, 包括其属性、buff
{
  public:
    int inst_id;
    string char_id;
    bm::RoomType room_type_mask; // 可以放置的房间类型, 位掩码
    Vector<RoomBuff *> buffs;    // 所有buff
    UInt32 duration = 86400;     // 干员在1X倍率下的剩余可工作时间, 单位: 秒

    OperatorModel(const PlayerCharacter &player_char, const bm::PlayerBuildingChar &building_char,
                  const bm::BuildingData &building_data)
        : inst_id(player_char.inst_id), char_id(player_char.char_id), room_type_mask(bm::RoomType::NONE),
          duration(building_char.ap)
    {
    }

    ~OperatorModel()
    {
        free_ptr_vector(buffs);
    }

    void Empower(const PlayerTroopLookup &lookup, const PlayerCharacter &player_char,
                 const bm::BuildingData &building_data, const bool error_on_buff_not_found = false,
                 const bool ignore_unlock_cond = false)
    // ref: https://github.com/flaneur2020/pua-lang
    {
        const auto evolve_phase = player_char.evolve_phase;
        const auto level = player_char.level;

        for (const auto &buff_char : building_data.chars.at(char_id)->buff_char)
        { // BuffChar : 角色可以同时拥有的不同buff槽位
            for (auto it = buff_char->buff_data.rbegin(); it != buff_char->buff_data.rend(); ++it)
            { // BuffData : 每个Buff槽位中对应角色当前等级数据的Buff
                const auto &buff_data = *it;

                if (!BuffMap::instance()->count(buff_data.buff_id))
                {
                    if (error_on_buff_not_found)
                    {
                        LOG_E << "Unable to find buff with given ID! Is buff data outdated? : " << buff_data.buff_id
                              << endl;
                    }
                    continue;
                }

                if (ignore_unlock_cond || buff_data.cond.Check(evolve_phase, level))
                {
                    auto buff = BuffMap::instance()->at(buff_data.buff_id)->Clone();
                    assert(!buff->buff_id.empty());

                    buff->owner_inst_id = inst_id;
                    buff->owner_char_id = char_id;
                    buff->name = building_data.buffs.at(buff->buff_id)->buff_name;
                    buff->description = xml::strip_xml_tags(building_data.buffs.at(buff->buff_id)->description);
                    buff->duration = duration;
                    buff->sort_id = building_data.buffs.at(buff->buff_id)->sort_id;
                    buff->UpdateLookup(lookup);
                    buffs.push_back(buff);

                    room_type_mask = merge_flag(room_type_mask, buff->room_type);
                    break;
                }
            }
        }

        ResolvePatches();
    }

  private:
    void ResolvePatches()
    {
        Set<string> patch_target;

        for (const auto buff : buffs)
        {
            for (const auto &patch : buff->patch_targets)
            {
                patch_target.insert(patch); // 暂不考虑复杂情况
            }
        }

        buffs.erase(std::remove_if(buffs.begin(), buffs.end(),
                                   [patch_target](const RoomBuff *buff) -> bool {
                                       bool remove = patch_target.count(buff->buff_id);
                                       if (remove)
                                       {
                                           LOG_D << "Removing buff " << buff->buff_id << " from operator "
                                                 << buff->owner_char_id << endl;
                                       }

                                       return remove;
                                   }),
                    buffs.end());
    }
};
} // namespace albc
#pragma once
#include "model_buff.h"
#include "data_building.h"
#include "data_player.h"
#include "albc_types.h"

namespace albc::model
{
class OperatorModel // 表明一个干员, 包括其属性、buff
{
public:
    int inst_id;
    std::string char_id;                     // 游戏数据ID
    std::string identifier;                  // 自定义标识符
    std::string sp_char_group;               // 是否是异格干员，同一个异格干员组中的干员不能同时存在在排班结果中
    data::building::RoomType room_type_mask; // 可以放置的房间类型, 位掩码
    Vector<buff::RoomBuff *> buffs;          // 所有buff
    UInt32 duration;                         // 干员在1X倍率下的剩余可工作时间, 单位: 秒

    OperatorModel(const data::player::PlayerCharacter &player_char,
                  const data::player::PlayerBuildingChar &building_char);
    OperatorModel(int inst_id, std::string char_id, UInt32 duration);

    OperatorModel(const OperatorModel &other) = delete;
    OperatorModel &operator=(const OperatorModel &other) = delete;
    OperatorModel(OperatorModel &&other) = default;
    OperatorModel &operator=(OperatorModel &&other) = default;
    ~OperatorModel();

    void Empower(const data::player::PlayerTroopLookup &lookup, const data::player::PlayerCharacter &player_char,
                 const data::building::BuildingData &building_data, bool error_on_buff_not_found = false,
                 bool ignore_unlock_cond = false);

    bool AddBuff(const data::player::PlayerTroopLookup &lookup, const data::building::BuildingData &building_data,
                 const std::string &buff_id);

    void ResolvePatches();
};
} // namespace albc
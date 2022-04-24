//
// Created by Nonary on 2022/4/24.
//
#include "data_game.h"

namespace albc::data::game
{

}
albc::data::UnlockCondition::UnlockCondition(const Json::Value &json)
    :
    phase(util::json_val_as_enum<EvolvePhase>(json["phase"])),
    level(json["level"].asInt())
{ }

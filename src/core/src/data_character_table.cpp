//
// Created by Nonary on 2022/4/24.
//
#include "data_character_table.h"

namespace albc::data::game
{

CharacterData::CharacterData(const Json::Value &json)
    : name(json["name"].asString()), appellation(json["appellation"].asString())
{
}
CharacterTable::CharacterTable(const Json::Value &json)
    : mem::PtrDictionary<std::string, CharacterData>(util::json_val_as_ptr_dictionary<CharacterData>(json))
{
}
}
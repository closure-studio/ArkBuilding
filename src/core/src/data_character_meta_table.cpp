//
// Created by Nonary on 2022/4/24.
//
#include "data_character_meta_table.h"

namespace albc::data::game
{

CharacterMetaTable::CharacterMetaTable(const Json::Value &json)
    : sp_char_groups(util::json_val_as_dictionary<Vector<std::string>>(
    json["spCharGroups"],
    [](const Json::Value& val) { return util::json_val_as_vector<std::string>(val, util::json_cast<std::string>); }))
{
}
bool CharacterMetaTable::IsSpCharacter(const std::string &char_id) const
{
    auto it = sp_char_groups.find(char_id);
    return it != sp_char_groups.end() && it->second.size() > 1;
}
std::string CharacterMetaTable::TryGetSpGroup(const std::string &char_id) const
{
    auto it = sp_char_groups.find(char_id);
    if (it != sp_char_groups.end())
    {
        return it->first;
    }
    return "";
}
}
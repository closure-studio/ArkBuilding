//
// Created by Nonary on 2022/4/24.
//
#include "data_character_lookup_table.h"

namespace albc::data::game
{

CharacterLookupTable::CharacterLookupTable(std::shared_ptr<CharacterTable> character_table)
    : character_table_(std::move(character_table))
{
    for (auto &[char_id, character] : *character_table_)
    {
        name_to_id_[character->name] = char_id;
        if (!character->appellation.empty())
            appellation_to_id_[character->appellation] = char_id;
    }
}
std::string CharacterLookupTable::NameToId(const std::string &name) const
{
    auto it = name_to_id_.find(name);
    return it == name_to_id_.end() ? "" : it->second;
}
std::string CharacterLookupTable::AppellationToId(const std::string &appellation) const
{
    auto it = appellation_to_id_.find(appellation);
    return it == appellation_to_id_.end() ? "" : it->second;
}
std::string CharacterLookupTable::IdToName(const std::string &id) const
{
    auto it = character_table_->find(id);
    return it == character_table_->end() ? "" : it->second->name;
}
std::string CharacterLookupTable::IdToAppellation(const std::string &id) const
{
    auto it = character_table_->find(id);
    return it == character_table_->end() ? "" : it->second->appellation;
}
bool CharacterLookupTable::HasId(const std::string &id) const
{
    return character_table_->find(id) != character_table_->end();
}
bool CharacterLookupTable::HasName(const std::string &name) const
{
    return name_to_id_.find(name) != name_to_id_.end();
}
}
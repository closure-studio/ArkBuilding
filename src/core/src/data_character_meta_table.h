#pragma once
#include "util_json.h"
#include "albc_types.h"

namespace albc::data::game
{
class CharacterMetaTable
{
  public:
    Dictionary<std::string, Vector<std::string>> sp_char_groups; // json: spCharGroups

    explicit CharacterMetaTable(const Json::Value &json);

    [[nodiscard]] bool IsSpCharacter(const std::string &char_id) const;

    [[nodiscard]] std::string TryGetSpGroup(const std::string &char_id) const;
};
} // namespace albc::data::game

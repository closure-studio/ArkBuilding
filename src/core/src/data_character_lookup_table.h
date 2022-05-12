//
// Created by Nonary on 2022/4/21.
//
#pragma once
#include <mutex>
#include <numeric>
#include "albc_types.h"
#include "data_building.h"
#include "data_character_table.h"
namespace albc::data::game
{
class ICharacterLookupTable
{
  public:
    [[nodiscard]] virtual std::string NameToId(const std::string &name) const = 0;

    [[nodiscard]] virtual std::string AppellationToId(const std::string &appellation) const = 0;

    [[nodiscard]] virtual std::string IdToName(const std::string &id) const = 0;

    [[nodiscard]] virtual std::string IdToAppellation(const std::string &id) const = 0;

    [[nodiscard]] virtual bool HasId(const std::string& id) const = 0;

    [[nodiscard]] virtual bool HasName(const std::string& name) const = 0;
};
class CharacterLookupTable: public ICharacterLookupTable
{
  public:
    explicit CharacterLookupTable(std::shared_ptr<CharacterTable> character_table, std::shared_ptr<building::BuildingData> building_data);

    [[nodiscard]] std::string NameToId(const std::string &name) const override;

    [[nodiscard]] std::string AppellationToId(const std::string &appellation) const override;

    [[nodiscard]] std::string IdToName(const std::string &id) const override;

    [[nodiscard]] std::string IdToAppellation(const std::string &id) const override;

    [[nodiscard]] bool HasId(const std::string &id) const override;

    [[nodiscard]] bool HasName(const std::string &name) const override;

  private:
    std::shared_ptr<CharacterTable> character_table_;
    std::shared_ptr<building::BuildingData> building_data_;
    Dictionary<std::string, std::string> name_to_id_;
    Dictionary<std::string, std::string> appellation_to_id_;
};
} // namespace albc::data::game
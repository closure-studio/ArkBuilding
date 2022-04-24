//
// Created by Nonary on 2022/4/21.
//
#pragma once
#include <mutex>
#include "albc_types.h"
#include "data_building.h"

namespace albc::data::game
{

class CharacterData
{
  public:
    std::string name;
    std::string appellation;

    explicit CharacterData(const Json::Value &json);
};

class CharacterTable : public mem::PtrDictionary<std::string, CharacterData>
{
  public:
    explicit CharacterTable(const Json::Value &json);
};
}

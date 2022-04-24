//
// Created by Nonary on 2022/4/24.
//
#pragma once
#include "albc_types.h"
#include "util_json.h"

namespace albc::api
{

class IJsonReader
{
  public:
    [[nodiscard]] virtual Json::Value Read(const char *json) const = 0;
};

class IJsonWriter
{
  public:
    [[nodiscard]] virtual std::string Write(const Json::Value &root) const = 0;
};

class JsonReader : public IJsonReader
{
  public:
    [[nodiscard]] Json::Value Read(const char *json) const override;
};

class JsonWriter : public IJsonWriter
{
  public:
    [[nodiscard]] std::string Write(const Json::Value &root) const override;
};
} // namespace albc::api
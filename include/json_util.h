#pragma once
#include "exception_util.h"
#include "log_util.h"
#include "mem_util.h"
#include "json/json.h"
#include <fstream>
#include <iostream>

namespace albc::json
{
template <typename T> constexpr auto json_ctor(const Json::Value &json) -> T *
{
    return new T(json);
} // the default constructor

template <typename TEnum> constexpr auto json_val_as_enum(const Json::Value &val) -> TEnum
{
    return static_cast<TEnum>(val.asInt());
} // converts a json value to an enum

template <typename TEnum> constexpr TEnum json_string_as_enum(const Json::Value &val, const TEnum default_val)
{
    const auto enum_or_null = magic_enum::enum_cast<TEnum>(val.asString());
    return enum_or_null.has_value() ? enum_or_null.value() : default_val;
}

template <typename TValue> static auto json_val_as_dictionary(const Json::Value &val) -> Dictionary<string, TValue>
{
    static_assert(std::is_constructible_v<TValue, const Json::Value &>,
                  "TValue must be constructible from Json::Value");
    Dictionary<string, TValue> dict;

    const auto &names = val.getMemberNames();
    auto it = names.begin();

    for (const auto &item : val)
    {
        dict.emplace(*it, item);
        ++it;
    }

    return dict;
} // Reads json object and assigns its keys and values to a Map

template <typename TValue>
static Dictionary<string, TValue> json_val_as_dictionary(const Json::Value &val,
                                                         TValue (*val_factory)(const Json::Value &json))
{
    static_assert(std::is_constructible_v<TValue, const Json::Value &>,
                  "TValue must be constructible from Json::Value");
    Dictionary<string, TValue> dict;

    const auto &names = val.getMemberNames();
    auto it = names.begin();

    for (const auto &item : val)
    {
        dict.insert({*it, val_factory(item)});
        ++it;
    }

    return dict;
} // Reads json object and assigns its keys and values to a Map, use a value factory to create the values

template <typename TValue, template <class> typename TPtr = std::unique_ptr>
static PtrDictionary<string, TValue, TPtr> json_val_as_ptr_dictionary(const Json::Value &val)
{
    static_assert(std::is_constructible_v<TValue, const Json::Value &>,
                  "TValue must be constructible from Json::Value");
    static_assert(std::is_constructible_v<TPtr<TValue>, TValue *>, "TPtr must be constructible from TValue*");

    PtrDictionary<string, TValue, TPtr> dict;

    const auto &names = val.getMemberNames();
    auto it = names.begin();

    for (const auto &item : val)
    {
        dict.emplace(*it, new TValue(item));
        ++it;
    }

    return dict;
}

template <typename T> [[maybe_unused]] static auto json_val_as_list(const Json::Value &val) -> List<T>
{
    static_assert(std::is_constructible_v<T, const Json::Value &>, "T must be constructible from Json::Value");
    List<T> list;
    for (const auto &item : val)
    {
        list.emplace_back(item);
    }

    return list;
} // Reads json array and assigns its values to a List

template <typename T>
[[maybe_unused]] auto json_val_as_list(const Json::Value &val, T (*val_factory)(const Json::Value &json)) -> List<T>
{
    static_assert(std::is_constructible_v<T, const Json::Value &>, "T must be constructible from Json::Value");
    List<T> list;
    for (const auto &item : val)
    {
        list.push_back(val_factory(item));
    }

    return list;
} // Reads json array and assigns its values to a List, use a value factory to create the values

template <typename T> static auto json_val_as_vector(const Json::Value &val) -> Vector<T>
{
    static_assert(std::is_constructible_v<T, const Json::Value &>, "T must be constructible from Json::Value");
    Vector<T> vec;
    for (const auto &item : val)
    {
        vec.emplace_back(item);
    }

    return vec;
} // Reads json array and assigns its items to a Vector

template <typename T>
static auto json_val_as_vector(const Json::Value &val, T (*val_factory)(const Json::Value &json)) -> Vector<T>
{
    static_assert(std::is_constructible_v<T, const Json::Value &>, "T must be constructible from Json::Value");
    Vector<T> vec;
    for (const auto &item : val)
    {
        vec.push_back(val_factory(item));
    }

    return vec;
} // Reads json array and assigns its items to a Vector, use a value factory to create the values

template <typename T, template <class> typename TPtr = std::unique_ptr>
static PtrVector<T, TPtr> json_val_as_ptr_vector(const Json::Value &val)
{
    static_assert(std::is_constructible_v<T, const Json::Value &>, "T must be constructible from Json::Value");
    static_assert(std::is_constructible_v<TPtr<T>, T *>, "TPtr must be constructible from T*");

    PtrVector<T, TPtr> vec;
    for (const auto &item : val)
    {
        vec.emplace_back(new T(item));
    }

    return vec;
} // Reads json array and assigns its items to a Vector

[[maybe_unused]] static auto read_json_from_file(const string &path) -> Json::Value
{
    Json::Value result;
    std::ifstream fs_read(path);
    fs_read >> result;
    fs_read.close();
    return result;
} // Reads json from a file
} // namespace albc::json

using namespace albc::json;
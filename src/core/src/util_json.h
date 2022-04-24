#pragma once
#include "util_exception.h"
#include "util_log.h"
#include "util_mem.h"
#include "json/json.h"
#include "external/byte_array_buffer.h"
#include <fstream>
#include <iostream>

namespace albc::util
{
template <typename T> constexpr T json_ctor(const Json::Value &json)
{
    return T(json);
}

template <typename T> constexpr T* json_new(const Json::Value &json)
{
    return new T(json);
}

template <template <class...> typename TPtr, typename T> constexpr TPtr<T> json_new(const Json::Value &json)
{
    return TPtr<T>(new T(json));
}

template <typename T> constexpr auto json_cast(const Json::Value &json) -> decltype(json.as<T>())
{
    return json.as<T>();
} // the default constructor

template <typename T> static Json::Value to_json_ctor(const T &t)
{
    return Json::Value(t);
}

template <typename T> static Json::Value to_json_cast(const T &t)
{
    return static_cast<Json::Value>(t);
}

template <typename TEnum> constexpr TEnum json_val_as_enum(const Json::Value &val)
{
    return static_cast<TEnum>(val.asInt());
}

template <typename TEnum> constexpr TEnum json_string_as_enum(const Json::Value &val, const TEnum default_val)
{
    return parse_enum_string(val.asString(), default_val);
}

template <typename TValue>
static Dictionary<std::string, TValue> json_val_as_dictionary(const Json::Value &val,
                                                              TValue (*val_factory)(const Json::Value &json),
                                                              bool throw_on_error = true)
{
    Dictionary<std::string, TValue> dict;

    const auto &names = val.getMemberNames();
    auto it = names.begin();

    for (const auto &item : val)
    {
        try
        {
            dict.template emplace(std::string(*it), val_factory(item));
        }
        catch (const std::exception &e)
        {
            LOG_E(__PRETTY_FUNCTION__, ": Error appending item: ", e.what(), "\nRaw: ", item.toStyledString());
            if (throw_on_error)
                throw;
        }
        ++it;
    }

    return dict;
}

template <typename TValue,
    std::enable_if_t<std::is_constructible_v<TValue, Json::Value>, bool> = true>
static Dictionary<std::string, TValue> json_val_as_dictionary(const Json::Value &val, bool throw_on_error = true)
{
    return json_val_as_dictionary(val, json_ctor<TValue>, throw_on_error);
}

template <typename TValue, template <class...> typename TPtr = std::unique_ptr,
    std::enable_if_t<std::is_constructible_v<TValue, Json::Value>, bool> = true>
static mem::PtrDictionary<std::string, TValue, TPtr> json_val_as_ptr_dictionary(const Json::Value &val, bool throw_on_error = true)
{
    return json_val_as_dictionary<TPtr<TValue>>(val, json_new<TPtr, TValue>, throw_on_error);
}

template <typename T>
[[maybe_unused]] List<T> json_val_as_list(const Json::Value &val, T (*val_factory)(const Json::Value &json), bool throw_on_error = true)
{
    List<T> list;
    for (const auto &item : val)
    {
        try
        {
            list.template emplace_back(val_factory(item));
        }
        catch (const std::exception &e)
        {
            LOG_E(__PRETTY_FUNCTION__, ": Error appending item: ", e.what(), "\nRaw: ", item.toStyledString());
            if (throw_on_error)
                throw;
        }
    }

    return list;
}

template <typename T,
    std::enable_if_t<std::is_constructible_v<T, Json::Value>, int> = 0>
[[maybe_unused]] List<T> json_val_as_list(const Json::Value &val, bool throw_on_error = true)
{
    return json_val_as_list(val, json_ctor<T>, throw_on_error);
}

template <typename T>
static Vector<T> json_val_as_vector(const Json::Value &val, T (*val_factory)(const Json::Value &json), bool throw_on_error = true)
{
    Vector<T> vec;
    for (const auto &item : val)
    {
        try
        {
            vec.template emplace_back(val_factory(item));
        }
        catch (const std::exception &e)
        {
            LOG_E(__PRETTY_FUNCTION__, ": Error inserting item: ", e.what(), "\nRaw: ", item.toStyledString());
            if (throw_on_error)
                throw;
        }
    }

    return vec;
}

template <typename T,
    std::enable_if_t<std::is_constructible_v<T, Json::Value>, bool> = true>
static Vector<T> json_val_as_vector(const Json::Value &val, bool throw_on_error = true)
{
    return json_val_as_vector(val, json_ctor<T>, throw_on_error);
}


template <typename T, template <class...> typename TPtr = std::unique_ptr,
    std::enable_if_t<std::is_constructible_v<T, Json::Value>, bool> = true>
static mem::PtrVector<T, TPtr> json_val_as_ptr_vector(const Json::Value &val, bool throw_on_error = true)
{
    return json_val_as_vector(val, json_new<TPtr, T>, throw_on_error);
}

[[maybe_unused]] static Json::Value read_json_from_file(const std::string &path)
{
    Json::Value result;
    std::ifstream fs_read(path);
    fs_read >> result;
    fs_read.close();
    return result;
} // Reads json from a file

[[maybe_unused]] static Json::Value read_json_from_char_array(const char *json_str)
{
    Json::Value result;
    Json::CharReaderBuilder builder;
    std::string errs;
    byte_array_buffer buffer(json_str, strlen(json_str));
    std::istream is(&buffer);
    if (!Json::parseFromStream(builder, is, &result, &errs))
    {
        throw std::runtime_error("Failed to parse json: " + errs);
    }
    return result;
} // Reads json from a char array

template <typename T>
static Json::Value json_val_from_vector(const Vector<T> &vec, Json::Value (*val_factory)(const T& val) = to_json_ctor<T>)
{
    Json::Value result(Json::arrayValue);
    for (const auto &item : vec)
    {
        result.append(val_factory(item));
    }
    return result;
} // Creates a json array from a Vector

template <typename T>
static Json::Value json_val_from_dictionary(const Dictionary<std::string, T> &dict, Json::Value (*val_factory)(const T& val) = to_json_ctor<T>)
{
    Json::Value result(Json::objectValue);
    for (const auto& [key, value] : dict)
    {
        result[key] = val_factory(value);
    }
    return result;
} // Creates a json object from a Dictionary
} // namespace albc::util
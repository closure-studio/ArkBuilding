#pragma once
#include "exception_util.h"
#include "json/json.h"
#include "log_util.h"
#include <iostream>
#include <fstream>

namespace albc::json
{
    template <typename T>
	constexpr auto json_ctor(const Json::Value& json) -> T*
	{
		return new T(json);
	} // the default constructor

	template <typename TEnum>
	constexpr auto json_val_as_enum(const Json::Value& val) -> TEnum
	{
		return static_cast<TEnum>(val.asInt());
	} // converts a json value to an enum

	template <typename TValue>
	static auto json_val_as_dictionary(const Json::Value& val) -> Dictionary<string, TValue>
	{
		Dictionary<string, TValue> dict;

		const auto& names = val.getMemberNames();
		auto it = names.begin();

		for (const auto& item : val)
		{
			dict.emplace(*it, item);
			++it;
		}

		return dict;
	}// Reads json object and assigns its keys and values to a Map

	template <typename TValue>
	static auto json_val_as_dictionary(const Json::Value& val, TValue (* val_factory)(const Json::Value& json)) ->
	Dictionary<string, TValue>
	{
		Dictionary<string, TValue> dict;

		const auto& names = val.getMemberNames();
		auto it = names.begin();

		for (const auto& item : val)
		{
			dict[*it] = val_factory(item);
			++it;
		}

		return dict;
	}// Reads json object and assigns its keys and values to a Map, use a value factory to create the values

	template <typename T>
	[[maybe_unused]]
	static auto json_val_as_list(const Json::Value& val) -> List<T>
	{
		List<T> list;
		for (const auto& item : val)
		{
			list.emplace_back(item);
		}

		return list;
	}// Reads json array and assigns its values to a List

    template <typename T>
    [[maybe_unused]]
    auto json_val_as_list(const Json::Value& val, T (* val_factory)(const Json::Value& json)) -> List<T>
	{
		List<T> list;
		for (const auto& item : val)
		{
			list.push_back(val_factory(item));
		}

		return list;
	}// Reads json array and assigns its values to a List, use a value factory to create the values

	template <typename T>
	static auto json_val_as_vector(const Json::Value& val) -> Vector<T>
	{
		Vector<T> vec;
		for (const auto& item : val)
		{
			vec.emplace_back(item);
		}

		return vec;
	}// Reads json array and assigns its items to a Vector

	template <typename T>
	static auto json_val_as_vector(const Json::Value& val, T (* val_factory)(const Json::Value& json)) -> Vector<T>
	{
		Vector<T> vec;
		for (const auto& item : val)
		{
			vec.push_back(val_factory(item));
		}

		return vec;
	}// Reads json array and assigns its items to a Vector, use a value factory to create the values

	[[maybe_unused]]
	static auto read_json_from_file(const string& path) -> Json::Value
	{
		Json::Value result;
		std::ifstream fs_read(path);
		fs_read >> result;
		fs_read.close();
		return result;
	}// Reads json from a file
}

using namespace albc::json;
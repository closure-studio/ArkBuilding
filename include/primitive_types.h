#pragma once
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <array>
#include <cassert>

namespace albc::primitive_types
{
	template<typename TKey, typename TValue>
	using Dictionary = std::map<TKey, TValue>;

	template<typename  T>
	using List = std::list<T>;

	template<typename  T>
	using Vector = std::vector<T>;

	template<typename T>
	using Set = std::set<T>;

	template<typename T1, size_t N>
	using Array = std::array<T1, N>;

	using string = std::string;

	using string_view = std::string_view;

	using wstring = std::wstring;

	using Int32 = int;

	using UInt32 = unsigned int;

	using Int64 = long long;

	using UInt64 = unsigned long long;

    using Exception = std::exception;
}

using namespace albc::primitive_types;
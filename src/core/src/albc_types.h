#pragma once
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedStructInspection"
#include <array>
#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <bitset>
#include <cstdint>

#include "albc_config.h"

namespace albc
{

template <typename TKey, typename TValue>
using Dictionary = std::map<TKey, TValue>;

template <typename T>
using List = std::list<T>;

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using Set = std::set<T>;

template <typename T1, size_t N>
using Array = std::array<T1, N>;

template <size_t N>
using BitSet = std::bitset<N>;

using Int32 = int32_t;

using UInt32 = uint32_t;

using Int64 = int64_t;

using UInt64 = uint64_t;
} // namespace albc::primitive_types
#pragma clang diagnostic pop
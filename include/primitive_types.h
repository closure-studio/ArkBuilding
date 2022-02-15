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

namespace albc::primitive_types
{

template <typename T, typename TCreatePolicy> 
class DefaultableType; // forward declaration

template <typename T>
struct DefaultableCreatePolicy
{
  private:
    using ThisType = DefaultableCreatePolicy<T>;
    using Defaultable = DefaultableType<T, ThisType>;

  public:
    static Defaultable Create()
    {
        return Defaultable();
    }
};

template <typename T, typename TCreatePolicy = DefaultableCreatePolicy<T>>
class DefaultableType : public T
{
  private:
    using ThisType = DefaultableType<T, TCreatePolicy>;

  public:
    using T::T;

    [[maybe_unused]] static const ThisType &Default()
    {
        static auto default_value = TCreatePolicy::Create();
        return default_value;
    };
};

template <typename TKey, typename TValue>
using Dictionary = DefaultableType<std::map<TKey, TValue>>;

template <typename T>
using List = DefaultableType<std::list<T>>;

template <typename T>
using Vector = DefaultableType<std::vector<T>>;

template <typename T>
using Set = DefaultableType<std::set<T>>;

template <typename T1, size_t N>
using Array = DefaultableType<std::array<T1, N>>;

template <size_t N>
using BitSet = DefaultableType<std::bitset<N>>;

using string = std::string;

using string_view = std::string_view;

using wstring = std::wstring;

using Int32 = int;

using UInt32 = unsigned int;

using Int64 = long long;

using UInt64 = unsigned long long;

using Exception = std::exception;
} // namespace albc::primitive_types

using namespace albc::primitive_types;
#pragma clang diagnostic pop
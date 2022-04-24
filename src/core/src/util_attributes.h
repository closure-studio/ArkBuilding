#pragma once
#include "albc_types.h"
#include "util.h"
#include <cmath>

namespace albc::util
{
template <typename T, typename TType,
          std::enable_if_t<std::is_enum_v<TType>, bool> = true>
class AttributeFields : public Array<T, util::enum_size_v<TType>>
{
  public:
    using Base = Array<T, util::enum_size_v<TType>>;
    using Base::Base;
    using TTypeUnderlying = std::underlying_type_t<TType>;

    T& operator[](TType type)
    {
        return Base::operator[](static_cast<TTypeUnderlying>(type));
    }

    const T& operator[](TType type) const
    {
        return Base::operator[](static_cast<TTypeUnderlying>(type));
    }

    template <typename TEnum>
    TEnum GetEnum(TType type) const
    {
        return static_cast<TEnum>((*this)[type]);
    }

    int GetInt(TType type) const
    {
        return (*this)[type];
    }

    double GetDouble(TType type) const
    {
        return static_cast<double>((*this)[type]);
    }
};

template <typename TField, size_t Size, typename TType, typename TVal,
          std::enable_if_t<std::is_enum_v<TType>, bool> = true>
constexpr void write_attribute(Array<TField, Size> &attributes_fields, TType attribute_type, TVal val)
{
    using TUnderlying = std::underlying_type_t<TType>;
    attributes_fields[static_cast<TUnderlying>(attribute_type)] = static_cast<TField>(val);
}

template <typename TField, size_t Size, typename TType,
          std::enable_if_t<std::is_enum_v<TType>, bool> = true>
constexpr int read_attribute_as_int(const Array<TField, Size> &attribute_fields, TType attribute_type)
{
    using TUnderlying = std::underlying_type_t<TType>;
    return static_cast<int>(floor(attribute_fields[static_cast<TUnderlying>(attribute_type)]));
}

template <typename TField, size_t Size, typename TType,
          std::enable_if_t<std::is_enum_v<TType>, bool> = true>
constexpr double read_attribute(const Array<TField, Size> &attribute_fields, TType attribute_type)
{
    using TUnderlying = std::underlying_type_t<TType>;
    return attribute_fields[static_cast<TUnderlying>(attribute_type)];
}

template <typename TEnum, typename TField, size_t Size, typename TType,
          std::enable_if_t<std::is_enum_v<TType>, bool> = true>
constexpr TEnum read_attribute_as_enum(const Array<TField, Size> &attribute_fields, TType attribute_type)
{
    using TUnderlying = std::underlying_type_t<TType>;
    return static_cast<TEnum>(attribute_fields[static_cast<TUnderlying>(attribute_type)]);
}
} // namespace albc::util

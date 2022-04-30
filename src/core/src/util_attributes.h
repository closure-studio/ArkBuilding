#pragma once
#include "albc_types.h"
#include "util.h"
#include <cmath>

namespace albc::util
{
template <typename T, typename TType,
            ALBC_REQUIRES(std::is_arithmetic_v<T>), ALBC_REQUIRES(std::is_enum_v<TType>)>
class AttributeFields : public Array<T, enum_size_v<TType>>
{
  public:
    using Base = Array<T, enum_size_v<TType>>;
    using Base::Base;
    using TTypeUnderlying = std::underlying_type_t<TType>;

    AttributeFields()
    {
        std::fill(this->begin(), this->end(), T{});
    }

    T& operator[](TType type)
    {
        return Base::operator[](static_cast<TTypeUnderlying>(type));
    }

    [[nodiscard]] const T &operator[](TType type) const
    {
        return Base::operator[](static_cast<TTypeUnderlying>(type));
    }

    template <typename TEnum>
    [[nodiscard]] TEnum GetEnum(TType type) const
    {
        return static_cast<TEnum>((*this)[type]);
    }

    [[nodiscard]] int GetInt(TType type) const
    {
        return static_cast<int>((*this)[type]);
    }

    [[nodiscard]] double GetDouble(TType type) const
    {
        return static_cast<double>((*this)[type]);
    }
};

template <typename TField, size_t Size, typename TType, typename TVal,
          ALBC_REQUIRES(std::is_enum_v<TType>)>
constexpr void write_attribute(Array<TField, Size> &attributes_fields, TType attribute_type, TVal val)
{
    using TUnderlying = std::underlying_type_t<TType>;
    attributes_fields[static_cast<TUnderlying>(attribute_type)] = static_cast<TField>(val);
}

template <typename TField, size_t Size, typename TType,
          ALBC_REQUIRES(!std::is_enum_v<TType>)>
constexpr int read_attribute_as_int(const Array<TField, Size> &attribute_fields, TType attribute_type)
{
    using TUnderlying = std::underlying_type_t<TType>;
    return static_cast<int>(floor(attribute_fields[static_cast<TUnderlying>(attribute_type)]));
}

template <typename TField, size_t Size, typename TType,
          ALBC_REQUIRES(!std::is_enum_v<TType>)>
constexpr double read_attribute(const Array<TField, Size> &attribute_fields, TType attribute_type)
{
    using TUnderlying = std::underlying_type_t<TType>;
    return attribute_fields[static_cast<TUnderlying>(attribute_type)];
}

template <typename TEnum, typename TField, size_t Size, typename TType,
          ALBC_REQUIRES(std::is_enum_v<TType>)>
constexpr TEnum read_attribute_as_enum(const Array<TField, Size> &attribute_fields, TType attribute_type)
{
    using TUnderlying = std::underlying_type_t<TType>;
    return static_cast<TEnum>(attribute_fields[static_cast<TUnderlying>(attribute_type)]);
}
} // namespace albc::util

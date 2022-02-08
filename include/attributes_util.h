#pragma once
#include <cmath>
#include "primitive_types.h"
#include "util.h"

namespace albc
{
	template<typename TField, size_t Size, typename TType, typename TVal>
	constexpr void write_attribute(Array<TField, Size> attributes_fields, TType attribute_type, TVal val)
	{
		attributes_fields[static_cast<magic_enum::underlying_type_t<TType>>(attribute_type)] = static_cast<TField>(val);
	}

	template<typename TField, size_t Size, typename TType>
	constexpr int read_attribute_as_int(Array<TField, Size> attribute_fields, TType attribute_type)
	{
		return static_cast<int>(floor(attribute_fields[static_cast<magic_enum::underlying_type_t<TType>>(attribute_type)]));
	}

	template<typename TField, size_t Size, typename TType>
	constexpr double read_attribute(Array<TField, Size> attribute_fields, TType attribute_type)
	{
		return attribute_fields[static_cast<magic_enum::underlying_type_t<TType>>(attribute_type)];
	}
}

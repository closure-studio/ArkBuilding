#pragma once
#include "util.h"

namespace albc::util
{
	template<typename T1, typename T2>
	constexpr bool check_flag(const T1 x, const T2 y)
	{
		return static_cast<magic_enum::underlying_type_t<T1>>(x) & static_cast<magic_enum::underlying_type_t<T2>>(y);
	}


	template<typename T>
	constexpr T merge_flag(const T x, const T y)
	{
		typedef magic_enum::underlying_type_t<T> TU;

		return static_cast<T>(static_cast<TU>(x) | static_cast<TU>(y));
	}
}
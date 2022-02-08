#pragma once

template<typename TVal, typename Tn> constexpr bool test_bit(const TVal val, const Tn n)
{
	return val & 1 << static_cast<int>(n);
}

template<typename TVal, typename Tn> constexpr TVal set_bit(const TVal val, const Tn n)
{
	return val | 1 << static_cast<int>(n);
}

template<typename TVal, typename Tn> constexpr TVal truncate_bit(const TVal val, const Tn n)
{
	return val & (1 << static_cast<int>(n)) - 1;
}
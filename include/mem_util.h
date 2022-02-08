#pragma once
#include <new>

namespace albc::mem
{
	template<typename Ty, typename... TArg>
	constexpr decltype(auto) aligned_new (TArg&&... arg)
	{
		return new(std::align_val_t{alignof(Ty)}) Ty(std::forward<TArg>(arg)...);
	} // allocates aligned memory (alignment must be power of 2)
}
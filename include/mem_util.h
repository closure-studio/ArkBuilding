#pragma once
#include <new>

namespace albc::mem
{
	template<typename Ty, typename... TArg>
	constexpr decltype(auto) aligned_new (TArg&&... arg)
	{
		return new(std::align_val_t{alignof(Ty)}) Ty(std::forward<TArg>(arg)...);
	} // allocates aligned memory (alignment must be power of 2)

    template <typename TKey, typename TValue, template<class> typename TPtr>
    void free_ptr_map(std::map<TKey, TPtr<TValue>>& map)
    {
        map.clear();
    }

    template <typename TKey, typename TValue>
    void free_ptr_map(std::map<TKey, TValue*>& map)
    {
        for (auto& pair : map)
        {
            delete pair.second;
        }
        map.clear();
    }

    template <typename TValue, template<class> typename TPtr>
    void free_ptr_vector(std::vector<TPtr<TValue>>& vector)
    {
        vector.clear();
    }

    template <typename TValue>
    void free_ptr_vector(std::vector<TValue*>& vector)
    {
        for (auto& value : vector)
        {
            delete value;
        }
        vector.clear();
    }

    template <typename TKey, typename TValue, template<class> typename TPtr = std::unique_ptr>
    using PtrDictionary = Dictionary<TKey, TPtr<TValue>>;

    template <typename TValue, template<class> typename TPtr = std::unique_ptr>
    using PtrVector = Vector<TPtr<TValue>>;
}

using namespace albc::mem;
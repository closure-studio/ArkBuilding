#pragma once
#include <new>
#include <algorithm>

namespace albc::mem
{
	template<typename Ty, typename... TArg>
	constexpr decltype(auto) aligned_new (TArg&&... arg)
	{
		//return new(std::align_val_t{alignof(Ty)}) Ty(std::forward<TArg>(arg)...);
        return new Ty(std::forward<TArg>(arg)...);
	}

    template <typename TKey, typename TValue, template<class...> typename TPtr>
    constexpr void free_ptr_map(std::map<TKey, TPtr<TValue>>& map)
    {
        map.clear();
    }

    template <typename TKey, typename TValue>
    constexpr void free_ptr_map(std::map<TKey, TValue*>& map)
    {
        for (auto& pair : map)
        {
            delete pair.second;
        }
        map.clear();
    }

    template <typename TValue, template<class...> typename TPtr>
    constexpr void free_ptr_vector(std::vector<TPtr<TValue>> &vector)
    {
        vector.clear();
    }

    template <typename TValue>
    constexpr void free_ptr_vector(std::vector<TValue*>& vector)
    {
        for (auto& value : vector)
        {
            delete value;
        }
        vector.clear();
    }

    template <typename TKey, typename TValue, template<class...> typename TPtr = std::unique_ptr>
    using PtrDictionary = Dictionary<TKey, TPtr<TValue>>;

    template <typename TValue, template<class...> typename TPtr = std::unique_ptr>
    using PtrVector = Vector<TPtr<TValue>>;

    template <typename TValue, template<class...> typename TPtr>
    static Vector<TValue *> get_raw_ptr_vector(const PtrVector<TValue, TPtr>& vector)
    {
        Vector<TValue *> raw_vector;
        raw_vector.reserve(vector.size());
        std::transform(vector.begin(), vector.end(), std::back_inserter(raw_vector),
                       [](const TPtr<TValue>& ptr) { return ptr.get(); });
        return raw_vector;
    }
}

using namespace albc::mem;
#pragma once
#include <new>
#include <algorithm>

namespace albc::mem
{
template<typename Ty, typename... TArg>
constexpr decltype(auto) aligned_new (TArg&&... arg)
{
    return new(std::align_val_t{alignof(Ty)}) Ty(std::forward<TArg>(arg)...);
}

template <typename Ty, typename... TArgs>
constexpr decltype(auto) nothrow_new(TArgs &&...arg) noexcept(noexcept(Ty(std::forward<TArgs>(arg)...)))
{
    return new (std::nothrow) Ty(std::forward<TArgs>(arg)...);
}

template <typename Ty, typename... TArgs>
constexpr decltype(auto) make_unique_nothrow(TArgs &&...arg) noexcept(noexcept(Ty(std::forward<TArgs>(arg)...)))
{
    return std::unique_ptr<Ty>(nothrow_new<Ty>(std::forward<TArgs>(arg)...));
}

template <typename Ty, typename... TArgs>
constexpr decltype(auto) make_shared_nothrow(TArgs &&...arg) noexcept(noexcept(Ty(std::forward<TArgs>(arg)...)))
{
    return std::shared_ptr<Ty>(nothrow_new<Ty>(std::forward<TArgs>(arg)...));
}

template <typename TKey, typename TValue, template <class...> typename TPtr>
constexpr void free_ptr_map(std::map<TKey, TPtr<TValue>> &map)
{
    map.clear();
}

template <typename TKey, typename TValue> constexpr void free_ptr_map(std::map<TKey, TValue *> &map)
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

    template <typename TValue>
    constexpr void free_ptr_list(List<TValue*>& list)
    {
        for (auto& value : list)
        {
            delete value;
        }
        list.clear();
    }

    template <typename TKey, typename TValue, template<class...> typename TPtr = std::unique_ptr>
    using PtrDictionary = Dictionary<TKey, TPtr<TValue>>;

    template <typename TValue, template<class...> typename TPtr = std::unique_ptr>
    using PtrVector = Vector<TPtr<TValue>>;

    template <typename TValue, template<class...> typename TPtr>
    static Vector<TValue *> unwrap_ptr_vector(const PtrVector<TValue, TPtr>& vector)
    {
        Vector<TValue *> raw_vector;
        raw_vector.reserve(vector.size());
        std::transform(vector.begin(), vector.end(), std::back_inserter(raw_vector),
                       [](const TPtr<TValue>& ptr) { return ptr.get(); });
        return raw_vector;
    }
}
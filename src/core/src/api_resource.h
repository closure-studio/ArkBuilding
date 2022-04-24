#pragma once
#include "albc/albc_common.h"
#include "albc_types.h"
#include "util.h"
#include "util_json.h"

namespace albc::api
{
template <typename TIndex, typename TStore,
    std::enable_if_t<std::is_enum_v<TIndex>, bool> = true >
class ResourceStorage
{
    Array<std::shared_ptr<TStore>, util::enum_size_v<TIndex>> storage_;
    std::atomic<UInt32> version_{0};
    std::shared_ptr<TStore>& operator[](TIndex index)
    {
        return storage_[static_cast<std::size_t>(index)];
    }

    const std::shared_ptr<TStore>& operator[](TIndex index) const
    {
        return storage_[static_cast<std::size_t>(index)];
    }

  public:
    [[nodiscard]] constexpr UInt32 GetVersion() const
    {
        return version_;
    }

    constexpr void Add(TIndex index, std::shared_ptr<TStore> store)
    {
        (*this)[index] = store;
    }

    template <typename... Args,
        std::enable_if_t<std::is_constructible_v<TStore, Args...>, bool> = true>
    constexpr void Store(TIndex index, Args&&... args)
    {
        version_++;
        (*this)[index] = std::make_shared<TStore>(std::forward<Args>(args)...);
    }

    std::shared_ptr<TStore> Get(TIndex index) const
    {
        return (*this)[index];
    }

    constexpr bool Has(TIndex index) const
    {
        return (*this)[index] != nullptr;
    }

    template <typename TGet,
              std::enable_if_t<std::is_constructible_v<TGet, TStore>, bool> = true>
    std::shared_ptr<TGet> Resolve(TIndex index) const
    {
        if (!Has(index))
        {
            throw std::runtime_error(
                std::string("Resource of type: ")
                    .append(util::TypeName<TGet>())
                    .append(" not resolvable: Store of index not found: ")
                    .append(util::enum_to_string(index)));
        }

        try
        {
            return std::make_shared<TGet>(*Get(index));
        }
        catch(const std::exception& e)
        {
            LOG_E("Resource resolve failed: index: ", util::enum_to_string(index), " type: ", util::TypeName<TGet>(), " , error: ", e.what());
            throw;
        }
    }

    constexpr void Clear()
    {
        version_++;
        for (auto &item : storage_)
        {
            item.reset();
        }
    }
};

using GameDataJsonStorage = ResourceStorage<AlbcGameDataDbType, Json::Value>;
}
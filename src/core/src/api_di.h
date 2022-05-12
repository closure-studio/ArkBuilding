#pragma once
#include "api_resource.h"
#include "api_storage.h"
#include "boost/di.hpp"
#include "data_character_lookup_table.h"
#include "data_character_meta_table.h"
#include "data_character_resolver.h"
#include "data_character_table.h"
#include "data_skill_lookup_table.h"
#include "algorithm_iface_runner.h"
#include "api_json_io.h"


namespace albc::api::di
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
static const auto& GetInjector()
{
    static auto injector = make_injector(
        boost::di::bind<data::building::BuildingData>().to([] { return GetGlobalGameDataStorage().Resolve<data::building::BuildingData>(ALBC_GAME_DATA_DB_BUILDING_DATA); }),
        boost::di::bind<data::game::CharacterTable>().to([] { return GetGlobalGameDataStorage().Resolve<data::game::CharacterTable>(ALBC_GAME_DATA_DB_CHARACTER_TABLE); }),
        boost::di::bind<data::game::CharacterMetaTable>().to([] { return GetGlobalGameDataStorage().Resolve<data::game::CharacterMetaTable>(ALBC_GAME_DATA_DB_CHAR_META_TABLE); }),
        boost::di::bind<data::game::ICharacterLookupTable>().to<data::game::CharacterLookupTable>(),
        boost::di::bind<data::game::ISkillLookupTable>().to<data::game::SkillLookupTable>(),
        boost::di::bind<data::game::ICharacterResolver>().to<data::game::CharacterResolver>(),
        boost::di::bind<algorithm::iface::IRunner>().to<algorithm::iface::MultiRoomIntegerProgramRunner>().in(boost::di::singleton),
        boost::di::bind<IJsonWriter>().to<JsonWriter>().in(boost::di::singleton),
        boost::di::bind<IJsonReader>().to<JsonReader>().in(boost::di::singleton));

    return injector;
}
#pragma clang diagnostic pop

template <typename TGet>
inline std::shared_ptr<TGet> Resolve()
{
    static std::atomic<UInt32> resource_version{UINT32_MAX};
    static std::shared_ptr<TGet> instance;

    try
    {
        if (resource_version != GetGlobalGameDataStorage().GetVersion())
        {
            instance = GetInjector().template create<std::shared_ptr<TGet>>();
            resource_version = GetGlobalGameDataStorage().GetVersion();
        }
        return instance;
    }
    catch (const std::exception& e)
    {
        LOG_E("Failed to resolve resource of type: ", util::TypeName<TGet>(), ": ", e.what());
        throw;
    }
}
} // namespace albc::api::di
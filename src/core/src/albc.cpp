#define ALBC_IS_INTERNAL
#include "albc/albc.h"
#include "api_impl.h"

#include "albc_config.h"
#include "api_util.h"
#include "game_data_tables.h"
#include "log_util.h"
#include "mem_util.h"
#include "primitive_types.h"

#include <cstdarg>
#include <memory>
#undef ALBC_API
#define ALBC_API [[maybe_unused]] ALBC_PUBLIC

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

//#define ALBC_MEMORY_DEBUG_ENABLED

namespace albc
    ALBC_PUBLIC_NAMESPACE
{
ALBC_API void * malloc(std::size_t size) noexcept
{
    auto ret = std::malloc(size);
#ifdef ALBC_MEMORY_DEBUG_ENABLED
    LOG_D("malloc(", size, ") = ", ret);
#endif
    return ret;
}
ALBC_API void free(void *ptr) noexcept
{
    std::free(ptr);
#ifdef ALBC_MEMORY_DEBUG_ENABLED
    LOG_D("free(", ptr, ")");
#endif
}
ALBC_API void *realloc(void *ptr, std::size_t size) noexcept
{
    auto ret = std::realloc(ptr, size);
#ifdef ALBC_MEMORY_DEBUG_ENABLED
    LOG_D("realloc(", ptr, ", ", size, ") = ", ret);
#endif
    return ret;
}
ALBC_API String::String() noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr,
                                           "calling API") // __FUNCTION__ adn __LINE__ is included in LOG_E macro
}
ALBC_API String::String(const char *str) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl(str);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API String::String(const String &str) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl(*str.impl_);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}

ALBC_API String::String(String &&str) noexcept
{
    impl_ = str.impl_;
    str.impl_ = nullptr;
    // this shouldn't throw
}
ALBC_API String::~String() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API String &String::operator=(const char *str) noexcept
{
    try
    {
        impl_->assign(str);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return *this;
}
ALBC_API String &String::operator=(const String &str) noexcept
{
    if (this != &str)
    {
        try
        {
            impl_->assign(str.impl_->c_str());
        }
        ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    }
    return *this;
}
ALBC_API String &String::operator=(String &&str) noexcept
{
    if (this != &str)
    {
        try
        {
            delete impl_;
            impl_ = str.impl_;
            str.impl_ = nullptr;
        }
        ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    }
    return *this;
}
ALBC_API const char *String::c_str() const noexcept
{
    return impl_->c_str();
}
ALBC_API size_t String::size() const noexcept
{
    return impl_->size();
}

ALBC_API void DoLog(AlbcLogLevel level, const char *message, AlbcException **e_ptr) noexcept
{
    try
    {
        LOG(static_cast<util::LogLevel>(level), message);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void SetLogLevel(AlbcLogLevel level, AlbcException **e_ptr) noexcept
{
    try
    {
        util::GlobalLogConfig::SetLogLevel(static_cast<util::LogLevel>(level));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void FlushLog(AlbcException **e_ptr) noexcept
{
    try
    {
        util::SingletonLogger::instance()->Flush();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void SetLogHandler(AlbcLogHandler handler, AlbcException **e_ptr) noexcept
{
    try
    {
        util::GlobalLogConfig::SetLogCallback(handler);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void SetFlushLogHandler(AlbcFlushLogHandler handler, AlbcException **e_ptr) noexcept
{
    try
    {
        util::GlobalLogConfig::SetFlushLogCallback(handler);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API AlbcLogLevel ParseLogLevel(const char *level, AlbcLogLevel default_level, AlbcException **e_ptr) noexcept
{
    try
    {
        std::string level_str(level);
        std::transform(level_str.begin(), level_str.end(), level_str.begin(), ::toupper);
        return static_cast<AlbcLogLevel>(
            albc::util::parse_enum_string(level_str, static_cast<albc::util::LogLevel>(default_level)));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return default_level;
}
ALBC_API AlbcTestMode ParseTestMode(const char *mode, AlbcTestMode default_mode, AlbcException **e_ptr) noexcept
{
    try
    {
        std::string mode_str(mode);
        std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::toupper);
        return static_cast<AlbcTestMode>(
            albc::util::parse_enum_string(mode_str, static_cast<albc::worker::TestMode>(default_mode)));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return default_mode;
}
ALBC_API bool SetGlobalBuildingDataInternal(const Json::Value &json)
{
    auto bd_ptr = mem::make_shared_nothrow<data::building::BuildingData>(json);
    global_building_data.swap(bd_ptr);
    data::game::SkillLookupTable::Init(*global_building_data);

    LOG_I("Successfully set global building data.");
    return true;
}

ALBC_API bool InitBuildingDataFromJson(const char *json, AlbcException **e_ptr) noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(global_building_data_mutex);
        auto json_obj = util::read_json_from_char_array(json);
        if (json_obj.empty())
        {
            LOG_E("Failed to parse building data JSON.");
            return false;
        }

        return SetGlobalBuildingDataInternal(json_obj);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API void FreeException(AlbcException *e) noexcept
{
    try
    {
        if (e)
        {
            delete[] e->what;
        }
        delete e;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API bool InitBuildingDataFromFile(const char *file_path, AlbcException **e_ptr) noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(global_building_data_mutex);
        auto json_obj = util::read_json_from_file(file_path);
        if (json_obj.empty())
        {
            LOG_E("Failed to read building json file: ", file_path);
            return false;
        }

        return SetGlobalBuildingDataInternal(json_obj);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API void RunTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config,
             AlbcException **e_ptr)
{
    try
    {
        const auto &player_data = util::read_json_from_char_array(player_data_json);
        const auto &game_data = util::read_json_from_char_array(game_data_json);

        auto log_level = static_cast<albc::util::LogLevel>(config->base_parameters.level);

        LOG_I("Running test with log level: ", enum_to_string(log_level));
        {
            const auto sc = SCOPE_TIMER_WITH_TRACE("albc::worker::work");
            albc::worker::launch_test(player_data, game_data, *config);
        }
        LOG_I("Test completed");
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void NotifySkillLookupTableNotInitialized()
{
    LOG_E("Skill lookup table is not initialized. Please call InitBuildingDataFromJson()"
          " or InitBuildingDataFromFile() first.");
}
ALBC_API ICharQuery *QueryChar(const char *skill_key, const char *char_key)
{
    try
    {
        auto result = new CharQueryImpl;
        std::string char_key_str;
        if (char_key)
            char_key_str.assign(char_key);

        if (data::game::SkillLookupTable::IsInitialized())
        {
            result->item = data::game::SkillLookupTable::instance()->QueryCharWithBuff(skill_key, char_key_str);
        }
        else
        {
            static std::once_flag flag;
            std::call_once(flag, NotifySkillLookupTableNotInitialized);
        }

        return result;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API ICharQuery *QueryChar(int n, const char* const* skill_keys, const char *char_key)
{
    try
    {
        auto result = new CharQueryImpl;
        std::string char_key_str;
        if (char_key)
            char_key_str.assign(char_key);

        Vector<std::string> skill_key_strs;
        for (int i = 0; i < n; ++i)
        {
            skill_key_strs.emplace_back(skill_keys[i]);
        }

        if (data::game::SkillLookupTable::IsInitialized())
        {
            result->item = data::game::SkillLookupTable::instance()->QueryCharWithBuffList(skill_key_strs, char_key_str);
        }
        else
        {
            static std::once_flag flag;
            std::call_once(flag, NotifySkillLookupTableNotInitialized);
        }

        return result;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API bool InitCharacterTableInternal(const Json::Value &json_obj)
{
    try
    {
        data::game::CharacterTable::Init(json_obj);

        LOG_I("Character table initialized.");
        return true;
    }
    catch (...)
    {
        LOG_E("Failed to initialize character table");
        throw;
    }
}
ALBC_API bool InitCharacterTableFromJson(const char *json, AlbcException **e_ptr) noexcept
{
    try
    {
        const auto &json_obj = util::read_json_from_char_array(json);
        if (json_obj.empty())
        {
            LOG_E("Failed to parse character table JSON.");
            return false;
        }

        return InitCharacterTableInternal(json_obj);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API bool InitCharacterTableFromFile(const char *file_path, AlbcException **e_ptr) noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(global_building_data_mutex);
        auto json_obj = util::read_json_from_file(file_path);
        if (json_obj.empty())
        {
            LOG_E("Failed to read character table JSON from file: ", file_path);
            return false;
        }

        return InitCharacterTableInternal(json_obj);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API Model *Model::FromFile(const char *player_data_path, AlbcException **e_ptr) noexcept
{
    try
    {
        auto model = new Model();
        model->impl_ = Impl::CreateFromFile(player_data_path);
        return model;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return nullptr;
}
ALBC_API Model *Model::FromJson(const char *player_data_json, AlbcException **e_ptr) noexcept
{
    try
    {
        auto model = new Model();
        model->impl_ = Impl::CreateFromJson(player_data_json);
        return model;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return nullptr;
}
ALBC_API Model::~Model() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API void Model::AddCharacter(Character *character, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddCharacter(character);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void Model::AddRoom(Room *room, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddRoom(room);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void Model::RemoveCharacter(Character *character, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->RemoveCharacter(character);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void Model::RemoveRoom(Room *room, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->RemoveRoom(room);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void Model::SetDblParam(AlbcModelParamType type, double value, AlbcException **e_ptr) noexcept
{
    try
    {
        if (!magic_enum::enum_contains(type))
        {
            throw std::invalid_argument("invalid argument: type of val: " + std::to_string(static_cast<int>(type)));
        }

        util::write_attribute(impl_->model_parameters, type, value);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API IResult *Model::GetResult(AlbcException **e_ptr) noexcept
{
    try
    {
        return impl_->GetResult();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return nullptr;
}
ALBC_API Model::Model(AlbcException **e_ptr) noexcept
{
    try
    {
        impl_ = new Impl();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

ALBC_API Character::Character(const char *identifier) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Character::Impl();
        if (impl_ && identifier)
        {
            impl_->identifier.assign(identifier);
        }
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API Character::~Character() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API Character *Character::FromGameDataId(const char *id) noexcept
{
    try
    {
        auto character = new (std::nothrow) Character(nullptr);
        character->impl_->AssignGameDataId(id);
        return character;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API Character *Character::FromName(const char *name) noexcept
{
    try
    {
        auto character = new (std::nothrow) Character(nullptr);
        character->impl_->AssignName(name);
        return character;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API void Character::SetLevel(int phase, int level, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->SetLevel(phase, level);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void Character::AddSkillByName(const char *name, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddSkillByName(name);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void Character::AddSkillByGameDataId(const char *id, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddSkillByGameDataId(id);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API bool Character::Prepare(AlbcException **e_ptr) noexcept
{
    try
    {
        return impl_->Prepare();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API String Character::GetIdentifier(AlbcException **e_ptr) const noexcept
{
    try
    {
        return String{impl_->identifier.c_str()};
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return String{};
}
ALBC_API void Character::SetMorale(double value, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->SetMorale(value);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

ALBC_API Room::Room(const char *identifier, AlbcRoomType type) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Room::Impl();
        if (impl_)
        {
            impl_->type = static_cast<data::building::RoomType>(type);
            impl_->identifier.assign(identifier);
        }
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API Room::~Room() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API void Room::SetDblParam(AlbcRoomParamType type, double value, AlbcException **e_ptr) noexcept
{
    try
    {
        if (!magic_enum::enum_contains(type))
        {
            throw std::invalid_argument("invalid argument: type of val: " + std::to_string(static_cast<int>(type)));
        }

        util::write_attribute(impl_->params, type, value);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API bool Room::Prepare(AlbcException **e_ptr) noexcept
{
    try
    {
        return impl_->Prepare();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API String Room::GetIdentifier(AlbcException **e_ptr) const noexcept
{
    try
    {
        return String{impl_->identifier.c_str()};
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return String{};
}

ALBC_API ICollection<String> *GetInfo(AlbcException **e_ptr)
{
    try
    {
        return new ICollectionVectorImpl<String>();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return nullptr;
}
ALBC_API bool SetGlobalLocale(const char *locale) noexcept
{
    if (!util::CheckTargetLocale(locale))
        return false;

    util::GlobalLocale::SetLocale(locale);
    return true;
}
} // namespace albc

#pragma clang diagnostic pop
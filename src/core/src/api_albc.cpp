#define ALBC_EXPORTS
#include "albc/albc.h"
#include "api_impl.h"

#include "albc_config.h"
#include "api_util.h"
#include "util_log.h"
#include "util_mem.h"
#include "util_time.h"
#include "albc_types.h"
#include "data_character_table.h"
#include "api_json_params.h"
#include "api_di.h"

#include <memory>

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
ALBC_API_MEMBER String::String() noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr,
                                           "calling API") // __FUNCTION__ adn __LINE__ is included in LOG_E macro
}
ALBC_API_MEMBER String::String(const char *str) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl(str);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API_MEMBER String::String(const String &str) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl(*str.impl_);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}

ALBC_API_MEMBER String::String(String &&str) noexcept
    : impl_(str.impl_)
{
    str.impl_ = nullptr;
    // this shouldn't throw
}
ALBC_API_MEMBER String::~String() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API_MEMBER String &String::operator=(const char *str) noexcept
{
    try
    {
        impl_->assign(str);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return *this;
}
ALBC_API_MEMBER String &String::operator=(const String &str) noexcept
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
ALBC_API_MEMBER String &String::operator=(String &&str) noexcept
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
ALBC_API_MEMBER const char *String::c_str() const noexcept
{
    return impl_->c_str();
}
ALBC_API_MEMBER std::size_t String::size() const noexcept
{
    return impl_->size();
}

ALBC_API void DoLog(AlbcLogLevel level, const char *message, AlbcException **e_ptr) noexcept
{
    try
    {
        LOG_TRACED(static_cast<util::LogLevel>(level), message);
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
ALBC_API void SetLogHandler(AlbcLogHandler handler, void *user_data, AlbcException **e_ptr) noexcept
{
    try
    {
        util::GlobalLogConfig::SetLogCallback(handler, user_data);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void SetFlushLogHandler(AlbcFlushLogHandler handler, void *user_data, AlbcException **e_ptr) noexcept
{
    try
    {
        util::GlobalLogConfig::SetFlushLogCallback(handler, user_data);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API AlbcLogLevel ParseLogLevel(const char *level, AlbcLogLevel default_level, AlbcException **e_ptr) noexcept
{
    try
    {
        std::string level_str(level);
        std::transform(level_str.begin(), level_str.end(), level_str.begin(), toupper);
        return static_cast<AlbcLogLevel>(
            parse_enum_string(level_str, static_cast<util::LogLevel>(default_level)));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return default_level;
}
ALBC_API AlbcTestMode ParseTestMode(const char *mode, AlbcTestMode default_mode, AlbcException **e_ptr) noexcept
{
    try
    {
        std::string mode_str(mode);
        std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), toupper);
        return static_cast<AlbcTestMode>(
            util::parse_enum_string(mode_str, static_cast<algorithm::iface::TestMode>(default_mode)));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return default_mode;
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
void LoadGameDataJson(AlbcGameDataDbType data_type, const char *json, AlbcException **e_ptr)
{
    try
    {
        api::GetGlobalGameDataStorage().Store(data_type, util::read_json_from_char_array(json));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
void LoadGameDataFile(AlbcGameDataDbType data_type, const char *path, AlbcException **e_ptr)
{
    try
    {
        api::GetGlobalGameDataStorage().Store(data_type, util::read_json_from_file(path));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API void RunTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config,
             AlbcException **e_ptr)
{
    try
    {
        const auto &player_data = util::read_json_from_char_array(player_data_json);
        const auto &game_data = util::read_json_from_char_array(game_data_json);

        auto log_level = static_cast<util::LogLevel>(config->base_parameters.level);

        LOG_I("Running test with log level: ", enum_to_string(log_level));
        {
            const auto sc = SCOPE_TIMER_WITH_TRACE("albc::algorithm::iface::RunTest");
            algorithm::iface::launch_test(player_data, game_data, *config);
        }
        LOG_I("Test completed");
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API ICharQuery *QueryChar(const char *skill_key, const char *char_key)
{
    try
    {
        auto result = std::make_unique<CharQueryImpl>();
        std::string char_key_str;
        if (char_key)
            char_key_str.assign(char_key);

        auto i_slt = api::di::Resolve<data::game::ISkillLookupTable>();
        auto i_clt = api::di::Resolve<data::game::ICharacterLookupTable>();
        result->item = i_slt->QueryCharWithBuff(skill_key, char_key_str);
        result->name = String {i_clt->IdToName(result->item.id).c_str() };
        return result.release();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API ICharQuery *QueryChar(int n, const char* const* skill_keys, const char *char_key)
{
    try
    {
        auto result = std::make_unique<CharQueryImpl>();
        std::string char_key_str;
        if (char_key)
            char_key_str.assign(char_key);

        Vector<std::string> skill_key_strs;
        for (int i = 0; i < n; ++i)
            skill_key_strs.emplace_back(skill_keys[i]);

        auto skill_lookup_table = api::di::Resolve<data::game::ISkillLookupTable>();
        auto char_lookup_table = api::di::Resolve<data::game::ICharacterLookupTable>();
        result->item = skill_lookup_table->QueryCharWithBuffList(skill_key_strs, char_key_str);
        result->name = String { char_lookup_table->IdToName(result->item.id).c_str() };
        return result.release();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API_MEMBER Model *Model::FromFile(const char *player_data_path, AlbcException **e_ptr) noexcept
{
    try
    {
        auto model = new Model(e_ptr);
        if (e_ptr && *e_ptr)
            return nullptr;

        if (model)
            model->impl_ = Impl::CreateFromFile(player_data_path);
        return model;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return nullptr;
}
ALBC_API_MEMBER Model *Model::FromJson(const char *player_data_json, AlbcException **e_ptr) noexcept
{
    try
    {
        auto model = new Model(e_ptr);
        if (e_ptr && *e_ptr)
            return nullptr;

        if (model)
            model->impl_ = Impl::CreateFromJson(player_data_json);
        return model;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return nullptr;
}
ALBC_API_MEMBER Model::~Model() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API_MEMBER void Model::AddCharacter(Character *character, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddCharacter(character);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER void Model::AddRoom(Room *room, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddRoom(room);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER void Model::RemoveCharacter(Character *character, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->RemoveCharacter(character);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER void Model::RemoveRoom(Room *room, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->RemoveRoom(room);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER void Model::SetDblParam(AlbcModelParamType type, double value, AlbcException **e_ptr) noexcept
{
    try
    {
        if (!magic_enum::enum_contains(type))
        {
            throw std::invalid_argument("invalid argument: type of val: " + std::to_string(type));
        }

        util::write_attribute(impl_->model_parameters, type, value);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER IResult *Model::GetResult(AlbcException **e_ptr) noexcept
{
    try
    {
        return impl_->GetResult();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return nullptr;
}
ALBC_API_MEMBER Model::Model(AlbcException **e_ptr) noexcept
{
    try
    {
        impl_ = new Impl();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

ALBC_API_MEMBER Character::Character(const char *identifier) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl;
        if (impl_ && identifier)
        {
            impl_->SetIdentifier(identifier);
        }
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API_MEMBER Character::~Character() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API_MEMBER Character *Character::FromGameDataId(const char *id) noexcept
{
    try
    {
        auto character = new (std::nothrow) Character(id);
        if (character)
            character->impl_->AssignGameDataId(id);
        return character;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API_MEMBER Character *Character::FromName(const char *name) noexcept
{
    try
    {
        auto character = new (std::nothrow) Character(name);
        if (character)
            character->impl_->AssignName(name);
        return character;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return nullptr;
}
ALBC_API_MEMBER void Character::SetLevel(int phase, int level, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->SetLevel(phase, level);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER void Character::AddSkillByName(const char *name, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddSkillByName(name);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER void Character::AddSkillByGameDataId(const char *id, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddSkillByGameDataId(id);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER void Character::AddSkillByIcon(const char *icon, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->AddSkillByIcon(icon);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER bool Character::Prepare(AlbcException **e_ptr) noexcept
{
    try
    {
        return impl_->Prepare();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API_MEMBER String Character::GetIdentifier(AlbcException **e_ptr) const noexcept
{
    try
    {
        return String{impl_->GetIdentifier().c_str()};
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return String{};
}
ALBC_API_MEMBER void Character::SetMorale(double value, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_->SetMorale(value);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

ALBC_API_MEMBER Room::Room(const char *identifier, AlbcRoomType type) noexcept
{
    try
    {
        impl_ = new (std::nothrow) Impl();
        if (impl_)
        {
            impl_->SetIdentifier(identifier);
            impl_->SetType((data::building::RoomType)type);
        }
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API_MEMBER Room::~Room() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_API_MEMBER void Room::SetDblParam(AlbcRoomParamType type, double value, AlbcException **e_ptr) noexcept
{
    try
    {
        if (!magic_enum::enum_contains(type))
        {
            throw std::invalid_argument("invalid argument: type of val: " + std::to_string(static_cast<int>(type)));
        }

        impl_->SetDblParam(type, value);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
ALBC_API_MEMBER bool Room::Prepare(AlbcException **e_ptr) noexcept
{
    try
    {
        return impl_->Prepare();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
ALBC_API_MEMBER String Room::GetIdentifier(AlbcException **e_ptr) const noexcept
{
    try
    {
        return String{impl_->GetIdentifier().c_str()};
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
ALBC_API String RunWithJsonParams(const char *json, AlbcException **e_ptr)
{
    using namespace model::buff;
    try
    {
        auto i_json_reader = api::di::Resolve<api::IJsonReader>();
        Json::Value in_params_json_obj = i_json_reader->Read(json);

        const api::JsonInParams in_params(in_params_json_obj);
        api::JsonOutParams out_params;
        algorithm::iface::CustomPackedInput input;

        for (const auto& [ident, room_data]: in_params.rooms)
        {
            try
            {
                algorithm::iface::CustomRoom room;
                room.SetType(room_data.type);
                switch (room_data.type)  // NOLINT(clang-diagnostic-switch-enum)
                {
                case data::building::RoomType::MANUFACTURE:
                    room.room_attributes.prod_type = room_data.prod_type;
                    break;
                case data::building::RoomType::TRADING:
                    room.room_attributes.order_type = room_data.order_type;
                    break;

                case data::building::RoomType::POWER:
                case data::building::RoomType::DORMITORY:
                    break;

                default:
                    LOG_E("Room: ", ident, " has unrecognized type: ", util::enum_to_string(room_data.type));
                    break;
                }

                auto& attr = room.room_attributes;
                attr.prod_cnt = room_data.attributes.prod_cnt;
                attr.base_prod_eff = room_data.attributes.base_prod_eff;
                attr.base_prod_cap = room_data.attributes.base_prod_cap;
                attr.base_char_cost = room_data.attributes.base_char_cost;

                room.SetIdentifier(ident);
                if (room_data.type != data::building::RoomType::DORMITORY)
                {
                    room.SetMaxSlotCnt(room_data.slot_count);
                    room.SetLevel(std::max(room_data.level, room_data.slot_count));
                }
                else
                {
                    room.SetMaxSlotCnt(5);
                    room.SetLevel(room_data.level);
                }

                if (auto opt_room_data = room.GenerateRoomData())
                    input.rooms.emplace_back(std::move(*opt_room_data));
                else
                    throw std::runtime_error("failed to generate room data");
            }
            catch (const std::exception& e)
            {
                LOG_E("Error creating room: ", ident, ": ", e.what());
                out_params.errors.rooms[ident] = e.what();
            }
        }

        auto i_slt = api::di::Resolve<data::game::ISkillLookupTable>();
        auto cmt = api::di::Resolve<data::game::CharacterMetaTable>();
        auto i_cr = api::di::Resolve<data::game::ICharacterResolver>();
        for (const auto& [ident, char_data]: in_params.chars)
        {
            try
            {
                algorithm::iface::CustomCharacter character(i_cr, cmt);

                character.SetIdentifier(ident);

                if (!char_data.name.empty())
                    character.SetIdResolveCond(char_data.name, data::game::CharIdentifierType::NAME);
                else if (!char_data.id.empty())
                    character.SetIdResolveCond(char_data.id, data::game::CharIdentifierType::ID);

                if (char_data.phase > 2 || char_data.level > 90)
                    throw std::invalid_argument(std::string("invalid argument: ") +
                                                "phase: " + std::to_string(char_data.phase) +
                                                ", level: " + std::to_string(char_data.level));

                if (char_data.phase >= 0 && char_data.level >= 0)
                    character.SetLevelCond((data::EvolvePhase)char_data.phase, char_data.level);

                for (const auto& skill_ident: char_data.skills)
                {
                    if (i_slt->HasId(skill_ident))
                        character.AddSkillById(skill_ident);
                    else if (i_slt->HasName(skill_ident))
                        character.AddSkillByName(skill_ident);
                    else if (i_slt->HasIcon(skill_ident))
                        character.AddSkillByIcon(skill_ident);
                    else
                        LOG_W("Unrecognized skill: ", skill_ident, " of character: ", ident);
                }

                character.SetMorale(char_data.morale);
                if (auto opt_char_data = character.GenerateCharacterData())
                    input.characters.emplace_back(std::move(*opt_char_data));
                else
                    throw std::runtime_error("failed to generate character data");
            }
            catch (const std::exception& e)
            {
                LOG_E("Error creating character: ", ident, ": ", e.what());
                out_params.errors.chars[ident] = e.what();
            }
        }

        const auto bd = api::di::Resolve<data::building::BuildingData>();
        algorithm::iface::AlgorithmParams alg_params(input, *bd);

        const auto i_runner = api::di::Resolve<algorithm::iface::IRunner>();
        algorithm::AlgorithmResult result;
        AlbcSolverParameters solver_params {};
        solver_params.solve_time_limit = in_params.solve_time_limit;
        solver_params.model_time_limit = in_params.model_time_limit;
        solver_params.gen_all_solution_details = in_params.gen_sol_details;
        solver_params.gen_lp_file = in_params.gen_lp_file;

        i_runner->Run(alg_params, solver_params, result);
        for (const auto& room: result.rooms)
        {
            api::JsonOutRoomStruct out_room;
            out_room.score = room.solution.productivity;
            out_room.duration = room.solution.duration;
            for (const auto* op: room.solution.operators)
                if (op)
                    out_room.chars.emplace_back(op->identifier);

            out_params.rooms.emplace(room.room->id, std::move(out_room));
        }

        auto i_json_writer = api::di::Resolve<api::IJsonWriter>();
        return String { i_json_writer->Write(static_cast<Json::Value>(out_params)).c_str() };
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return String("{}");
}

} // namespace albc

#pragma clang diagnostic pop
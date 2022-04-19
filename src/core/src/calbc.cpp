#define ALBC_IS_INTERNAL
#include "albc/calbc.h"
#include "albc/albc.h"
#include "api_util.h"
#include "api_impl.h"
#undef CALBC_API
#if _WIN32
#define CALBC_API(ret) ret __cdecl
#else
#define CALBC_API(ret) ret
#endif

#include "worker.h"
#include "json/json.h"
#include "api_util.h"
#include "mem_util.h"

#include <memory>


#define ALBC_PUBLIC_API [[maybe_unused]] ALBC_PUBLIC
static std::unique_ptr<albc::data::building::BuildingData> global_building_data;

CALBC_HANDLE_IMPL(AlbcModel, albc::Model)
CALBC_HANDLE_IMPL(AlbcResult, albc::IResult)
CALBC_HANDLE_IMPL(AlbcRoomResult, albc::IRoomResult)
CALBC_HANDLE_IMPL(AlbcCharacter, albc::Character)
CALBC_HANDLE_IMPL(AlbcRoom, albc::Room)
CALBC_HANDLE_IMPL(AlbcCollection, albc::ICollectionBase)
CALBC_HANDLE_IMPL(AlbcString, albc::String)

namespace albc
{

template <typename THandle, typename TStore, typename TNativeCollection = ICollection<TStore>,
          std::enable_if_t< std::is_base_of_v< AlbcObjectHandleBase, THandle >, bool > = true>
struct [[maybe_unused]] ObjectHandleCollection : public albc::ICollectionVectorImpl<THandle *>
{
    std::unique_ptr<const TNativeCollection> native_collection_;
    std::unique_ptr<albc::mem::PtrVector<TStore>> values_;
  public:
    ObjectHandleCollection(TNativeCollection* native_collection_val, bool take_ownership)
    {
        if (take_ownership)
            native_collection_.reset(native_collection_val);

        constexpr bool do_copy_values = !std::is_pointer_v<TStore>;
        if constexpr (do_copy_values)
            values_.reset(new albc::mem::PtrVector<TStore>());

        for (const auto& val: *native_collection_val)
        {
            if constexpr (do_copy_values)
            {
                auto* new_val = new TStore(val);
                values_->emplace_back(new_val);
                this->emplace_back(new THandle(new_val))->SetRef(true);
            }
            else
            {
                this->emplace_back(new THandle(val))->SetRef(true);
            }
        }
    }

    ~ObjectHandleCollection() override
    {
        albc::mem::free_ptr_vector(*this);
    }
};
} // namespace albc

ALBC_PUBLIC_API
CALBC_API(void) AlbcTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, AlbcException**e_ptr)
{
    albc::RunTest(game_data_json, player_data_json, config, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcInitBuildingDataFromJson(const char *json, AlbcException**e_ptr)
{
    albc::InitBuildingDataFromJson(json, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcInitBuildingDataFromFile(const char *file_path, AlbcException **e_ptr)
{
    albc::InitBuildingDataFromFile(file_path, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcSetLogLevel(AlbcLogLevel level, AlbcException** e_ptr)
{
    albc::SetLogLevel(level, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcFlushLog(AlbcException** e_ptr)
{
    albc::FlushLog(e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(AlbcLogLevel) AlbcParseLogLevel(const char *level, AlbcLogLevel default_level, AlbcException** e_ptr)
{
    return albc::ParseLogLevel(level, default_level, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(AlbcTestMode) AlbcParseTestMode(const char *mode, AlbcTestMode default_mode, AlbcException** e_ptr)
{
    return albc::ParseTestMode(mode, default_mode, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcFreeException(AlbcException *exception)
{
    albc::FreeException(exception);
}

ALBC_PUBLIC_API
CALBC_API(const) char *AlbcGetExceptionMessage(AlbcException *exception)
{
    return exception->what;
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcModelDestroy(AlbcModel *model)
{
    try
    {
        delete model;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcResultDestroy(AlbcResult *result)
{
    try
    {
        delete result;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcCharacterDestroy(AlbcCharacter *character)
{
    try
    {
        delete character;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcRoomDestroy(AlbcRoom *room)
{
    try
    {
        delete room;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_PUBLIC_API
CALBC_API(void) AlbcCollectionDestroy(AlbcCollection *collection)
{
    try
    {
        delete collection;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_PUBLIC_API
CALBC_API(void) AlbcStringDestroy(AlbcString *string)
{
    try
    {
        delete string;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_PUBLIC_API
CALBC_API(void) AlbcRoomResultDestroy(AlbcRoomResult *result)
{
    try
    {
        delete result;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
ALBC_PUBLIC_API
AlbcModel *AlbcModelFromFile(const char *player_data_path, AlbcException **e_ptr)
{
    return new(std::nothrow) AlbcModel { albc::Model::FromFile(player_data_path, e_ptr) };
}

ALBC_PUBLIC_API
AlbcModel *AlbcModelFromEmpty(AlbcException **e_ptr)
{
    return new(std::nothrow) AlbcModel {albc::mem::make_unique_nothrow<albc::Model>(e_ptr) };
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcModelAddChar(AlbcModel *model, AlbcCharacter *character, AlbcException **e_ptr)
{
    auto char_ptr = character->Unwrap();
    model->impl->AddCharacter(char_ptr, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcModelRemoveChar(AlbcModel *model, AlbcCharacter *character, AlbcException **e_ptr)
{
    character->Rebind(character->impl.get());
    model->impl->RemoveCharacter(character->impl.get(), e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcModelAddRoom(AlbcModel *model, AlbcRoom *room, AlbcException **e_ptr)
{
    auto room_ptr = room->Unwrap();
    model->impl->AddRoom(room_ptr, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcModelRemoveRoom(AlbcModel *model, AlbcRoom *room, AlbcException **e_ptr)
{
    room->Rebind(room->impl.get());
    model->impl->RemoveRoom(room->impl.get(), e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcModelSetDblParam(AlbcModel *model, AlbcModelParamType type, double value, AlbcException **e_ptr)
{
    model->impl->SetDblParam(type, value, e_ptr);
}

ALBC_PUBLIC_API
AlbcResult *AlbcModelGetResult(AlbcModel *model, AlbcException **e_ptr)
{
    return new(std::nothrow) AlbcResult { std::unique_ptr<albc::IResult> (model->impl->GetResult(e_ptr)) };
}

ALBC_PUBLIC_API
AlbcCharacter *AlbcCharacterFromId(const char *id)
{
    return new(std::nothrow) AlbcCharacter { std::unique_ptr<albc::Character>(albc::Character::FromGameDataId(id)) };
}

ALBC_PUBLIC_API
AlbcCharacter *AlbcCharacterFromName(const char *name)
{
    return new(std::nothrow) AlbcCharacter { std::unique_ptr<albc::Character>(albc::Character::FromName(name)) };
}

ALBC_PUBLIC_API
AlbcCharacter *AlbcCharacterFromEmpty(const char* identifier)
{
    return new(std::nothrow) AlbcCharacter { albc::mem::make_unique_nothrow<albc::Character>(identifier) };
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcCharacterSetLevel(AlbcCharacter *character, int phase, int level, AlbcException **e_ptr)
{
    character->impl->SetLevel(phase, level, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcCharacterSetMorale(AlbcCharacter *character, double morale, AlbcException **e_ptr)
{
    try
    {
        character->impl->SetMorale(morale);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcCharacterAddSkillById(AlbcCharacter *character, const char* id, AlbcException **e_ptr)
{
    character->impl->AddSkillByGameDataId(id, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcCharacterAddSkillByName(AlbcCharacter *character, const char *name, AlbcException **e_ptr)
{
    character->impl->AddSkillByName(name, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(bool) AlbcCharacterPrepare(AlbcCharacter *character, AlbcException **e_ptr)
{
    return character->impl->Prepare(e_ptr);
}

ALBC_PUBLIC_API
AlbcRoom *AlbcRoomFromType(const char *identifier, AlbcRoomType type)
{
    return new(std::nothrow) AlbcRoom { albc::mem::make_unique_nothrow<albc::Room>(identifier, type) };
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcRoomSetDblParam(AlbcRoom *room, AlbcRoomParamType type, double value, AlbcException **e_ptr)
{
    room->impl->SetDblParam(type, value, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(bool) AlbcRoomPrepare(AlbcRoom *room, AlbcException **e_ptr)
{
    return room->impl->Prepare(e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcDoLog(AlbcLogLevel level, const char *msg, AlbcException **e_ptr)
{
    albc::DoLog(level, msg, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcSetLogHandler(AlbcLogHandler handler, AlbcException **e_ptr)
{
    albc::SetLogHandler(handler, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcSetFlushLogHandler(AlbcFlushLogHandler handler, AlbcException **e_ptr)
{
    albc::SetFlushLogHandler(handler, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcInitCharacterTableFromJson(const char *json, AlbcException **e_ptr)
{
    albc::InitCharacterTableFromJson(json, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcInitCharacterTableFromFile(const char *file_path, AlbcException **e_ptr)
{
    albc::InitCharacterTableFromFile(file_path, e_ptr);
}

ALBC_PUBLIC_API
CALBC_API(AlbcCollection* /*<AlbcString>*/) AlbcGetInfo(AlbcException **e_ptr)
{
    return new AlbcCollection(
        new albc::ObjectHandleCollection<AlbcString, albc::String>(
            albc::GetInfo(e_ptr), true));
}

ALBC_PUBLIC_API
CALBC_API(size_t) AlbcStringGetLength(AlbcString *string)
{
    return string->impl->size();
}

ALBC_PUBLIC_API
CALBC_API(void) AlbcStringCopyTo(AlbcString *string, char *buffer, size_t buffer_size)
{
    ::strncpy(buffer, string->impl->c_str(), buffer_size);
}

ALBC_PUBLIC_API
const char *AlbcStringGetContent(AlbcString *string)
{
    return string->impl->c_str();
}

ALBC_PUBLIC_API
CALBC_API(int) AlbcResultGetStatus(AlbcResult *result)
{
    return result->impl->GetStatus();
}

ALBC_PUBLIC_API
AlbcCollection *AlbcResultGetRooms(AlbcResult *result)
{
    return new AlbcCollection(
        new albc::ObjectHandleCollection<AlbcRoomResult, albc::IRoomResult*>(
            result->impl->GetRoomDetails(), false));
}

ALBC_PUBLIC_API
AlbcString *AlbcRoomResultGetIdentifier(AlbcRoomResult *result)
{
    return new AlbcString(new albc::String(result->impl->GetIdentifier()));
}

ALBC_PUBLIC_API
CALBC_API(double) AlbcRoomResultGetScore(AlbcRoomResult *result)
{
    return result->impl->GetScore();
}

ALBC_PUBLIC_API
CALBC_API(double) AlbcRoomResultGetDuration(AlbcRoomResult *result)
{
    return result->impl->GetDuration();
}

ALBC_PUBLIC_API
AlbcCollection *AlbcRoomResultGetCharacterIdentifiers(AlbcRoomResult *result)
{
    return new AlbcCollection(
        new albc::ObjectHandleCollection<AlbcString, albc::String>(
            result->impl->GetCharacterIdentifiers(), false));
}

ALBC_PUBLIC_API
AlbcString *AlbcRoomResultGetReadableInfo(AlbcRoomResult *result)
{
    return new AlbcString(new albc::String(result->impl->GetReadableInfo()));
}

ALBC_PUBLIC_API
CALBC_API(bool) AlbcSetGlobalLocale(const char *locale)
{
    return albc::SetGlobalLocale(locale);
}

ALBC_PUBLIC_API
CALBC_API(int) AlbcCollectionGetCount(const AlbcCollection *collection)
{
    return collection->impl->GetCount();
}

ALBC_PUBLIC_API
CALBC_API(void)
AlbcCollectionForEach(const AlbcCollection *collection, AlbcForEachCallback callback, void *user_data)
{
    collection->impl->ForEach(callback, user_data);
}

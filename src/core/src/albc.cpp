#include "albc/albc.h"

#include "albc_config.h"
#include "api_utils.h"
#include "log_util.h"
#include "primitive_types.h"
#include "worker.h"
#include <memory>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

static std::shared_ptr<albc::bm::BuildingData> global_building_data;
static std::mutex global_building_data_mutex;
namespace albc
#ifndef ALBC_CONFIG_MSVC
    ALBC_PUBLIC // set up namespace-wide __attribute__((visibility("default"))), but not for MSVC
#endif
{
struct String::Impl : public string
{
    using string::string;
};
String::String() noexcept
{
    try
    {
        impl_ = new Impl();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr,
                                           "calling API") // __FUNCTION__ adn __LINE__ is included in LOG_E macro
}
String::String(const char *str) noexcept
{
    try
    {
        impl_ = new Impl(str);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
String::String(const String &str) noexcept
{
    try
    {
        impl_ = new Impl(str.impl_->c_str());
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}

String::String(String &&str) noexcept
{
    impl_ = str.impl_;
    str.impl_ = nullptr;
    // this shouldn't throw
}
String::~String() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
String &String::operator=(const char *str) noexcept
{
    try
    {
        impl_->assign(str);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
    return *this;
}
String &String::operator=(const String &str) noexcept
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
String &String::operator=(String &&str) noexcept
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
const char *String::c_str() const noexcept
{
    return impl_->c_str();
}
size_t String::size() const noexcept
{
    return impl_->size();
}

void Log(AlbcLogLevel level, const char *message, AlbcException **e_ptr) noexcept
{
    try
    {
        LOG(static_cast<LogLevel>(level), message);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
void SetLogLevel(AlbcLogLevel level, AlbcException **e_ptr) noexcept
{
    try
    {
        GlobalLogConfig::SetLogLevel(static_cast<LogLevel>(level));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
void FlushLog(AlbcException **e_ptr) noexcept
{
    try
    {
        SingletonLogger::instance()->Flush();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
void SetLogCallback(AlbcLogCallback callback, AlbcException **e_ptr) noexcept
{
    try
    {
        GlobalLogConfig::SetLogCallback(callback);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
void SetFlushLogCallback(AlbcFlushLogCallback callback, AlbcException **e_ptr) noexcept
{
    try
    {
        GlobalLogConfig::SetFlushLogCallback(callback);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
AlbcLogLevel ParseLogLevel(const char *level, AlbcLogLevel default_level, AlbcException **e_ptr) noexcept
{
    try
    {
        std::string level_str(level);
        std::transform(level_str.begin(), level_str.end(), level_str.begin(), ::toupper);
        return static_cast<AlbcLogLevel>(
            albc::util::parse_enum_string(level, static_cast<albc::diagnostics::LogLevel>(default_level)));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return default_level;
}
AlbcTestMode ParseTestMode(const char *mode, AlbcTestMode default_mode, AlbcException **e_ptr) noexcept
{
    try
    {
        std::string mode_str(mode);
        std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::toupper);
        return static_cast<AlbcTestMode>(
            albc::util::parse_enum_string(mode, static_cast<albc::worker::TestMode>(default_mode)));
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return default_mode;
}
bool SetGlobalBuildingData(const char *json, AlbcException **e_ptr) noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(global_building_data_mutex);
        auto json_obj = read_json_from_char_array(json);
        if (json_obj.empty())
            return false;

        auto bd_ptr = std::make_shared<bm::BuildingData>(json_obj);
        global_building_data.swap(bd_ptr);
        return true;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
void FreeException(AlbcException *e) noexcept
{
    try
    {
        if (e)
        {
            delete e->what;
        }
        delete e;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
bool ReadGlobalBuildingDataFromFile(const char *file_path, AlbcException **e_ptr) noexcept
{
    try
    {
        std::lock_guard<std::mutex> lock(global_building_data_mutex);
        auto json_obj = read_json_from_file(file_path);
        if (json_obj.empty())
            return false;

        auto bd_ptr = std::make_shared<bm::BuildingData>(json_obj);
        global_building_data.swap(bd_ptr);
        return true;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
    return false;
}
void RunTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config,
              AlbcException **e_ptr)
{
    try
    {
        const auto& player_data = read_json_from_char_array(player_data_json);
        const auto& game_data = read_json_from_char_array(game_data_json);

        auto log_level = static_cast<albc::diagnostics::LogLevel>(config->base_parameters.level);

        LOG_I("Running test with log level: ", to_string(log_level));
        {
            const auto sc = SCOPE_TIMER_WITH_TRACE("albc::worker::work");
            albc::worker::launch_test(player_data, game_data, *config);
        }
        LOG_I("Test completed");
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
class AlbcModel::Impl
{
  private:
    std::shared_ptr<bm::BuildingData> building_data_;
    std::unique_ptr<PlayerDataModel> player_data_;
    Impl(const Json::Value &game_data_json, const Json::Value &player_data_json)
        : building_data_(std::make_shared<bm::BuildingData>(game_data_json)),
          player_data_(std::make_unique<PlayerDataModel>(player_data_json))
    {
    }

  public:
    Impl(const char *game_data_path, const char *player_data_path)
        : Impl(read_json_from_file(game_data_path), read_json_from_file(player_data_path))
    {
    }

    Impl()
    {
        if (!global_building_data)
        {
            throw std::invalid_argument("global building data is not set");
        }

        building_data_ = global_building_data;
        player_data_ = std::make_unique<PlayerDataModel>();
    }

    [[nodiscard]] worker::WorkerParams CreateWorkerParams() const
    {
        return {*player_data_, *building_data_};
    }
};
AlbcModel::AlbcModel(const char *game_data_path, const char *player_data_path, AlbcException **e_ptr) noexcept
{
    try
    {
        impl_ = new Impl(game_data_path, player_data_path);
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
AlbcModel::AlbcModel(AlbcException **e_ptr) noexcept
{
    try
    {
        impl_ = new Impl();
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(e_ptr, "calling API")
}
AlbcModel::~AlbcModel() noexcept
{
    try
    {
        delete impl_;
    }
    ALBC_API_CATCH_AND_TRANSLATE_EXCEPTION(nullptr, "calling API")
}
}

#pragma clang diagnostic pop
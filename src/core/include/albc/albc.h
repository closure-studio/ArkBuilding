// ALBC C++ API, which C API is based on.
#pragma once
#include "albc_api_common.h"
#include <cstddef>

#if !defined(__cplusplus)
#error "Use 'calbc.h' instead of 'albc.h' for C code."
#endif

#if __cplusplus >= 201703L
#define ALBC_NODISCARD [[nodiscard]]
#else
#define ALBC_NODISCARD
#endif

#define ALBC_THROWS AlbcException **e_ptr = nullptr

#if _WIN32
#define ALBC_API(ret) ret __stdcall
#else
#define ALBC_API(ret) ret
#endif

namespace albc
{
ALBC_API (void) FreeException(AlbcException *e) noexcept;
/**
 * 该字符串类是对STL字符串的简单封装，提供了一些常用的操作，为了避免ABI问题。
 * API规范中，字符串入参统一使用UTF8 const char*，出参统一使用UTF8 String。
 */
struct String
{
  public:
    ALBC_API (explicit) String() noexcept;
    ALBC_API (explicit) String(const char *str) noexcept;
    ALBC_API () String(const String &str) noexcept;
    ALBC_API () String(String &&str) noexcept;
    ALBC_API () ~String() noexcept;

    ALBC_API (String) &operator=(const char *str) noexcept;
    ALBC_API (String) &operator=(const String &str) noexcept;
    ALBC_API (String) &operator=(String &&str) noexcept;
    ALBC_API (ALBC_NODISCARD const char) * c_str() const noexcept;
    ALBC_API (ALBC_NODISCARD size_t) size() const noexcept;

  private:
    struct Impl;
    Impl *impl_;
};
class AlbcModel
{
  public:
    /**
     * 根据游戏数据及玩家数据创建一个模型
     * @param game_data_path
     * @param player_data_path
     */
    ALBC_API () AlbcModel(const char *game_data_path, const char *player_data_path, ALBC_THROWS) noexcept;
    /**
     * 创建一个空模型，以备后续添加数据
     */
    ALBC_API (explicit) AlbcModel(ALBC_THROWS) noexcept;
    ALBC_API () ~AlbcModel() noexcept;

  private:
    class Impl;
    Impl *impl_;
};

ALBC_API (bool) SetGlobalBuildingData(const char *json, ALBC_THROWS) noexcept;
ALBC_API (bool) ReadGlobalBuildingDataFromFile(const char *file_path, ALBC_THROWS) noexcept;
ALBC_API (void) Log(AlbcLogLevel level, const char *message, ALBC_THROWS) noexcept;
ALBC_API (void) SetLogLevel(AlbcLogLevel level, ALBC_THROWS) noexcept;
ALBC_API (void) FlushLog(ALBC_THROWS) noexcept;
ALBC_API (void) SetLogCallback(AlbcLogCallback callback, ALBC_THROWS) noexcept;
ALBC_API (void) SetFlushLogCallback(AlbcFlushLogCallback callback, ALBC_THROWS) noexcept;
ALBC_API (AlbcLogLevel) ParseLogLevel(const char *level, AlbcLogLevel default_level, ALBC_THROWS) noexcept;
ALBC_API (AlbcTestMode) ParseTestMode(const char *mode, AlbcTestMode default_mode, ALBC_THROWS) noexcept;
ALBC_API (void)
RunTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, ALBC_THROWS);
} // namespace albc
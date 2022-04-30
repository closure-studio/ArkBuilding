// ALBC API commons
// Language: C
// 该文件为 C API 与 C++ API 公用的部分
#pragma once
#ifndef _ALBC_COMMON_H_
#define _ALBC_COMMON_H_
#define _CRT_SECURE_NO_WARNINGS
#ifndef __cplusplus
#include <stdbool.h>
#endif // __cplusplus

#if defined(__MINGW32__) || defined(__CYGWIN__)
#   define ALBC_CONFIG_WIN_GCC
#endif // defined(__MINGW32__) || defined(__CYGWIN__)

#if defined(ALBC_CONFIG_WIN_GCC) && defined(ALBC_BUILD_DLL)
#define ALBC_EXPORT __attribute__((dllexport))
#define ALBC_EXPORT_CLASS
#define ALBC_EXPORT_MEMBER ALBC_EXPORT
#define ALBC_IMPORT
#define ALBC_IMPORT_CLASS
#define ALBC_IMPORT_MEMBER
#elif __GNUC__ >= 4
#define ALBC_EXPORT __attribute__((visibility("default")))
#define ALBC_EXPORT_CLASS
#define ALBC_EXPORT_MEMBER ALBC_EXPORT
#define ALBC_IMPORT
#define ALBC_IMPORT_CLASS
#define ALBC_IMPORT_MEMBER
#elif _MSC_VER
#define ALBC_EXPORT __declspec(dllexport)
#define ALBC_EXPORT_CLASS __declspec(dllexport)
#define ALBC_EXPORT_MEMBER
#define ALBC_IMPORT __declspec(dllimport)
#define ALBC_IMPORT_CLASS __declspec(dllimport)
#define ALBC_IMPORT_MEMBER
#else
#define ALBC_EXPORT
#define ALBC_EXPORT_CLASS
#define ALBC_IMPORT
#define ALBC_IMPORT_CLASS
#endif // defined(ALBC_CONFIG_WIN_GCC) && defined(ALBC_BUILD_DLL)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum AlbcLogLevel
    {
    ALBC_LOG_LEVEL_ALL = 0,
    ALBC_LOG_LEVEL_DEBUG = 1,
    ALBC_LOG_LEVEL_INFO = 2,
    ALBC_LOG_LEVEL_WARN = 3,
    ALBC_LOG_LEVEL_ERROR = 4,
    ALBC_LOG_LEVEL_NONE = 5,
} AlbcLogLevel;

typedef enum AlbcTestMode
{
    ALBC_TEST_MODE_ONCE = 0,
    ALBC_TEST_MODE_SEQUENTIAL = 1,
    ALBC_TEST_MODE_PARALLEL = 2
} AlbcTestMode;

typedef struct AlbcSolverParameters
{
    bool gen_lp_file;
    bool gen_all_solution_details;
    double solve_time_limit;
    double model_time_limit;
} AlbcSolverParameters;

typedef struct AlbcParameters
{
    AlbcSolverParameters solver_parameters;
    AlbcLogLevel level;
} AlbcParameters;

typedef struct AlbcTestConfig
{
    AlbcParameters base_parameters;
    AlbcTestMode mode;
    int param;
    bool show_all_ops;
} AlbcTestConfig;

typedef struct AlbcException
{
    char *what;
} AlbcException;

typedef enum AlbcRoomType
{
    ALBC_ROOM_NONE = 0,
    ALBC_ROOM_CONTROL = 1 << 0,
    ALBC_ROOM_POWER = 1 << 1,
    ALBC_ROOM_MANUFACTURE = 1 << 2,
    ALBC_ROOM_SHOP = 1 << 3,
    ALBC_ROOM_DORMITORY = 1 << 4,
    ALBC_ROOM_MEETING = 1 << 5,
    ALBC_ROOM_HIRE = 1 << 6,
    ALBC_ROOM_ELEVATOR = 1 << 7,
    ALBC_ROOM_CORRIDOR = 1 << 8,
    ALBC_ROOM_TRADING = 1 << 9,
    ALBC_ROOM_WORKSHOP = 1 << 10,
    ALBC_ROOM_TRAINING = 1 << 11,
    ALBC_ROOM_FUNCTIONAL = 0b111001111110,
    ALBC_ROOM_ALL = (1 << 12) - 1
} AlbcRoomType;

typedef enum AlbcRoomProductType
{
    ALBC_ROOM_PROD_NONE = 0,
    ALBC_ROOM_PROD_GOLD = 1,
    ALBC_ROOM_PROD_RECORD_1 = 2,
    ALBC_ROOM_PROD_RECORD_2 = 2,
    ALBC_ROOM_PROD_RECORD_3 = 2,
    ALBC_ROOM_PROD_ORIGINIUM_SHARD_ORIROCK = 3,
    ALBC_ROOM_PROD_ORIGINIUM_SHARD_DEVICE = 3,
    ALBC_ROOM_PROD_CHIP_1 = 4,
    ALBC_ROOM_PROD_CHIP_2 = 4,
    ALBC_ROOM_PROD_CHIP_3 = 4,
    ALBC_ROOM_PROD_CHIP_4 = 4,
    ALBC_ROOM_PROD_CHIP_5 = 4,
    ALBC_ROOM_PROD_CHIP_6 = 4,
    ALBC_ROOM_PROD_CHIP_7 = 4,
    ALBC_ROOM_PROD_CHIP_8 = 4,
} AlbcRoomProductType;

typedef enum AlbcRoomOrderType
{
    ALBC_ROOM_ORDER_NONE = 0,
    ALBC_ROOM_ORDER_GOLD = 1,
    ALBC_ROOM_ORDER_ORUNDUM = 2
} AlbcRoomOrderType;

typedef enum AlbcModelParamType
{
    ALBC_MODEL_PARAM_DURATION = 0,
    ALBC_MODEL_PARAM_SOLVE_TIME_LIMIT = 1,
} AlbcModelParamType;

typedef enum AlbcRoomParamType
{
    ALBC_ROOM_PARAM_SLOT_COUNT = 0,
    ALBC_ROOM_PARAM_PRODUCT_TYPE = 1,
    ALBC_ROOM_PARAM_ORDER_TYPE = 2,
    ALBC_ROOM_PARAM_PRODUCT_COUNT = 3, // 包含产品及订单数量
    ALBC_ROOM_PARAM_BASE_PRODUCT_CAP = 4, // 产品容量
    ALBC_ROOM_PARAM_BASE_CHAR_COST = 5, // 基础角色心情消耗
    ALBC_ROOM_PARAM_BASE_PROD_EFF = 6, // 基础生产效率
} AlbcRoomParamType;

typedef enum AlbcGameDataDbType
{
    ALBC_GAME_DATA_DB_UNDEFINED = 0,
    ALBC_GAME_DATA_DB_BUILDING_DATA = 1,  // building_data.json, UTF-8 encoded string
    ALBC_GAME_DATA_DB_CHARACTER_TABLE = 2,// character_table_.json, UTF-8 encoded string
    ALBC_GAME_DATA_DB_CHAR_META_TABLE = 3,// char_meta_table.json, UTF-8 encoded string
} AlbcGameDataDbType;

// log callback
typedef bool (*AlbcLogHandler)(unsigned long logger_id, const char *message, void *user_data);
typedef bool (*AlbcFlushLogHandler)(unsigned long logger_id, void *user_data);
typedef void (*AlbcForEachCallback)(int i, const void *item, void *user_data);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ALBC_COMMON_H
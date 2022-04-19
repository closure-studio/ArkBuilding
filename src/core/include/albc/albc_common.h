// ALBC API commons
// Language: C
// 该文件为 C API 与 C++ API 公用的部分
#ifndef __cplusplus
#include <stdbool.h>
#endif

#if defined(__MINGW32__) || defined(__CYGWIN__)
#   define ALBC_CONFIG_WIN_GCC
#endif

#if defined(ALBC_CONFIG_WIN_GCC) && defined(ALBC_BUILD_DLL)
#define ALBC_EXPORT __attribute__((dllexport))
#elif __GNUC__ >= 4
#define ALBC_EXPORT __attribute__((visibility("default")))
#else
#define ALBC_EXPORT
#endif

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ALBC_LOG_LEVEL_ALL = 0,
    ALBC_LOG_LEVEL_DEBUG = 1,
    ALBC_LOG_LEVEL_INFO = 2,
    ALBC_LOG_LEVEL_WARN = 3,
    ALBC_LOG_LEVEL_ERROR = 4,
    ALBC_LOG_LEVEL_NONE = 5,
} AlbcLogLevel;

typedef enum
{
    ALBC_TEST_MODE_ONCE = 0,
    ALBC_TEST_MODE_SEQUENTIAL = 1,
    ALBC_TEST_MODE_PARALLEL = 2
} AlbcTestMode;

typedef struct
{
    bool gen_lp_file;
    bool gen_all_solution_details;
    double solve_time_limit;
    double model_time_limit;
} AlbcSolverParameters;

typedef struct
{
    AlbcSolverParameters solver_parameters;
    AlbcLogLevel level;
} AlbcParameters;

typedef struct
{
    AlbcParameters base_parameters;
    AlbcTestMode mode;
    int param;
    bool show_all_ops;
} AlbcTestConfig;

typedef struct
{
    char *what;
} AlbcException;

typedef enum
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

typedef enum
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

typedef enum
{
    ALBC_ROOM_ORDER_NONE = 0,
    ALBC_ROOM_ORDER_GOLD = 1,
    ALBC_ROOM_ORDER_ORUNDUM = 2
} AlbcRoomOrderType;

typedef enum
{
    ALBC_MODEL_PARAM_DURATION = 0,
    ALBC_MODEL_PARAM_SOLVE_TIME_LIMIT = 1,
} AlbcModelParamType;

typedef enum
{
    ALBC_ROOM_PARAM_SLOT_COUNT = 0,
    ALBC_ROOM_PARAM_PRODUCT_TYPE = 1,
    ALBC_ROOM_PARAM_ORDER_TYPE = 2,
    ALBC_ROOM_PARAM_PRODUCT_COUNT = 3, // 包含产品及订单数量
} AlbcRoomParamType;

// log callback
typedef void (*AlbcLogHandler)(const char * message);
typedef void (*AlbcFlushLogHandler)();
typedef void (*AlbcForEachCallback)(int i, const void* item, void* user_data);

#ifdef __cplusplus
}
#endif
// ALBC C API, wrapper for C++ API
#pragma once
#include "albc_api_common.h"

#define CALBC_HANDLE_DECL(name) \
    struct name;\
    typedef struct name name;

#define CALBC_THROWS AlbcException** e_ptr
#define CALBC_API(ret) ret __cdecl
#ifdef __cplusplus
extern "C"
{
#endif
CALBC_API (const char*) AlbcGetExceptionMessage(AlbcException* exception);
CALBC_API (void) AlbcFreeException(AlbcException* exception);

CALBC_HANDLE_DECL (AlbcModel)
CALBC_HANDLE_DECL (AlbcWorker)
CALBC_HANDLE_DECL (AlbcWorkerParams)
CALBC_HANDLE_DECL (AlbcResult)

CALBC_API (void) AlbcModelDestroy(AlbcModel* model, CALBC_THROWS);
CALBC_API (void) AlbcWorkerDestroy(AlbcWorker* worker, CALBC_THROWS);
CALBC_API (void) AlbcWorkerParamsDestroy(AlbcWorkerParams* params, CALBC_THROWS);
CALBC_API (void) AlbcResultDestroy(AlbcResult* result, CALBC_THROWS);

CALBC_API (AlbcModel*) AlbcModelCreateByFile(const char* game_data_path, const char* player_data_path, CALBC_THROWS);
CALBC_API (AlbcModel*) AlbcModelCreateEmpty(CALBC_THROWS);
CALBC_API (void) AlbcModelAddCharByName(AlbcModel* model, const char* name, CALBC_THROWS);
CALBC_API (void) AlbcModelAddCharById(AlbcModel* model, const char* char_id, CALBC_THROWS);
CALBC_API (void) AlbcModelSetCharCost(AlbcModel* model, const char* name, double cost, CALBC_THROWS);
CALBC_API (void) AlbcModelResolveCharByLevel(AlbcModel* model, const char* name, int phase, int level, CALBC_THROWS);
CALBC_API (void) AlbcModelAppendCharSkill(AlbcModel* model, const char* name, const char* skill_name, CALBC_THROWS);
CALBC_API (void) AlbcModelAppendCharSkillById(AlbcModel* model, const char* name, const char* skill_id, CALBC_THROWS);
CALBC_API (void) AlbcModelResolveCharBySkills(AlbcModel* model, CALBC_THROWS);
CALBC_API (void) AlbcModelAddManuRoom(AlbcModel* model, AlbcRoomType type, AlbcRoomProductType product_type, CALBC_THROWS);
CALBC_API (void) AlbcModelAddTradeRoom(AlbcModel* model, AlbcRoomType type, AlbcRoomOrderType order_type, CALBC_THROWS);
CALBC_API (AlbcWorkerParams*) AlbcModelGetWorkerParams(AlbcModel* model, CALBC_THROWS);

CALBC_API (AlbcWorker*) AlbcWorkerCreateEmpty(CALBC_THROWS);
CALBC_API (AlbcWorker*) AlbcWorkerGetDefault(CALBC_THROWS);
CALBC_API (AlbcResult*) AlbcWorkerRun(AlbcWorker* worker, AlbcWorkerParams* params, CALBC_THROWS);

/**
 * 运行测试
 * @param game_data_json 游戏数据
 * @param player_data_json 玩家数据
 * @param config 配置
 * @remarks 入参及出参由调用方释放，线程安全
 */
CALBC_API (void) AlbcTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, CALBC_THROWS);

/**
 * 初始化游戏基建数据，重复调用将会覆盖之前的数据
 * @param json 游戏基建数据
 * @remarks 线程不安全
 */
CALBC_API (void) AlbcSetGlobalBuildingData(const char *json, CALBC_THROWS);

/**
 * 设置全局日志等级
 * @param level 日志等级
 * @remarks 线程安全
 */
CALBC_API (void) AlbcSetLogLevel(AlbcLogLevel level, CALBC_THROWS);

/**
 * 刷新日志缓冲区（依据日志记录器实现而定，可能不会有任何效果）
 * @remarks 线程安全
 */
CALBC_API (void) AlbcFlushLog(CALBC_THROWS);

/**
 * 解析日志等级字符串
 * @param level 日志等级字符串
 * @param default_level 默认日志等级
 * @return 日志等级，如果解析失败，则返回默认日志等级
 */
CALBC_API (AlbcLogLevel) AlbcParseLogLevel(const char *level, AlbcLogLevel default_level, CALBC_THROWS);

/**
 * 解析测试模式字符串
 * @param mode 测试模式字符串
 * @param default_mode 默认测试模式
 * @return 测试模式，如果解析失败，则返回默认测试模式
 */
CALBC_API (AlbcTestMode) AlbcParseTestMode(const char *mode, AlbcTestMode default_mode, CALBC_THROWS);

#ifdef __cplusplus
}
#endif
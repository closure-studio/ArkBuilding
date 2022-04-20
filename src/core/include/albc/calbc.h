// ALBC C API, wrapper around classes in albc.h
#pragma once
#include "calbc_internal.h"
#include "albc_common.h"
#include "stddef.h" // NOLINT(modernize-deprecated-headers)
#include "stdbool.h" // NOLINT(modernize-deprecated-headers)

#ifdef __cplusplus
extern "C"
{
#endif
// 该API中所有字符串入参、出参均为UTF-8编码

CALBC_HANDLE_DECL (AlbcModel)
CALBC_HANDLE_DECL (AlbcResult)
CALBC_HANDLE_DECL (AlbcRoomResult)
CALBC_HANDLE_DECL (AlbcCharacter)
CALBC_HANDLE_DECL (AlbcRoom)
CALBC_HANDLE_DECL (AlbcCollection)
CALBC_HANDLE_DECL (AlbcString)

/*
 * 资源释放函数
 */

// 释放一个AlbcModel对象
CALBC_API void AlbcModelDel(AlbcModel* model);

// 释放一个AlbcResult对象
CALBC_API void AlbcResultDel(AlbcResult* result);

// 释放一个AlbcCharacter对象
CALBC_API void AlbcCharacterDel(AlbcCharacter* character);

// 释放一个AlbcRoom对象
CALBC_API void AlbcRoomDel(AlbcRoom* room);

// 释放一个AlbcRoomResult对象
CALBC_API void AlbcRoomResultDel(AlbcRoomResult* result);

// 释放一个AlbcCollection对象
CALBC_API void AlbcCollectionDel(AlbcCollection *collection);

// 释放一个AlbcString对象
CALBC_API void AlbcStringDel(AlbcString* string);

/*
 * AlbcCollection 成员函数
 */

// 获取元素数量
CALBC_API int AlbcCollectionGetCount(const AlbcCollection* collection);

// 遍历集合元素
CALBC_API void AlbcCollectionForEach(const AlbcCollection* collection, AlbcForEachCallback callback, void* user_data);

/*
 * AlbcString 成员函数
 */

// 获取字符串长度，不包括结束符
CALBC_API size_t AlbcStringGetLength(AlbcString* string);

// 将字符串内容复制到指定的缓冲区
CALBC_API void AlbcStringCopyTo(AlbcString* string, char* buffer, size_t buffer_size);

// 获取字符串内容，包括结束符
CALBC_API const char* AlbcStringGetContent(AlbcString* string);

/*
 * AlbcModel 成员函数
 */

// 从游戏数据文件、玩家数据文件加载AlbcModel
// （玩家数据是什么，如何获取？如果你不知道或者无法获取，请用下面的FromEmpty创建一个空的模型，并添加角色和房间。此项目不打算解释玩家数据的含义和获取方式。）
CALBC_API AlbcModel* AlbcModelFromFile(const char* player_data_path, CALBC_E_PTR);

// 从空的AlbcModel创建一个新的AlbcModel
CALBC_API AlbcModel* AlbcModelFromEmpty(CALBC_E_PTR);

// 向模型中增加一个角色。添加后对象的生命周期由模型管理。以下所有向模型添加对象的函数同理。
CALBC_API void AlbcModelAddChar(AlbcModel* model, AlbcCharacter* character, CALBC_E_PTR);

// 移除模型中的一个角色。移除后对象的生命周期不再由模型管理。以下所有向模型添加对象的函数同理。
CALBC_API void AlbcModelRemoveChar(AlbcModel* model, AlbcCharacter* character, CALBC_E_PTR);

// 向模型中增加一个房间。
CALBC_API void AlbcModelAddRoom(AlbcModel* model, AlbcRoom* room, CALBC_E_PTR);

// 移除模型中的一个房间。
CALBC_API void AlbcModelRemoveRoom(AlbcModel* model, AlbcRoom* room, CALBC_E_PTR);

// 设置模型参数
CALBC_API void AlbcModelSetDblParam(AlbcModel* model, AlbcModelParamType type, double value, CALBC_E_PTR);

// 获取运行结果
CALBC_API AlbcResult* AlbcModelGetResult(AlbcModel* model, CALBC_E_PTR);

/*
 * AlbcCharacter 成员函数
 */

// 根据角色ID获取一个角色
CALBC_API AlbcCharacter* AlbcCharacterFromId(const char* id);

// 根据角色名称获取一个角色
CALBC_API AlbcCharacter* AlbcCharacterFromName(const char* name);

// 创建一个空的角色，可以随便加Buff。最多4个Buff。
CALBC_API AlbcCharacter* AlbcCharacterFromEmpty(const char* identifier);

// 设置角色的精英阶段和等级
CALBC_API void AlbcCharacterSetLevel(AlbcCharacter* character, int phase, int level, CALBC_E_PTR);

// 设置角色心情值
CALBC_API void AlbcCharacterSetMorale(AlbcCharacter* character, double morale, CALBC_E_PTR);

// 根据技能ID给角色一个技能
CALBC_API void AlbcCharacterAddSkillById(AlbcCharacter* character, const char* id, CALBC_E_PTR);

// 根据技能名称给角色一个技能
CALBC_API void AlbcCharacterAddSkillByName(AlbcCharacter* character, const char* name, CALBC_E_PTR);

// 在添加进AlbcModel之前校验角色
CALBC_API bool AlbcCharacterPrepare(AlbcCharacter* character, CALBC_E_PTR);

/*
 * AlbcRoom 成员函数
 */

// 根据房间类型创建一个房间
CALBC_API AlbcRoom* AlbcRoomFromType(const char* identifier, AlbcRoomType type);

// 设置房间Double类型参数
CALBC_API void AlbcRoomSetDblParam(AlbcRoom* room, AlbcRoomParamType type, double value, CALBC_E_PTR);

// 在添加进AlbcModel之前校验房间
CALBC_API bool AlbcRoomPrepare(AlbcRoom* room, CALBC_E_PTR);


/*
 * AlbcResult 成员函数
 */

// 获取结果状态值
CALBC_API int AlbcResultGetStatus(AlbcResult* result);

// 获取结果中的房间
CALBC_API AlbcCollection* /* <AlbcRoomResult*> */ AlbcResultGetRooms(AlbcResult* result);


/*
 * AlbcRoomResult 成员函数
 */

// 获取房间ID
CALBC_API AlbcString* AlbcRoomResultGetIdentifier(AlbcRoomResult* result);

// 获取产能
CALBC_API double AlbcRoomResultGetScore(AlbcRoomResult* result);

// 获取持续时长
CALBC_API double AlbcRoomResultGetDuration(AlbcRoomResult* result);

// 获取干员列表
CALBC_API AlbcCollection* /* <AlbcString*> */ AlbcRoomResultGetCharacterIdentifiers(AlbcRoomResult* result);

// 获取方案可读描述信息
CALBC_API AlbcString* AlbcRoomResultGetReadableInfo(AlbcRoomResult* result);

/*
 * 全局函数
 */

// 设定输出字符串的编码。
CALBC_API bool AlbcSetGlobalLocale(const char* locale);

// 获取异常信息
CALBC_API const char* AlbcGetExceptionMessage(AlbcException* exception);

// 释放异常
CALBC_API void AlbcFreeException(AlbcException* exception);

// 根据给定的游戏数据和玩家数据运行一次测试
CALBC_API void AlbcTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, CALBC_E_PTR);

// 设置全局基建数据（既 https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/building_data.json
// 文件中的数据）。需要在进行求解前调用。重复设置会覆盖先前的数据。
CALBC_API void AlbcInitBuildingDataFromJson(const char *json, CALBC_E_PTR);

// 从文件中加载上述数据。
CALBC_API void AlbcInitBuildingDataFromFile(const char *file_path, CALBC_E_PTR);

// 设置全局干员数据（见 https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/character_table.json）
// 需要在进行求解前调用。重复设置会覆盖先前的数据。
CALBC_API void AlbcInitCharacterTableFromJson(const char *json, CALBC_E_PTR);

// 从文件中读取上述数据。
CALBC_API void AlbcInitCharacterTableFromFile(const char *file_path, CALBC_E_PTR);

// 设置全局日志等级。从ALL输出所有，到NONE不输出，将输出指定等级（包括）及以上等级的日志
CALBC_API void AlbcSetLogLevel(AlbcLogLevel level, CALBC_E_PTR);

// 清空日志缓冲区。
CALBC_API void AlbcFlushLog(CALBC_E_PTR);

// 打印一条日志。供测试目的用。
CALBC_API void AlbcDoLog(AlbcLogLevel level, const char* msg, CALBC_E_PTR);

// 解析日志等级字符串(ALL, DEBUG, INFO, WARN, ERROR, NONE)，大小写不敏感
CALBC_API AlbcLogLevel AlbcParseLogLevel(const char* level, AlbcLogLevel default_level, CALBC_E_PTR);

// 解析测试模式字符串(ONCE, SEQUENTIAL, PARALLEL)，大小写不敏感
CALBC_API AlbcTestMode AlbcParseTestMode(const char* mode, AlbcTestMode default_mode, CALBC_E_PTR);

// 设置输出日志的方式。设为空指针来还原成默认值。
CALBC_API void AlbcSetLogHandler(AlbcLogHandler handler, CALBC_E_PTR);

// 设置清除日志缓冲区的方式。设为空指针来还原成默认值。
CALBC_API void AlbcSetFlushLogHandler(AlbcFlushLogHandler handler, CALBC_E_PTR);

// 获取信息。测试用。
CALBC_API AlbcCollection* /* <AlbcString*> */ AlbcGetInfo(CALBC_E_PTR);

#ifdef __cplusplus
}
#endif
// ALBC C API, wrapper around classes in albc.h
#pragma once
#ifndef CALBC_H
#define CALBC_H
#include "calbc_internal.h"
#include "albc_common.h"
#include "stddef.h" // NOLINT(modernize-deprecated-headers)
#include "stdbool.h" // NOLINT(modernize-deprecated-headers)

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
// 该API中所有字符串入参、出参均为UTF-8编码

CALBC_HANDLE_DECL (AlbcString)

/*
 * 资源释放函数
 */

// 释放一个AlbcString对象
CALBC_API void AlbcStringDel(AlbcString* string);

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
 * 全局函数
 */

/*
 * JSON API
 */
CALBC_API AlbcString* AlbcRunWithJsonParams(const char* json, CALBC_E_PTR);


// 设定输出字符串的编码。
CALBC_API bool AlbcSetGlobalLocale(const char* locale);

// 获取异常信息
CALBC_API const char* AlbcGetExceptionMessage(AlbcException* exception);

// 释放异常
CALBC_API void AlbcFreeException(AlbcException* exception);

// 根据给定的游戏数据和玩家数据运行一次测试
CALBC_API void AlbcTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, CALBC_E_PTR);

// https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/building_data.json
// https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/character_table.json
// https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/char_meta_table.json
CALBC_API void AlbcLoadGameDataJson(AlbcGameDataDbType type, const char *json, CALBC_E_PTR);
CALBC_API void AlbcLoadGameDataFile(AlbcGameDataDbType type, const char *path, CALBC_E_PTR);

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

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // CALBC_H
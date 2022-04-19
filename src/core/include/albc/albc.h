// ALBC C++ API, which C API is based on.
#pragma once
#include "albc_internal.h"

namespace albc
{
class Character
{
  public:
    // 根据指定标识符（任意标识角色的唯一的字符串）创建一个空的角色。
    ALBC_API (explicit) Character(const char* identifier) noexcept;
    ALBC_API () ~Character() noexcept;
    // 根据游戏数据ID创建一个角色。参考 https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/character_table.json
    ALBC_NODISCARD ALBC_API (static Character *) FromGameDataId(const char* id) noexcept;
    // 根据角色的中文名或者英文名创建一个角色。
    ALBC_NODISCARD ALBC_API (static Character *) FromName(const char* name) noexcept;

    // 设置角色的等级。只对从ID或者名称创建的角色有效。
    ALBC_API (void) SetLevel(int phase, int level, ALBC_E_PTR) noexcept;
    // 设置角色的心情值（0.~24.）。对任意角色有效。
    ALBC_API (void) SetMorale(double value, ALBC_E_PTR) noexcept;
    // 根据技能名称给角色添加一个技能。对任意角色都有效。
    ALBC_API (void) AddSkillByName(const char *name, ALBC_E_PTR) noexcept;
    // 根据技能的ID给角色添加一个技能。对任意角色都有效。参考 https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/building_data.json
    // 中的buffs字段。
    ALBC_API (void) AddSkillByGameDataId(const char *id, ALBC_E_PTR) noexcept;
    // 用以上函数提供的参数并初始化角色。须在将角色添加到房间前调用。
    // 有效的角色参数选项:
    // 1.指定ID或名称，并指定等级。将从游戏数据中解析出角色拥有的技能。若干员Buff未建模则不会计算。
    // 2.指定ID或名称的同时并指定若干个技能，将尝试用技能反推出角色等级，如可反推则相当于选项1，不能反推将会使用角色初始阶段技能（孑：还有这种好事？）
    // 3.只指定若干个技能。将尝试反推出角色及等级，如可反推则相当于选项1，不能反推则也会使用指定的技能代入模型。
    ALBC_API (bool) Prepare(ALBC_E_PTR) noexcept;
    // 获取角色的标识符。与创建角色时的参数一致（标识符，ID或名称）
    ALBC_NODISCARD ALBC_API (String) GetIdentifier(ALBC_E_PTR) const noexcept;

    ALBC_PIMPL
    ALBC_MEM_DELEGATE
};

class Room
{
  public:
    // 根据标识符（为标识房间的任意唯一的字符串）及房间类型创建房间。
    ALBC_API (explicit) Room(const char* identifier, AlbcRoomType type) noexcept;
    ALBC_API () ~Room() noexcept;

    // 设置房间的double类型的参数。
    ALBC_API (void) SetDblParam(AlbcRoomParamType type, double value, ALBC_E_PTR) noexcept;
    // 检查参数并初始化房间。
    ALBC_API (bool) Prepare(ALBC_E_PTR) noexcept;
    // 获取创建时提供的标识符。
    ALBC_NODISCARD ALBC_API (String) GetIdentifier(ALBC_E_PTR) const noexcept;

    ALBC_PIMPL
    ALBC_MEM_DELEGATE
};

class IRoomResult
{
  public:
    // 获取干员标识符列表
    ALBC_NODISCARD ALBC_API (virtual ICollection<String>* /* ref */) GetCharacterIdentifiers() const noexcept = 0;
    // 获取计算出的房间产能
    ALBC_NODISCARD ALBC_API (virtual double) GetScore() const noexcept = 0;
    // 获取房间生成方案的预期可持续时间（从开始到任一干员心情耗尽的间隔）
    ALBC_NODISCARD ALBC_API (virtual double) GetDuration() const noexcept = 0;
    // 获取房间标识符
    ALBC_NODISCARD ALBC_API (virtual String) GetIdentifier() const noexcept = 0;
    // 获取方案信息
    ALBC_NODISCARD ALBC_API (virtual String) GetReadableInfo() const noexcept = 0;
    ALBC_API(virtual) ~IRoomResult() noexcept = default;

    ALBC_MEM_DELEGATE
};

class IResult
{
  public:
    // 获取求解状态。0为正常，其他值出错。
    ALBC_NODISCARD ALBC_API (virtual int) GetStatus() const noexcept = 0;
    // 获取各个房间的生产方案。
    ALBC_NODISCARD ALBC_API (virtual ICollection< IRoomResult* /* ref */ >* /* ref */) GetRoomDetails() const noexcept = 0;
    ALBC_API(virtual) ~IResult() noexcept = default;

    ALBC_MEM_DELEGATE
};

class ICharQuery
{
  public:
    // 该角色查询是否有效
    ALBC_NODISCARD ALBC_API (virtual bool) IsValid() const noexcept = 0;
    // 角色 Id
    ALBC_NODISCARD ALBC_API (virtual String) Id() const noexcept = 0;
    // 角色名称，如果CharacterTable未初始化，则返回空字符串
    ALBC_NODISCARD ALBC_API (virtual String) Name() const noexcept = 0;
    // 精英阶段
    ALBC_NODISCARD ALBC_API (virtual int) Phase() const noexcept = 0;
    // 等级
    ALBC_NODISCARD ALBC_API (virtual int) Level() const noexcept = 0;
    ALBC_API (virtual) ~ICharQuery() noexcept = default;

    ALBC_MEM_DELEGATE
};

class Model
{
  public:
    // 根据玩家数据创建一个模型
    ALBC_API (static Model*) FromFile(const char *player_data_path, ALBC_E_PTR) noexcept;
    // 根据玩家数据创建一个模型
    ALBC_API (static Model*) FromJson(const char *player_data_json, ALBC_E_PTR) noexcept;
    // 创建一个空模型，以备后续添加数据
    ALBC_API (explicit) Model(ALBC_E_PTR) noexcept;
    ALBC_API () ~Model() noexcept;

    // 向模型添加一个干员。
    ALBC_API (void) AddCharacter(Character *character, ALBC_E_PTR) noexcept;
    // 移除模型中的一个干员。
    ALBC_API (void) RemoveCharacter(Character *character, ALBC_E_PTR) noexcept;
    // 向模型添加一个房间。
    ALBC_API (void) AddRoom(Room *room, ALBC_E_PTR) noexcept;
    // 移除模型中的一个房间。
    ALBC_API (void) RemoveRoom(Room *room, ALBC_E_PTR) noexcept;

    // 设置double类型的模型参数。
    ALBC_API (void) SetDblParam(AlbcModelParamType type, double value, ALBC_E_PTR) noexcept;
    // 对模型求解。
    ALBC_API (IResult *) GetResult(ALBC_E_PTR) noexcept;

    ALBC_PIMPL
    ALBC_MEM_DELEGATE
};

// 释放异常。
ALBC_API (void) FreeException(AlbcException *e) noexcept;

// 设定输出字符串使用的编码。（目前使用iconv实现，可用的编码名称为iconv所支持的编码名称。）
ALBC_API (bool) SetGlobalLocale(const char* locale) noexcept;

// 以下所有文件/JSON字符串输入函数需为UTF-8编码。

// 设置全局干员数据（见 https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/character_table.json）
// 需要在进行求解前调用。重复设置会覆盖先前的数据。
// CharacterTable需要先于BuildingData初始化，或者在只初始化BuildingData的情况下，初始化CharacterTable后再次初始化BuildingData。
ALBC_API (bool) InitCharacterTableFromJson(const char *json, ALBC_E_PTR) noexcept;

// 从文件中读取上述数据。
ALBC_API (bool) InitCharacterTableFromFile(const char *file_path, ALBC_E_PTR) noexcept;

// 设置全局基建数据（既 https://github.com/Kengxxiao/ArknightsGameData/blob/master/zh_CN/gamedata/excel/building_data.json
// 文件中的数据）。需要在进行求解前调用。重复设置会覆盖先前的数据。
ALBC_API (bool) InitBuildingDataFromJson(const char *json, ALBC_E_PTR) noexcept;

// 从文件中读取上述数据。
ALBC_API (bool) InitBuildingDataFromFile(const char *file_path, ALBC_E_PTR) noexcept;

// 根据单个技能的ID或名称查询角色。支持提供角色ID或名称来进行更加精确的查询。
// 需要初始化BuildingData。如果使用角色名字作为char_key，或则需要在查询结果中获取角色名字，还需初始化CharacterTable。
ALBC_API (ICharQuery*) QueryChar(const char *skill_key, const char* char_key = nullptr);

// 根据多个技能的ID或名称查询角色。约定同上。
ALBC_API (ICharQuery*) QueryChar(int n, const char* const* skill_keys, const char *char_key = nullptr);

// 打印一条日志。供测试目的用。
ALBC_API (void) DoLog(AlbcLogLevel level, const char *message, ALBC_E_PTR) noexcept;

// 设置全局日志等级。从ALL输出所有，到NONE不输出，将输出指定等级（包括）及以上等级的日志
ALBC_API (void) SetLogLevel(AlbcLogLevel level, ALBC_E_PTR) noexcept;

// 清空日志缓冲区。
ALBC_API (void) FlushLog(ALBC_E_PTR) noexcept;

// 设置输出日志的方式。设为空指针来还原成默认值。
ALBC_API (void) SetLogHandler(AlbcLogHandler handler, ALBC_E_PTR) noexcept;

// 设置清空日志缓冲区的方式。设为空指针来还原成默认值。
ALBC_API (void) SetFlushLogHandler(AlbcFlushLogHandler handler, ALBC_E_PTR) noexcept;

// 解析日志等级字符串(ALL, DEBUG, INFO, WARN, ERROR, NONE)，大小写不敏感
ALBC_API (AlbcLogLevel) ParseLogLevel(const char *level, AlbcLogLevel default_level, ALBC_E_PTR) noexcept;

// 解析测试模式字符串(ONCE, SEQUENTIAL, PARALLEL)，大小写不敏感
ALBC_API (AlbcTestMode) ParseTestMode(const char *mode, AlbcTestMode default_mode, ALBC_E_PTR) noexcept;

// 根据给定的游戏数据和玩家数据运行一次测试
ALBC_API (void) RunTest(const char *game_data_json, const char *player_data_json, const AlbcTestConfig *config, ALBC_E_PTR);

ALBC_API (ICollection<String>*) GetInfo(ALBC_E_PTR);
} // namespace albc
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma once
#include "game_data_model.h"
#include "json_util.h"
#include "primitive_types.h"
#include "util.h"

namespace albc::bm
{
static constexpr size_t kRoomTypeCount = 12;

enum class RoomType
{
    NONE = 0,
    CONTROL = 1 << 0,
    POWER = 1 << 1,
    MANUFACTURE = 1 << 2,
    SHOP = 1 << 3,
    DORMITORY = 1 << 4,
    MEETING = 1 << 5,
    HIRE = 1 << 6,
    ELEVATOR = 1 << 7,
    CORRIDOR = 1 << 8,
    TRADING = 1 << 9,
    WORKSHOP = 1 << 10,
    TRAINING = 1 << 11,
    FUNCTIONAL = 0b111001111110,
    ALL = (1 << 12) - 1
};

struct SlotItem
{
    string buff_id;
    UnlockCondition cond;

    explicit SlotItem(const Json::Value &json);
};

class BuildingBuffCharSlot
{
  public:
    Vector<SlotItem> buff_data;

    explicit BuildingBuffCharSlot(const Json::Value &json);
};

class BuildingCharacter
{
  public:
    string char_id;
    Int64 max_man_power;
    PtrVector<BuildingBuffCharSlot> buff_char;

    explicit BuildingCharacter(const Json::Value &json);
};

class BuildingBuff
{
  public:
    string buff_id;
    string buff_name;
    int sort_id;
    RoomType room_type;
    string description;

    explicit BuildingBuff(const Json::Value &json);
};

class BuildingData
{
  public:
    PtrDictionary<string, BuildingCharacter> chars;
    PtrDictionary<string, BuildingBuff> buffs;

    explicit BuildingData(const Json::Value &json);
};
} // namespace albc::bm

template <> struct magic_enum::customize::enum_range<albc::bm::RoomType>
{
    static constexpr bool is_flags = true;
    static constexpr int min = 0;
    static constexpr int max = 4095;
};
#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma once
#include "data_game.h"
#include "util_json.h"
#include "albc_types.h"
#include "util.h"

namespace albc::data::building
{
enum class RoomType;
}

template <>
struct magic_enum::customize::enum_range<albc::data::building::RoomType>
{
    static constexpr bool is_flags = true;
    static constexpr int min = 0;
    static constexpr int max = 4095;
};

namespace albc::data::building
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
    FUNCTIONAL = 0b1110'0111'1110,
    ALL = (1 << 12) - 1
};

struct SlotItem
{
    std::string buff_id;
    UnlockCondition cond;

    explicit SlotItem(const Json::Value &json);
    SlotItem() = default;
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
    std::string char_id;
    Int64 max_man_power;
    mem::PtrVector<BuildingBuffCharSlot> buff_char;

    explicit BuildingCharacter(const Json::Value &json);
};

class BuildingBuff
{
  public:
    std::string buff_id;
    std::string buff_name;
    int sort_id;
    RoomType room_type;
    std::string description;

    explicit BuildingBuff(const Json::Value &json);
};

class BuildingData
{
  public:
    mem::PtrDictionary<std::string, BuildingCharacter> chars;
    mem::PtrDictionary<std::string, BuildingBuff> buffs;

    BuildingData() = default;
    explicit BuildingData(const Json::Value &json);
};
} // namespace albc::data::building

#pragma clang diagnostic pop
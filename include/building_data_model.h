#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma once
#include "util.h"
#include "primitive_types.h"
#include "json_util.h"
#include "game_data_model.h"

namespace albc::bm
{
	enum class RoomType {
		NONE = 0,
		CONTROL = 1,
		POWER = 2,
		MANUFACTURE = 4,
		SHOP = 8,
		DORMITORY = 16,
		MEETING = 32,
		HIRE = 64,
		ELEVATOR = 128,
		CORRIDOR = 256,
		TRADING = 512,
		WORKSHOP = 1024,
		TRAINING = 2048,
		FUNCTIONAL = 3710,
		ALL = 4095
	};

	struct SlotItem
	{
		string buff_id;
		UnlockCondition cond;

		explicit SlotItem(const Json::Value& json);
	};

	class BuildingBuffCharSlot
	{
	public:
		Vector<SlotItem> buff_data;

		explicit BuildingBuffCharSlot(const Json::Value& json);
	};

	class BuildingCharacter
	{
	public:
		string char_id;
		Int64 max_man_power;
		PtrVector<BuildingBuffCharSlot> buff_char;

		explicit BuildingCharacter(const Json::Value& json);
	};

	class BuildingBuff
	{
	public:
		string buff_id;
		string buff_name;
		int sort_id;
		RoomType room_type;
		string description;

		explicit BuildingBuff(const Json::Value& json);
	};

	class BuildingData
	{
	public:
		PtrDictionary<string, BuildingCharacter> chars;
		PtrDictionary<string, BuildingBuff> buffs;

		explicit BuildingData(const Json::Value& json);
	};
}

template <>
struct magic_enum::customize::enum_range<albc::bm::RoomType>
{
	static constexpr bool is_flags = true;
	static constexpr int min = 0;
	static constexpr int max = 4095;
};
#pragma clang diagnostic pop
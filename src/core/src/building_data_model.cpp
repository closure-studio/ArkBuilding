#include "building_data_model.h"

namespace albc::data::building
{
SlotItem::SlotItem(const Json::Value& json):
	buff_id(json["buffId"].asString()),
	cond(UnlockCondition(json["cond"]))
{ }

BuildingBuffCharSlot::BuildingBuffCharSlot(const Json::Value& json):
	buff_data(util::json_val_as_vector<SlotItem>(json["buffData"]))
{ }

BuildingCharacter::BuildingCharacter(const Json::Value& json):
	char_id(json["charId"].asString()),
	max_man_power(json["maxManpower"].asInt64()),
	buff_char(util::json_val_as_ptr_vector<BuildingBuffCharSlot>(json["buffChar"]))
{ }

BuildingBuff::BuildingBuff(const Json::Value &json) :
	buff_id(json["buffId"].asString()),
	buff_name(json["buffName"].asString()),
	sort_id(json["sortId"].asInt()),
	room_type(util::json_string_as_enum(json["roomType"], RoomType::NONE)),
	description(json["description"].asString())
{ }

BuildingData::BuildingData(const Json::Value& json):
	chars(util::json_val_as_ptr_dictionary<BuildingCharacter>(json["chars"])),
	buffs(util::json_val_as_ptr_dictionary<BuildingBuff>(json["buffs"]))
{ }
}
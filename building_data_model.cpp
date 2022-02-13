#include "building_data_model.h"

albc::bm::SlotItem::SlotItem(const Json::Value& json):
	buff_id(json["buffId"].asString()),
	cond(UnlockCondition(json["cond"]))
{ }

albc::bm::BuildingBuffCharSlot::BuildingBuffCharSlot(const Json::Value& json):
	buff_data(json_val_as_vector<SlotItem>(json["buffData"]))
{ }

albc::bm::BuildingCharacter::BuildingCharacter(const Json::Value& json):
	char_id(json["charId"].asString()),
	max_man_power(json["maxManpower"].asInt64()),
	buff_char(json_val_as_ptr_vector<BuildingBuffCharSlot>(json["buffChar"]))
{ }

albc::bm::BuildingBuff::BuildingBuff(const Json::Value &json) : 
	buff_id(json["buffId"].asString()),
	buff_name(json["buffName"].asString()),
	sort_id(json["sortId"].asInt()),
	room_type(json_string_as_enum(json["roomType"], RoomType::NONE)),
	description(json["description"].asString())
{ }

albc::bm::BuildingData::BuildingData(const Json::Value& json):
	chars(json_val_as_ptr_dictionary<BuildingCharacter>(json["chars"])),
	buffs(json_val_as_ptr_dictionary<BuildingBuff>(json["buffs"]))
{ }

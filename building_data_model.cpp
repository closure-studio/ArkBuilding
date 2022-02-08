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
	buff_char(json_val_as_vector<BuildingBuffCharSlot *>(json["buffChar"], json_ctor<BuildingBuffCharSlot>))
{ }

albc::bm::BuildingBuff::BuildingBuff(const Json::Value &json) : 
	buff_id(json["buffId"].asString()),
	buff_name(json["buffName"].asString()),
	sort_id(json["sortId"].asInt()),
	room_type(magic_enum::enum_cast<RoomType>(json["roomType"].asString()).has_value()
					? magic_enum::enum_cast<RoomType>(json["roomType"].asString()).value()
					: RoomType::NONE),
	description(json["description"].asString())
{ }

albc::bm::BuildingData::BuildingData(const Json::Value& json):
	chars(json_val_as_dictionary<BuildingCharacter *>(json["chars"], json_ctor<BuildingCharacter>)),
	buffs(json_val_as_dictionary<BuildingBuff *>(json["buffs"], json_ctor<BuildingBuff>))
{ }

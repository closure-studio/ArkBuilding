#include "player_data_model.h"

namespace albc
{
	PlayerCharacter::PlayerCharacter(const Json::Value& json):
		inst_id(json["instId"].asInt()),
		char_id(json["charId"].asString()),
		level(json["level"].asInt()),
		exp(json["exp"].asInt()),
		evolve_phase(json_val_as_enum<EvolvePhase>(json["evolvePhase"]))
	{}

	PlayerTroop::PlayerTroop(const Json::Value& json):
		chars(json_val_as_dictionary<PlayerCharacter *>(json["chars"], json_ctor<PlayerCharacter>))
	{}

	PlayerDataModel::PlayerDataModel(const Json::Value& json):
		troop(new PlayerTroop(json["troop"])),
		building(new bm::PlayerBuilding(json["building"]))
	{}
}

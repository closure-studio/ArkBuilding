#pragma once

#include "primitive_types.h"
#include "player_building_model.h"
#include "building_data_model.h"
#include "json_util.h"

namespace albc
{
	class PlayerCharacter
	{
	public:
		int inst_id;
		string char_id;
		int level;
		int exp;
		EvolvePhase evolve_phase;

		explicit PlayerCharacter(const Json::Value& json);
	};

	class PlayerTroop
	{
	public:
		Dictionary<string, PlayerCharacter *> chars;

		explicit PlayerTroop(const Json::Value& json);
	};

	class PlayerDataModel
	{
	public:
		PlayerTroop* troop;
		bm::PlayerBuilding* building;

		explicit PlayerDataModel(const Json::Value& json);
	};
}

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

		PlayerCharacter() = default;
		explicit PlayerCharacter(const Json::Value& json);
	};

	class PlayerTroop
	{
	public:
		PtrDictionary<string, PlayerCharacter> chars;

        PlayerTroop() = default;
		explicit PlayerTroop(const Json::Value& json);
	};

	class PlayerDataModel
	{
	public:
		PlayerTroop troop;
		bm::PlayerBuilding building;

        PlayerDataModel() = default;
		explicit PlayerDataModel(const Json::Value& json);
	};

    class PlayerTroopLookup
    {
    public:
        explicit PlayerTroopLookup(const PlayerTroop& troop);

        [[nodiscard]] int GetInstId(const string& char_id) const;

        [[nodiscard]] string GetCharId(int inst_id) const;

    private:
        Dictionary<string, int> char_id_to_inst_id;
        Dictionary<int, string> inst_id_to_char_id;
    };
}

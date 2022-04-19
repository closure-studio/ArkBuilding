#pragma once

#include "primitive_types.h"
#include "player_building_model.h"
#include "building_data_model.h"
#include "json_util.h"

namespace albc::data::player
{
	class PlayerCharacter
	{
	public:
		int inst_id = -1;
		std::string char_id;
		int level = 0;
		int exp = 0;
		EvolvePhase evolve_phase = EvolvePhase::PHASE_0;

		PlayerCharacter() = default;
		explicit PlayerCharacter(const Json::Value& json);
	};

	class PlayerTroop
	{
	public:
		mem::PtrDictionary<std::string, PlayerCharacter> chars;

        PlayerTroop() = default;
		explicit PlayerTroop(const Json::Value& json);
	};

	class PlayerDataModel
	{
	public:
		PlayerTroop troop;
		PlayerBuilding building;

        PlayerDataModel() = default;
		explicit PlayerDataModel(const Json::Value& json);
	};

    class PlayerTroopLookup
    {
    public:
        explicit PlayerTroopLookup(const PlayerTroop& troop);
        explicit PlayerTroopLookup(const Vector<std::pair<int, std::string>> &troop_lookup);

        [[nodiscard]] int GetInstId(const std::string& char_id) const;

        [[nodiscard]] std::string GetCharId(int inst_id) const;

    private:
        Dictionary<std::string, int> char_id_to_inst_id;
        Dictionary<int, std::string> inst_id_to_char_id;
    };
}

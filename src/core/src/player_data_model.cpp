#include "player_data_model.h"

namespace albc::data::player
{
	PlayerCharacter::PlayerCharacter(const Json::Value& json):
		inst_id(json["instId"].asInt()),
		char_id(json["charId"].asString()),
		level(json["level"].asInt()),
		exp(json["exp"].asInt()),
		evolve_phase(util::json_val_as_enum<EvolvePhase>(json["evolvePhase"]))
	{}

	PlayerTroop::PlayerTroop(const Json::Value& json):
		chars(util::json_val_as_ptr_dictionary<PlayerCharacter>(json["chars"]))
	{}

    PlayerDataModel::PlayerDataModel(const Json::Value& json):
		troop(json["troop"]),
		building(json["building"])
	{}

    PlayerTroopLookup::PlayerTroopLookup(const PlayerTroop& troop)
    {
        for (const auto& [id, char_data] : troop.chars)
        {
            inst_id_to_char_id.insert( { char_data->inst_id, char_data->char_id } );
            char_id_to_inst_id.insert( { char_data->char_id, char_data->inst_id } );
        }
    }

    PlayerTroopLookup::PlayerTroopLookup(const Vector<std::pair<int, std::string>> &troop_lookup)
    {
        for (const auto& [inst_id, char_id] : troop_lookup)
        {
            inst_id_to_char_id.insert( { inst_id, char_id } );
            char_id_to_inst_id.insert( { char_id, inst_id } );
        }
    }

    std::string PlayerTroopLookup::GetCharId(int inst_id) const
    {
        auto it = inst_id_to_char_id.find(inst_id);
        if (it == inst_id_to_char_id.end())
        {
            return "";
        }
        return it->second;
    }

    int PlayerTroopLookup::GetInstId(const std::string& char_id) const
    {
        auto it = char_id_to_inst_id.find(char_id);
        if (it == char_id_to_inst_id.end())
        {
            return -1;
        }
        return it->second;
    }
    }

// ReSharper disable StringLiteralTypo
#include "worker.h"
#include "player_data_model.h"
#include "buff_def.h"
#include "locale_util.h"
#include "algorithm.h"
#include "xml_util.h"
namespace albc::worker
{
	void work(const string& player_data_path, const string& game_data_path, int logLevel)
	{
		bool verbose = logLevel <= Logger::DEBUG;

        LOG_I << "Initializing internal buff models" << std::endl;
		init_buffs();
		LOG_I << "Loaded " << buffs.size() << " internal building buff models" << std::endl;

		bm::BuildingData* building_data;
		PlayerDataModel* player_data_model;

		LOG_I << "Parsing game building data file: " << game_data_path << std::endl;
		try
		{
		const Json::Value& building_data_json = read_json_from_file(game_data_path);
		building_data = new bm::BuildingData(building_data_json);
		LOG_I << "Loaded " << building_data->chars.size() << " building character definitions." << std::endl;
		LOG_I << "Loaded " << building_data->buffs.size() << " building buff definitions." << std::endl;
		}
		catch (const std::exception& e)
		{
			LOG_E << "Error: Unable to parse game building data file: " << e.what() << std::endl;
			return;
		}

		int unsupported_buff_cnt = 0;
		for (const auto& [buff_id, buff] : building_data->buffs)
		{
			if (buffs.count(buff_id) <= 0)
			{
				if (verbose)
				{
					std::cout << "\"" << buff->buff_id
							  << "\": " << toOSCharset(buff->buff_name)
							  << ": " << toOSCharset(xml::strip_xml_tags(buff->description)) << std::endl;
				}
				++unsupported_buff_cnt;
			}
		}
		if (!verbose)
		{
			LOG_W << unsupported_buff_cnt <<
 R"( unsupported buff found in building data buff definitions. Add "--verbose"("-v") param to check all.)" << std::endl;
		}


		LOG_I << "Parsing player data file: " << player_data_path << std::endl;
		try
		{
            const Json::Value& player_data_json = read_json_from_file(player_data_path);
            player_data_model = new PlayerDataModel(player_data_json);
            LOG_I << "Added " << player_data_model->troop->chars.size() << " existing character instance" << std::endl;
            LOG_I << "Added " << player_data_model->building->player_building_room->manufacture.size() << " factories." << std::endl;
            LOG_I << "Added " << player_data_model->building->player_building_room->trading.size() << " trading posts." << std::endl;
            LOG_I << "Player building data parsing completed." << std::endl;
		}
		catch(std::exception& e)
		{
			LOG_E << "Error: Unable to parse player data file: " << e.what() << std::endl;
			return;
		}

		LOG_I << "Data feeding completed." << std::endl;
		LOG_I << "Building abstract buff model..." << std::endl;

		Vector<OperatorModel*> operators;
		/*for (const auto& pair : player_data_model->troop->chars)
			{
				if(player_data_model->building->chars.count(pair.first) <= 0)
				{
					LOG_E << "Unable to find troop character in building characters! : " << pair.second->char_id;
					continue;
				}
	
				if(building_data->chars.count(pair.second->char_id) <= 0)
				{
					LOG_E << "Unable to find building data definition for character with given ID! Is building data outdated? : " << pair.second->char_id;
					continue;
				}
	
				const auto& provider = new AbstractAgent(strtol(pair.first.c_str(), nullptr, 0), pair.second->char_id, bm::RoomType::NONE);
				operators.push_back(provider);
	
				for (const auto& buff_char : building_data->chars[pair.second->char_id]->buff_char)
				{
					for(auto it = buff_char->buff_data.rbegin(); it != buff_char->buff_data.rend(); ++it)
					{
						const auto& buff_data = *it;
						if(pair.second->evolve_phase < buff_data.cond.phase || pair.second->level < buff_data.cond.level)
							continue;
	
						if(!buffs.count(buff_data.buff_id))
							continue;
	
						if(!building_data->buffs.count(buff_data.buff_id))
						{
							LOG_E << "Unable to find \"" << buff_data.buff_id << "\" in buff definitions of building data!";
							continue;
						}
	
						const auto& buff = buffs[buff_data.buff_id]->Clone();
						buff->owner_inst_id = provider->inst_id;
						buff->owner_char_id = provider->char_id;
						buff->buff_id = buff_data.buff_id;
						buff->sort_id = building_data->buffs[buff_data.buff_id]->sort_id;
						provider->buffs.push_back(buff);
						provider->room_type_mask = merge_flag(provider->room_type_mask, buff->room_type);
	
						if(verbose)
							LOG_I << "Added [" << std::setw(5) << buff->sort_id << "] \"" << buff_data.buff_id << "\" buff for character: " << pair.second->char_id;
						break;
					}
				}
	
				if(check_flag(provider->room_type_mask, bm::RoomType::MANUFACTURE))
					factory_providers.push_back(provider);
			}*/

		//用于生成全角色最高等级技能
		for (const auto& [char_id, char_inst] : building_data->chars)
		{
			const auto& provider = new OperatorModel(strtol(char_id.c_str(), nullptr, 0), char_inst->char_id,
			                                         bm::RoomType::NONE);
			operators.push_back(provider);

			static auto& char_existing_buff_type = *new Set<RoomBuffType>;
			char_existing_buff_type.clear();

			for (auto it = char_inst->buff_char.rbegin(); it != char_inst->buff_char.rend(); ++it)
			{
				const auto& buff_char = *it;
				if (buff_char->buff_data.empty())
				{
					continue;
				}

				const auto& buff_data = buff_char->buff_data.back();

				if (buffs.count(buff_data.buff_id) <= 0)
				{
					continue;
				}

				const auto& buff_model = buffs[buff_data.buff_id];

				if (char_existing_buff_type.count(buff_model->inner_type) != 0U)
				{
					continue;
				}

				char_existing_buff_type.insert(buff_model->inner_type);

				const auto& buff = buff_model->Clone();
				buff->owner_inst_id = provider->inst_id;
				buff->owner_char_id = provider->char_id;
				buff->buff_id = buff_data.buff_id;
				buff->sort_id = building_data->buffs[buff_data.buff_id]->sort_id;
				provider->buffs.push_back(buff);
				provider->room_type_mask = merge_flag(provider->room_type_mask, buff->room_type);

				if (verbose)
				{
					LOG_I << "Added [" << std::setw(5) << buff->sort_id << "] \"" << buff_data.buff_id <<
 "\" buff for character: " << char_inst->char_id << std::endl;
				}
			}
		}

		LOG_I << "Abstract buff model build completed. Added " << operators.size() << " agents." << std::endl;
		LOG_I << "Calculating..." << std::endl;

		//for (const auto& pair : player_data_model->building->player_building_room->manufacture)
		//{}
		{
			auto *const room = new RoomModel();
			room->type = bm::RoomType::TRADING;
			room->room_attributes.order_type = OrderType::GOLD;
			room->max_slot_count = 5;
			room->room_attributes.base_prod_eff = 1.03;
			room->id = "fake_room";

			const auto &alg = new BruteForce(*new Vector<RoomModel *>({room}), operators);
			auto sc = SCOPE_TIMER_WITH_TRACE("Trading room: Order: Gold");
			alg->Run(); // run the algorithm
		}

		{
			auto *const room = new RoomModel();
			room->type = bm::RoomType::TRADING;
			room->room_attributes.order_type = OrderType::ORUNDUM;
			room->max_slot_count = 5;
			room->room_attributes.base_prod_eff = 1.03;
			room->id = "fake_room";

			const auto &alg = new BruteForce(*new Vector<RoomModel *>({room}), operators);
			auto sc = SCOPE_TIMER_WITH_TRACE("Trade room: Order: ORUNDUM");
			alg->Run(); // run the algorithm
		}

		{
			auto *const room = new RoomModel();
			room->type = bm::RoomType::MANUFACTURE;
			room->room_attributes.prod_type = ProdType::RECORD;
			room->max_slot_count = 5;
			room->room_attributes.base_prod_eff = 1.03;
			room->id = "fake_room";

			const auto &alg = new BruteForce(*new Vector<RoomModel *>({room}), operators);
			auto sc = SCOPE_TIMER_WITH_TRACE("Manufacture room: Prod: RECORD");
			alg->Run(); // run the algorithm
		}

		{
			auto *const room = new RoomModel();
			room->type = bm::RoomType::MANUFACTURE;
			room->room_attributes.prod_type = ProdType::GOLD;
			room->max_slot_count = 5;
			room->room_attributes.base_prod_eff = 1.03;
			room->id = "fake_room";

			const auto &alg = new BruteForce(*new Vector<RoomModel *>({room}), operators);
			auto sc = SCOPE_TIMER_WITH_TRACE("Manufacture room: Prod: GOLD");
			alg->Run(); // run the algorithm
		}

		{
			auto *const room = new RoomModel();
			room->type = bm::RoomType::MANUFACTURE;
			room->room_attributes.prod_type = ProdType::ORIGINIUM_SHARD;
			room->max_slot_count = 5;
			room->room_attributes.base_prod_eff = 1.03;
			room->id = "fake_room";

			const auto &alg = new BruteForce(*new Vector<RoomModel *>({room}), operators);
			auto sc = SCOPE_TIMER_WITH_TRACE("Manufacture room: Prod: ORIGINIUM_SHARD");
			alg->Run(); // run the algorithm
		}
	}
}

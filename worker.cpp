// ReSharper disable StringLiteralTypo
#include "worker.h"
#include "player_data_model.h"
#include "buff_def.h"
#include "locale_util.h"
#include "algorithm.h"
namespace albc::worker
{
	void work(const string& player_data_path, const string& game_data_path, int logLevel)
	{
		bool verbose = logLevel <= Logger::DEBUG;

        LOG_I << "Initializing internal buff models" << std::endl;
		LOG_I << "Loaded " << BuffMap::instance()->size() << " internal building buff models" << std::endl;

		std::shared_ptr<bm::BuildingData> building_data;
        std::shared_ptr<PlayerDataModel> player_data_model;

		LOG_I << "Parsing game building data file: " << game_data_path << std::endl;
		try
		{
		const Json::Value& building_data_json = read_json_from_file(game_data_path);
		building_data = std::make_shared<bm::BuildingData>(building_data_json);
		LOG_I << "Loaded " << building_data->chars.size() << " building character definitions." << std::endl;
		LOG_I << "Loaded " << building_data->buffs.size() << " building buff definitions." << std::endl;
		}
		catch (const std::exception& e)
		{
			LOG_E << "Error: Unable to parse game building data file: " << e.what() << std::endl;
			throw;
		}

		int unsupported_buff_cnt = 0;
		for (const auto& [buff_id, buff] : building_data->buffs)
		{
			if (BuffMap::instance()->count(buff_id) <= 0)
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
 R"( unsupported buff found in building data buff definitions. Add "--debug"("-d") param to check all.)" << std::endl;
		}


		LOG_I << "Parsing player data file: " << player_data_path << std::endl;
		try
		{
            const Json::Value& player_data_json = read_json_from_file(player_data_path);
            player_data_model = std::make_shared<PlayerDataModel>(player_data_json);
            LOG_I << "Added " << player_data_model->troop.chars.size() << " existing character instance" << std::endl;
            LOG_I << "Added " << player_data_model->building.player_building_room.manufacture.size() << " factories." << std::endl;
            LOG_I << "Added " << player_data_model->building.player_building_room.trading.size() << " trading posts." << std::endl;
            LOG_I << "Player building data parsing completed." << std::endl;
		}
		catch(std::exception& e)
		{
			LOG_E << "Error: Unable to parse player data file: " << e.what() << std::endl;
			throw;
		}

		LOG_I << "Data feeding completed." << std::endl;
		LOG_I << "Building abstract buff model..." << std::endl;

		Vector<OperatorModel*> operators;
        auto lookup = std::make_shared<PlayerTroopLookup>(player_data_model->troop);

		for (const auto& [inst_id, player_char] : player_data_model->troop.chars)
        {
            if(player_data_model->building.chars.count(inst_id) <= 0)
            {
                LOG_E << "Unable to find troop character in building characters! : " << player_char->char_id << endl;
                continue;
            }
            if(building_data->chars.count(player_char->char_id) <= 0)
            {
                LOG_E << "Unable to find building data definition for character with given ID! Is building data outdated? : "
                    << player_char->char_id << endl;
                continue;
            }

            const auto& op = new OperatorModel(*player_char, *player_data_model->building.chars.at(inst_id),
                                                *building_data);
            op->Empower(*lookup, *player_char, *building_data, false, true);
            operators.push_back(op);
        }

		LOG_I << "Abstract buff model build completed. Added " << operators.size() << " agents." << std::endl;
		LOG_I << "Calculating..." << std::endl;

        RoomModel trade_room;
        trade_room.type = bm::RoomType::TRADING;
        trade_room.max_slot_count = 3;
        trade_room.room_attributes.base_prod_eff = 1.03;
        trade_room.id = "fake_room";
        write_attribute(trade_room.global_attributes, GlobalAttributeType::DORM_SUM_LEVEL, 25);

        {
            trade_room.room_attributes.order_type = OrderType::GOLD;

            BruteForce alg(Vector<RoomModel *>({&trade_room}), operators);
            auto sc = SCOPE_TIMER_WITH_TRACE("Trading room: Order: Gold");
			alg.Run(); // run the algorithm
		}

		
		{
            trade_room.room_attributes.order_type = OrderType::ORUNDUM;

            BruteForce alg(Vector<RoomModel *>({&trade_room}), operators);
            auto sc = SCOPE_TIMER_WITH_TRACE("Trade room: Order: ORUNDUM");
			alg.Run(); // run the algorithm
		}

        RoomModel manu_room;
        manu_room.type = bm::RoomType::MANUFACTURE;
        manu_room.max_slot_count = 3;
        manu_room.room_attributes.base_prod_eff = 1.03;
        manu_room.id = "fake_room";
        write_attribute(manu_room.global_attributes, GlobalAttributeType::POWER_PLANT_CNT, 3);

        {
			manu_room.room_attributes.prod_type = ProdType::RECORD;

            BruteForce alg(Vector<RoomModel *>({&manu_room}), operators);
            auto sc = SCOPE_TIMER_WITH_TRACE("Manufacture room: Prod: RECORD");
			alg.Run(); // run the algorithm
		}

		{
			manu_room.room_attributes.prod_type = ProdType::GOLD;

            BruteForce alg(Vector<RoomModel *>({&manu_room}), operators);
            auto sc = SCOPE_TIMER_WITH_TRACE("Manufacture room: Prod: GOLD");
			alg.Run(); // run the algorithm
		}

		{
			manu_room.room_attributes.prod_type = ProdType::ORIGINIUM_SHARD;

            BruteForce alg(Vector<RoomModel *>({&manu_room}), operators);
            auto sc = SCOPE_TIMER_WITH_TRACE("Manufacture room: Prod: ORIGINIUM_SHARD");
			alg.Run(); // run the algorithm
		}
	}
}

#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>

#include "albcexample.h"
#include "albc/albc.h"
#include "util.h"

void run(const char* building_data_path, const char *character_table_path, const char *char_meta_table_path, const char *player_data_path)
{
    albc::SetLogLevel(ALBC_LOG_LEVEL_ALL);
    albc::LoadGameDataFile(ALBC_GAME_DATA_DB_BUILDING_DATA, building_data_path, THROW_ON_ALBC_ERROR);
    albc::LoadGameDataFile(ALBC_GAME_DATA_DB_CHARACTER_TABLE, character_table_path, THROW_ON_ALBC_ERROR);
    albc::LoadGameDataFile(ALBC_GAME_DATA_DB_CHAR_META_TABLE, char_meta_table_path, THROW_ON_ALBC_ERROR);
    std::ifstream player_data(player_data_path);
    std::string player_data_str((std::istreambuf_iterator<char>(player_data)), std::istreambuf_iterator<char>());
    std::unique_ptr<albc::Model> model(albc::Model::FromJson(player_data_str.c_str(), THROW_ON_ALBC_ERROR));
    std::unique_ptr<albc::IResult> result(model->GetResult(THROW_ON_ALBC_ERROR));
    for (const auto& room: *result->GetRoomDetails())
    {
        std::cout
            << "Room: "        << room->GetIdentifier().c_str()
            << ", Score: "      << room->GetScore()
            << ", Duration: "   << room->GetDuration()
            << std::endl;

        for (const auto& char_id: *room->GetCharacterIdentifiers())
        {
            std::cout
                << "\t- " << char_id.c_str()
                << std::endl;
        }
    }
    albc::FlushLog();
}

int albc_example_main(const char* building_data_path, const char *character_table_path, const char *char_meta_table_path, const char *player_data_path)
{
    try
    {
        run(building_data_path, character_table_path, char_meta_table_path, player_data_path);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
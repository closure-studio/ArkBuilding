#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include "albc/albc.h"
#include "util.h"


void run()
{
#ifdef _WIN32
    albc::SetGlobalLocale("gbk");
#endif
    albc::SetLogLevel(ALBC_LOG_LEVEL_ALL);
    albc::LoadGameDataFile(ALBC_GAME_DATA_DB_CHARACTER_TABLE, "../test/character_table.json", THROW_ON_ALBC_ERROR);
    albc::LoadGameDataFile(ALBC_GAME_DATA_DB_BUILDING_DATA, "../test/building_data.json", THROW_ON_ALBC_ERROR);
    albc::LoadGameDataFile(ALBC_GAME_DATA_DB_CHAR_META_TABLE, "../test/char_meta_table.json", THROW_ON_ALBC_ERROR);
    std::ifstream player_data("../test/player_data.json");
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

int albc_example_main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
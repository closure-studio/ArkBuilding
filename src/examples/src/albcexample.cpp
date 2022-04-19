#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <fstream>
#include "albc/albc.h"

struct AlbcExceptionDelegate
{
    AlbcException *e = nullptr;
    AlbcException** operator()() { return &e; }

    ~AlbcExceptionDelegate() noexcept(false)
    {
        if (!e)
            return;

        try
        {
            std::string msg(e->what);
            albc::FreeException(e);
            throw std::runtime_error(msg);
        }
        catch (...)
        {
            if (!std::uncaught_exceptions())
                std::rethrow_exception(std::current_exception());
        }
    }
    // 语言丁真，鉴定为：坏文明
    // 推荐使用C风格的异常处理（详见C接口样例）
    // 此处仅为了省事以及示例代码的简洁
};

#define THROW_ON_ALBC_ERROR AlbcExceptionDelegate()()

void run()
{
#ifdef _WIN32
    albc::SetGlobalLocale("gbk");
#endif
    albc::InitCharacterTableFromFile("../test/character_table.json", THROW_ON_ALBC_ERROR);
    albc::InitBuildingDataFromFile("../test/building_data.json", THROW_ON_ALBC_ERROR);
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
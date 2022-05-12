#include "albcexample.h"
extern "C"
{
#include "c_albcexample.h"
}

#include <iostream>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef ERROR
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::string test_data_dir;
    if (std::filesystem::exists("test"))
    {
        test_data_dir = "test";
    }
    else if (std::filesystem::exists("../test"))
    {
        test_data_dir = "../test";
    }
    else
    {
        std::cerr << "Test data path not found." << std::endl;
        return 1;
    }

    std::string building_data_path = test_data_dir + "/building_data.json";
    std::string character_table_path = test_data_dir + "/character_table.json";
    std::string char_meta_table_path = test_data_dir + "/char_meta_table.json";
    std::string player_data_path = test_data_dir + "/player_data.json";
    std::string test_data_path = test_data_dir + "/test_data.jsonc";
    
    std::cout <<
        "Enter a number: \n"
        "1: C++\n"
        "2: C\n"
        "3: Exit\n"
        ">";

    int choice;
    std::cin >> choice;
    std::ifstream test_data_file(test_data_path);
    std::string test_data((std::istreambuf_iterator<char>(test_data_file)),
                            std::istreambuf_iterator<char>());

    switch (choice)
    {
        case 1:
            albc_example_main(building_data_path.c_str(), character_table_path.c_str(), char_meta_table_path.c_str(), player_data_path.c_str());
            break;
        case 2:
            c_albc_example_main(building_data_path.c_str(), character_table_path.c_str(), char_meta_table_path.c_str(), test_data.c_str());
            break;

        default:
            std::cout << "Invalid choice\n";
            break;
    }
    return 0;
}
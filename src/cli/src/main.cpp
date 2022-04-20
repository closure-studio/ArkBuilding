#include "albc/albc.h"

#include <fstream>

#define PROGRAMOPTIONS_NO_COLORS
#include "ProgramOptions/ProgramOptions.hxx"

#ifdef _WIN32
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#   undef ERROR
#endif


void read_file_to_ss(const std::string &filename, std::stringstream &ss)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }
    ss << ifs.rdbuf();
    std::cout << "Read file: " << filename << ", size: " << ss.str().size() << std::endl;
}

int main(const int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    po::parser parser;
    std::string game_data, player_data, character_table;
    std::stringstream player_data_json, game_data_json, character_table_json;
    std::string log_level_str;
    std::string model_time_limit_str = "57600";
    std::string solve_time_limit_str = "60";
    std::string albc_test_mode_str;
    std::string albc_test_param_str = "0";

    // add options to parser
    // add playerdata and gamedata to parser
    parser["playerdata"]
        .abbreviation('p')
        .description(
            "Path to player data file.\n"
            "PATH                            : string")
        .bind(player_data);

    parser["gamedata"]
        .abbreviation('g')
        .description("Path to Arknights building data file.\n"
                     "PATH                            : string")
        .bind(game_data);

    parser["character-table"]
        .abbreviation('c')
        .description("Path to character table file.\n"
                     "PATH                            : string")
        .bind(character_table);

    parser["log-level"]
        .abbreviation('l')
        .description("Log level.\nDefault is WARN.\n"
                     "<ALL|DEBUG|INFO|WARN|ERROR|NONE>: string")
        .bind(log_level_str);

    parser["model-max-time"]
        .abbreviation('t')
        .description(
            "Model time limit in seconds.\n"
            "Default is 57600 (16 hours). \n"
            "TIME                            : double")
        .bind(model_time_limit_str);

    parser["sovle-max-time"]
        .abbreviation('T')
        .description("Problem solving timeout in seconds.\n"
                     "Default is 60. \n"
                     "TIME                            : double")
        .bind(solve_time_limit_str);

    parser["test-mode"]
        .abbreviation('m')
        .description("Test mode. Leave empty for normal mode.\n"
                     "<ONCE|SEQUENTIAL|PARALLEL>      : string")
        .bind(albc_test_mode_str);

    parser["test-param"]
        .abbreviation('P')
        .description("Test param.\n"
                     "NUM_CONCURRENCY|NUM_ITERATIONS  : int")
        .bind(albc_test_param_str);

    auto &gen_lp = parser["lp-file"].abbreviation('L').description(
        "Generate a lp-format file describing the problem.         : FLAG");

    auto &gen_sol_details = parser["solution-detail"].abbreviation('S').description(
        "Generate a text file describing all feasible solution.    : FLAG");

    auto &all_ops = parser["all-ops"].abbreviation('a').description(
        "Show all operators info.                                  : FLAG");

    // add help to parser
    auto &help = parser["help"].abbreviation('h').description(
        "Produce help message.                                     : FLAG");

    // parse command line
    if (!parser.parse(argc, argv))
    {
        std::cerr << "Error: Unable to parse commandline args!" << std::endl;
        return -1;
    }

    if (help.was_set())
    {
        std::cout << parser << std::endl;
        return 0;
    }

    bool test_enabled = !albc_test_mode_str.empty();
    AlbcTestMode test_mode = albc::ParseTestMode(albc_test_mode_str.c_str(), ALBC_TEST_MODE_ONCE);
    // check if all required options are set
    try
    {
        // print missing options name
        if (player_data.empty())
        {
            std::cerr << "must specify the path to player data file!" << std::endl;
            throw std::invalid_argument("playerdata");
        }
        if (game_data.empty())
        {
            std::cerr << "must specify the path to building data file!" << std::endl;
            throw std::invalid_argument("gamedata");
        }
        if (character_table.empty())
        {
            std::cerr << "must specify the path to character table file!" << std::endl;
            throw std::invalid_argument("character-table");
        }
        if (test_enabled && test_mode != ALBC_TEST_MODE_ONCE && !parser["test-param"].was_set())
        {
            std::cerr << "must specify the test param!" << std::endl;
            throw std::invalid_argument("test-param");
        }
    } catch (const std::invalid_argument& e)
    {
        std::cerr << "Error: Missing required options! : " << e.what() << std::endl;
        return -1;
    }

    try
    {
        std::cout << "Main process started." << std::endl;
        std::cout << "Reading game data file: " << game_data << std::endl;
        read_file_to_ss(game_data, game_data_json);
        read_file_to_ss(character_table, character_table_json);
        albc::InitCharacterTableFromJson(character_table_json.str().c_str());
        albc::InitBuildingDataFromJson(game_data_json.str().c_str());
        std::cout << "Reading player data file: " << player_data << std::endl;
        read_file_to_ss(player_data, player_data_json);

        if (test_enabled)
        {
            std::cout << "Test mode: " << albc_test_mode_str << std::endl;
            std::cout << "Test param: " << albc_test_param_str << std::endl;
            std::cout << "Running test..." << std::endl;

            auto test_cfg = std::make_unique<AlbcTestConfig>();
            test_cfg->base_parameters.level = albc::ParseLogLevel(log_level_str.c_str(), ALBC_LOG_LEVEL_WARN);
            test_cfg->mode = test_mode;
            test_cfg->param = std::stoi(albc_test_param_str);
            test_cfg->show_all_ops = all_ops.was_set();

            auto &sp = test_cfg->base_parameters.solver_parameters;
            sp.gen_lp_file = gen_lp.was_set();
            sp.gen_all_solution_details = gen_sol_details.was_set();
            sp.model_time_limit = std::stod(model_time_limit_str);
            sp.solve_time_limit = std::stod(solve_time_limit_str);
            albc::RunTest(game_data_json.str().c_str(), player_data_json.str().c_str(), test_cfg.get());
        }
        else // if (test_enabled)
        {
            // TODO: normal mode
            std::cout << "Normal mode" << std::endl;
        }

        albc::FlushLog();
        std::cout << "Main process successfully completed." << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
}
#include <chrono>
#include <pthread.h>
#include <fstream>

#include "albc/capi.h"

#define PROGRAMOPTIONS_NO_COLORS
#include "ProgramOptions/ProgramOptions.hxx"

void read_file_to_ss(const std::string& filename, std::stringstream& ss)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }
    ss << ifs.rdbuf();
    std::cout << "Read file: " << filename << ", size: " << ss.str().size() << std::endl;
}

int main(const int argc, char* argv[])
{    
    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);

    po::parser parser;
    std::string game_data;
    std::string player_data;
    std::string log_level_str;
    std::stringstream player_data_json, game_data_json;
    std::string model_time_limit_str;
    std::string solve_time_limit_str;

    // add options to parser
    // add playerdata and gamedata to parser
    parser["playerdata"]
        .abbreviation('p')
        .description("path to player data file")
        .bind(player_data);

    parser["gamedata"]
        .abbreviation('g')
        .description("path to arknights building data file")
        .bind(game_data);

    parser["log-level"]
        .abbreviation('l')
        .description("log level")
        .bind(log_level_str);

    parser["model-max-time"]
        .abbreviation('t')
        .description("how long should simulator ")
        .bind(model_time_limit_str);

    parser["sovle-max-time"]
        .abbreviation('T')
        .description("problem solving timeout")
        .bind(solve_time_limit_str);

    auto& gen_lp = parser["lp-file"]
        .abbreviation('L')
        .description("generate a lp-format file describing the problem");

    auto& gen_sol_details = parser["solution-detail"]
        .abbreviation('S')
        .description("generate a text file describing all feasible solution");

    auto &all_ops = parser["all-ops"]
        .abbreviation('a')
        .description("show all operators info");

    // add help to parser
    auto& help = parser["help"]
        .abbreviation('h')
        .description("produce help message");

    auto& go_test = parser["go-test"]
        .description("run golang API test");

    auto& parallel_test = parser["parallel-test"]
        .description("run parallel test");

    auto& seq_test = parser["seq-test"]
        .description("run sequential test");

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

    // check if all required options are set
    if (player_data.empty() || game_data.empty())
    {
        std::cerr << "Error: Missing required options!" << std::endl;
        // print missing options name
        if (player_data.empty())
        {
            std::cerr << "must specify the path to player data file!" << std::endl;
        }
        if (game_data.empty())
        {
            std::cerr << "must specify the path to building data file!" << std::endl;
        }

        std::cout << parser << std::endl;
        return -1;
    }
	try
	{
        std::cout << "Main process started." << std::endl;
        auto level = AlbcParseLogLevel(log_level_str.c_str(), ALBC_LOG_LEVEL_WARN);
        auto test_cfg = std::make_unique<AlbcTestConfig>();
        test_cfg->base_parameters.level = level;

        if (parallel_test.was_set())
        {
            test_cfg->mode = ALBC_TEST_MODE_PARALLEL;
        }
        else if (seq_test.was_set())
        {
            test_cfg->mode = ALBC_TEST_MODE_SEQUENTIAL;
        }
        else
        {
            test_cfg->mode = ALBC_TEST_MODE_ONCE;
        }

        if (gen_lp.was_set())
        {
            test_cfg->base_parameters.solver_parameters.gen_lp_file = true;
        }

        if (gen_sol_details.was_set())
        {
            test_cfg->base_parameters.solver_parameters.gen_all_solution_details = true;
        }

        double model_time_limit = 3600 * 16, solve_time_limit = 20;

        if (!model_time_limit_str.empty())
        {
            model_time_limit = std::stod(model_time_limit_str);
        }

        if (!solve_time_limit_str.empty())
        {
            solve_time_limit = std::stod(solve_time_limit_str);
        }

        test_cfg->base_parameters.solver_parameters.model_time_limit = model_time_limit;
        test_cfg->base_parameters.solver_parameters.solve_time_limit = solve_time_limit;

        std::cout << "Reading player data file: " << player_data << std::endl;
        read_file_to_ss(player_data, player_data_json);

        std::cout << "Reading game data file: " << game_data << std::endl;
        read_file_to_ss(game_data, game_data_json);

        AlbcSetGlobalBuildingData(game_data_json.str().c_str());
        AlbcTest(game_data_json.str().c_str(), player_data_json.str().c_str(), test_cfg.get());
        std::cout << "Main process successfully completed." << std::endl;
		return 0;
	}
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    //AlbcFlushLog();
}
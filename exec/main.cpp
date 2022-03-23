#if defined(NDEBUG) && defined(__clang__)
#pragma clang optimize on
#endif // DEBUG

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include <chrono>
#include "worker.h"
#include <pthread.h>

#define PROGRAMOPTIONS_NO_COLORS
#include "ProgramOptions/ProgramOptions.hxx"

int main(const int argc, char* argv[])
{    
    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);

#ifdef _WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
    
    //print build info
    std::cout << "Build: " << __DATE__ << " " << __TIME__
              << ", Debug: " << is_debug_build()
              << ", System: " << system_name()
              << ", " << system_architecture() << std::endl;
    po::parser parser;
    std::string game_data;
    std::string player_data;
    std::string log_level_str;
    auto log_level = albc::diagnostics::LogLevel::WARN;

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

    log_level = albc::util::parse_enum_string(log_level_str, albc::diagnostics::LogLevel::WARN);
    cout << "Log level: " << magic_enum::enum_name(log_level) << endl;

    if (all_ops.was_set())
    {
        albc::worker::show_all_ops = true;
    }

    GlobalLogConfig::SetLogLevel(log_level);

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
        cout << "Main process started." << std::endl;

        double elapsed;
        if (go_test.was_set())
        {
            //albc::worker::run_go_test();
            elapsed = 0;
        }
        else if (parallel_test.was_set())
        {
            elapsed = MeasureTime(albc::worker::run_parallel_test, player_data, game_data, log_level, 20).count();
        }
        else if (seq_test.was_set())
        {
            elapsed = MeasureTime(albc::worker::run_sequential_test, player_data, game_data, log_level, 1024).count();
        }
        else
        {
            elapsed = MeasureTime(albc::worker::run_test, player_data, game_data, log_level).count();
        }

        cout << "Main process successfully completed in " << elapsed << "s." << std::endl;

#ifdef _WIN32
        _CrtDumpMemoryLeaks();
#endif
        std::this_thread::sleep_for(std::chrono::seconds(1)); // wait for console output to flush
		return 0;
	}
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    } /* and catch c str as well */ catch (const char* e) {
        std::cerr << "Exception: " << e << std::endl;
        return -1;
    } /* and catch c++ string as well */ catch (const std::string& e) {
        std::cerr << "Exception: " << e << std::endl;
        return -1;
    } /* std::cerr other exceptions */ catch (...) {
        std::cerr << "Exception: Unknown exception." << std::endl;
        return -1;
    }// end try
}
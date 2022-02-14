#include <chrono>
#include "worker.h"

#define PROGRAMOPTIONS_NO_COLORS
#include "ProgramOptions.hxx"

int main(const int argc, char* argv[])
{
	//print build info
    std::cout << "Build: " << __DATE__ << " " << __TIME__
              << ", Debug: " << is_debug_build()
              << ", System: " << system_name()
              << ", " << system_architecture() << std::endl;
    po::parser parser;
    std::string game_data;
    std::string player_data;
    int logLevel = albc::diagnostics::Logger::LogLevel::INFO;

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

    // add debug options to parser
    auto& debug = parser["debug"]
        .abbreviation('d')
        .description("enable debug mode");

    // add help to parser
    auto& help = parser["help"]
        .abbreviation('h')
        .description("produce help message");

    auto& go_test = parser["go-test"]
        .description("run golang API test");

    // parse command line
    if (!parser.parse(argc, argv))
    {
        LOG_E << "Error: Unable to parse commandline args!" << std::endl;
        return -1;
    }

    if (help.was_set())
    {
        std::cout << parser << std::endl;
        return 0;
    }

    if (debug.was_set())
    {
        logLevel = albc::diagnostics::Logger::LogLevel::DEBUG;
    }

    // check if all required options are set
    if (player_data.empty() || game_data.empty())
    {
        LOG_E << "Error: Missing required options!" << std::endl;
        // print missing options name
        if (player_data.empty())
        {
            LOG_E << "must specify the path to player data file!" << std::endl;
        }
        if (game_data.empty())
        {
            LOG_E << "must specify the path to building data file!" << std::endl;
        }

        std::cout << parser << std::endl;
        return -1;
    }
	try
	{
		// profile the elapsed time_t
        auto start = std::chrono::high_resolution_clock::now();

        LOG_I << "Main process started." << std::endl;
        const auto& elapsed = MeasureTime(albc::worker::work, player_data, game_data, logLevel);
        LOG_I << "Main process successfully completed in " << elapsed.count() << "s." << std::endl;

		return 0;
	}
    catch (const std::exception& e)
    {
        LOG_E << "Exception: " << e.what() << std::endl;
        return -1;
    } /* and catch c str as well */ catch (const char* e) {
        LOG_E << "Exception: " << e << std::endl;
        return -1;
    } /* and catch c++ string as well */ catch (const std::string& e) {
        LOG_E << "Exception: " << e << std::endl;
        return -1;
    } /* catch other exceptions */ catch (...) {
        LOG_E << "Exception: Unknown exception." << std::endl;
        return -1;
    }// end try
}
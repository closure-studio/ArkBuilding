#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

enum AlbcLogLevel
{
    ALBC_LOG_LEVEL_DEBUG = 0,
    ALBC_LOG_LEVEL_INFO = 1,
    ALBC_LOG_LEVEL_WARN = 2,
    ALBC_LOG_LEVEL_ERROR = 3
};

enum AlbcTestMode
{
    ALBC_TEST_MODE_SEQUENTIAL,
    ALBC_TEST_MODE_PARALLEL,
    ALBC_TEST_MODE_ONCE
};

typedef struct
{
    bool gen_lp_file;
    bool gen_all_solution_details;
    double time_limit;
} AlbcSolverParameters;

typedef struct
{
    AlbcSolverParameters solver_parameters;
    AlbcLogLevel level;
} AlbcParameters;

typedef struct
{
    AlbcParameters base_parameters;
    AlbcTestMode mode;
    int param;
} AlbcTestConfig;

typedef struct
{
    AlbcParameters base_parameters;
    const char *json;
} AlbcRunParams;

typedef struct
{
    char *json;
} AlbcRunResult;

int AlbcRun(const AlbcRunParams *in, AlbcRunResult **out);

void AlbcFree(const AlbcRunResult *out);

void AlbcTest(const char *game_data_path, const char *player_data_path, const AlbcTestConfig *config);

void AlbcSetGlobalBuildingData(const char *json);

void AlbcSetLogLevel(AlbcLogLevel level);

void AlbcFlushLog();

AlbcLogLevel AlbcParseLogLevel(const char *level, AlbcLogLevel default_level);

#ifdef __cplusplus
}
#endif
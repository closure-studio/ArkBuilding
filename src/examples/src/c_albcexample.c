#include "c_albcexample.h"
#include "albc/calbc.h"
#include "json_input.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define ALBC_CHECK(lbl, stm, e)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        e = NULL;                                                                                                      \
        stm;                                                                                                           \
        if (!call_success(e, __FILE__ ":" S__LINE__ ": " #stm))                                                        \
            goto lbl;                                                                                                  \
    } while (0)

bool call_success(AlbcException *e, const char *error_msg)
{
    if (e)
    {
        printf("%s: %s\n", error_msg, e->what);
        AlbcFreeException(e);
        return false;
    }
    return true;
}

void c_albc_example_main(const char* building_data_path, const char *character_table_path, const char *char_meta_table_path)
{
    AlbcException *e = NULL;
    AlbcString *out = NULL;

    AlbcDoLog(ALBC_LOG_LEVEL_INFO, "Starting ALBC C example", NULL);
    AlbcSetLogLevel(ALBC_LOG_LEVEL_ALL, NULL);
    ALBC_CHECK(assets_fail, AlbcLoadGameDataFile(ALBC_GAME_DATA_DB_BUILDING_DATA, building_data_path, &e), e);
    ALBC_CHECK(assets_fail, AlbcLoadGameDataFile(ALBC_GAME_DATA_DB_CHARACTER_TABLE, character_table_path, &e), e);
    ALBC_CHECK(assets_fail, AlbcLoadGameDataFile(ALBC_GAME_DATA_DB_CHAR_META_TABLE, char_meta_table_path, &e), e);

    ALBC_CHECK(json_run_fail, out = AlbcRunWithJsonParams(GetTestJsonInput(), &e), e);

    printf("%s\n", AlbcStringGetContent(out));

json_run_fail:
    AlbcStringDel(out);
assets_fail:
    AlbcFlushLog(NULL);
}
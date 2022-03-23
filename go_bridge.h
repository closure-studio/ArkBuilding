//
// Created by User on 2022-02-05.
//

// used to bridge C++ and Go(Golang), using a c wrapper and cgo
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    const char *json;
} InParams;

typedef struct
{
    char *json;
} OutParams;

int AlbcRun(const InParams *in, OutParams **out);

void AlbcFree(const OutParams *out);

void AlbcTest(const char *game_data_path, const char *player_data_path);

void AlbcSetGlobalBuildingData(const char *json);

#ifdef __cplusplus
}
#endif
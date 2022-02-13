//
// Created by User on 2022-02-05.
//

// used to bridge C++ and Go(Golang), using a c wrapper and cgo

typedef struct
{
    const char *json;
} InParams;

typedef struct
{
    const char *json;
} OutParams;

int AlbcRun(InParams *in, OutParams **out);

void AlbcFree(OutParams *out);

void AlbcTest(const char *game_data_path, const char *player_data_path);
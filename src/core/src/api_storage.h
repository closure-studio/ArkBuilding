#pragma once
#include "api_resource.h"

namespace albc::api
{
inline GameDataJsonStorage& GetGlobalGameDataStorage()
{
    static GameDataJsonStorage api_global_gamedata_storage;
    return api_global_gamedata_storage;
}
}
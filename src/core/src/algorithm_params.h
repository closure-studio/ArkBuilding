#pragma once
#include "primitive_types.h"
#include "algorithm_primitives.h"

namespace albc::algorithm
{

struct RoomResult
{
    SolutionData solution;
    model::buff::RoomModel* room = nullptr;
};

struct AlgorithmResult
{
    Vector<RoomResult> rooms;

    void Clear()
    {
        rooms.clear();
    }
};

} // namespace albc::algorithm
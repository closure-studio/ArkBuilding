//
// Created by Nonary on 2022/4/24.
//
#include "algorithm_iface_runner.h"

namespace albc::algorithm::iface
{

void MultiRoomIntegerProgramRunner::Run(const AlgorithmParams &params, const AlbcSolverParameters &solver_params,
                                        AlgorithmResult &out_result) const
{
    using namespace algorithm;
    Vector<model::buff::RoomModel *> all_rooms;
    const auto &manu_rooms = mem::unwrap_ptr_vector(params.GetRoomsOfType(data::building::RoomType::MANUFACTURE));
    const auto &trade_rooms = mem::unwrap_ptr_vector(params.GetRoomsOfType(data::building::RoomType::TRADING));
    all_rooms.insert(all_rooms.end(), manu_rooms.begin(), manu_rooms.end());
    all_rooms.insert(all_rooms.end(), trade_rooms.begin(), trade_rooms.end());

    AlbcSolverParameters actual_solver_params = solver_params;
    if (actual_solver_params.model_time_limit <= 0) actual_solver_params.model_time_limit = kDefaultModelTimeLimit;
    if (actual_solver_params.solve_time_limit <= 0) actual_solver_params.solve_time_limit = kDefaultSolveTimeLimit;

    MultiRoomIntegerProgramming alg_all(all_rooms, params.GetOperators(), actual_solver_params);
    alg_all.Run(out_result);
}
void TestRunner::Run(const AlgorithmParams &params, const AlbcSolverParameters &solver_params,
                     AlgorithmResult &out_result) const
{
    (void)params; (void)solver_params; (void)out_result;
    throw std::runtime_error("TestRunner is not implemented");
}
}
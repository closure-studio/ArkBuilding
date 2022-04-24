#pragma once
#include "algorithm_iface_params.h"
#include "albc/albc_common.h"
#include "algorithm_params.h"
#include "algorithm_consts.h"
#include "algorithm.h"

namespace albc::algorithm::iface
{
class IRunner
{
public:
    virtual void Run(const algorithm::iface::AlgorithmParams & params, const AlbcSolverParameters& solver_params, algorithm::AlgorithmResult& out_result) const = 0;
};

class MultiRoomIntegerProgramRunner : public IRunner
{
public:
    MultiRoomIntegerProgramRunner() = default;
    void Run(const algorithm::iface::AlgorithmParams & params, const AlbcSolverParameters& solver_params, algorithm::AlgorithmResult& out_result) const override;
};

class TestRunner : public IRunner
{
public:
    TestRunner() = default;
    void Run(const algorithm::iface::AlgorithmParams & params, const AlbcSolverParameters& solver_params, algorithm::AlgorithmResult& out_result) const override;
};
}
#pragma once
#include "albc_types.h"
#include "algorithm_consts.h"
#include "model_buff.h"
#include "model_operator.h"

namespace albc::algorithm
{
class SolutionData
{
  public:
    SolutionData() = default;

    Array<model::OperatorModel *, model::buff::kRoomMaxOperators> operators = {};
    Array<Array<model::buff::ModifierApplier, model::buff::kOperatorMaxBuffs>, model::buff::kRoomMaxOperators> snapshot = {};
    double productivity = -1;
    double duration = -1;

    void Reset()
    {
        std::fill(operators.begin(), operators.end(), nullptr);
        std::for_each(snapshot.begin(), snapshot.end(), [](auto &v) {
            std::for_each(v.begin(), v.end(), [](auto &v) { v.MarkInvalid(); });
        });
        productivity = -1;
    }

    [[nodiscard]] std::string ToString() const;
};
struct GreedySolutionHolder
{
    SolutionData max_solution;
    UInt32 calc_cnt = 0;

    void Reserve(size_t)
    {
        // do nothing
    }

    void OnSolutionFound(const Array<model::OperatorModel *, model::buff::kRoomMaxOperators> &solution, double productivity, double duration)
    {
        if (productivity > this->max_solution.productivity)
        {
            auto &sol = this->max_solution;
            sol.productivity = productivity;
            sol.duration = duration;
            std::copy(solution.begin(), solution.end(), sol.operators.begin());
            int i = 0;
            for (const auto* op : solution)
            {
                if (!op)
                    continue;

                std::transform(op->buffs.begin(), op->buffs.end(), sol.snapshot[i].begin(),
                               [](const model::buff::RoomBuff* buff) { return buff->applier; });
                i++;
            }
        }
    }

    void UpdateCalcCnt(UInt32 cnt)
    {
        this->calc_cnt = cnt;
    }
};

struct AllSolutionHolder
{
    Vector<SolutionData> solutions;
    UInt32 calc_cnt = 0;
    size_t sol_cnt = 0;

    void Reserve(size_t size)
    {
        Vector<SolutionData> tmp(size);
        solutions.swap(tmp);
        sol_cnt = 0;
    }

    void OnSolutionFound(const Array<model::OperatorModel *, model::buff::kRoomMaxOperators> &solution, double productivity, double duration)
    {
        assert(sol_cnt < solutions.capacity());
        auto &sol = solutions[sol_cnt++];
        sol.productivity = productivity;
        sol.duration = duration;
        std::copy(solution.begin(), solution.end(), sol.operators.begin());
        int i = 0;
        for (const auto* op : solution)
        {
            if (!op)
                continue;

            std::transform(op->buffs.begin(), op->buffs.end(), sol.snapshot[i].begin(),
                           [](const model::buff::RoomBuff* buff) { return buff->applier; });
            i++;
        }
    }

    void UpdateCalcCnt(UInt32 cnt)
    {
        this->calc_cnt = cnt;
    }
};

} // namespace albc::algorithm
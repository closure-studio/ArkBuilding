#pragma once

#include "buff_model.h"
#include "operator_model.h"
#include "operator_pattern.h"
#include <bitset>

namespace albc::algorithm
{
static constexpr size_t kAlgOperatorSize = 512;
static constexpr size_t kRoomMaxOperators = 5;

class SolutionData
{
  public:
    SolutionData() : operators(kRoomMaxOperators), snapshot(kRoomMaxOperators)
    {
    }

    Vector<OperatorModel *> operators;
    Vector<Vector<ModifierApplier>> snapshot;
    double productivity = -1;

    void Reset()
    {
        operators.clear();
        snapshot.clear();
        productivity = -1;
    }

    string ToString(double max_allowed_duration = 3600 * 16) const;
};

/**
 *
 * @brief The Algorithm class
 *
 */
class Algorithm
{
  public:
    virtual ~Algorithm() = default;

    Algorithm(const Vector<RoomModel*> &rooms, const PtrVector<OperatorModel> &operators,
              double max_allowed_duration = 3600 * 16)
        : rooms_(rooms), all_ops_(get_raw_ptr_vector(operators)),
          max_allowed_duration_(max_allowed_duration)
    {
    }
    
    Algorithm(const PtrVector<RoomModel> &rooms, const PtrVector<OperatorModel> &operators,
              double max_allowed_duration = 3600 * 16)
        : Algorithm(get_raw_ptr_vector(rooms), operators, max_allowed_duration)
    {
    }

    virtual void Run() = 0; // 实现算法

  protected:
    Vector<RoomModel *> rooms_;
    Vector<OperatorModel *> all_ops_;
    Vector<OperatorModel *> inbound_ops_;
    double max_allowed_duration_;

    void FilterOperators(const RoomModel *room);

    [[nodiscard]] string GetSolutionInfo(const RoomModel &room, const SolutionData &solution) const;
};

class HardMutexResolver
{
  public:
    std::bitset<kAlgOperatorSize> mutex_ops;
    std::bitset<kAlgOperatorSize> enabled_ops_for_partial_comb;
    Vector<OperatorModel *> ops_for_partial_comb;
    Vector<OperatorModel *> non_mutex_ops;

    HardMutexResolver(const Vector<OperatorModel *> &ops, bm::RoomType room_type);
    [[nodiscard]] bool MoveNext();
    [[nodiscard]] bool HasMutexBuff() const;
    [[nodiscard]] UInt32 MutexCombCnt() const;
    [[nodiscard]] UInt32 MutexGroupCnt() const;

  protected:
    Vector<Vector<OperatorModel *>> mutex_groups_;
    Vector<UInt32> group_pos_;
};

struct GreedySolutionHolder
{
    SolutionData max_solution;
    UInt32 calc_cnt = 0;

    void Reserve(size_t size)
    {
        max_solution.operators.reserve(1);
        max_solution.snapshot.reserve(1);
    }

    void OnSolutionFound(const Vector<OperatorModel *> &solution, double productivity)
    {
        if (productivity > this->max_solution.productivity)
        {
            this->max_solution.productivity = productivity;

            this->max_solution.operators.assign(solution.begin(), solution.end());
            std::transform(solution.begin(), solution.end(), this->max_solution.snapshot.begin(),
                           [](const OperatorModel *o) {
                               Vector<ModifierApplier> mods(o->buffs.size());
                               std::transform(o->buffs.begin(), o->buffs.end(), mods.begin(),
                                              [](const RoomBuff *b) { return b->applier; });

                               return std::move(mods);
                           });
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

    void Reserve(size_t size)
    {
        solutions.reserve(size);
    }

    void OnSolutionFound(const Vector<OperatorModel *> &solution, double productivity)
    {
        assert(solutions.size() < solutions.capacity());
        auto &sol = solutions.emplace_back();
        sol.productivity = productivity;
        sol.operators.assign(solution.begin(), solution.end());
        std::transform(solution.begin(), solution.end(), sol.snapshot.begin(), [](const OperatorModel *o) {
            Vector<ModifierApplier> mods(o->buffs.size());
            std::transform(o->buffs.begin(), o->buffs.end(), mods.begin(),
                           [](const RoomBuff *b) { return b->applier; });
            
            return std::move(mods);
        });
    }

    void UpdateCalcCnt(UInt32 cnt)
    {
        this->calc_cnt = cnt;
    }
};

class CombMaker : public Algorithm
{
  protected:
    using Algorithm::Algorithm;

    template <typename TSolutionHolder>
    void MakePartialComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                         const std::bitset<kAlgOperatorSize> &enabled_root_ops, TSolutionHolder &solution_holder) const;
};

class BruteForce : public CombMaker // 暴力枚举，计算出所有可能的组合
{
  public:
    using CombMaker::CombMaker;

    void Run() override;

  protected:
    template <typename TSolutionHolder>
    void MakeComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                  TSolutionHolder &solution_holder);
};

class MultiRoomGreedy : public BruteForce
{
    // 多房间贪心算法
    // 1. 选中某个房间，筛选当前房间可用的干员，按照房间类型/产品类型过滤
    // 2. 在当前可用的干员列表中选出加成最高的干员组合，作为当前房间的干员
    // 3. 从可用干员列表中剔除已选择的干员，重复步骤1、2，直到所有房间都完成
    // 默认房间优先级为：干员容量大的房间优先，因为这样更有可能排列出生产效率高的Buff组合
    // TODO: 房间优先级的其他设置
  public:
    using BruteForce::BruteForce;

    void Run() override;

  protected:
    Dictionary<string, SolutionData> room_solution_map_; // map[room_id] = SolutionData
};

class PatternMatching
{
  public:
};

class MultiRoomIntegerProgramming : public BruteForce
{
  public:
    using BruteForce::BruteForce;

    void Run() override;

  protected:
    Dictionary<string, SolutionData> room_solution_map_; // map[room_id] = SolutionData
};
} // namespace albc::algorithm

using namespace albc::algorithm;
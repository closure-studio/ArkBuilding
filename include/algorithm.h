#pragma once

#include "buff_model.h"
#include "operator_model.h"
#include <bitset>

namespace albc::algorithm
{
static constexpr size_t kAlgOperatorSize = 512;

class SolutionData
{
  public:
    SolutionData()
        : operators(kRoomMaxBuffSlots), snapshot(kRoomMaxBuffSlots)
    { }

    Vector<OperatorModel *> operators;
    Vector<Vector<ModifierApplier>> snapshot;
    double productivity = -1;
    UInt32 calc_cnt = 0;

    void Reset()
    {
        operators.clear();
        snapshot.clear();
        productivity = -1;
        calc_cnt = 0;
    }
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

    Algorithm(const PtrVector<RoomModel> &rooms, const PtrVector<OperatorModel> &operators,
              double max_allowed_duration = 3600 * 16)
        : rooms_(get_raw_ptr_vector(rooms)),
          all_ops_(get_raw_ptr_vector(operators)),
          max_allowed_duration_(max_allowed_duration)
    { }

    virtual void Run() = 0; // 实现算法

  protected:
    Vector<RoomModel *> rooms_;
    Vector<OperatorModel *> all_ops_;
    Vector<OperatorModel *> inbound_ops_;
    double max_allowed_duration_;
    SolutionData current_solution_;

    void FilterOperators(const RoomModel *room);

    [[nodiscard]] string GetSolutionInfo(const RoomModel &room, const SolutionData &solution) const;
    void UpdateSolution(double max_delta, UInt32 calc_cnt, Vector<OperatorModel *> solution,
                        Vector<Vector<ModifierApplier>> snapshot);

    void ResetSolution();
};

class HardMutexResolver
{
  public:
    std::bitset<kAlgOperatorSize> mutex_ops;
    std::bitset<kAlgOperatorSize> enabled_ops_for_partial_comb;
    Vector<OperatorModel *> ops_for_partial_comb;
    Vector<OperatorModel *> non_mutex_ops;

    HardMutexResolver(const Vector<OperatorModel *> &ops, bm::RoomType room_type);
    bool MoveNext();
    bool HasMutexBuff();

  protected:
    Vector<Vector<OperatorModel *>> mutex_groups_;
    Vector<UInt32> group_pos_;
};

class CombMaker : public Algorithm
{
  protected:
    using Algorithm::Algorithm;

    std::tuple<double, UInt32, Vector<OperatorModel *>, Vector<Vector<ModifierApplier>>> MakePartialComb(
        const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
        const std::bitset<kAlgOperatorSize> &enabled_root_ops) const;
};

class BruteForce : public CombMaker // 暴力枚举，计算出所有可能的组合
{
  public:
    using CombMaker::CombMaker;

    void Run() override;

  protected:
    void MakeComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room);
    void MakePartialCombAndUpdateSolution(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                                          const std::bitset<kAlgOperatorSize> &enabled_root_ops);
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
    MultiRoomGreedy(const PtrVector<RoomModel> &rooms, const PtrVector<OperatorModel> &operators,
                    double max_allowed_duration = 3600 * 16);

    void Run() override;

  protected:
    Dictionary<string, SolutionData> room_solution_map_; //map[room_id] = SolutionData
};
} // namespace albc::algorithm

using namespace albc::algorithm;
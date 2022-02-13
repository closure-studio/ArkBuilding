#pragma once

#include "buff_model.h"
#include "operator_model.h"
#include <bitset>

namespace albc
{
static constexpr size_t kAlgOperatorSize = 512;

class HardMutexResolver
{
  public:
    std::bitset<kAlgOperatorSize> mutex_ops;
    std::bitset<kAlgOperatorSize> enabled_ops_for_partial_comb;
    Vector<OperatorModel *> ops_for_partial_comb;
    Vector<OperatorModel *> non_mutex_ops;

    HardMutexResolver(const Vector<OperatorModel *> &ops, bm::RoomType room_type);
    bool MoveNext();

  protected:
    Vector<Vector<OperatorModel *>> mutex_groups_;
    Vector<UInt32> group_pos_;
};

class Algorithm
{
  public:
    virtual ~Algorithm() = default;
    Vector<RoomModel *> rooms;
    Vector<OperatorModel *> all_ops;

    Algorithm(const Vector<RoomModel *> &rooms, const Vector<OperatorModel *> &providers)
        : rooms(rooms), all_ops(providers)
    {
    }

    virtual void Run() = 0; // 实现算法
};

class BruteForce : public Algorithm // 暴力枚举，计算出所有可能的组合
{
  public:
    BruteForce(const Vector<RoomModel *> &rooms, const Vector<OperatorModel *> &operators,
               double max_allowed_duration = 3600 * 8);

    void Run() override;

  protected:
    Vector<OperatorModel *> solution_;
    Vector<OperatorModel *> inbound_ops_;
    double max_tot_delta_;
    UInt32 calc_cnt_;
    double max_allowed_duration_;

    void FilterAgents(const RoomModel *room);

    void MakeComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room);

    void MakePartialCombAndUpdateSolution(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                                          const std::bitset<kAlgOperatorSize> &enabled_root_ops);

    std::tuple<double, UInt32, Vector<OperatorModel *>> MakePartialComb(
        const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
        const std::bitset<kAlgOperatorSize> &enabled_root_ops) const;

    void PrintSolution(const RoomModel &room);
};

class PatternSearch : public Algorithm // 模式搜索，根据预设的模式，搜索出最优的组合
{
  public:
    PatternSearch(const Vector<RoomModel *> &rooms, const Vector<OperatorModel *> &operators,
                  double max_allowed_duration = 3600 * 8);

    void Run() override;
};
} // namespace albc
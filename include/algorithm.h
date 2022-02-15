#pragma once

#include "buff_model.h"
#include "operator_model.h"
#include <bitset>

namespace albc::algorithm
{
static constexpr size_t kAlgOperatorSize = 512;

/**
 *
 * @brief The Algorithm class
 *
 */
class Algorithm
{
  public:
    virtual ~Algorithm() = default;

    Algorithm(const Vector<RoomModel *> &rooms, const Vector<OperatorModel *> &operators,
              double max_allowed_duration = 3600 * 16)
        : rooms_(rooms), all_ops_(operators), max_allowed_duration_(max_allowed_duration), 
          solution_(kRoomMaxBuffSlots), snapshot_(kRoomMaxBuffSlots)
    { }

    virtual void Run() = 0; // 实现算法

  protected:
    Vector<RoomModel *> rooms_;
    Vector<OperatorModel *> all_ops_;
    Vector<OperatorModel *> inbound_ops_;
    Vector<OperatorModel *> solution_;
    Vector<Vector<ModifierApplier>> snapshot_;
    double max_allowed_duration_;

    void FilterAgents(const RoomModel *room);
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

class BruteForce : public Algorithm // 暴力枚举，计算出所有可能的组合
{
  public:
    using Algorithm::Algorithm;

    void Run() override;

  protected:
    double max_tot_delta_ = -1;
    UInt32 calc_cnt_ = 0;

    void MakeComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room);

    void MakePartialCombAndUpdateSolution(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                                          const std::bitset<kAlgOperatorSize> &enabled_root_ops);

    std::tuple<double, UInt32, Vector<OperatorModel *>, Vector<Vector<ModifierApplier>>> MakePartialComb(
        const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
        const std::bitset<kAlgOperatorSize> &enabled_root_ops) const;

    void PrintSolution(const RoomModel &room, const LogLevel log_level = LogLevel::DEBUG);
};
} // namespace albc::algorithm

using namespace albc::algorithm;
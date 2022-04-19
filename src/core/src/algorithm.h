#pragma once

#include "albc/calbc.h"
#include "algorithm_params.h"
#include <bitset>

namespace albc::algorithm
{

/**
 *
 * @brief The Algorithm class
 *
 */
class Algorithm
{
  public:
    virtual ~Algorithm() = default;

    Algorithm(const Vector<model::buff::RoomModel *> &rooms, const mem::PtrVector<model::OperatorModel> &operators,
              const AlbcSolverParameters &params)
        : rooms_(rooms), all_ops_(mem::unwrap_ptr_vector(operators)), params_(params)
    {
    }

    Algorithm(const mem::PtrVector<model::buff::RoomModel> &rooms, const mem::PtrVector<model::OperatorModel> &operators,
              const AlbcSolverParameters &params)
        : Algorithm(mem::unwrap_ptr_vector(rooms), operators, params)
    {
    }

    virtual void Run(AlgorithmResult &result) = 0; // 实现算法

  protected:
    Vector<model::buff::RoomModel *> rooms_;
    Vector<model::OperatorModel *> all_ops_;
    Vector<model::OperatorModel *> inbound_ops_;
    AlbcSolverParameters params_;

    void FilterOperators(const model::buff::RoomModel *room);

    [[nodiscard]] static std::string GetSolutionInfo(const model::buff::RoomModel &room, const SolutionData &solution);
};

class HardMutexResolver
{
  public:
    std::bitset<model::buff::kAlgOperatorSize> mutex_ops;
    std::bitset<model::buff::kAlgOperatorSize> enabled_ops_for_partial_comb;
    Vector<model::OperatorModel *> ops_for_partial_comb;
    Vector<model::OperatorModel *> non_mutex_ops;

    HardMutexResolver(const Vector<model::OperatorModel *> &ops, data::building::RoomType room_type);
    [[nodiscard]] bool MoveNext();
    [[nodiscard]] bool HasMutexBuff() const;
    [[nodiscard]] UInt32 MutexCombCnt() const;
    [[nodiscard]] UInt32 MutexGroupCnt() const;

  protected:
    Vector<Vector<model::OperatorModel *>> mutex_groups_;
    Vector<UInt32> group_pos_;
};

class CombMaker : public Algorithm
{
  public:
    void Run(AlgorithmResult &result) override;

  protected:
    using Algorithm::Algorithm;

    template <typename TSolutionHolder>
    void MakePartialComb(const Vector<model::OperatorModel *> &operators, UInt32 max_n, model::buff::RoomModel *room,
                         const std::bitset<model::buff::kAlgOperatorSize> &enabled_root_ops, TSolutionHolder &solution_holder) const;

    template <typename TSolutionHolder>
    void MakeComb(const Vector<model::OperatorModel *> &operators, UInt32 max_n, model::buff::RoomModel *room,
                  TSolutionHolder &solution_holder);
};

class MultiRoomGreedy : public CombMaker
{
    // 多房间贪心算法
    // 1. 选中某个房间，筛选当前房间可用的干员，按照房间类型/产品类型过滤
    // 2. 在当前可用的干员列表中选出加成最高的干员组合，作为当前房间的干员
    // 3. 从可用干员列表中剔除已选择的干员，重复步骤1、2，直到所有房间都完成
    // 默认房间优先级为：干员容量大的房间优先，因为这样更有可能排列出生产效率高的Buff组合
  public:
    using CombMaker::CombMaker;

    void Run(AlgorithmResult &result) override;
};

class MultiRoomIntegerProgramming : public CombMaker
{
  public:
    using CombMaker::CombMaker;

    void Run(AlgorithmResult &result) override;

  protected:
    static void GenSolDetails(const Vector<model::buff::RoomModel *> &rooms, const Vector<Vector<SolutionData>> &room_solutions,
                              Vector<UInt32> &room_ranges, size_t col_cnt);

    void GenLpFile(Vector<Vector<SolutionData>> &room_solutions, const Vector<double> &obj, UInt32 row_cnt,
                   UInt32 col_cnt, const Vector<double> &elems, const Vector<int> &row_indices,
                   Vector<int> &col_indices, UInt32 room_start_row, const Vector<double> &row_ub) const;

    void GenCombForRooms(Vector<Vector<SolutionData>> &room_solutions, Vector<UInt32> &room_ranges, UInt32 &col_cnt);

    [[nodiscard]] static UInt32 GetRoomIdx(UInt32 col, const Vector<UInt32> &room_ranges) ;

    [[nodiscard]] static UInt32 GetIndexInRoom(UInt32 col, const Vector<UInt32> &room_ranges) ;
};
} // namespace albc::algorithm
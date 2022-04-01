#pragma once

#include "albc/calbc.h"
#include "buff_model.h"
#include "operator_model.h"
#include "operator_pattern.h"
#include <bitset>

namespace albc::algorithm
{
static constexpr size_t kAlgOperatorSize = 512;
static constexpr size_t kRoomMaxOperators = 5;

/**
 * Column-major sparse matrix
 * @tparam T type of elements
 */
template <typename T> class Matrix2D
{
  public:
    Matrix2D(size_t row, size_t col, const T &x) : row_(row), col_(col)
    {
        data_.resize(row * col, x);
        index_.resize(row);
        for (size_t i = 0; i < row; ++i)
        {
            index_[i] = &data_[i * col];
        }
    }

    [[nodiscard]] size_t Row() const
    {
        return row_;
    }

    [[nodiscard]] size_t Col() const
    {
        return col_;
    }

    const T *operator[](size_t row) const
    {
        return index_[row];
    }

    T *operator[](size_t row)
    {
        return index_[row];
    }

    T *Data()
    {
        return data_.data();
    }

    const T *Data() const
    {
        return data_.data();
    }

  private:
    Vector<T> data_;
    Vector<double *> index_;
    size_t row_;
    size_t col_;
};

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

    [[nodiscard]] string ToString(double max_allowed_duration = 3600 * 16) const;
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

    Algorithm(const Vector<RoomModel *> &rooms, const PtrVector<OperatorModel> &operators,
              const AlbcSolverParameters &params)
        : rooms_(rooms), all_ops_(get_raw_ptr_vector(operators)), params_(params)
    {
    }

    Algorithm(const PtrVector<RoomModel> &rooms, const PtrVector<OperatorModel> &operators,
              const AlbcSolverParameters &params)
        : Algorithm(get_raw_ptr_vector(rooms), operators, params)
    {
    }

    virtual void Run() = 0; // 实现算法

  protected:
    Vector<RoomModel *> rooms_;
    Vector<OperatorModel *> all_ops_;
    Vector<OperatorModel *> inbound_ops_;
    AlbcSolverParameters params_;

    void FilterOperators(const RoomModel *room);

    [[nodiscard]] static string GetSolutionInfo(const RoomModel &room, const SolutionData &solution);
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

    void Reserve(size_t)
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

                               return mods;
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

            return mods;
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
    static void GenSolDetails(const Vector<Vector<SolutionData>> &room_solutions, Vector<UInt32> &room_ranges,
                              size_t col_cnt);

    void GenLpFile(Vector<Vector<SolutionData>> &room_solutions, const Vector<double> &obj, UInt32 row_cnt,
                   UInt32 col_cnt, const Vector<double> &elems, const Vector<int> &row_indices,
                   Vector<int> &col_indices, UInt32 room_start_row, const Vector<double> &row_ub) const;

    void GenCombForRooms(Vector<Vector<SolutionData>> &room_solutions, Vector<UInt32> &room_ranges, UInt32 &col_cnt);

    [[nodiscard]] static UInt32 GetRoomIdx(UInt32 col, const Vector<UInt32> &room_ranges) ;

    [[nodiscard]] static UInt32 GetIndexInRoom(UInt32 col, const Vector<UInt32> &room_ranges) ;
};
} // namespace albc::algorithm

using namespace albc::algorithm;
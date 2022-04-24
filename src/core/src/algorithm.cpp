#include "algorithm.h"
#include "util_flag.h"
#include "util_locale.h"
#include "model_simulator.h"

#include "CbcModel.hpp"
#include "CoinModel.hpp"
#include "OsiClpSolverInterface.hpp"

#include <bitset>
#include <fstream>
#include <random>
#include <regex>
#include <unordered_set>


namespace albc::algorithm
{
static void ResolveSpCharGroup(const Vector<model::OperatorModel*>& ops,
                               Dictionary<std::string, Vector<UInt32 /* index of op */  >>& group_ops_map)
{
    group_ops_map.clear();
    std::unordered_multiset<size_t> group_ops_set;
    for (const auto* op: ops)
        if (!op->sp_char_group.empty())
            group_ops_set.insert(std::hash<std::string>()(op->sp_char_group));

    for (size_t i = 0; i < ops.size(); ++i)
    {
        if (ops[i]->sp_char_group.empty()
            || group_ops_set.count(std::hash<std::string>()(ops[i]->sp_char_group)) <= 1)
            continue;

        group_ops_map[ops[i]->sp_char_group].push_back(i);
    }
}

std::string SolutionData::ToString() const
{
    using namespace util;
    char buf[16384];
    char *p = buf;
    size_t size = sizeof buf;

    int n_op = 1;
    append_snprintf(p, size, "**** Solution ****\n");
    for (auto *const op : operators)
    {
        if (op == nullptr)
        {
            break;
        }

        append_snprintf(p, size, "**** Operator #%d: %-20s ****\n", n_op, op->char_id.c_str());

        int n_buff = 1;
        for (const auto &buff : op->buffs)
        {
            append_snprintf(p, size, "\tBuff #%d: %8s %s\n", n_buff, buff->name.c_str(),
                            buff->buff_id.c_str());
            append_snprintf(p, size, "\t%s\n", buff->description.c_str());
            append_snprintf(p, size, "\tMod:      %s\n\tFinal Mod:%s\n\tCost Mod: %s\n\n",
                            snapshot[n_op - 1][n_buff - 1].room_mod.to_string().c_str(),
                            snapshot[n_op - 1][n_buff - 1].final_mod.to_string().c_str(),
                            snapshot[n_op - 1][n_buff - 1].cost_mod.to_string().c_str());

            ++n_buff;
        }

        ++n_op;
    }
    double avg_prod = productivity / duration * 100.;
    append_snprintf(p, size, "**** Total productivity: %.2f in %.2fs(Duration), avg scale: %.2f%% (+%.2f%%) ****\n", productivity,
                    duration, avg_prod, avg_prod - 100);

    return buf;
}

std::string IAlgorithm::GetSolutionInfo(const model::buff::RoomModel &room, const SolutionData &solution)
{
    std::string result;
    result.append("**** Solution ****\n").append(room.to_string()).append(solution.ToString());
    return result;
}

template <typename TSolutionHolder>
void CombMaker::MakeComb(const Vector<model::OperatorModel *> &operators, UInt32 max_n, model::buff::RoomModel *room,
                          TSolutionHolder &solution_holder)
{
    if (room->max_slot_count <= 0)
    {
        LOG_E("Room: ", room->id, " has no slots");
        return;
    }

    if (operators.empty())
    {
        LOG_E("No operators for room: ", room->id);
        return;
    }

    max_n = std::min(max_n, static_cast<UInt32>(operators.size()));
    std::bitset<model::buff::kAlgOperatorSize> all_ops;
    all_ops.flip();

    HardMutexResolver mutex_handler(operators, room->type);
    UInt32 calc_cnt = std::max(util::n_choose_k(mutex_handler.non_mutex_ops.size(), max_n), (UInt64)1);
    auto mutex_cnt = mutex_handler.MutexCombCnt();
    calc_cnt += mutex_cnt * (util::n_choose_k(mutex_handler.ops_for_partial_comb.size(), max_n) -
                             calc_cnt); // 见MakePartialComb中防止重复计算部分
    solution_holder.Reserve(calc_cnt);

    MakePartialComb(mutex_handler.non_mutex_ops, max_n, room, all_ops, solution_holder);

    if (mutex_handler.HasMutexBuff())
    {
        do
        {
            MakePartialComb(mutex_handler.ops_for_partial_comb, max_n, room, mutex_handler.enabled_ops_for_partial_comb,
                            solution_holder);
        } while (mutex_handler.MoveNext());
    }
}

void IAlgorithm::FilterOperators(const model::buff::RoomModel *room)
{
    inbound_ops_.clear();

    for (auto *const ops : all_ops_)
    {
        if (ops->buffs.empty())
            continue;

        if (!util::check_flag(ops->room_type_mask, room->type))
            continue;

        if (std::all_of(ops->buffs.begin(), ops->buffs.end(),
                        [room](model::buff::RoomBuff *buff) -> bool { return !buff->ValidateTarget(room); }))
            continue;

        inbound_ops_.push_back(ops);
    }
    LOG_D("Filtered ", inbound_ops_.size(), " operators for room: ", room->id,
          " : [P]", util::enum_to_string(room->room_attributes.prod_type),
          " [O]", util::enum_to_string(room->room_attributes.order_type));
}

template <typename TSolutionHolder>
ALBC_FLATTEN void CombMaker::MakePartialComb(const Vector<model::OperatorModel *> &operators, UInt32 max_n,
                                             model::buff::RoomModel *room,
                                             const std::bitset<model::buff::kAlgOperatorSize> &enabled_root_ops,
                                             TSolutionHolder &solution_holder) const
{
    using namespace model::buff;

    // 该函数是由递归写法DFS到迭代写法DFS的转换
    // enabled_root_ops 为允许的DFS根节点，在原有排列组合（所有干员可用）基础上新增若干个干员后，
    // 将原先干员设为不可用，新干员设为可用，即可防止重复计算
    if (operators.empty()) return;

    UInt32 size = operators.size();
    max_n = std::min(size, max_n);
    UInt32 calc_cnt = 0;

    // 栈变量，用于模拟递归栈
    UInt32 pos[kRoomMaxBuffSlots]{};      // 第i层递归选中干员的位置
    UInt32 buff_cnt[kRoomMaxBuffSlots]{}; // 第i层递归的buff数量
    bool status[kRoomMaxBuffSlots]{};     // 第i层递归的状态，false为正在入栈，true为正在出栈

    Array<BitSet<kOperatorMaxBuffs>, kAlgOperatorSize> cached_enabled_buff;
    std::transform(operators.begin(), operators.end(), cached_enabled_buff.begin(),
                   [room](model::OperatorModel *op) -> BitSet<kOperatorMaxBuffs>{
                       BitSet<kOperatorMaxBuffs> result;
                       int i = 0;
                       for (auto *buff : op->buffs)
                       {
                           if (buff != nullptr && util::check_flag(buff->room_type, room->type) && buff->ValidateTarget(room))
                               result.set(i);

                           ++i;
                       }
                       return result;
                   });

    Array<model::OperatorModel *, kRoomMaxOperators> current = {}; // 当前递归选中的干员
    double max_duration = IAlgorithm::params_.model_time_limit;
    bool is_all_ops = enabled_root_ops.all();

    UInt32 dep = 0; // 当dep==max_n-1时，得到一个组合
    while (true)
    {
        UInt32 &cur_pos = pos[dep];
        bool &cur_status = status[dep];
        if (!cur_status) // 入栈
        {
            cur_status = true;

            if (is_all_ops || dep > 0 || enabled_root_ops[cur_pos])
            {
                current[dep] = operators[cur_pos];
                for (int i = 0; i < (int)kOperatorMaxBuffs; ++i)
                {
                    if (cached_enabled_buff[cur_pos][i])
                    {
                        room->PushBuff(operators[cur_pos]->buffs[i]);
                        ++buff_cnt[dep];
                    }
                }

                if (dep >= max_n - 1)
                {
                    ++calc_cnt;
                    double result, duration;
                    Simulator::DoCalc(room, max_duration, result, duration);
                    solution_holder.OnSolutionFound(current, result, duration);
                }
                else
                {
                    pos[dep + 1] = cur_pos + 1;
                    ++dep; // 进入下一层递归
                    continue;
                }
            }
        }

        if (cur_status) // 出栈
        {
            room->n_buff -= buff_cnt[dep];
            buff_cnt[dep] = 0;

            cur_status = false;
            if (cur_pos < size - max_n + dep)
            {
                ++cur_pos;
            }
            else
            {
                if (dep <= 0)
                    break;

                --dep; // 返回上一层递归或者退出
            }
        }
    }

    solution_holder.UpdateCalcCnt(calc_cnt);
}

void CombMaker::Run(AlgorithmResult &result)
{
    result.Clear();
    auto *const room = rooms_[0];

    // filter operators
    FilterOperators(room);

    GreedySolutionHolder solution_holder;

    // measures the time of MakeComb()
    const double elapsedSec = util::MeasureTime(&CombMaker::MakeComb<GreedySolutionHolder>, this, inbound_ops_,
                                          room->max_slot_count, room, solution_holder)
                                  .count();

    // prints the number of calculations
    LOG_D("calc cnt: ", solution_holder.calc_cnt);
    // print elapsed time
    LOG_D("elapsed time: ", elapsedSec);
    // prints average calculations per second
    LOG_D("calc/sec: ", solution_holder.calc_cnt / elapsedSec);
    // prints the best operators
    if (!std::any_of(solution_holder.max_solution.operators.begin(), solution_holder.max_solution.operators.end(),
                    [](auto* op) { return op; }))
    {
        LOG_E("No operators!");
        return;
    }

    LOG_D(GetSolutionInfo(*room, solution_holder.max_solution));
    auto& room_result = result.rooms.emplace_back();
    room_result.room = room;
    room_result.solution = solution_holder.max_solution;
}

HardMutexResolver::HardMutexResolver(const Vector<model::OperatorModel *> &ops, data::building::RoomType room_type)
{
    // HardMutex 干员互斥模型，假设处于一个互斥组中的干员不能同时出现在同一个房间中
    // 且一个干员最多只有一个互斥Buff，既只处于一个互斥组中
    // 用于处理不能同时生效的Buff和异格干员
    static constexpr size_t buff_type_cnt = util::enum_size<model::buff::RoomBuffType>::value;
    UInt32 buff_type_mutex_group_map[buff_type_cnt];
    Dictionary<std::string /*sp_char_group*/, UInt32> sp_char_group_mutex_group_map;
    std::fill_n(buff_type_mutex_group_map, buff_type_cnt, UINT32_MAX);

    Dictionary<std::string, Vector<UInt32>> sp_char_group_map;
    ResolveSpCharGroup(ops, sp_char_group_map);

    for (const auto& [sp_char_group, op_indices] : sp_char_group_map)
    {
        auto it = sp_char_group_mutex_group_map.find(sp_char_group);
        UInt32 sp_char_group_in_mutex_groups = UINT32_MAX;
        if (it == sp_char_group_mutex_group_map.end())
        {
            sp_char_group_in_mutex_groups = mutex_groups_.size();
            mutex_groups_.emplace_back();
            sp_char_group_mutex_group_map.emplace(sp_char_group, sp_char_group_in_mutex_groups);
        }
        else
        {
            sp_char_group_in_mutex_groups = it->second;
        }

        for (auto op_idx: op_indices)
        {
            mutex_groups_[sp_char_group_in_mutex_groups].push_back(ops[op_idx]);
            mutex_ops[op_idx] = true;
        }
    }

    {
        int op_idx = -1;
        for (const auto op : ops)
        {
            ++op_idx;
            bool op_is_mutex = mutex_ops[op_idx];
            for (const auto buff : op->buffs)
            {
                if (buff->room_type == room_type && buff->is_mutex)
                {
                    if (op_is_mutex)
                    {
                        LOG_E("Logic error: operator ", op->char_id, " has more than one mutex buff or operator is SP char! "
                                                                     "The buff: ", buff->buff_id, " will be ignored");
                        continue;
                    }

                    auto &type_pos_in_mutex_groups = buff_type_mutex_group_map[static_cast<UInt32>(buff->inner_type)];
                    if (type_pos_in_mutex_groups == UINT32_MAX)
                    {
                        type_pos_in_mutex_groups = mutex_groups_.size();
                        mutex_groups_.emplace_back();
                    }

                    mutex_groups_[type_pos_in_mutex_groups].push_back(op);
                    op_is_mutex = true;
                }
            }// TODO: 解决异格干员的Buff也可能是互斥Buff的问题

            mutex_ops[op_idx] = op_is_mutex;
        }
    }

    for (const auto &group : mutex_groups_)
    {
        ops_for_partial_comb.push_back(group[0]);    }

    {
        int op_idx = -1;
        for (const auto op : ops)
        {
            ++op_idx;
            if (!mutex_ops[op_idx])
            {
                ops_for_partial_comb.push_back(op);
                non_mutex_ops.push_back(op);
            }
        }
    }

    auto group_cnt = static_cast<UInt32>(mutex_groups_.size());
    group_pos_.resize(group_cnt, 0);
    for (UInt32 i = 0; i < group_cnt; ++i)
    {
        enabled_ops_for_partial_comb.set(i);
    }
}

bool HardMutexResolver::MoveNext()
{
    bool next = true;
    UInt32 group_idx = 0;
    for (const auto &group : mutex_groups_)
    {
        if (!next)
            break;

        auto &cur_group_pos = group_pos_[group_idx];
        cur_group_pos++;
        if (next = cur_group_pos >= group.size(); next)
        {
            cur_group_pos = 0;
        }

        ops_for_partial_comb[group_idx] = group[cur_group_pos];


        group_idx++;
    }
    return !next;
}
bool HardMutexResolver::HasMutexBuff() const
{
    return !mutex_groups_.empty();
}
UInt32 HardMutexResolver::MutexCombCnt() const
{
    return std::accumulate(mutex_groups_.begin(), mutex_groups_.end(), 1,
                           [](UInt32 sum, const Vector<model::OperatorModel *> &group) { return sum * group.size(); });
}
UInt32 HardMutexResolver::MutexGroupCnt() const
{
    return mutex_groups_.size();
}

void MultiRoomGreedy::Run(AlgorithmResult &result)
{
    result.Clear();
    std::shuffle(rooms_.begin(), rooms_.end(), std::mt19937(std::random_device()()));

    std::sort(rooms_.begin(), rooms_.end(),
              [](const auto *a, const auto *b) { return a->max_slot_count > b->max_slot_count; });

    for (auto room : rooms_)
    {
        GreedySolutionHolder solution_holder;

        FilterOperators(room);
        MakeComb(inbound_ops_, room->max_slot_count, room, solution_holder);
        if (std::all_of(solution_holder.max_solution.operators.begin(), solution_holder.max_solution.operators.end(),
                        [](const auto* p){return !p;}))
        {
            LOG_W("No solution for room ", room->id);
            continue;
        }

        if (util::GlobalLogConfig::CanLog(util::LogLevel::DEBUG))
        {
            LOG_D(GetSolutionInfo(*room, solution_holder.max_solution));
        }

        auto& room_result = result.rooms.emplace_back();
        room_result.solution = solution_holder.max_solution;
        room_result.room = room;

        // 剔除已选择的干员
        all_ops_.erase(std::remove_if(all_ops_.begin(), all_ops_.end(),
                                      [solution_holder](const model::OperatorModel *op) {
                                          return std::find(solution_holder.max_solution.operators.begin(),
                                                           solution_holder.max_solution.operators.end(),
                                                           op) != solution_holder.max_solution.operators.end();
                                      }),
                       all_ops_.end());
    }
}

void MultiRoomIntegerProgramming::Run(AlgorithmResult &out_result)
{
    out_result.Clear();
    Vector<Vector<SolutionData>> room_solutions;
    Vector<UInt32> room_ranges;
    UInt32 total_solution_count = 0;
    GenCombForRooms(room_solutions, room_ranges, total_solution_count);

    if (total_solution_count < 1)
    {
        LOG_W("No solution!");
        return;
    }

    /**
     * 多房间线性规划
     * 建立整数规划模型：
     * 输入：
     * wi1, ..., win为第i个房间内所有组合的收益值
     * xi1, ..., xin ∈ {0, 1} 为第i个房间内对应组合是否被选中
     * 对于一个房间, xi1 + ... + xin <= 1 (每个房间最多选择一个组合)
     * 对于组合中的某个干员, 包含其的组合的集为Z, ΣZ ∈ {0, 1} (每个干员最多选中一次)
     * max W = Σ(xi * wi)
     */


    RowRangeMap row_range_map;
    // 干员行定义
    row_range_map[RowType::OP_CONS] = {
        0,
        all_ops_.size()
    };
    // 房间行定义
    row_range_map[RowType::ROOM_CONS] = {
        row_range_map[RowType::OP_CONS].End(),
        rooms_.size()
    };

    // 构造干员inst_id到干员行的映射
    Vector<UInt32> op_inst_id_to_op_row_map(model::buff::kAlgOperatorSize, 0);
    for (UInt32 op_idx = 0; op_idx < all_ops_.size(); op_idx++)
    {
        op_inst_id_to_op_row_map[all_ops_[op_idx]->inst_id] = row_range_map[RowType::OP_CONS].start + op_idx;
    }

    // 建立异格干员行定义，构造从干员行到异格干员行的映射
    UInt32 sp_group_cnt = 0;
    UInt32 sp_op_elem_cnt = 0;
    Vector<UInt32> op_row_to_sp_group_row_map(all_ops_.size(), UINT32_MAX);
    {
        Dictionary<std::string, Vector<UInt32>> sp_char_group_map;
        ResolveSpCharGroup(all_ops_, sp_char_group_map);
        auto sp_group_row_start_idx = row_range_map[RowType::ROOM_CONS].End();
        sp_group_cnt = sp_char_group_map.size();

        row_range_map[RowType::OP_MUTEX_CONS] = {
            sp_group_row_start_idx,
            sp_group_cnt
        };

        UInt32 group_idx = 0;
        for (const auto &[sp_group, ops] : sp_char_group_map)
        {
            for (auto op_idx : ops)
            {
                op_row_to_sp_group_row_map[op_inst_id_to_op_row_map[all_ops_[op_idx]->inst_id]] = sp_group_row_start_idx + group_idx;
            }
            sp_op_elem_cnt += ops.size() * (all_ops_.size() - ops.size()); // 偏大，忽略了其他互斥组
        }
    }

    const UInt32 col_cnt = total_solution_count;
    const UInt32 row_cnt = all_ops_.size() + rooms_.size() + sp_group_cnt;
    UInt32 elem_reserve_cnt = sp_op_elem_cnt;
    UInt32 elem_cnt = 0;
    for (int i = 0; i < (int)room_solutions.size(); ++i)
        elem_reserve_cnt += (1 + rooms_[i]->max_slot_count) * room_solutions[i].size();

    Vector<double> obj(col_cnt);
    Vector<double> elems(elem_reserve_cnt, 1);
    Vector<int> row_indices(elem_reserve_cnt);
    Vector<int> col_indices(elem_reserve_cnt);
    Vector<double> row_lb(row_cnt, 0);
    Vector<double> row_ub(row_cnt, 1);
    Vector<double> col_lb(col_cnt, 0);
    Vector<double> col_ub(col_cnt, 1);

    {
        UInt32 c = 0;
        for (const auto &solutions : room_solutions)
        {
            for (const auto &solution : solutions)
            {
                obj[c] = solution.productivity;
                if (std::abs(obj[c]) > 1e25)
                {
                    LOG_E("Invalid solution: ", obj[c], " at c#", c);
                    assert(false);
                    obj[c] = 0;
                }
                c++;
            }
        }
    }

    {
        UInt32 c = 0;
        for (UInt32 room_idx = 0; room_idx < room_solutions.size(); ++room_idx)
        {
            for (const auto &solution : room_solutions[room_idx])
            {
                // 干员约束
                for (const auto op : solution.operators)
                {
                    if (!op)
                        continue;

                    UInt32 op_row = op_inst_id_to_op_row_map[op->inst_id];

                    row_indices[elem_cnt] = (int)op_row;
                    col_indices[elem_cnt] = (int)c;
                    elem_cnt++;

                    // 异格约束
                    if (op_row_to_sp_group_row_map[op_row] != UINT32_MAX)
                    {
                        row_indices[elem_cnt] = (int)op_row_to_sp_group_row_map[op_row];
                        col_indices[elem_cnt] = (int)c;
                        elem_cnt++;
                    }
                }

                // 房间约束
                row_indices[elem_cnt] = (int)(row_range_map[RowType::ROOM_CONS].start + room_idx);
                col_indices[elem_cnt] = (int)c;
                elem_cnt++;
                c++;
            }
        }
    }

    LOG_D("Inserted ", elem_cnt, " elements out of ", elem_reserve_cnt, " reserved.");
    LOG_I("Solving using Cbc solver");
    {
        const auto &sc = SCOPE_TIMER_WITH_TRACE("Solving using Cbc solver");
        OsiClpSolverInterface solver;

        CoinPackedMatrix m(true, row_indices.data(), col_indices.data(), elems.data(), (int)elem_cnt);
        solver.loadProblem(m, col_lb.data(), col_ub.data(), obj.data(), row_lb.data(), row_ub.data());
        solver.messageHandler()->setLogLevel(1);
        solver.setHintParam(OsiDoReducePrint, true, OsiHintTry);
        for (int c = 0; c < (int)col_cnt; ++c)
            solver.setInteger(c);

        CbcModel model(solver);
        model.setDblParam(CbcModel::CbcMaximumSeconds, params_.solve_time_limit);
        model.setObjSense(-1);
        model.initialSolve();
        model.branchAndBound();

        switch (model.status())
        {
        case 0:
            // success
            break;

        case 1:
            LOG_W("Solving terminated.");
            if (model.secondaryStatus() == 4)
            {
                LOG_W("Solving time limit exceeded.");
            }
            else
            {
                LOG_W("Unrecognizable secondary status code: ", model.secondaryStatus());
            }
            break;

        default:
            LOG_E("Unrecognizable Cbc Model status code: ", model.status());
            return;
        }

        LOG_I("Solving finished. Optimal: ", model.isProvenOptimal());
        LOG_I("Objective value: ", model.getObjValue());

        if (!model.status() && model.getMinimizationObjValue() < 1e50)
        {
            UInt32 solution_cols = model.solver()->getNumCols();
            const double *solution = model.solver()->getColSolution();

            // print overall solution info
            for (UInt32 c = 0; c < solution_cols; ++c)
            {
                if (util::fp_eq(solution[c], 0.))
                    continue;

                UInt32 room_idx = GetRoomIdx(c, room_ranges);
                UInt32 sol_idx_in_room = GetIndexInRoom(c, room_ranges);
                char buf[128];
                char *p = buf;
                size_t l = sizeof(buf);
                double duration = room_solutions[room_idx][sol_idx_in_room].duration;
                double prod = obj[c];
                double time_eff = prod / duration;
                const auto &room = *rooms_[room_idx];
                util::append_snprintf(p, l, "Room#%d [Prod %10s][Ord %10s][nSlot %d]: avg %3.f%% (+%3.f%%) (%.2f / %.2f)",
                                room_idx,
                                util::enum_to_string(room.room_attributes.prod_type).data(),
                                util::enum_to_string(room.room_attributes.order_type).data(),
                                room.max_slot_count,
                                time_eff * 100,
                                (time_eff - 1) * 100,
                                prod,
                                duration);
                LOG_I(buf);
            }

            // print solution details
            for (UInt32 c = 0; c < solution_cols; ++c)
            {
                if (util::fp_eq(solution[c], 0.))
                    continue;

                UInt32 room = GetRoomIdx(c, room_ranges);
                UInt32 sol_idx_in_room = GetIndexInRoom(c, room_ranges);
                LOG_D("***** Solution: col#", c, " at room#", room, " index#", sol_idx_in_room, " *****");
                LOG_D(GetSolutionInfo(*rooms_[room], room_solutions[room][sol_idx_in_room]));
            }

            for (UInt32 c = 0; c < solution_cols; ++c)
            {
                if (util::fp_eq(solution[c], 0.))
                    continue;

                UInt32 room = GetRoomIdx(c, room_ranges);
                UInt32 sol_idx_in_room = GetIndexInRoom(c, room_ranges);
                auto &room_result = out_result.rooms.emplace_back();
                room_result.room = rooms_[room];
                room_result.solution = room_solutions[room][sol_idx_in_room];
            }
        }
    }

    if (params_.gen_lp_file)
    {
        GenLpFile(room_solutions, obj, row_cnt, col_cnt, elems, row_indices, col_indices, row_range_map, row_ub);
    }

    if (params_.gen_all_solution_details)
    {
        GenSolDetails(rooms_, room_solutions, room_ranges, total_solution_count);
    }
}

void MultiRoomIntegerProgramming::GenCombForRooms(Vector<Vector<SolutionData>> &room_solutions,
                                                  Vector<UInt32> &room_ranges, UInt32 &col_cnt)
{
    const auto &sc = SCOPE_TIMER_WITH_TRACE("Generating combinations");
    for (auto room : this->rooms_)
    {
        this->FilterOperators(room);
        if (inbound_ops_.empty())
        {
            LOG_W("No inbound operators for room#", room->id);
        }

        AllSolutionHolder solution_holder;
        this->MakeComb(this->inbound_ops_, room->max_slot_count, room, solution_holder);

        if (solution_holder.solutions.empty())
        {
            LOG_W("No solution for room ", room->id);
        }

        room_ranges.push_back(col_cnt);
        col_cnt += solution_holder.solutions.size();
        room_solutions.emplace_back(std::move(solution_holder.solutions));
    }
    LOG_I("Generated ", col_cnt, " combinations.");
}

void MultiRoomIntegerProgramming::GenLpFile(Vector<Vector<SolutionData>> &room_solutions, const Vector<double> &obj,
                                            UInt32 row_cnt, UInt32 col_cnt, const Vector<double> &elems,
                                            const Vector<int> &row_indices, Vector<int> &col_indices,
                                            const RowRangeMap& ranges, const Vector<double> &row_ub) const
{
    const auto lp_file_path = "./problem.lp";
    const auto &sc = SCOPE_TIMER_WITH_TRACE("Writing LP File");
    LOG_I("Exporting LP file:", lp_file_path);

    std::ofstream lp_file(lp_file_path);
    if (!lp_file.is_open())
    {
        LOG_E("open lp file failed: ", lp_file_path);
    }

    Vector<std::string> col_name_map(col_cnt);
    for (auto &col_name : col_name_map)
        col_name.reserve(48);

    UInt64 col = 0;
    UInt32 room_idx = 0;
    // regex: match "char_<number>_<char_name>", and extract <char_name>
    std::regex e("char_(\\d+)_(.+)");
    for (const auto &solutions : room_solutions)
    {
        for (const auto &solution : solutions)
        {
            auto &col_name = col_name_map[col];

            col_name.append("x");
            col_name.append(std::to_string(col));
            col_name.append("_");
            col_name.append(this->rooms_[room_idx]->id);
            for (const auto op : solution.operators)
            {
                if (!op)
                    continue;

                col_name.append("_");
                // extract char_name
                std::smatch m;
                std::regex_search(op->char_id, m, e);
                // if not matched, use char_id directly
                if (m.empty())
                    col_name.append(op->char_id);
                else
                    col_name.append(m[2]);
            }
            col++;
        }
        ++room_idx;
    }

    lp_file << R"(\Arknights building arrangement problem generated by algorithm.)" << std::endl;
    lp_file << R"(\Problem info:)" << std::endl;
    lp_file << R"(\  - Room count: )" << this->rooms_.size() << std::endl;
    lp_file << R"(\  - Combination count: )" << col_cnt << std::endl;
    lp_file << R"(\  - Model time limit: )" << this->params_.model_time_limit << std::endl;
    lp_file
        << R"(\To view details of all combinations, please add cli parameter "--solution-detail"/"-S" or set AlbcSolverParameters.gen_all_solution_details to true in api.)"
        << std::endl;
    lp_file << "Maximize\n";
    lp_file << " obj: ";

    for (UInt32 c = 0; c < col_cnt; c++)
    {
        lp_file << obj[c] << ' ' << col_name_map[c];
        if (c != col_cnt - 1)
        {
            lp_file << " + ";
        }
    }

    Vector<Vector<std::pair<int, double>>> row_elems(row_cnt);
    for (int i = 0; i < (int)elems.size(); i++)
    {
        row_elems[row_indices[i]].emplace_back(col_indices[i], elems[i]);
    }

    lp_file << "\nSubject To\n";
    for (UInt32 r = 0; r < row_cnt; r++)
    {
        bool has_constraint = false;
        std::string row_name;
        size_t row_index_in_type;
        switch(ranges.GetType(r, row_index_in_type))
        {
        case RowType::NONE:
            row_name = std::string("c").append(std::to_string(r));
            break;

        case RowType::OP_CONS:
            row_name = this->all_ops_[row_index_in_type]->char_id;
            break;

        case RowType::ROOM_CONS:
            row_name = this->rooms_[row_index_in_type]->id;
            break;

        case RowType::OP_MUTEX_CONS:
            row_name = std::string("sp_char_mutex_").append(std::to_string(row_index_in_type));
            break;
        }

        for (const auto &[c, elem] : row_elems[r])
        {
            if (!has_constraint)
            {
                lp_file << " "
                        << row_name
                        << ": ";
                has_constraint = true;
            }
            else
            {
                lp_file << " + ";
            }

            if (!util::fp_eq(elem, 1.))
            {
                lp_file << elem << col_name_map[c];
            }
            else
            {
                lp_file << col_name_map[c];
            }
        }

        if (has_constraint)
        {
            lp_file << " <= " << row_ub[r] << "\n";
        }
    }

    lp_file << "Binary\n";
    for (UInt32 i = 0; i < col_cnt; i++)
    {
        lp_file << col_name_map[i] << "\n";
    }

    lp_file << "End\n";
    lp_file.close();
}

void MultiRoomIntegerProgramming::GenSolDetails(const Vector<model::buff::RoomModel *> &rooms,
                                                const Vector<Vector<SolutionData>> &room_solutions,
                                                Vector<UInt32> &room_ranges, size_t col_cnt)
{
    const auto sol_details_file_path = "./solution_details.txt";
    const auto &sc = SCOPE_TIMER_WITH_TRACE("Writing solution details to File");
    LOG_I("Exporting solution details file:", sol_details_file_path);

    std::ofstream sol_details_file(sol_details_file_path);
    if (!sol_details_file.is_open())
    {
        LOG_E("open solution details file failed: ", sol_details_file_path);
        return;
    }

    // print room details
    for (UInt64 i = 0; i < rooms.size(); i++)
    {
        const auto* room = rooms[i];
        sol_details_file << "######## Room#" << i <<": " << room->id << std::endl;
        sol_details_file << room->to_string() << std::endl;
    }

    // print solution details
    for (UInt64 c = 0; c < col_cnt; ++c)
    {
        UInt64 room_idx = -1;
        UInt64 sol_idx_in_room = 0;
        for (auto si : room_ranges)
        {
            if (c >= si)
            {
                room_idx++;
                sol_idx_in_room = c - si;
            }
        }

        sol_details_file << "######## x" << c + 1 << ", Room#" << room_idx << "#" << sol_idx_in_room << std::endl;
        sol_details_file << room_solutions[room_idx][sol_idx_in_room].ToString() << std::endl;
    }
    sol_details_file.close();
}
UInt32 MultiRoomIntegerProgramming::GetRoomIdx(UInt32 col, const Vector<UInt32> &room_ranges)
{
    for (UInt32 i = 0; i < room_ranges.size(); ++i)
    {
        if (col < room_ranges[i])
        {
            return i - 1;
        }
    }

    return room_ranges.size() - 1;
}
UInt32 MultiRoomIntegerProgramming::GetIndexInRoom(UInt32 col, const Vector<UInt32> &room_ranges)
{
    return col - room_ranges[GetRoomIdx(col, room_ranges)];
}
} // namespace albc::algorithm
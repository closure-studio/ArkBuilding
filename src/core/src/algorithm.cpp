#include "algorithm.h"
#include "flag_util.h"
#include "locale_util.h"
#include "simulator.h"

#include "CbcModel.hpp"
#include "OsiCbcSolverInterface.hpp"

#include <bitset>
#include <random>
#include <fstream>

string SolutionData::ToString(double max_allowed_duration) const
{
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
            append_snprintf(p, size, "\tBuff #%d: %8s %s\n", n_buff, toOSCharset(buff->name).c_str(),
                            buff->buff_id.c_str());
            append_snprintf(p, size, "\t%s\n", toOSCharset(buff->description).c_str());
            append_snprintf(p, size, "\tMod:      %s\n\tFinal Mod:%s\n\tCost Mod: %s\n\n",
                            snapshot[n_op - 1][n_buff - 1].room_mod.to_string().c_str(),
                            snapshot[n_op - 1][n_buff - 1].final_mod.to_string().c_str(),
                            snapshot[n_op - 1][n_buff - 1].cost_mod.to_string().c_str());

            ++n_buff;
        }

        ++n_op;
    }
    double avg_prod = productivity / max_allowed_duration * 100.;
    append_snprintf(p, size, "**** Total productivity: %.2f in %.2fs, avg scale: %.2f%% (+%.2f%%) ****\n",
                    productivity, max_allowed_duration, avg_prod, avg_prod - 100);

    return buf;
}

string Algorithm::GetSolutionInfo(const RoomModel &room, const SolutionData &solution) const
{
    std::string result;
    result
        .append("**** Solution ****\n")
        .append(room.to_string())
        .append(solution.ToString());
    return result;
}

// this function is a transform from recursive DFS to iterative DFS, using stack
template <typename TSolutionHolder>
void BruteForce::MakeComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                          TSolutionHolder &solution_holder)
{
    max_n = std::min(max_n, static_cast<UInt32>(operators.size()));
    std::bitset<kAlgOperatorSize> all_ops;
    all_ops.flip();

    HardMutexResolver mutex_handler(operators, room->type);
    UInt32 calc_cnt = n_choose_k(mutex_handler.non_mutex_ops.size(), max_n);
    auto mutex_cnt = mutex_handler.MutexCombCnt();
    calc_cnt += mutex_cnt * (n_choose_k(mutex_handler.ops_for_partial_comb.size(), max_n) - calc_cnt);
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
namespace albc::algorithm
{
void Algorithm::FilterOperators(const RoomModel *room)
{
    inbound_ops_.clear();

    for (auto *const ops : all_ops_)
    {
        if (ops->buffs.empty())
        {
            continue;
        }

        if (!check_flag(ops->room_type_mask, room->type))
        {
            continue;
        }

        if (!std::all_of(ops->buffs.begin(), ops->buffs.end(),
                         [room](RoomBuff *buff) -> bool { return buff->ValidateTarget(room); }))
        {
            continue;
        }

        inbound_ops_.push_back(ops);
    }
    LOG_D << "Filtered " << inbound_ops_.size() << " operators for room: " << room->id << " : [P]"
          << to_string(room->room_attributes.prod_type) << " [O]" << to_string(room->room_attributes.order_type)
          << std::endl;
}

template <typename TSolutionHolder>
void CombMaker::MakePartialComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                                const std::bitset<kAlgOperatorSize> &enabled_root_ops,
                                TSolutionHolder &solution_holder) const
// root operator is the first operator in a DFS path
{
    UInt32 size = operators.size();
    max_n = std::min(size, max_n);
    UInt32 calc_cnt = 0;
    UInt32 pos[kRoomMaxBuffSlots]{}; // pos[i] is the index of i-th recursion
                                     // a position indicates the index of the operator in the operators vector

    UInt32 buff_cnt[kRoomMaxBuffSlots]{}; // buff_cnt[i] is the number of buffs in i-th recursion
    bool status[kRoomMaxBuffSlots]{};     // status[i] is the status of i-th recursion
    Vector<OperatorModel *> current(
        max_n); // stores the ops_for_partial_comb operators, including operators in a unique combination
    Vector<OperatorModel *> solution(max_n); // stores the best operators
    double max_duration = Algorithm::max_allowed_duration_;
    bool is_all_ops = enabled_root_ops.all(); // avoid unnecessary checks of the root operators

    UInt32 dep = 0; // depth of recursion, when dep + 1 == max_n, we have a unique combination
    while (true)
    {                               // when dep >= 0, the root of the recursion is not finished
        UInt32 &cur_pos = pos[dep]; // ops_for_partial_comb position
        bool &cur_status = status[dep];
        if (!cur_status) // pushing
        {
            cur_status = true;

            if (is_all_ops || dep > 0 || enabled_root_ops[cur_pos])
            {
                current[dep] = operators[cur_pos];
                for (auto *const buff : current[dep]->buffs)
                {
                    if (buff == nullptr || !check_flag(buff->room_type, room->type))
                    {
                        continue;
                    }
                    room->PushBuff(buff);
                    ++buff_cnt[dep];
                }

                if (dep >= max_n - 1)
                {
                    ++calc_cnt;
                    double result;
                    Simulator::DoCalc(room, max_duration, result);
                    solution_holder.OnSolutionFound(current, result);
                }
                else
                {
                    // in the next recursion, the position of the operator will be increased by 1
                    pos[dep + 1] = cur_pos + 1;
                    ++dep; // move to the next recursion
                    continue;
                }
            }
        }

        if (cur_status) // popping
        {
            room->n_buff -= buff_cnt[dep];
            buff_cnt[dep] = 0;

            cur_status = false;
            if (cur_pos < size - max_n + dep)
            {
                ++cur_pos; // move to the next operator
            }
            else
            {
                if (dep <= 0) break;
                
                --dep; // the list of operators is exhausted, move to the previous recursion
            }
        }
    }

    solution_holder.UpdateCalcCnt(calc_cnt);
}

void BruteForce::Run()
{
    auto *const room = rooms_[0];

    // filter operators
    FilterOperators(room);

    GreedySolutionHolder solution_holder;

    // measures the time of MakeComb()
    const double elapsedSec = MeasureTime(&BruteForce::MakeComb<GreedySolutionHolder>, this, inbound_ops_,
                                          room->max_slot_count, room, solution_holder)
                                  .count();

    // prints the number of calculations
    LOG_D << "calc cnt: " << solution_holder.calc_cnt << std::endl;

    // print elapsed time
    LOG_D << "elapsed time: " << elapsedSec << std::endl;

    // prints average calculations per second
    LOG_D << "calc/sec: " << solution_holder.calc_cnt / elapsedSec << std::endl;

    // prints the best operators
    if (!solution_holder.max_solution.operators.empty())
    {
        LOG_D << GetSolutionInfo(*room, solution_holder.max_solution) << std::endl;
    }
    else
    {
        LOG_E << "No operators!" << std::endl;
    }
}

HardMutexResolver::HardMutexResolver(const Vector<OperatorModel *> &ops, bm::RoomType room_type)
// Hard mutual exclusion: make sure that no two operators has the same type of mutex buff in the same time
{
    static constexpr size_t buff_type_cnt = enum_size<RoomBuffType>::value;
    UInt32 buff_type_map[buff_type_cnt];
    std::fill_n(buff_type_map, buff_type_cnt, buff_type_cnt);

    int op_idx = -1;
    for (const auto op : ops)
    {
        ++op_idx;
        bool op_has_mutex_buff = false;

        for (const auto buff : op->buffs)
        // NOTE: this method is based on the assumption that each operator has at most one mutex buff
        // if not so, we have to change the logic
        {
            if (buff->room_type == room_type && buff->is_mutex)
            {
                if (op_has_mutex_buff)
                {
                    LOG_E << "Logic error: operator " << op->char_id << " has more than one mutex buff" << endl;
                    LOG_E << "The buff: " << buff->buff_id << " will be ignored" << endl;
                    continue;
                }

                auto &map_val = buff_type_map[static_cast<UInt32>(buff->inner_type)];
                if (map_val >= buff_type_cnt)
                {
                    map_val = mutex_groups_.size();
                    mutex_groups_.emplace_back();
                }

                mutex_groups_[map_val].push_back(op);

                op_has_mutex_buff = true;
            }
        }

        if (op_has_mutex_buff)
        {
            mutex_ops[op_idx] = true;
        }
    }

    for (const auto &group : mutex_groups_)
    {
        ops_for_partial_comb.push_back(group[0]);
    }

    op_idx = -1;
    for (const auto op : ops)
    {
        ++op_idx;
        if (!mutex_ops[op_idx])
        {
            ops_for_partial_comb.push_back(op);
            non_mutex_ops.push_back(op);
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
                           [](UInt32 sum, const Vector<OperatorModel *> &group) { return sum * group.size(); });
}
UInt32 HardMutexResolver::MutexGroupCnt() const
{
    return mutex_groups_.size();
}

void MultiRoomGreedy::Run()
{
    // 打乱房间排序，该算法会运行多次以尽可能接近最优解
    std::shuffle(rooms_.begin(), rooms_.end(), std::mt19937(std::random_device()()));

    std::sort(rooms_.begin(), rooms_.end(),
              [](const RoomModel *a, const RoomModel *b) { return a->max_slot_count > b->max_slot_count; });
    
    for (auto room : rooms_)
    {
        GreedySolutionHolder solution_holder;

        FilterOperators(room);
        MakeComb(inbound_ops_, room->max_slot_count, room, solution_holder);
        if (solution_holder.max_solution.operators.empty())
        {
            LOG_W << "No solution for room " << room->id << std::endl;
            continue;
        }

        if (GlobalLogConfig::CanLog(LogLevel::DEBUG))
        {
            LOG_D << GetSolutionInfo(*room, solution_holder.max_solution) << std::endl;
        }

        room_solution_map_[room->id] = solution_holder.max_solution;

        // 剔除已选择的干员
        all_ops_.erase(std::remove_if(all_ops_.begin(), all_ops_.end(),
                                      [solution_holder](const OperatorModel *op) {
                                          return std::find(solution_holder.max_solution.operators.begin(),
                                                           solution_holder.max_solution.operators.end(),
                                                           op) != solution_holder.max_solution.operators.end();
                                      }),
                       all_ops_.end());

    }
}

void MultiRoomIntegerProgramming::Run()
{
    Vector<Vector<SolutionData>> room_solution_map;
    Vector<UInt64> room_solution_start_idx;
    size_t total_solution_count = 0;
    for (auto room: rooms_)
    {
        FilterOperators(room);

        AllSolutionHolder solution_holder;
        MakeComb(inbound_ops_, room->max_slot_count, room, solution_holder);

        if (solution_holder.solutions.empty())
        {
            LOG_W << "No solution for room " << room->id << std::endl;
            continue;
        }

        room_solution_start_idx.push_back(total_solution_count);
        total_solution_count += solution_holder.solutions.size();
        room_solution_map.emplace_back(std::move(solution_holder.solutions));
    }

    if (total_solution_count < 1) {
        LOG_E << "No solution!" << endl;
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

    auto mip = std::make_unique<OsiCbcSolverInterface>();
    const auto var_cnt = total_solution_count;

    Vector<double> w(var_cnt);
    Vector<double> z(var_cnt);
    Vector<double> bndl(var_cnt);
    Vector<double> bndu(var_cnt);

    Int64 comb_idx = 0;
    for (const auto& room_solutions : room_solution_map)
    {
        for (const auto& solution : room_solutions)
        {
            w[comb_idx] = solution.productivity;
            comb_idx++;
        }
    }
    for (UInt64 i = 0; i < total_solution_count; i++)
    {
        bndl[i] = 0;
        bndu[i] = 1;
    }

    // 构造干员inst_id到干员在all_ops_中的索引的映射
    Vector<UInt32> op_inst_id_map(kAlgOperatorSize);
    std::fill(op_inst_id_map.begin(), op_inst_id_map.end(), 0);

    Vector<UInt32> op_inst_ids(all_ops_.size());
    std::transform(all_ops_.begin(), all_ops_.end(), op_inst_ids.begin(),
                   [](const OperatorModel *op) { return op->inst_id; });

    std::sort(op_inst_ids.begin(), op_inst_ids.end());
    for (UInt32 i = 0; i < op_inst_ids.size(); i++)
    {
        op_inst_id_map[op_inst_ids[i]] = i;
    }
    
    // 约束：每个干员最多只能选择一次， 每个房间最多选择一个组合
    // 约束个数 = 干员数 + 房间数
    const auto cons_cnt = all_ops_.size() + rooms_.size();
    const auto room_cons_idx = all_ops_.size(); // 房间约束的起始索引

    Vector<Vector<double>> a(cons_cnt, Vector<double>(var_cnt, 0));
    Vector<double> al(cons_cnt);
    Vector<double> au(cons_cnt);

    for (UInt32 i = 0; i < cons_cnt; i++)
    {
        al[i] = 0;
        au[i] = 1;
    }

    comb_idx = 0;
    for (UInt32 room_idx = 0; room_idx < room_solution_map.size(); ++room_idx)
    {
        for (const auto& solution : room_solution_map[room_idx])
        {
            // 干员约束
            for (const auto& op : solution.operators)
            {
                a[op_inst_id_map[op->inst_id]][comb_idx] = 1;
            }

            // 房间约束
            a[room_cons_idx + room_idx][comb_idx] = 1;
            comb_idx++;
        }
    }



    // generate .lp format file
    const auto lp_file_name = "./solution.lp";
    const auto sol_details_file_name = "./solution_details.txt";
    std::ofstream lp_file(lp_file_name);
    if (!lp_file.is_open())
    {
        LOG_E << "open lp file failed: " << lp_file_name << std::endl;
        return;
    }

    lp_file << "Maximize\n";
    lp_file << " obj: ";
    for (UInt32 i = 0; i < total_solution_count; i++)
    {
        lp_file << w[i] << " x" << i + 1;
        if (i != total_solution_count - 1)
        {
            lp_file << " + ";
        }
    }

    lp_file << "\nSubject To\n";
    for (UInt32 i = 0, cur_c = 1; i < cons_cnt; i++)
    {
        bool has_constraint = false;
        for (UInt32 j = 0; j < total_solution_count; j++)
        {
            if (a[i][j] <= 0) continue;

            if (!has_constraint)
            {
                lp_file << " c" << cur_c++ << ": ";
                has_constraint = true;
            } else {
                lp_file << " + ";
            }
            
            if (!fp_eq(a[i][j], 1.)) {
                lp_file << a[i][j] << " x" << j + 1;
            } else {
                lp_file << "x" << j + 1;
            }
        }

        if (has_constraint)
        {
            lp_file << " <= " << au[i] << "\n";
        }
    }

    lp_file << "Binary\n";
    for (UInt32 i = 0; i < total_solution_count; i++)
    {
        lp_file << " x" << i + 1 << "\n";
    }

    lp_file << "End\n";
    lp_file.close();

    std::ofstream sol_details_file(sol_details_file_name);
    for (UInt64 i = 0; i < total_solution_count; ++i) {
        UInt64 room_idx = -1;
        UInt64 sol_idx_in_room = 0;
        for (auto si: room_solution_start_idx) {
            if (i >= si) {
                room_idx++;
                sol_idx_in_room = i - si;
            }
        }

        sol_details_file << "######## x" << i + 1 << ", Room#" << room_idx << ", Index In Room#" << sol_idx_in_room << endl;
        sol_details_file << room_solution_map[room_idx][sol_idx_in_room].ToString() << endl;
    }
    sol_details_file.close();
}
} // namespace albc::algorithm
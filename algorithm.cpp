#include "algorithm.h"
#include "flag_util.h"
#include "locale_util.h"
#include "simulator.h"
#include <bitset>

namespace albc::algorithm
{
void Algorithm::FilterAgents(const RoomModel *room)
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

void BruteForce::Run()
{
    auto *const room = rooms_[0];
    calc_cnt_ = 0;

    // filter operators
    FilterAgents(room);

    // measures the time of MakeComb()
    const double elapsedSec =
        MeasureTime(&BruteForce::MakeComb, this, inbound_ops_, room->max_slot_count, room).count();

    // prints the number of calculations
    LOG_D << "calc cnt: " << calc_cnt_ << std::endl;

    // print elapsed time
    LOG_D << "elapsed time: " << elapsedSec << std::endl;

    // prints average calculations per second
    LOG_D << "calc/sec: " << calc_cnt_ / elapsedSec << std::endl;

    // prints the best solution
    if (!solution_.empty())
    {
        PrintSolution(*room);
    }
    else
    {
        LOG_W << "No solution!" << std::endl;
    }
}

// this function is a transform from recursive DFS to iterative DFS, using stack
void BruteForce::MakeComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room)
{
    max_n = std::min(max_n, static_cast<UInt32>(operators.size()));
    std::bitset<kAlgOperatorSize> enabled_ops;
    enabled_ops.flip();
    HardMutexResolver mutex_handler(operators, room->type);

    MakePartialCombAndUpdateSolution(mutex_handler.non_mutex_ops, max_n, room, enabled_ops);

    if (mutex_handler.HasMutexBuff())
    {
        do
        {
            MakePartialCombAndUpdateSolution(mutex_handler.ops_for_partial_comb, max_n, room,
                                             mutex_handler.enabled_ops_for_partial_comb);
        } while (mutex_handler.MoveNext());
    }
}

void BruteForce::MakePartialCombAndUpdateSolution(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
                                                  const std::bitset<kAlgOperatorSize> &enabled_root_ops)
{
    auto [max_delta, calc_cnt, solution, snapshot] = MakePartialComb(operators, max_n, room, enabled_root_ops);

    calc_cnt_ += calc_cnt;
    if (max_delta > max_tot_delta_)
    {
        solution_.assign(solution.begin(), solution.end());
        snapshot_.assign(snapshot.begin(), snapshot.end());
        max_tot_delta_ = max_delta;
    }
}

std::tuple<double, UInt32, Vector<OperatorModel *>, Vector<Vector<ModifierApplier>>> BruteForce::MakePartialComb(
    const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
    const std::bitset<kAlgOperatorSize> &enabled_root_ops) const
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
        max_n); // stores the ops_for_partial_comb solution, including operators in a unique combination
    Vector<OperatorModel *> solution(max_n); // stores the best solution
    Vector<Vector<ModifierApplier>> snapshot(max_n);
    double max_tot_delta = 0.; // max delta of the total production of the solution
    double max_duration = max_allowed_duration_;
    bool is_all_ops = enabled_root_ops.all(); // avoid unnecessary checks of the root operators

    int dep = 0; // depth of recursion, when dep + 1 == max_n, we have a unique combination
    while (dep >= 0)
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
                    double result, loss;
                    Simulator::DoCalc(room, max_duration, result, loss);
                    if (result > max_tot_delta)
                    {
                        solution.assign(current.begin(), current.begin() + max_n);
                        std::transform(current.begin(), current.begin() + max_n, snapshot.begin(),
                                       [](const OperatorModel *o) {
                                           Vector<ModifierApplier> mods(o->buffs.size());
                                           std::transform(o->buffs.begin(), o->buffs.end(), mods.begin(),
                                                          [](const RoomBuff *b) { return b->applier; });

                                           return mods;
                                       });

                        max_tot_delta = result;
                    }
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
                --dep; // the list of operators is exhausted, move to the previous recursion
            }
        }
    }

    return std::make_tuple(max_tot_delta, calc_cnt, solution, snapshot);
}

void BruteForce::PrintSolution(const RoomModel &room, const LogLevel log_level)
{
    if (!GlobalLogConfig::CanLog(log_level))
    {
        return;
    }
    
    int n_op = 1;
    printf("**** Solution ****\n");
    printf(room.to_string().c_str());
    for (auto *const op : solution_)
    {
        if (op == nullptr)
        {
            break;
        }

        printf("**** Operator #%d: %20s ****\n", n_op, op->char_id.c_str());

        int n_buff = 1;
        for (const auto &buff : op->buffs)
        {
            printf("\tBuff #%d: %8s %s\n", n_buff, toOSCharset(buff->name).c_str(), buff->buff_id.c_str());
            printf("\t%s\n", toOSCharset(buff->description).c_str());
            printf("\tMod:      %s\n\tFinal Mod:%s\n\tCost Mod: %s\n\n",
                   snapshot_[n_op-1][n_buff-1].room_mod.to_string().c_str(),
                   snapshot_[n_op-1][n_buff-1].final_mod.to_string().c_str(),
                   snapshot_[n_op-1][n_buff-1].cost_mod.to_string().c_str());

            ++n_buff;
        }

        ++n_op;
    }
    double avg_prod = max_tot_delta_ / max_allowed_duration_ * 100.;
    printf("**** Total productivity: %.2f in %.2fs, avg scale: %.2f%% (+%.2f%%) ****\n", max_tot_delta_,
           max_allowed_duration_, avg_prod, avg_prod - 100);
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

    group_pos_.resize(mutex_groups_.size(), 0);
    for (int i = 0; i < mutex_groups_.size(); ++i)
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
bool HardMutexResolver::HasMutexBuff()
{
    return !mutex_groups_.empty();
}
} // namespace albc
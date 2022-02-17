#include "algorithm.h"
#include "flag_util.h"
#include "locale_util.h"
#include "simulator.h"
#include <bitset>
#include <random>

string Algorithm::GetSolutionInfo(const RoomModel &room, const SolutionData &solution) const
{
    char buf[16384];
    char *p = buf;
    size_t size = sizeof buf;

    int n_op = 1;
    append_snprintf(p, size, "**** Solution ****\n");
    append_snprintf(p, size, room.to_string().c_str());
    for (auto *const op : solution.operators)
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
                            solution.snapshot[n_op - 1][n_buff - 1].room_mod.to_string().c_str(),
                            solution.snapshot[n_op - 1][n_buff - 1].final_mod.to_string().c_str(),
                            solution.snapshot[n_op - 1][n_buff - 1].cost_mod.to_string().c_str());

            ++n_buff;
        }

        ++n_op;
    }
    double avg_prod = solution.productivity / max_allowed_duration_ * 100.;
    append_snprintf(p, size, "**** Total productivity: %.2f in %.2fs, avg scale: %.2f%% (+%.2f%%) ****\n",
                    solution.productivity, max_allowed_duration_, avg_prod, avg_prod - 100);

    return buf;
}
void Algorithm::UpdateSolution(double max_delta, UInt32 calc_cnt, Vector<OperatorModel *> solution,
                               Vector<Vector<ModifierApplier>> snapshot)
{
    current_solution_.calc_cnt += calc_cnt;
    if (max_delta > current_solution_.productivity)
    {
        current_solution_.operators.assign(solution.begin(), solution.end());
        current_solution_.snapshot.assign(snapshot.begin(), snapshot.end());
        current_solution_.productivity = max_delta;
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
void BruteForce::MakePartialCombAndUpdateSolution(const Vector<OperatorModel *> &operators, UInt32 max_n,
                                                  RoomModel *room,
                                                  const std::bitset<kAlgOperatorSize> &enabled_root_ops)
{
    std::apply(&BruteForce::UpdateSolution,
               std::tuple_cat(std::make_tuple(this), MakePartialComb(operators, max_n, room, enabled_root_ops)));
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
void Algorithm::ResetSolution()
{
    current_solution_.Reset();
}

std::tuple<double, UInt32, Vector<albc::OperatorModel *>, Vector<Vector<albc::ModifierApplier>>> CombMaker::
    MakePartialComb(const Vector<OperatorModel *> &operators, UInt32 max_n, RoomModel *room,
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
        max_n); // stores the ops_for_partial_comb operators, including operators in a unique combination
    Vector<OperatorModel *> solution(max_n); // stores the best operators
    Vector<Vector<ModifierApplier>> snapshot(max_n);
    double max_tot_delta = 0.; // max delta of the total production of the operators
    double max_duration = Algorithm::max_allowed_duration_;
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

void BruteForce::Run()
{
    auto *const room = rooms_[0];
    ResetSolution();

    // filter operators
    FilterOperators(room);

    // measures the time of MakeComb()
    const double elapsedSec =
        MeasureTime(&BruteForce::MakeComb, this, inbound_ops_, room->max_slot_count, room).count();

    // prints the number of calculations
    LOG_D << "calc cnt: " << current_solution_.calc_cnt << std::endl;

    // print elapsed time
    LOG_D << "elapsed time: " << elapsedSec << std::endl;

    // prints average calculations per second
    LOG_D << "calc/sec: " << current_solution_.calc_cnt / elapsedSec << std::endl;

    // prints the best operators
    if (!current_solution_.operators.empty())
    {
        LOG_D << GetSolutionInfo(*room, current_solution_) << std::endl;
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

MultiRoomGreedy::MultiRoomGreedy(const PtrVector<RoomModel> &rooms, const PtrVector<OperatorModel> &operators,
                                 double max_allowed_duration)
    : BruteForce(rooms, operators, max_allowed_duration)
{
    // 打乱房间排序，该算法会运行多次以尽可能接近最优解
    std::shuffle(rooms_.begin(), rooms_.end(), std::mt19937(std::random_device()()));

    std::sort(rooms_.begin(), rooms_.end(),
              [](const RoomModel *a, const RoomModel *b) { return a->max_slot_count > b->max_slot_count; });
}

void MultiRoomGreedy::Run()
{
    for (auto room : rooms_)
    {
        FilterOperators(room);
        MakeComb(inbound_ops_, room->max_slot_count, room);
        if (current_solution_.operators.empty())
        {
            LOG_W << "No solution for room " << room->id << std::endl;
            continue;
        }

        LOG_D << GetSolutionInfo(*room, current_solution_) << std::endl;

        room_solution_map_[room->id] = current_solution_;

        // 剔除已选择的干员
        all_ops_.erase(std::remove_if(all_ops_.begin(), all_ops_.end(),
                                      [this](const OperatorModel *op) {
                                          return std::find(current_solution_.operators.begin(),
                                                           current_solution_.operators.end(),
                                                           op) != current_solution_.operators.end();
                                      }),
                       all_ops_.end());

        ResetSolution();
    }
}
} // namespace albc::algorithm
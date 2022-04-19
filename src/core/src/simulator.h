#pragma once

#include "buff_model.h"
#include "func_piecewise.h"
#include "primitive_types.h"
#include "util.h"

#define SIMULATOR_UNROLL_MAX_BUFF_CNT UNROLL_LOOP(albc::model::buff::kRoomMaxBuffSlots)

namespace albc::model::buff
{
class Simulator
{
  public:
    static void 
    ALBC_FLATTEN
    ALBC_INLINE
    DoCalc(const RoomModel *room, double max_allowed_duration, double& result, double& duration)
    {
        PiecewiseMap<kFuncPiecewiseMaxSegmentCount> eff_piecewise;
        ModifierScopeData scope;
        scope.room = room;

        SIMULATOR_UNROLL_MAX_BUFF_CNT
        for (UInt32 i = 0; i < room->n_buff; ++i)
        {
            room->buffs[i]->UpdateScopeOnNeed(scope);
        }

        double char_cost_mod[kRoomMaxBuffSlots]{}; // cost modifier of each buff slot
        double room_cost_mul = 1;
        SIMULATOR_UNROLL_MAX_BUFF_CNT
        for (UInt32 i = 0; i < room->n_buff; ++i)
        {
            const auto &cost_mod = room->buffs[i]->applier.cost_mod;

            if (cost_mod.type == CharCostModifierType::ROOM_CLEAR_ALL)
            {
                std::fill_n(char_cost_mod, kRoomMaxBuffSlots, 0);
                room_cost_mul = 1; // reset room cost multiplier
                break;
            }

            switch (cost_mod.type)
            {
            case CharCostModifierType::NONE:
                break;

            case CharCostModifierType::SELF:
                char_cost_mod[i] += cost_mod.value;
                break;

            case CharCostModifierType::ROOM_ALL:
                room_cost_mul += cost_mod.value;
                break;

            case CharCostModifierType::ROOM_EXCEPT_SELF:
                room_cost_mul += cost_mod.value;
                char_cost_mod[i] -= cost_mod.value; // subtract from self
                break;

            case CharCostModifierType::ROOM_CLEAR_ALL:
            default:
                ALBC_UNREACHABLE();
            }
        }

        double estimated_duration = INFINITY; // estimated duration of the room
        double base_acc = 0;                  // base productivity acceleration, unit: 1/s
        int buff_cnt = 0;
        SIMULATOR_UNROLL_MAX_BUFF_CNT
        for (UInt32 i = 0; i < room->n_buff; ++i)
        {
            ++buff_cnt;
            const double cost_mul = room_cost_mul + char_cost_mod[i];
            estimated_duration =
                std::min(estimated_duration, cost_mul > 0 ? room->buffs[i]->duration / cost_mul : estimated_duration);
        }

        estimated_duration = std::min(estimated_duration, max_allowed_duration);

        int base_cap_delta = 0;    // base capacity delta, unit: 1
        double base_eff_delta = 0; // base effective delta, unit: 1
        SIMULATOR_UNROLL_MAX_BUFF_CNT
        for (UInt32 i = 0; i < room->n_buff; ++i)
        {
            const auto &buff_mod = room->buffs[i]->applier.room_mod;
            if (!buff_mod.IsValid())
                continue;

            base_cap_delta += buff_mod.cap_delta;
            base_eff_delta += buff_mod.eff_delta;

            if (!util::fp_eq(buff_mod.eff_inc_per_hour, 0.))
            {
                const double acc = buff_mod.eff_inc_per_hour * 2.777777777777778e-4; // div by 3600, unit: 1/s
                base_acc += acc;
                if (const double acc_finish_ts = abs(buff_mod.max_extra_eff_delta / base_acc);
                    acc_finish_ts < estimated_duration)
                // if the buff will finish before the total duration
                {
                    eff_piecewise.Insert(acc_finish_ts, buff_mod.max_extra_eff_delta, -acc, 1, 0);
                } // else: the buff will finish after the total duration, so no need to insert
            }
        }

        double final_eff_mul = 1;                                     // final effective multiplier
        double final_eff_delta = 0; // final effective delta, unit: 1
        double indirect_eff_mul = 1;
        double indirect_eff_delta = 0;
        const RoomFinalAttributeModifier *override = nullptr;
        SIMULATOR_UNROLL_MAX_BUFF_CNT
        for (UInt32 i = 0; i < room->n_buff; ++i)
        {
            const auto &final_mod = room->buffs[i]->applier.final_mod;
            if (!final_mod.IsValid())
                continue;

            switch (final_mod.final_mod_type)
            {
            case RoomFinalAttributeModifierType::ADDITIONAL: // add
                final_eff_mul *= final_mod.eff_scale;
                final_eff_delta += final_mod.eff_delta;
                break;

            case RoomFinalAttributeModifierType::OVERRIDE_AND_CANCEL_ALL: // override
                if (override == nullptr || !RoomFinalAttributeModifier::validate(*override) ||
                    override->owner->sort_id < final_mod.owner->sort_id)
                {
                    override = &final_mod;
                }
                break;

            case RoomFinalAttributeModifierType::INDIRECT: // indirect
                indirect_eff_mul *= final_mod.eff_scale;
                indirect_eff_delta += final_mod.eff_delta;
                break;

            case RoomFinalAttributeModifierType::NONE:
            default:
                ALBC_UNREACHABLE();
            }
        }

        if (override != nullptr && override->IsValid()) // OVERRIDE_AND_CANCEL_ALL下, 除override外的所有modifier均失效
        {
            result = estimated_duration * (room->room_attributes.base_prod_eff + 
                                            override->eff_scale * indirect_eff_mul * override->eff_delta + 
                                            indirect_eff_delta);

            
        }

        if (util::fp_eq(final_eff_mul, 0.)) // 所有非Final的modifier均失效
        {
            result = estimated_duration *
                   (room->room_attributes.base_prod_eff + final_eff_delta * indirect_eff_mul + indirect_eff_delta);
        }

        // insert parameter at t = 0
        eff_piecewise.Insert(0., base_eff_delta, base_acc, final_eff_mul * indirect_eff_mul,
                             final_eff_delta + indirect_eff_delta);

        SIMULATOR_UNROLL_MAX_BUFF_CNT
        for (UInt32 i = 0; i < room->n_buff; ++i)
        {
            const auto &final_mod = room->buffs[i]->applier.final_mod;

            if (!final_mod.IsValid() || util::fp_eq(final_mod.eff_scale, 1.))
            {
                continue;
            }

            double perv_ts = 0;
            double perv_base = base_eff_delta;
            double perv_acc = base_acc;
            SIMULATOR_UNROLL_MAX_BUFF_CNT
            for (const auto &[ts, def] : eff_piecewise)
            {
                const double reach_top_ts =
                    final_mod.max_extra_eff_delta <= perv_base * final_mod.eff_scale
                        ? perv_ts
                        : util::fp_round_n((final_mod.max_extra_eff_delta / final_mod.eff_scale - perv_base) / perv_acc);
                // handle a multiplier of other buffs reaching its maximum value because the effective increment over
                // time_t
                if (reach_top_ts >= 0 && reach_top_ts <= ts)
                {
                    // pending_piecewise.push(std::make_pair(reach_top_ts, PiecewiseDef(0., 0., 1 / final_mod.eff_scale,
                    // final_mod.max_extra_eff_delta)));
                    eff_piecewise.Insert(reach_top_ts, 0., 0., 1. / final_mod.eff_scale, final_mod.max_extra_eff_delta);
                    break;
                }

                if (util::fp_eq(ts, 0.))
                {
                    continue;
                }

                perv_ts = ts;
                perv_base += def.base_delta;
                perv_acc += def.acc_delta;
            }
        }
        result = Integrate(eff_piecewise, 0., estimated_duration, room->room_attributes.base_prod_eff); // integrate the piecewise function
        duration = estimated_duration;
    }

  protected:
    static double
    Integrate(const PiecewiseMap<kFuncPiecewiseMaxSegmentCount> &map, const double lower_bound,
              const double upper_bound, const double eff_delta)

    {
        if (map.n <= 0)
        {
            return 0.;
        }

        auto it = map.begin();
        const auto &[origin_ts, origin_def] = *it; // begin
        double base = origin_def.base_delta;
        double acc = origin_def.acc_delta;
        double mul = origin_def.mul;
        double extra = origin_def.extra_delta + eff_delta;

        if (map.n == 1)
        {
            return (mul * base + mul * acc * (lower_bound + upper_bound) * .5 + extra) * (upper_bound - lower_bound);
        }

        double seg_lb;
        double seg_ub = origin_ts;
        double sum = 0.;
        UNROLL_LOOP(kFuncPiecewiseMaxSegmentCount)
        for (++it; true; ++it) // starts from the second segment
        {                      // NOTE: the first segment is not included, so skip it
            const auto &[ts, def] = *it;

            seg_lb = seg_ub;
            seg_ub = it.IsEnd() ? upper_bound : ts;

            const double calc_lb = std::max(seg_lb, lower_bound);
            const double calc_ub = std::min(seg_ub, upper_bound);

            // f(t) = mul * (base + acc * t) + extra
            sum += (mul * base + mul * acc * (calc_lb + calc_ub) * .5 + extra) * (calc_ub - calc_lb);

            if (it.IsEnd() || seg_ub >= upper_bound)
            {
                break;
            }

            base += def.base_delta;
            acc += def.acc_delta;
            mul *= def.mul;
            extra += def.extra_delta;
            // aggregates ops_for_partial_comb segment and the previous one
        }

        return sum;
    }
};
} // namespace albc

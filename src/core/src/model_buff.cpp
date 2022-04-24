//
// Created by Nonary on 2022/4/24.
//
#include "model_buff.h"

namespace albc::model::buff
{

void ModifierScopeData::Reset()
{
    this->room = nullptr;
}
void ModifierApplier::MarkInvalid()
{
    scope.data.Reset();
    RoomAttributeModifier::mark_invalid(this->room_mod);
    RoomFinalAttributeModifier::mark_invalid(this->final_mod);
    CharacterCostModifier::mark_invalid(this->cost_mod);
}
RoomBuff::RoomBuff() : prototype(static_cast<RoomBuff *>(this))
{
}
RoomBuff::RoomBuff(data::building::RoomType room_type, RoomBuffType inner_type)
    : inner_type(inner_type), room_type(room_type), prototype(static_cast<RoomBuff *>(this))
{
}
bool RoomBuff::ValidateTarget(const RoomModel *room)
{
    return std::all_of(validators.begin(), validators.end(),
                       [room](const std::shared_ptr<RoomBuffTargetValidator>& validator) -> bool { return validator->validate(room); });
}
RoomBuff *RoomBuff::AddValidator(RoomBuffTargetValidator *validator)
{
    validators.emplace_back(validator);
    return this;
}
void RoomBuff::UpdateScopeOnNeed(const ModifierScopeData &data)
{
    assert(this->inner_type != RoomBuffType::UNDEFINED && "RoomBuffType is undefined");
    assert(this != prototype && "prototype should not be updated!");
    if (NeedUpdateScope(data))
    {
        UpdateScope(data);
        if (this->applier.scope.type == ModifierScopeType::DEPEND_ON_ROOM)
            this->applier.scope.data.room = data.room;
    }
}
bool RoomBuff::NeedUpdateScope(const ModifierScopeData &data) const
{
    switch (this->applier.scope.type)
    {
    case ModifierScopeType::INDEPENDENT:
        return false;

    case ModifierScopeType::DEPEND_ON_ROOM:
        return this->applier.scope.data.room != data.room;

    case ModifierScopeType::DEPEND_ON_OTHER_CHAR:
        return true;
    }
    ALBC_UNREACHABLE();
}
BasicInc::BasicInc(const double eff_delta, const int cap_delta, const data::building::RoomType room_type_1,
                   const RoomBuffType inner_type)
    : CloneableRoomBuff(room_type_1, inner_type) // 指定房间的效率, 房间容量, 内部类型(用于处理某些同类buff互斥)
{
    RoomAttributeModifier::init(applier.room_mod, this,
                                inner_type,
                                eff_delta, // eff_delta
                                cap_delta); // cap_delta
}
BasicInc::BasicInc(const double eff_delta, const int cap_delta, const double cost_mod,
                   const CharCostModifierType cost_mod_type, const data::building::RoomType room_type_1,
                   const RoomBuffType inner_type)
    : BasicInc(eff_delta, cap_delta, room_type_1, inner_type)
{
    CharacterCostModifier::init(applier.cost_mod, this,
                                cost_mod_type,
                                cost_mod);
}
IncEffOverTime::IncEffOverTime(const double base_eff_delta, const double eff_inc_per_hour,
                               const double max_extra_eff_inc, const data::building::RoomType room_type_1,
                               const RoomBuffType inner_type)
    : CloneableRoomBuff(room_type_1, inner_type)
{
    RoomAttributeModifier::init(applier.room_mod, this,
                                inner_type,
                                base_eff_delta, // eff
                                0, // cap
                                eff_inc_per_hour, // eff_delta_inc_hour
                                max_extra_eff_inc); // max_extra_eff_delta
}
IncEffByPowerPlantCnt::IncEffByPowerPlantCnt(const double addition_per_power_plant)
    : CloneableRoomBuff(data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_POWER_PLANT),
      addition_per_power_plant_(addition_per_power_plant)
{
    applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
}
void IncEffByPowerPlantCnt::UpdateScope(const ModifierScopeData &data)
{
    const int power_plant_count =
        util::read_attribute_as_int(data.room->global_attributes, GlobalAttributeType::POWER_PLANT_CNT);

    RoomFinalAttributeModifier::init(applier.final_mod, this,
                                     inner_type,
                                     power_plant_count * addition_per_power_plant_,
                                     0, // cap
                                     0,  // eff_delta_inc_hour
                                     INFINITY, // max_extra_eff_delta
                                     RoomFinalAttributeModifierType::ADDITIONAL,
                                     0); // eff_scale
}
IncEffByOtherEffInc::IncEffByOtherEffInc(const double unit_addition, const double unit_factor,
                                         const double max_extra_addition, const data::building::RoomType room_type_1,
                                         const RoomBuffType inner_type)
    : CloneableRoomBuff(room_type_1, inner_type)
{
    RoomFinalAttributeModifier::init(applier.final_mod, this,
                                     inner_type,
                                     0,  // eff
                                     0, // cap
                                     0, // eff_inc_hour
                                     max_extra_addition, // max_extra_eff
                                     RoomFinalAttributeModifierType::ADDITIONAL,
                                     1 + unit_addition / unit_factor); // eff_scale
}
IncEffByOtherCapInc::IncEffByOtherCapInc(const int threshold, const double below_addition_per_limit,
                                         const double above_addition_per_limit)
    : CloneableRoomBuff(data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_CAP_ADDITION),
      threshold_(threshold), below_addition_(below_addition_per_limit), above_addition_(above_addition_per_limit)
{
    this->is_mutex = true;
    applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
}
void IncEffByOtherCapInc::UpdateScope(const ModifierScopeData &data)
{
    double eff_delta = 0;
    UNROLL_LOOP(kRoomMaxBuffSlots)
    for (UInt32 i = 0; i < data.room->n_buff; ++i)
    {
        const auto buff = data.room->buffs[i];
        const auto &applier = buff->applier;
        int cap_delta;
        if (!applier.room_mod.IsValid() || (cap_delta = applier.room_mod.cap_delta) <= 0)
            continue;

        eff_delta += cap_delta >= threshold_ ? cap_delta * above_addition_ : cap_delta * below_addition_;
    }
    RoomAttributeModifier::init(applier.room_mod, this,
                                inner_type,
                                eff_delta); // eff
}
IncEffByGlobalAttribute::IncEffByGlobalAttribute(const double unit, const double addition_per_unit,
                                                 const GlobalAttributeType global_attribute_type,
                                                 const data::building::RoomType room_type_1,
                                                 const RoomBuffType inner_type)
    : CloneableRoomBuff(room_type_1, inner_type), unit_(unit), addition_per_unit_(addition_per_unit),
      global_attribute_type_(global_attribute_type)
{
    applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
}
IncEffByGlobalAttribute::IncEffByGlobalAttribute(const double base_delta, const double unit,
                                                 const double addition_per_unit,
                                                 const GlobalAttributeType global_attribute_type,
                                                 const data::building::RoomType room_type_1,
                                                 const RoomBuffType inner_type)
    : IncEffByGlobalAttribute(unit, addition_per_unit, global_attribute_type, room_type_1, inner_type)
{
    base_delta_ = base_delta;
}
void IncEffByGlobalAttribute::UpdateScope(const ModifierScopeData &data)
{
    RoomAttributeModifier::init(applier.room_mod, this,
                                inner_type,
                                base_delta_ + addition_per_unit_ *
                                              floor(util::read_attribute(data.room->global_attributes, global_attribute_type_)) / unit_);
    // eff
}
IncEffByStandardizationCnt::IncEffByStandardizationCnt(const double addition_per_unit,
                                                       const data::building::RoomType room_type_1,
                                                       const RoomBuffType inner_type)
    : CloneableRoomBuff(room_type_1, inner_type), addition_per_unit_(addition_per_unit)
{
    applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
}
void IncEffByStandardizationCnt::UpdateScope(const ModifierScopeData &data)
{
    double addition = 0.;

    UNROLL_LOOP(kRoomMaxBuffSlots)
    for (UInt32 i = 0; i < data.room->n_buff; ++i)
    {
        const auto buff = data.room->buffs[i];
        if (buff->owner_inst_id == this->owner_inst_id)
            continue;

        if (buff->inner_type == RoomBuffType::FACTORY_STANDARDIZATION)
        {
            addition += addition_per_unit_;
        }
    }

    RoomAttributeModifier::init(applier.room_mod, this,
                                inner_type,
                                addition); // eff
}
VodfoxTradeBuff::VodfoxTradeBuff()
    : CloneableRoomBuff(data::building::RoomType::TRADING, RoomBuffType::TRADING_WHISPERS)
{
    applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
}
void VodfoxTradeBuff::UpdateScope(const ModifierScopeData &data)
{
    RoomFinalAttributeModifier::init(applier.final_mod, this,
                                     inner_type,
                                     (data.room->max_slot_count - 1) * 0.45, // eff
                                     0.,  // cap
                                     0., // eff_inc_hour
                                     INFINITY, // max_eff
                                     RoomFinalAttributeModifierType::OVERRIDE_AND_CANCEL_ALL);

    CharacterCostModifier::init(applier.cost_mod, this,
                                CharCostModifierType::ROOM_ALL,
                                0.25);
}
JayeTradeBuff::JayeTradeBuff(bool is_above_elite_one)
    : CloneableRoomBuff(data::building::RoomType::TRADING, RoomBuffType::TRADING_BASIC_NEEDS),
      is_above_elite_one_(is_above_elite_one)
{
    applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;

    if (is_above_elite_one)
        this->patch_targets.push_back("trade_ord_limit_diff[000]"); // 干掉孑哥的满血buff
}
void JayeTradeBuff::UpdateScope(const ModifierScopeData &data)
{
    int total_cap = data.room->room_attributes.base_prod_cap;
    double eff_delta = 0.;

    UNROLL_LOOP(kRoomMaxBuffSlots)
    for (UInt32 i = 0; i < data.room->n_buff; ++i)
    {
        const auto buff = data.room->buffs[i];
        if (buff->owner_inst_id == this->owner_inst_id)
            continue;

        const auto &applier = buff->applier;
        if (!applier.room_mod.IsValid())
            continue;

        total_cap += applier.room_mod.cap_delta;
        eff_delta += applier.room_mod.eff_delta;
    }

    if (is_above_elite_one_)
    {
        total_cap = std::max(1, total_cap - static_cast<int>(std::max(floor(eff_delta / 0.10), 0.)));
    }

    RoomAttributeModifier::init(applier.room_mod, this,
                                inner_type,
                                0.04 * std::max(total_cap - data.room->room_attributes.prod_cnt, 0), // eff
                                total_cap - data.room->room_attributes.base_prod_cap); // cap
}
LapplandTradeBuff::LapplandTradeBuff(double cost_delta, int cap_delta)
    : CloneableRoomBuff(data::building::RoomType::TRADING, RoomBuffType::TRADING_HIDDEN_PURPOSE),
      cost_delta_(cost_delta),
      cap_delta_(cap_delta)
{
    applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
}
void LapplandTradeBuff::UpdateLookup(const data::player::PlayerTroopLookup &lookup)
{
    texas_char_inst_id_ = lookup.GetInstId("char_102_texas");
    enabled_ = texas_char_inst_id_ >= 0;
}
void LapplandTradeBuff::UpdateScope(const ModifierScopeData &data)
{
    if (!enabled_)
        return;

    auto begin = data.room->buffs;
    auto end = data.room->buffs + data.room->n_buff;

    auto texas = std::find_if(begin, end, [this](const RoomBuff *buff) -> bool {
      return buff->owner_inst_id == this->texas_char_inst_id_;
    });

    if (texas != end)
    {
        RoomAttributeModifier::init(this->applier.room_mod, this,
                                    inner_type,
                                    0., // eff_delta
                                    cap_delta_); // cap_delta

        CharacterCostModifier::init(this->applier.cost_mod, this,
                                    CharCostModifierType::SELF,
                                    cost_delta_);
    }
    else
    {
        RoomAttributeModifier::mark_invalid(this->applier.room_mod);
        CharacterCostModifier::mark_invalid(this->applier.cost_mod);
    }
}
TexasTradeBuff::TexasTradeBuff(bool affected_by_angel)
    : CloneableRoomBuff<TexasTradeBuff>(data::building::RoomType::TRADING, RoomBuffType::TRADING_FEUD),
      affected_by_angel_(affected_by_angel)
{
    if (affected_by_angel)
    {
        this->patch_targets = {"trade_ord_spd&cost_P[000]"};
    }

    applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
}
void TexasTradeBuff::UpdateLookup(const data::player::PlayerTroopLookup &lookup)
{
    angel_char_inst_id_ = lookup.GetInstId("char_103_angel");
    lappland_char_inst_id_ = lookup.GetInstId("char_140_whitew");

    enabled_ = angel_char_inst_id_ >= 0 || lappland_char_inst_id_ >= 0;
}
void TexasTradeBuff::UpdateScope(const ModifierScopeData &data)
{
    if (!enabled_)
        return;

    bool has_lappland = false;
    bool has_angel = false;

    UNROLL_LOOP(kRoomMaxBuffSlots)
    for (UInt32 i = 0; i < data.room->n_buff; ++i)
    {
        const auto buff = data.room->buffs[i];

        if (buff->owner_inst_id == lappland_char_inst_id_)
        {
            has_lappland = true;
        }
        else if (buff->owner_inst_id == angel_char_inst_id_)
        {
            has_angel = true;
        }
    }

    double eff_delta = 0.;
    double cost_delta = 0.;
    if (has_lappland)
    {
        eff_delta = 0.65;
        cost_delta = 0.3;
    }

    if (affected_by_angel_ && has_angel)
    {
        cost_delta -= 0.3;
    }

    RoomAttributeModifier::init(this->applier.room_mod, this,
                                inner_type,
                                eff_delta, // eff_delta
                                0.); // cap_delta

    CharacterCostModifier::init(this->applier.cost_mod, this,
                                CharCostModifierType::SELF,
                                cost_delta);
}
TradeChanceBuff::TradeChanceBuff(TradeChanceType type)
    : CloneableRoomBuff(data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_ORDER_CHANCE),
      indirect_addition_(GetEquivalentEffInc(type))
{
    this->applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
}
void TradeChanceBuff::UpdateScope(const ModifierScopeData &data)
{
    if (data.room->max_slot_count < 3)
    {
        RoomFinalAttributeModifier::mark_invalid(applier.final_mod);
        return;
    }

    if (indirect_addition_ > data.room->dynamic_fields.trade_chance_indirect_eff_mul)
    {
        RoomFinalAttributeModifier::init(this->applier.final_mod,
                                         this, RoomBuffType::TRADING_INC_ORDER_CHANCE,
                                         indirect_addition_ - data.room->dynamic_fields.trade_chance_indirect_eff_mul,
                                         0,
                                         0,
                                         INFINITY,
                                         RoomFinalAttributeModifierType::INDIRECT);
    }
}
constexpr double TradeChanceBuff::GetEquivalentEffInc(TradeChanceType type)
{
    switch (type)
    {
    case TradeChanceType::DISABLED:
        return kOrigFourGoldChance;

    case TradeChanceType::LEVEL_1:
        return kLvl1FourGoldChance;

    case TradeChanceType::LEVEL_2:
        return kLvl2FourGoldChance;
    }
    ALBC_UNREACHABLE();
}
constexpr double TradeChanceBuff::GetChanceOf4GoldOrder(TradeChanceType type)
{
    switch (type)
    {
    case TradeChanceType::DISABLED:
        return kOrigFourGoldChance;

    case TradeChanceType::LEVEL_1:
        return kLvl1FourGoldChance;

    case TradeChanceType::LEVEL_2:
        return kLvl2FourGoldChance;
    }
    ALBC_UNREACHABLE();
}
}
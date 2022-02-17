#pragma once

#include "attributes_util.h"
#include "bit_ops.h"
#include "buff_primitives.h"
#include "building_data_model.h"
#include "log_util.h"
#include "mem_util.h"
#include "player_data_model.h"
#include "primitive_types.h"
#include "util.h"
#include <algorithm>
#include <cstdarg>

namespace albc
{
/**
 * @brief 描述Buff的作用范围类型, 即Buff效果由哪些因素决定
 */
enum class ModifierScopeType
{
    INDEPENDENT,         //效果始终不变, 构造实例时初始化
    DEPEND_ON_ROOM,      //效果受全局变量影响, 进入新房间时初始化
    DEPEND_ON_OTHER_CHAR //效果受同一房间中角色影响, 每次计算时初始化
};

struct ModifierScopeData
{
    const RoomModel *room = nullptr; //该buff最后生效的房间

    void Reset()
    {
        this->room = nullptr;
    }
};

/**
 * @brief 描述Buff当前的作用范围
 */
struct ModifierScope
{
    ModifierScopeType type = ModifierScopeType::INDEPENDENT; //默认为独立
    ModifierScopeData data;
};

/**
 * @brief 描述Buff的作用效果
 */
class ModifierApplier
{
  public:
    // bool enabled{};						  //是否生效
    ModifierScope scope;                  //作用范围
    RoomAttributeModifier room_mod;       //房间属性修改
    RoomFinalAttributeModifier final_mod; //房间最终属性修改
    CharacterCostModifier cost_mod;       //角色心情消耗修改
};

/**
 * @brief 房间buff
 */
class RoomBuff
{
  public:
    int owner_inst_id = 0;  //拥有该Buff的干员的实例Id
    string owner_char_id{}; //拥有该Buff的角色Id
    string buff_id{};       // Buff的Id
    string name;
    string description;
    RoomBuffType inner_type{RoomBuffType::UNDEFINED}; //内部类型
    PtrVector<RoomBuffTargetValidator, std::shared_ptr> validators;     //作用范围验证器
    bm::RoomType room_type{bm::RoomType::NONE};       //作用房间类型
    int sort_id = 0;                                  //排序id
    double duration = 86400;                          //持续时间，由干员的心情决定
    ModifierApplier applier{};
    RoomBuff *const prototype;
    Vector<string> patch_targets; //指定该buff将会替代掉哪些buff的效果
    bool is_mutex = false;        //是否会与同类Buff互斥

    RoomBuff() : prototype(static_cast<RoomBuff *>(this))
    {
    }

    explicit RoomBuff(bm::RoomType room_type, RoomBuffType inner_type)
        : inner_type(inner_type), room_type(room_type), prototype(static_cast<RoomBuff *>(this))
    {
    }

    virtual ~RoomBuff() = default;

    RoomBuff(const RoomBuff &src) = default;
    RoomBuff &operator=(const RoomBuff &rhs) = delete;
    RoomBuff(RoomBuff &&src) noexcept = default;
    RoomBuff &operator=(RoomBuff &&src) noexcept = delete;

    virtual RoomBuff *Clone() = 0;

    virtual bool ValidateTarget(const RoomModel *room)
    {
        return std::all_of(validators.begin(), validators.end(),
            [room](std::shared_ptr<RoomBuffTargetValidator> validator) -> bool { return validator->validate(room); });
    }

    virtual void UpdateScope(const ModifierScopeData &data)
    {
    }

    virtual void UpdateLookup(const PlayerTroopLookup &lookup)
    {
    }

    virtual RoomBuff *AddValidator(RoomBuffTargetValidator *validator)
    {
        validators.emplace_back(validator);
        return this;
    }

    void UpdateScopeOnNeed(const ModifierScopeData &data)
    {
        assert(this->inner_type != RoomBuffType::UNDEFINED && "buff类型未定义!");
        assert(this != prototype && "prototype should not be updated!");
        if (NeedUpdateScope(data))
        {
            UpdateScope(data);
            if (this->applier.scope.type == ModifierScopeType::DEPEND_ON_ROOM)
                this->applier.scope.data.room = data.room;
        }
    }

  private:
    [[nodiscard]] bool NeedUpdateScope(const ModifierScopeData &data) const
    {
        switch (this->applier.scope.type)
        {
        case ModifierScopeType::INDEPENDENT:
            return false;

        case ModifierScopeType::DEPEND_ON_ROOM:
            return this->applier.scope.data.room != data.room;

        case ModifierScopeType::DEPEND_ON_OTHER_CHAR:
            return true;

        default: // should not reach here
            assert(false);
            return false;
        }
    }
};

/**
 * @brief 依据模板类型参数复制Buff, 实现虚函数Clone()
 */
template <typename TDerived> class CloneableRoomBuff : public RoomBuff
{
  public:
    // inherit constructor
    using RoomBuff::RoomBuff;

    RoomBuff *Clone() final
    {
        return prototype == this ? mem::aligned_new<TDerived>(static_cast<const TDerived &>(*this))
                                 : prototype->Clone();
    }
};

/**
 * @brief 最基础的加成，直接增减房间属性
 */
class BasicInc : public CloneableRoomBuff<BasicInc>
{
  public:
    BasicInc(const double eff_delta, const int cap_delta, const bm::RoomType room_type_1, const RoomBuffType inner_type)
        : CloneableRoomBuff(room_type_1, inner_type) // 指定房间的效率, 房间容量, 内部类型(用于处理某些同类buff互斥)
    {
        RoomAttributeModifier::init(applier.room_mod, this,
                                    inner_type,
                                    eff_delta, // eff_delta
                                    cap_delta); // cap_delta
    }

    BasicInc(const double eff_delta, const int cap_delta, const double cost_mod,
             const CharCostModifierType cost_mod_type, const bm::RoomType room_type_1, const RoomBuffType inner_type)
        : BasicInc(eff_delta, cap_delta, room_type_1, inner_type)
    {
        CharacterCostModifier::init(applier.cost_mod, this,
                                    cost_mod_type,
                                    cost_mod);
    }

    BasicInc(const BasicInc &src) = default;
};

/**
 * @brief 慢性子/急性子: 一段时间内持续增加房间效率
 */
class IncEffOverTime : public CloneableRoomBuff<IncEffOverTime>
{
  public:
    IncEffOverTime(const double base_eff_delta, const double eff_inc_per_hour, const double max_extra_eff_inc,
                   const bm::RoomType room_type_1, const RoomBuffType inner_type)
        : CloneableRoomBuff(room_type_1, inner_type)
    {
        RoomAttributeModifier::init(applier.room_mod, this,
                                    inner_type,
                                    base_eff_delta, // eff
                                    0, // cap
                                    eff_inc_per_hour, // eff_delta_inc_hour
                                    max_extra_eff_inc); // max_extra_eff_delta
    }
};

/**
 * @brief 自动化: 由发电站数量增加房间属性
 */
class IncEffByPowerPlantCnt : public CloneableRoomBuff<IncEffByPowerPlantCnt>
{
  public:
    explicit IncEffByPowerPlantCnt(const double addition_per_power_plant)
        : CloneableRoomBuff(bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_POWER_PLANT),
          addition_per_power_plant_(addition_per_power_plant)
    {
        this->is_mutex = true;
        applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
    }

    void UpdateScope(const ModifierScopeData &data) override
    {
        const int power_plant_count =
            read_attribute_as_int(data.room->global_attributes, GlobalAttributeType::POWER_PLANT_CNT);

        RoomFinalAttributeModifier::init(applier.final_mod, this,
                                         inner_type,
                                         power_plant_count * addition_per_power_plant_,
                                         0, // cap
                                         0,  // eff_delta_inc_hour
                                         INFINITY, // max_extra_eff_delta
                                         RoomFinalAttributeModifierType::ADDITIONAL,
                                         0); // eff_scale
    }

  protected:
    double addition_per_power_plant_;
};

/**
 * @brief 根据房间内其他干员的效率增加，提供效率增加
 */
class IncEffByOtherEffInc : public CloneableRoomBuff<IncEffByOtherEffInc>
{
  public:
    IncEffByOtherEffInc(const double unit_addition, const double unit_factor, const double max_extra_addition,
                        const bm::RoomType room_type_1, const RoomBuffType inner_type)
        : CloneableRoomBuff(room_type_1, inner_type)
    {
        RoomFinalAttributeModifier::init(applier.final_mod, this,
                                         inner_type,
                                         0,  // eff
                                         0, // cap
                                         0, // eff_inc_hour
                                         max_extra_addition, // max_extra_eff
                                         RoomFinalAttributeModifierType::ADDITIONAL,
                                         2); // eff_scale
    }
};

/**
 * @brief 根据房间内其他干员的容量增加，提供效率增加
 */
class IncEffByOtherCapInc : public CloneableRoomBuff<IncEffByOtherCapInc>
{
  public:
    IncEffByOtherCapInc(const int threshold, const double below_addition_per_limit,
                        const double above_addition_per_limit)
        : CloneableRoomBuff(bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_CAP_ADDITION),
          threshold_(threshold), below_addition_(below_addition_per_limit), above_addition_(above_addition_per_limit)
    {
        this->is_mutex = true;
        applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
    }

    void UpdateScope(const ModifierScopeData &data) override
    {
        double eff_delta = 0;
        UNROLL_LOOP(kRoomMaxBuffSlots)
        for (UInt32 i = 0; i < data.room->n_buff; ++i)
        {
            const auto buff = data.room->buffs[i];
            if (buff->owner_inst_id == this->owner_inst_id)
                continue;

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

  protected:
    int threshold_;
    double below_addition_;
    double above_addition_;
};

/**
 * @brief 根据全局属性增加房间属性
 */
class IncEffByGlobalAttribute : public CloneableRoomBuff<IncEffByGlobalAttribute>
{
  public:
    inline IncEffByGlobalAttribute(const double unit, const double addition_per_unit, const GlobalAttributeType global_attribute_type,
                                   const bm::RoomType room_type_1, const RoomBuffType inner_type)
        : CloneableRoomBuff(room_type_1, inner_type), unit_(unit), addition_per_unit_(addition_per_unit),
          global_attribute_type_(global_attribute_type)
    {
        applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
    }

    inline IncEffByGlobalAttribute(const double base_delta, const double unit, const double addition_per_unit,
                                   const GlobalAttributeType global_attribute_type, const bm::RoomType room_type_1,
                                   const RoomBuffType inner_type)
        : IncEffByGlobalAttribute(unit, addition_per_unit, global_attribute_type, room_type_1, inner_type)
    {
        base_delta_ = base_delta;
    }

    void UpdateScope(const ModifierScopeData &data) override
    {
        RoomAttributeModifier::init(applier.room_mod, this,
                                    inner_type,
                                    base_delta_ + addition_per_unit_ *
                                        floor(read_attribute(data.room->global_attributes, global_attribute_type_)) / unit_);
                                    // eff
    }

  protected:
    double base_delta_ = 0.;
    double unit_;
    double addition_per_unit_;
    GlobalAttributeType global_attribute_type_;
};

/**
 * @brief 根据标准化数量增加房间属性
 */
class IncEffByStandardizationCnt : public CloneableRoomBuff<IncEffByStandardizationCnt>
{
  public:
    inline IncEffByStandardizationCnt(const double addition_per_unit, const bm::RoomType room_type_1,
                                      const RoomBuffType inner_type)
        : CloneableRoomBuff(room_type_1, inner_type), addition_per_unit_(addition_per_unit)
    {
        applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
    }

    void UpdateScope(const ModifierScopeData &data) override
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

  protected:
    double addition_per_unit_;
};

/**
 * @brief 低语: 进驻贸易站时，
 * 当前贸易站内其他干员提供的订单获取效率全部归零，
 * 且每人为自身+45%订单获取效率，同时全体心情每小时消耗+0.25
 */
class VodfoxTradeBuff : public CloneableRoomBuff<VodfoxTradeBuff>
{
  public:
    inline VodfoxTradeBuff() : CloneableRoomBuff(bm::RoomType::TRADING, RoomBuffType::TRADING_WHISPERS)
    {
        applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
    }

    void UpdateScope(const ModifierScopeData &data) override
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
};

/**
 * @brief "trade_ord_limit_count[000]": 市井之道: 进驻贸易站时，
 * 当前贸易站内其他干员提供的每10%订单获取效率使订单上限-1（订单最少为1），
 * 同时每有1笔订单就+4%订单获取效率
 *
 * "trade_ord_limit_diff[000]": 摊贩经济: 进驻贸易站时，
 * 当前订单数与订单上限每差1笔订单，则订单获取效率+4%
 */
class JayeTradeBuff : public CloneableRoomBuff<JayeTradeBuff>
{
  public:
    inline explicit JayeTradeBuff(bool is_above_elite_one)
        : CloneableRoomBuff(bm::RoomType::TRADING, RoomBuffType::TRADING_BASIC_NEEDS),
          is_above_elite_one_(is_above_elite_one)
    {
        applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;

        if (is_above_elite_one)
            this->patch_targets.push_back("trade_ord_limit_diff[000]"); // 干掉孑哥的满血buff
    }

    void UpdateScope(const ModifierScopeData &data) override
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

  private:
    bool is_above_elite_one_;
};

/**
 * @brief "trade_ord_limit&cost_P[000]/[001]": 醉翁之意·α:
 * 当与德克萨斯在同一个贸易站时，心情每小时消耗-0.1，订单上限+2"
 */
class LapplandTradeBuff : public CloneableRoomBuff<LapplandTradeBuff>
{
  public:
    inline LapplandTradeBuff(double cost_delta, int cap_delta)
        : CloneableRoomBuff(bm::RoomType::TRADING, RoomBuffType::TRADING_HIDDEN_PURPOSE),
          cost_delta_(cost_delta),
          cap_delta_(cap_delta)
    {
        applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
    }

    void UpdateLookup(const PlayerTroopLookup &lookup) override
    {
        texas_char_inst_id_ = lookup.GetInstId("char_102_texas");
        enabled_ = texas_char_inst_id_ >= 0;
    }

    void UpdateScope(const ModifierScopeData &data) override
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

  protected:
    bool enabled_ = false;
    int texas_char_inst_id_ = -1;
    double cost_delta_;
    int cap_delta_;
};

/**
 * @brief "trade_ord_spd&cost_P[000]": 恩怨:
 * 当与拉普兰德在同一个贸易站时，心情每小时消耗+0.3，订单获取效率+65%
 *
 * "trade_ord_limit&cost_P[010]": 默契:
 * 当与能天使在同一个贸易站时，心情每小时消耗-0.3
 */
class TexasTradeBuff : public CloneableRoomBuff<TexasTradeBuff>
{
  public:
    inline explicit TexasTradeBuff(bool affected_by_angel)
        : CloneableRoomBuff<TexasTradeBuff>(bm::RoomType::TRADING, RoomBuffType::TRADING_FEUD),
          affected_by_angel_(affected_by_angel)
    {
        if (affected_by_angel)
        {
            this->patch_targets = {"trade_ord_spd&cost_P[000]"};
        }

        applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
    }

    void UpdateLookup(const PlayerTroopLookup &lookup) override
    {
        angel_char_inst_id_ = lookup.GetInstId("char_103_angel");
        lappland_char_inst_id_ = lookup.GetInstId("char_140_whitew");

        enabled_ = angel_char_inst_id_ >= 0 || lappland_char_inst_id_ >= 0;
    }

    void UpdateScope(const ModifierScopeData &data) override
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

  protected:
    bool affected_by_angel_;
    bool enabled_ = false;
    int angel_char_inst_id_ = -1;
    int lappland_char_inst_id_ = -1;
};
} // namespace albc

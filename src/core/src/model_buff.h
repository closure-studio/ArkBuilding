#pragma once
#include "util_attributes.h"
#include "model_buff_primitives.h"
#include "data_building.h"
#include "util_log.h"
#include "util_mem.h"
#include "data_player.h"
#include "albc_types.h"
#include "util.h"
#include <algorithm>
#include <cstdarg>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace albc::model::buff
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

    void Reset();
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

    void MarkInvalid();
};

/**
 * @brief 房间buff
 */
class RoomBuff
{
  public:
    int owner_inst_id;  //拥有该Buff的干员的实例Id
    std::string owner_char_id{}; //拥有该Buff的角色Id
    std::string buff_id{};       // Buff的Id
    std::string name;
    std::string description;
    RoomBuffType inner_type{RoomBuffType::UNDEFINED}; //内部类型
    mem::PtrVector<RoomBuffTargetValidator, std::shared_ptr> validators;     //作用范围验证器
    data::building::RoomType room_type{data::building::RoomType::NONE};       //作用房间类型
    int sort_id;                                  //排序id
    double duration;                          //持续时间，由干员的心情决定
    ModifierApplier applier{};
    RoomBuff *const prototype;
    Vector<std::string> patch_targets; //指定该buff将会替代掉哪些buff的效果
    bool is_mutex;        //是否会与同类Buff互斥

    RoomBuff();

    explicit RoomBuff(data::building::RoomType room_type, RoomBuffType inner_type);

    virtual ~RoomBuff() = default;

    RoomBuff(const RoomBuff &src) = default;
    RoomBuff &operator=(const RoomBuff &rhs) = delete;
    RoomBuff(RoomBuff &&src) noexcept = default;
    RoomBuff &operator=(RoomBuff &&src) noexcept = delete;

    virtual RoomBuff *Clone() = 0;

    virtual bool ValidateTarget(const RoomModel *room);

    virtual void UpdateScope(const ModifierScopeData&)
    {
    }

    virtual void UpdateLookup(const data::player::PlayerTroopLookup&)
    {
    }

    virtual RoomBuff *AddValidator(RoomBuffTargetValidator *validator);

    void UpdateScopeOnNeed(const ModifierScopeData &data);

  private:
    [[nodiscard]] bool NeedUpdateScope(const ModifierScopeData &data) const;
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
        return prototype == this ? new TDerived(static_cast<const TDerived &>(*this))
                                 : prototype->Clone();
    }
};

/**
 * @brief 最基础的加成，直接增减房间属性
 */
class BasicInc : public CloneableRoomBuff<BasicInc>
{
  public:
    BasicInc(double eff_delta, int cap_delta, const data::building::RoomType room_type_val, const RoomBuffType inner_type_val);

    BasicInc(double eff_delta, int cap_delta, const double cost_mod,
             CharCostModifierType cost_mod_type, data::building::RoomType room_type_val, const RoomBuffType inner_type_val);

    BasicInc(const BasicInc &src) = default;
};

/**
 * @brief 慢性子/急性子: 一段时间内持续增加房间效率
 */
class IncEffOverTime : public CloneableRoomBuff<IncEffOverTime>
{
  public:
    IncEffOverTime(double base_eff_delta, double eff_inc_per_hour, const double max_extra_eff_inc,
                   data::building::RoomType room_type_val, RoomBuffType inner_type_val);
};

/**
 * @brief 自动化: 由发电站数量增加房间属性
 */
class IncEffByPowerPlantCnt : public CloneableRoomBuff<IncEffByPowerPlantCnt>
{
  public:
    explicit IncEffByPowerPlantCnt(double addition_per_power_plant);

    void UpdateScope(const ModifierScopeData &data) override;

  protected:
    double addition_per_power_plant_;
};

/**
 * @brief 根据房间内其他干员的效率增加，提供效率增加
 */
class IncEffByOtherEffInc : public CloneableRoomBuff<IncEffByOtherEffInc>
{
  public:
    IncEffByOtherEffInc(double unit_addition, double unit_factor, const double max_extra_addition,
                        data::building::RoomType room_type_val, RoomBuffType inner_type_val);
};

/**
 * @brief 根据房间内其他干员的容量增加，提供效率增加
 */
class IncEffByOtherCapInc : public CloneableRoomBuff<IncEffByOtherCapInc>
{
  public:
    IncEffByOtherCapInc(int threshold, double below_addition_per_limit,
                        double above_addition_per_limit);

    void UpdateScope(const ModifierScopeData &data) override;

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
    IncEffByGlobalAttribute(double unit, double addition_per_unit, const GlobalAttributeType global_attribute_type,
                                   data::building::RoomType room_type_val, RoomBuffType inner_type_val);

    IncEffByGlobalAttribute(double base_delta, double unit, const double addition_per_unit,
                                   GlobalAttributeType global_attribute_type, data::building::RoomType room_type_val,
                                   RoomBuffType inner_type_val);

    void UpdateScope(const ModifierScopeData &data) override;

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
    IncEffByStandardizationCnt(double addition_per_unit, data::building::RoomType room_type_val,
                                      RoomBuffType inner_type_val);

    void UpdateScope(const ModifierScopeData &data) override;

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
    VodfoxTradeBuff();

    void UpdateScope(const ModifierScopeData &data) override;
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
    explicit JayeTradeBuff(bool is_above_elite_one);

    void UpdateScope(const ModifierScopeData &data) override;

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
    LapplandTradeBuff(double cost_delta, int cap_delta);

    void UpdateLookup(const data::player::PlayerTroopLookup &lookup) override;

    void UpdateScope(const ModifierScopeData &data) override;

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
    explicit TexasTradeBuff(bool affected_by_angel);

    void UpdateLookup(const data::player::PlayerTroopLookup &lookup) override;

    void UpdateScope(const ModifierScopeData &data) override;

  protected:
    bool affected_by_angel_;
    bool enabled_ = false;
    int angel_char_inst_id_ = -1;
    int lappland_char_inst_id_ = -1;
};

enum class TradeChanceType
{
    DISABLED = 0,
    LEVEL_1 = 1,
    LEVEL_2 = 2,
};

class TradeChanceBuff final : public CloneableRoomBuff<TradeChanceBuff>
{
public:
    static constexpr double kOrigFourGoldChance = 0.2;
    static constexpr double kLvl1FourGoldChance = 0.45;
    static constexpr double kLvl2FourGoldChance = 0.7;

    explicit TradeChanceBuff(TradeChanceType type);

    void UpdateScope(const ModifierScopeData &data) override;

    static constexpr double GetEquivalentEffInc(TradeChanceType type);

    static constexpr double GetChanceOf4GoldOrder(TradeChanceType type);
protected:
    double indirect_addition_;
};
} // namespace albc
#pragma clang diagnostic pop
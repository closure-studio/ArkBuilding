//
// Created by User on 2022-02-05.
//
#pragma once
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#include "model_buff_consts.h"
#include "util_attributes.h"
#include "util_bitops.h"
#include "data_building.h"
#include "util_log.h"
#include "util_mem.h"
#include "albc_types.h"
#include "util.h"

namespace albc::model::buff
{
struct RoomAttributeModifier; // forward declaration
class RoomBuff;               // forward declaration

enum class OrderType // order type
{
    UNDEFINED = 0,
    GOLD = 1,
    ORUNDUM = 2
};

// production type
enum class ProdType
{
    UNDEFINED = 0,
    GOLD = 1,
    RECORD = 2,
    ORIGINIUM_SHARD = 3,
    CHIP = 4
};

///基建技能类型,每种类型对应一种图标
enum class RoomBuffType
{
    UNDEFINED,                              //占位符
    FACTORY_STANDARDIZATION,                //标准化
    FACTORY_INC_EFF_ALL,                    //除标准化外的加效率Buff
    FACTORY_INC_EFF_BY_POWER_PLANT,         //自动化
    FACTORY_INC_EFF_BY_CAP_ADDITION,        //大就是好/回收利用
    FACTORY_INC_EFF_RECORD,                 //录像加速类
    FACTORY_INC_EFF_ORIGINIUM,              //搓玉加速类
    FACTORY_INC_EFF_BY_CHAIN_OF_THOUGHT,    //迷迭香思维链环加速
    FACTORY_INC_EFF_GOLD,                   //赤金加速
    FACTORY_INC_EFF_AND_CAP,                //仓库管理
    FACTORY_JUNKMAN,                        //加仓库减心情消耗类
    FACTORY_CRAFTSMANSHIP_SPIRIT,           //工匠精神
    FACTORY_EDITING,                        //视频剪辑
    FACTORY_VLOG,                           // VLOG
    FACTORY_HOTHEAD,                        //急性子
    FACTORY_SLOWCOACH,                      //慢性子
    FACTORY_EXTRASENSORY,                   //超感
    FACTORY_INC_EFF_BY_TRADING_POST_CNT,    //清流
    FACTORY_TRAM_SPIRIT,                    //团队精神
    FACTORY_INC_EFF_BY_OTHER_EFF_INC,       //配合意识
    FACTORY_TROUBLE_MAKER,                  //加生产力减心情类
    FACTORY_INC_EFF_BY_STANDARDIZATION_CNT, //意识协议
    FACTORY_RELIABLE_HELPER,                //"可靠"助手
    TRADING_INC_ORDER_CHANCE,               //加高产订单概率
    TRADING_INC_EFF_AND_CAP,                //订单管理类
    TRADING_INC_EFF,                        //加效率
    TRADING_COMM,                           //交际
    TRADING_HEAVENLY_REWARD,                //天道酬勤
    TRADING_FUNDRAISING,                    //虔诚筹款
    TRADING_BASIC_NEEDS,                    //市井之道
    TRADING_STREET_ECO,                     //摊贩经济
    TRADING_NEGOTIATION,                    //谈判
    TRADING_WHISPERS,                       //低语
    TRADING_HIDDEN_PURPOSE,                 //醉翁之意
    TRADING_FEUD,                           //恩怨
    TRADING_TACIT_UNDERSTANDING,            //默契
    TRADING_LOGISTICS_PLANNING,             //物流规划
    TRADING_ORDER_FLOW_VISUALIZATION,       //订单流可视化
    TRADING_OBVIOUS_DECEPTION,              //愿者上钩
    TRADING_INVESTMENT,                     //投资

    POWER_INC_DRONE_RECOVERY,   //无人机加速
    POWER_DEC_MORALE_CONSUMING, //发电站减心情消耗

    CC_INC_TRADING_EFF,                //加贸易站效率
    CC_DEC_ALTERNATE_MORALE_CONSUMING, //异格者
    CC_DEC_MORALE_CONSUMING,           //减中枢内心情消耗
    CC_LGD,                            //德才兼备
    CC_EUNECTES,                       //我寻思能行
    CC_HUNG,                           //坚毅随和
    CC_DISCERNMENT,                    //至察
    CC_URSUS_STUDENT_GROUP,            //学生会会长
    CC_TEAM_RAINBOW,                   //彩虹小队
    CC_INTELLIGENCE_RESERVE,           //情报储备,
    CC_UNSTIRRED_BY_GAIN,              //不以物喜
    CC_UNMOVED_BY_LOSS,                //不以己悲
    CC_VODKA,                          //乌萨斯特饮
    CC_TIDEWATCHER,                    //潮汐守望
    CC_PACK_HUNTERS,                   //集群狩猎
    CC_NEUROTICISM                    //神经质
};

// global attribute types
enum class GlobalAttributeType
{
    CHAIN_OF_THOUGHT = 0,     //思维链环
    PERCEPTION_INFO = 1,      //感知信息
    WORLDLY_PLIGHT = 2,       //人间烟火
    INTELLIGENCE_RESERVE = 3, //情报储备
    MEMORY_FRAGMENT = 4,      //记忆碎片
    VODKA = 5,                //乌萨斯特饮

    FACTORY_EFF_INC = 6,    //全局生产效率（如凯尔希）
    TRADING_EFF_INC = 7,    //全局交易效率
    POWER_PLANT_CNT = 8,    //全局发电站数量
    TRADING_POST_CNT = 9,   //全局交易站数
    DORM_OPERATOR_CNT = 10,  //宿舍内干员数
    GOLD_PROD_LINE_CNT = 11, //全局赤金生产线数
    DORM_SUM_LEVEL = 12,     //宿舍总等级
};

using GlobalAttributeFields = Array<double, util::enum_size<GlobalAttributeType>::value>;

struct RoomAttributeFields
{
    ProdType prod_type = ProdType::UNDEFINED;
    OrderType order_type = OrderType::UNDEFINED;
    double base_prod_eff = 1;  //基础生产效率
    int base_prod_cap = 10;    //基础产品容量
    double base_char_cost = 1; //基础心情消耗
    int prod_cnt = 0;          // 当前库存产品容量
};

enum class ModifierAttributeType
{
    EFF_DELTA, //生产效率增减
    CAP_DELTA, //生产容量增减
    EFF_SCALE, //生产效率倍数
};

enum class CharCostModifierType
{
    NONE,             //无
    SELF,             //影响自身
    ROOM_ALL,         //影响房间内所有
    ROOM_EXCEPT_SELF, //影响房间内除自身外的所有
    ROOM_CLEAR_ALL,   //清除房间内所有
};

enum class RoomFinalAttributeModifierType //房间最终属性修改类型
{
    NONE,                    //无
    ADDITIONAL,              //最终加成
    OVERRIDE_AND_CANCEL_ALL, //清除其他最终属性修改，只保留本属性修改
    INDIRECT,                //间接加成(如提高高品质订单概率), 当没有具体数据但是有确切的等效数据时使用
};

struct RoomAttributeModifier
{
    RoomBuff *owner = nullptr;                        //指向房间buff
    RoomBuffType buff_type = RoomBuffType::UNDEFINED; // buff类型
    double eff_delta = 0.;                            //生产效率增减
    int cap_delta = 0;                                //生产容量增减
    double eff_inc_per_hour = 0.;                     //每小时增加生产效率
    double max_extra_eff_delta = 0.;                  //最大额外增加效率

    constexpr bool operator==(const RoomAttributeModifier &other) const
    {
        return owner == other.owner && buff_type == other.buff_type;
    }

    constexpr bool operator!=(const RoomAttributeModifier &other) const
    {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool IsValid() const
    {
        return validate(*this);
    }

    [[nodiscard]] std::string to_string() const;

    static constexpr void init(RoomAttributeModifier &modifier, RoomBuff *buff, RoomBuffType buff_type,
                               double eff_delta = 0, int cap_delta = 0, double eff_delta_inc_hour = 0,
                               double max_extra_eff_delta = 0)
    {
        modifier.owner = buff;
        modifier.buff_type = buff_type;

        modifier.eff_delta = eff_delta;
        modifier.cap_delta = cap_delta;

        modifier.eff_inc_per_hour = eff_delta_inc_hour;
        modifier.max_extra_eff_delta = max_extra_eff_delta;
    }

    [[maybe_unused]] static constexpr void mark_invalid(RoomAttributeModifier &modifier)
    {
        modifier.owner = nullptr;
        modifier.buff_type = RoomBuffType::UNDEFINED;
    }

    static constexpr bool validate(const RoomAttributeModifier &val)
    {
        return val.owner != nullptr && val.buff_type != RoomBuffType::UNDEFINED;
    }
};

struct RoomFinalAttributeModifier : RoomAttributeModifier
{
    RoomFinalAttributeModifierType final_mod_type = RoomFinalAttributeModifierType::NONE; //最终属性修改类型
    double eff_scale = 1.; //生产效率倍数

    [[nodiscard]] std::string to_string() const;

    static constexpr void init(RoomFinalAttributeModifier &modifier, RoomBuff *buff, RoomBuffType buff_type,
                               double eff_delta, int cap_delta, double eff_delta_inc_hour,
                               double max_extra_eff_delta, RoomFinalAttributeModifierType final_mod_type,
                               double eff_scale = 1)
    {
        RoomAttributeModifier::init(modifier, buff, buff_type, eff_delta, cap_delta, eff_delta_inc_hour,
                                    max_extra_eff_delta);

        modifier.final_mod_type = final_mod_type;
        modifier.eff_scale = eff_scale;
    }
};

struct CharacterCostModifier
{
    RoomBuff *owner = nullptr;                              //指向房间buff
    CharCostModifierType type = CharCostModifierType::NONE; //修改类型
    double value = 0;                                       //修改值

    [[nodiscard]] constexpr bool IsValid() const
    {
        return validate(*this);
    }

    bool operator==(const CharacterCostModifier &other) const
    {
        return owner == other.owner && type == other.type;
    }

    bool operator!=(const CharacterCostModifier &other) const
    {
        return !(*this == other);
    }

    [[nodiscard]] std::string to_string() const;

    static constexpr void init(CharacterCostModifier &modifier, RoomBuff *buff, CharCostModifierType type,
                               double value)
    {
        modifier.owner = buff;
        modifier.type = type;
        modifier.value = value;
    }

    static constexpr void mark_invalid(CharacterCostModifier &modifier)
    {
        modifier.owner = nullptr;
        modifier.type = CharCostModifierType::NONE;
    }

    static constexpr bool validate(const CharacterCostModifier &modifier)
    {
        return modifier.owner != nullptr && modifier.type != CharCostModifierType::NONE;
    }
};

struct RoomDynamicFields
{
    double trade_chance_indirect_eff_mul = 1;
    double trade_four_gold_chance = 0.2;
};

class RoomModel //房间模型
{
  public:
    std::string id;                                 //房间id
    int max_slot_count = 3;                    //房间最大槽位数
    GlobalAttributeFields global_attributes{}; //全局属性
    RoomAttributeFields room_attributes{};     //房间属性
    RoomDynamicFields dynamic_fields{};
    RoomBuff *buffs[kRoomMaxBuffSlots]{};      //房间buff
    UInt32 n_buff = 0U;                        //房间buff数量
    data::building::RoomType type = data::building::RoomType::NONE;    //房间类型

    //向房间内添加一个buff
    constexpr void PushBuff(RoomBuff *buff)
    {
        buffs[n_buff] = buff;
        n_buff++;
    }

    //移除房间内的一个buff
    constexpr void PopBuff()
    {
        assert(n_buff > 0);
        buffs[n_buff] = nullptr;
        --n_buff;
    }

    constexpr void ResetState()
    {
        if (max_slot_count >= 3)
        {
            dynamic_fields.trade_chance_indirect_eff_mul = 1;
            dynamic_fields.trade_four_gold_chance = 0.2;
        }
    }

    [[nodiscard]] std::string to_string() const;
};

class RoomBuffTargetValidator
{
  public:
    virtual ~RoomBuffTargetValidator() = default;
    RoomBuffTargetValidator() = default;
    RoomBuffTargetValidator(const RoomBuffTargetValidator &src) = default;
    RoomBuffTargetValidator & operator=(const RoomBuffTargetValidator &rhs) = default;
    RoomBuffTargetValidator(RoomBuffTargetValidator &&src) = default;
    RoomBuffTargetValidator & operator=(RoomBuffTargetValidator &&src) = default;

    virtual bool validate(const RoomModel * /*room*/) ;
};

class ProdTypeSelector : public RoomBuffTargetValidator
{
  public:
    explicit ProdTypeSelector(ProdType prod_type);

    bool validate(const RoomModel *room) override;

  protected:
    ProdType prod_type_;
};
} // namespace albc

#pragma clang diagnostic pop
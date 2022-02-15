//
// Created by User on 2022-02-05.
//
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma once
#include "attributes_util.h"
#include "bit_ops.h"
#include "building_data_model.h"
#include "log_util.h"
#include "mem_util.h"
#include "primitive_types.h"
#include "util.h"

#define kRoomMaxBuffSlots 10

namespace albc
{
static constexpr UInt32 kInvalidSlot = static_cast<unsigned>(0xFFFF);

// check if the slot is valid, indicating a valid, or an "occupied" slot
static constexpr auto validate_room_slot(const UInt32 slot) -> bool
{
    return slot < static_cast<unsigned>(kRoomMaxBuffSlots);
}

struct RoomAttributeModifier; // forward declaration
class RoomBuff;               // forward declaration

enum class OrderType // order type
{
    UNDEFINED,
    GOLD,
    ORUNDUM
};

// production type
enum class ProdType
{
    UNDEFINED,
    GOLD,
    RECORD,
    ORIGINIUM_SHARD,
    CHIP
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
    CHAIN_OF_THOUGHT,     //思维链环
    PERCEPTION_INFO,      //感知信息
    WORLDLY_PLIGHT,       //人间烟火
    INTELLIGENCE_RESERVE, //情报储备
    MEMORY_FRAGMENT,      //记忆碎片
    VODKA,                //乌萨斯特饮

    FACTORY_EFF_INC,    //全局生产效率（如凯尔希）
    TRADING_EFF_INC,    //全局交易效率
    POWER_PLANT_CNT,    //全局发电站数量
    TRADING_POST_CNT,   //全局交易站数
    DORM_OPERATOR_CNT,  //宿舍内干员数
    GOLD_PROD_LINE_CNT, //全局赤金生产线数
    DORM_SUM_LEVEL,     //宿舍总等级
};

using GlobalAttributeFields = Array<double, enum_size<GlobalAttributeType>::value>;

struct __attribute__((aligned(64))) RoomAttributeFields
{
    ProdType prod_type = ProdType::UNDEFINED;
    OrderType order_type = OrderType::UNDEFINED;
    // time_t time_span = 0;
    double base_prod_eff = 1;  //基础生产效率
    int base_prop_cap = 10;    //基础产品容量
    double base_char_cost = 1; //基础心情消耗
    int prod_cnt = 0;          // 当前库存产品容量
};

enum class ModifierAttributeType
{
    EFF_DELTA, //生产效率增减
    CAP_DELTA, //生产容量增减
    EFF_SCALE, //生产效率倍数
    CAP_SCALE, //生产容量倍数（尚未使用）
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

struct __attribute__((aligned(64))) RoomAttributeModifier
{
    RoomBuff *owner = nullptr;                        //指向房间buff
    RoomBuffType buff_type = RoomBuffType::UNDEFINED; // buff类型
    double eff_delta = 0.;                            //生产效率增减
    int cap_delta = 0;                                //生产容量增减
    double eff_inc_per_hour = 0.;                     //每小时增加生产效率
    double max_extra_eff_delta = 0.;                  //最大额外增加效率

    auto operator==(const RoomAttributeModifier &other) const -> bool; //比较两个属性修改是否相等

    auto operator!=(const RoomAttributeModifier &other) const -> bool; //比较两个属性修改是否不相等

    [[nodiscard]] constexpr auto IsValid() const -> bool
    {
        return validate(*this);
    }

    [[nodiscard]] string to_string() const
    {
        string str;
        str.reserve(256);
        sprintf(str.data(), "[%-35s]dE:%3.f%%, dC:%2d, A:%3.f%%, mE%3.f%%",
                ::to_string(buff_type).data(), eff_delta * 100, cap_delta, eff_inc_per_hour * 100, max_extra_eff_delta * 100);

        return str;
    }

    static constexpr void init(RoomAttributeModifier &modifier, RoomBuff *buff, const RoomBuffType buff_type,
                               const double eff_delta = 0, const int cap_delta = 0, const double eff_delta_inc_hour = 0,
                               const double max_extra_eff_delta = 0)
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

    static constexpr auto validate(const RoomAttributeModifier &val) -> bool
    {
        return val.owner != nullptr && val.buff_type != RoomBuffType::UNDEFINED;
    }
};

struct __attribute__((aligned(16))) RoomFinalAttributeModifier : RoomAttributeModifier
{
    RoomFinalAttributeModifierType final_mod_type = RoomFinalAttributeModifierType::NONE; //最终属性修改类型
    double eff_scale = 1.; //生产效率倍数

    string to_string() const
    {
        string str;
        str.reserve(256);
        sprintf(str.data(), "[%-35s]dE:%3.f%%, dC:%2d, A:%3.f%%, mE:%3.f%%, [%-12s]nE%3.f%%", 
                ::to_string(buff_type).data(),
                eff_delta * 100, cap_delta, eff_inc_per_hour * 100, max_extra_eff_delta * 100,
                ::to_string(final_mod_type).data(), eff_scale * 100);

        return str;
    }

    static constexpr void init(RoomFinalAttributeModifier &modifier, RoomBuff *buff, const RoomBuffType buff_type,
                               const double eff_delta, const int cap_delta, const double eff_delta_inc_hour,
                               const double max_extra_eff_delta, const RoomFinalAttributeModifierType final_mod_type,
                               const double eff_scale = 1)
    {
        RoomAttributeModifier::init(modifier, buff, buff_type, eff_delta, cap_delta, eff_delta_inc_hour,
                                    max_extra_eff_delta);

        modifier.final_mod_type = final_mod_type;
        modifier.eff_scale = eff_scale;
    }
};

struct __attribute__((aligned(32))) CharacterCostModifier
{
    RoomBuff *owner = nullptr;                              //指向房间buff
    CharCostModifierType type = CharCostModifierType::NONE; //修改类型
    double value = 0;                                       //修改值

    [[nodiscard]] constexpr auto IsValid() const -> bool
    {
        return validate(*this);
    }

    auto operator==(const CharacterCostModifier &other) const -> bool;

    auto operator!=(const CharacterCostModifier &other) const -> bool;

    [[nodiscard]] string to_string() const
    {
        string str;
        str.reserve(256);
        sprintf(str.data(),
                "[%-15s]dC:%3.f%%",
                ::to_string(type).data(), value * 100);

        return str;
    }

    static constexpr void init(CharacterCostModifier &modifier, RoomBuff *buff, const CharCostModifierType type,
                               const double value)
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

    static constexpr auto validate(const CharacterCostModifier &modifier) -> bool
    {
        return modifier.owner != nullptr && modifier.type != CharCostModifierType::NONE;
    }
};

class MutualExclusionData //用于表明多个buff是否互斥
{
  public:
    Vector<UInt32> sort_ids; //会互斥的所有buff的排序id，当同时存在多个buff时，排序id大的生效

    MutualExclusionData() : sort_ids(enum_size<RoomBuffType>::value)
    {
    }
};

class MutualExclusionHandler //用于处理多个buff是否互斥
{
  public:
};

class RoomModel //房间模型
{
  public:
    string id;                                 //房间id
    int max_slot_count = 3;                    //房间最大槽位数
    GlobalAttributeFields global_attributes{}; //全局属性
    RoomAttributeFields room_attributes{};     //房间属性
    RoomBuff *buffs[kRoomMaxBuffSlots]{};      //房间buff
    UInt32 n_buff = 0U;                        //房间buff数量
    bm::RoomType type = bm::RoomType::NONE;    //房间类型

    //向房间内添加一个buff
    constexpr auto PushBuff(RoomBuff *buff) -> void
    {
        // return Store(buffs, kRoomMaxBuffs, next_buff_slot, [](const auto& ptr) -> bool{ return ptr != nullptr; },
        // buff);
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

    [[nodiscard]] string to_string() const
    {
        string str;
        str.reserve(8192);
        str.append("[RoomModel]\n");
        str.append("Type    :").append(::to_string(type)).append("\n");
        str.append("ID      :").append(id).append("\n");
        str.append("SlotCnt :").append(std::to_string(max_slot_count)).append("\n");
        str.append("[GlobalAttributes]\n");

        string global_attr;
        UInt32 attr_type = 0;
        for (auto attr : global_attributes)
        {
            //global_attr += string(::to_string(static_cast<GlobalAttributeType>(attr_type))).append(": ").append(std::to_string(attr)).append("\n");
            char line[256];
            sprintf(line, "\t- %-20s: %3.f\n", ::to_string(static_cast<GlobalAttributeType>(attr_type)).data(), attr);
            global_attr.append(line);

            ++attr_type;
        }

        str.append(global_attr);
        str.append("[RoomAttributes]\n");
        char room_attr[1024];
        sprintf(room_attr, 
R"(        - Prod    :%s
        - Order   :%s
        - BaseEff :%3.f%%
        - BaseCap :%d
        - BaseCost:%3.f%%
        - ProdCnt :%d
)", 
            ::to_string(room_attributes.prod_type).data(), ::to_string(room_attributes.order_type).data(),
            room_attributes.base_prod_eff * 100, room_attributes.base_prop_cap, room_attributes.base_char_cost * 100, room_attributes.prod_cnt);
        str.append(room_attr);

        return str;
    }
};

class RoomBuffTargetValidator
{
  public:
    virtual ~RoomBuffTargetValidator() = default;
    RoomBuffTargetValidator() = default;
    RoomBuffTargetValidator(const RoomBuffTargetValidator &src) = default;
    auto operator=(const RoomBuffTargetValidator &rhs) -> RoomBuffTargetValidator & = default;
    RoomBuffTargetValidator(RoomBuffTargetValidator &&src) = default;
    auto operator=(RoomBuffTargetValidator &&src) -> RoomBuffTargetValidator & = default;

    virtual auto validate(const RoomModel * /*room*/) -> bool
    {
        return true;
    }
};

inline auto RoomAttributeModifier::operator==(const RoomAttributeModifier &other) const -> bool
{
    return owner == other.owner && buff_type == other.buff_type;
}

inline auto RoomAttributeModifier::operator!=(const RoomAttributeModifier &other) const -> bool
{
    return !(*this == other);
}

inline auto CharacterCostModifier::operator==(const CharacterCostModifier &other) const -> bool
{
    return owner == other.owner && type == other.type;
}

inline auto CharacterCostModifier::operator!=(const CharacterCostModifier &other) const -> bool
{
    return !(*this == other);
}

class ProdTypeSelector : public RoomBuffTargetValidator
{
  public:
    explicit ProdTypeSelector(ProdType prod_type) : prod_type_(prod_type)
    {
    }

    bool validate(const RoomModel *room) override
    {
        return room->room_attributes.prod_type == prod_type_;
    }

  protected:
    ProdType prod_type_;
};
} // namespace albc

#pragma clang diagnostic pop
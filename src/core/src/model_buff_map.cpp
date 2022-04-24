//
// Created by Nonary on 2022/4/24.
//
#include "model_buff_map.h"

namespace albc::model::buff
{

std::shared_ptr<const std::map<std::string, RoomBuff*>> BuffMap::instance()
{
    static std::shared_ptr<const Dictionary<std::string, RoomBuff *>> instance{new BuffMap()};
    return instance;
}
BuffMap::~BuffMap()
{
    for (auto &pair : *this)
    {
        delete pair.second;
    }

    LOG_D("BuffMap destroyed");
}
BuffMap::BuffMap()
{
    init_buffs(*this);
}
void init_buffs(Dictionary<std::string, RoomBuff *> &buffs)
{
    const auto& sc = SCOPE_TIMER_WITH_TRACE("Initialize Buffs");
    const auto& defer = util::make_defer([&buffs]() {
        for (auto &[id, buff] : buffs)
        {
            buff->buff_id = id;
        }
    });

    // ============================ Manufacture Buffs ============================
    buffs["manu_prod_spd&power[000]"] = new IncEffByPowerPlantCnt(0.05);
    buffs["manu_prod_spd&power[010]"] = new IncEffByPowerPlantCnt(0.10);
    buffs["manu_prod_spd&power[020]"] = new IncEffByPowerPlantCnt(0.15);

    buffs["manu_prod_spd[000]"] = new BasicInc(0.15, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
    buffs["manu_prod_spd[001]"] = new BasicInc(0.15, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
    buffs["manu_prod_spd[002]"] = new BasicInc(0.15, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
    buffs["manu_prod_spd[010]"] = new BasicInc(0.25, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
    buffs["manu_prod_spd[011]"] = new BasicInc(0.25, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
    buffs["manu_prod_spd[021]"] = new BasicInc(0.3, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

    buffs["manu_prod_spd_addition[030]"] =
        new IncEffOverTime(0.2, 0.01, 0.05, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_HOTHEAD);
    buffs["manu_prod_spd_addition[031]"] =
        new IncEffOverTime(0.2, 0.01, 0.05, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_HOTHEAD);
    buffs["manu_prod_spd_addition[040]"] =
        new IncEffOverTime(0.15, 0.02, 0.10, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_SLOWCOACH);
    buffs["manu_prod_spd_addition[041]"] =
        new IncEffOverTime(0.15, 0.02, 0.10, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_SLOWCOACH);

    buffs["manu_prod_spd&limit[000]"] =
        new BasicInc(0.1, 6, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_AND_CAP);
    buffs["manu_prod_spd&limit[001]"] =
        new BasicInc(0.1, 10, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_AND_CAP);

    buffs["manu_prod_limit&cost[000]"] =
        new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
    buffs["manu_prod_limit&cost[003]"] =
        new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
    buffs["manu_prod_limit&cost[0000]"] =
        new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
    buffs["manu_prod_limit&cost[001]"] =
        new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
    buffs["manu_prod_limit&cost[002]"] =
        new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
    buffs["manu_prod_limit&cost[010]"] = new BasicInc(0, 10, -0.25, CharCostModifierType::SELF,
                                                      data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
    buffs["manu_prod_limit&cost[020]"] = new BasicInc(0, 16, -0.25, CharCostModifierType::SELF,
                                                      data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);

    buffs["manu_formula_cost[000]"] =
        new BasicInc(0, 0, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_VLOG);
    buffs["manu_formula_cost[000]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

    buffs["manu_formula_limit[0000]"] = new BasicInc(0, 12, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_VLOG);
    buffs["manu_formula_limit[0000]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

    buffs["manu_formula_limit[010]"] = new BasicInc(0, 15, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_VLOG);
    buffs["manu_formula_limit[010]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

    buffs["manu_prod_spd&limit&cost[000]"] =
        new BasicInc(-0.05, 16, -0.15, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE,
                     RoomBuffType::FACTORY_CRAFTSMANSHIP_SPIRIT);
    buffs["manu_prod_spd&limit&cost[001]"] =
        new BasicInc(-0.05, 19, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE,
                     RoomBuffType::FACTORY_CRAFTSMANSHIP_SPIRIT);
    buffs["manu_prod_spd&limit&cost[010]"] = new BasicInc(
        0.25, -12, 0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_TROUBLE_MAKER);
    buffs["manu_prod_spd&limit&cost[011]"] = new BasicInc(
        0.25, -12, 0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_TROUBLE_MAKER);
    buffs["manu_prod_spd&limit&cost[020]"] = new BasicInc(
        -0.2, 17, -0.25, CharCostModifierType::SELF, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_TROUBLE_MAKER);
    buffs["manu_prod_spd_variable[000]"] = new IncEffByOtherCapInc(0, 0.02, 0.02); // TODO:优先级
    buffs["manu_prod_spd_variable2[000]"] = new IncEffByOtherEffInc(0.05, 0.05, 0.4, data::building::RoomType::MANUFACTURE,
                                                                    RoomBuffType::FACTORY_INC_EFF_BY_OTHER_EFF_INC);
    buffs["manu_prod_spd_variable3[000]"] = new IncEffByOtherCapInc(16, 0.01, 0.03);
    //"manu_prod_spd_bd_n1[000]"
    buffs["manu_prod_spd_bd[000]"] =
        new IncEffByGlobalAttribute(1, 0.005, GlobalAttributeType::CHAIN_OF_THOUGHT, data::building::RoomType::MANUFACTURE,
                                    RoomBuffType::FACTORY_INC_EFF_BY_CHAIN_OF_THOUGHT);
    buffs["manu_prod_spd_bd[010]"] =
        new IncEffByGlobalAttribute(1, 0.01, GlobalAttributeType::CHAIN_OF_THOUGHT, data::building::RoomType::MANUFACTURE,
                                    RoomBuffType::FACTORY_INC_EFF_BY_CHAIN_OF_THOUGHT);

    buffs["manu_formula_spd[000]"] =
        new BasicInc(0.3, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_RECORD);
    buffs["manu_formula_spd[000]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

    buffs["manu_formula_spd[010]"] =
        new BasicInc(0.3, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_RECORD);
    buffs["manu_formula_spd[010]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

    buffs["manu_formula_spd[020]"] =
        new BasicInc(0.35, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_RECORD);
    buffs["manu_formula_spd[020]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

    buffs["manu_prod_spd&trade[000]"] =
        new IncEffByGlobalAttribute(1, 0.2, GlobalAttributeType::TRADING_POST_CNT, data::building::RoomType::MANUFACTURE,
                                    RoomBuffType::FACTORY_INC_EFF_BY_TRADING_POST_CNT);
    buffs["manu_prod_spd&trade[000]"]->AddValidator(new ProdTypeSelector(ProdType::GOLD)); //条件

    buffs["manu_formula_spd[100]"] =
        new BasicInc(0.3, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_GOLD);
    buffs["manu_formula_spd[100]"]->AddValidator(new ProdTypeSelector(ProdType::GOLD));

    buffs["manu_formula_spd[101]"] =
        new BasicInc(0.35, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_GOLD);
    buffs["manu_formula_spd[101]"]->AddValidator(new ProdTypeSelector(ProdType::GOLD));

    buffs["manu_formula_spd[200]"] =
        new BasicInc(0.30, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
    buffs["manu_formula_spd[200]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

    buffs["manu_formula_spd[201]"] =
        new BasicInc(0.30, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
    buffs["manu_formula_spd[201]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

    buffs["manu_formula_spd[210]"] =
        new BasicInc(0.35, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
    buffs["manu_formula_spd[210]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

    buffs["manu_formula_spd[211]"] =
        new BasicInc(0.35, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
    buffs["manu_formula_spd[211]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

    buffs["manu_formula_spd[212]"] =
        new BasicInc(0.35, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
    buffs["manu_formula_spd[212]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

    buffs["manu_formula_spd[213]"] =
        new BasicInc(0.35, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
    buffs["manu_formula_spd[213]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

    // "manu_cost_all[000]": 团队精神: 进驻制造站时，消除当前制造站内所有干员自身心情消耗的影响
    buffs["manu_cost_all[000]"] = new BasicInc(0, 0, 0, CharCostModifierType::ROOM_CLEAR_ALL, data::building::RoomType::MANUFACTURE,
                                               RoomBuffType::FACTORY_TRAM_SPIRIT);

    // "manu_prod_spd[003]": 磐蟹·豆豆: 进驻制造站时，生产力+15%
    buffs["manu_prod_spd[003]"] = new BasicInc(0.15, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

    // "manu_prod_spd[012]": 红松骑士团·β: 进驻制造站时，生产力+25%
    buffs["manu_prod_spd[012]"] = new BasicInc(0.25, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

    // "manu_prod_spd[020]": 咪波·制造型: 进驻制造站时，生产力+30%
    buffs["manu_prod_spd[020]"] = new BasicInc(0.30, 0, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

    // "manu_prod_spd_bd_n1[000]": 超感: 进驻制造站时，宿舍内每有1名干员则感知信息+1，同时每1点感知信息转化为1点思维链环

    // "manu_skill_spd1[010]": 意识协议: 进驻制造站时，当前制造站内每个标准化类技能为自身+5%的生产力
    buffs["manu_skill_spd1[010]"] = new IncEffByStandardizationCnt(
        0.05, data::building::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_STANDARDIZATION_CNT);

    //  ============================ Trading Post Buffs ============================
    buffs["trade_ord_spd[000]"] = new BasicInc(0.2, 0, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
    buffs["trade_ord_spd[001]"] = new BasicInc(0.3, 0, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
    buffs["trade_ord_spd[010]"] = new BasicInc(0.2, 0, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
    buffs["trade_ord_spd[011]"] = new BasicInc(0.3, 0, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
    buffs["trade_ord_spd[020]"] = new BasicInc(0.35, 0, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
    buffs["trade_ord_spd_variable2[000]"] =
        new IncEffByOtherEffInc(0.05, 0.05, 0.25, data::building::RoomType::TRADING, RoomBuffType::TRADING_HEAVENLY_REWARD);
    buffs["trade_ord_spd_variable2[001]"] =
        new IncEffByOtherEffInc(0.05, 0.05, 0.35, data::building::RoomType::TRADING, RoomBuffType::TRADING_HEAVENLY_REWARD);
    buffs["trade_ord_spd&limit[010]"] =
        new BasicInc(0.25, 1, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
    buffs["trade_ord_spd&limit[020]"] =
        new BasicInc(0.15, 2, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
    buffs["trade_ord_spd&limit[021]"] =
        new BasicInc(0.15, 4, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
    buffs["trade_ord_spd&limit[022]"] =
        new BasicInc(0.20, 4, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
    buffs["trade_ord_spd&limit[030]"] =
        new BasicInc(0.20, 0, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
    buffs["trade_ord_spd&limit[031]"] =
        new BasicInc(0.30, 1, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
    buffs["trade_ord_spd&limit[032]"] =
        new BasicInc(0.20, 0, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
    buffs["trade_ord_spd&limit[033]"] =
        new BasicInc(0.30, 1, data::building::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);

    // "trade_ord_limit&cost[000]": 谈判: 进驻贸易站时，订单上限+5，心情每小时消耗-0.25
    buffs["trade_ord_limit&cost[000]"] =
        new BasicInc(0, 5, -0.25, CharCostModifierType::SELF, data::building::RoomType::TRADING, RoomBuffType::TRADING_NEGOTIATION);

    // "trade_ord_limit&cost_P[000]": 醉翁之意·α: 当与德克萨斯在同一个贸易站时，心情每小时消耗-0.1，订单上限+2
    buffs["trade_ord_limit&cost_P[000]"] = new LapplandTradeBuff(-0.1, 2);

    // "trade_ord_limit&cost_P[001]": 醉翁之意·β: 当与德克萨斯在同一个贸易站时，心情每小时消耗-0.1，订单上限+4
    buffs["trade_ord_limit&cost_P[001]"] = new LapplandTradeBuff(-0.1, 4);

    // "trade_ord_limit&cost_P[010]": 默契: 当与能天使在同一个贸易站时，心情每小时消耗-0.3
    buffs["trade_ord_limit&cost_P[010]"] = new TexasTradeBuff(true);

    // "trade_ord_limit_count[000]": 市井之道: 进驻贸易站时，当前贸易站内其他干员提供的每10%订单获取效率使订单上限-1（订单最少为1），同时每有1笔订单就+4%订单获取效率
    buffs["trade_ord_limit_count[000]"] = new JayeTradeBuff(true);

    // "trade_ord_limit_diff[000]": 摊贩经济: 进驻贸易站时，当前订单数与订单上限每差1笔订单，则订单获取效率+4%
    buffs["trade_ord_limit_diff[000]"] = new JayeTradeBuff(false);

    // "trade_ord_line_gold[000]": 订单流可视化·α: 进驻贸易站时，订单获取效率+5%，每有4条赤金生产线，则赤金生产线额外+2
    // "trade_ord_line_gold[010]": 订单流可视化·β: 进驻贸易站时，订单获取效率+5%，每有2条赤金生产线，则赤金生产线额外+2
    // "trade_ord_long[000]": 投资·α: 进驻贸易站后，如果下笔赤金订单交付数大于3，则其龙门币收益+250，心情每小时消耗-0.25
    // "trade_ord_long[010]": 投资·β: 进驻贸易站后，如果下笔赤金订单交付数大于3，则其龙门币收益+500，心情每小时消耗-0.25
    // "trade_ord_spd&cost[000]": 交际: 进驻贸易站时，订单获取效率+30%，心情每小时消耗-0.25
    buffs["trade_ord_spd&cost[000]"] =
        new BasicInc(0.3, 0, -0.25, CharCostModifierType::SELF, data::building::RoomType::TRADING, RoomBuffType::TRADING_COMM);

    // "trade_ord_spd&cost_P[000]": 恩怨: 当与拉普兰德在同一个贸易站时，心情每小时消耗+0.3，订单获取效率+65%
    buffs["trade_ord_spd&cost_P[000]"] = new TexasTradeBuff(false);

    // "trade_ord_spd&dorm&lv[000]": 虔诚筹款·α: 进驻贸易站时，每间宿舍每级+1%获取效率
    buffs["trade_ord_spd&dorm&lv[000]"] = new IncEffByGlobalAttribute(
        1, 0.01, GlobalAttributeType::DORM_SUM_LEVEL, data::building::RoomType::TRADING, RoomBuffType::TRADING_FUNDRAISING);

    // "trade_ord_spd&dorm&lv[010]": 虔诚筹款·β: 进驻贸易站时，每间宿舍每级+2%获取效率
    buffs["trade_ord_spd&dorm&lv[010]"] = new IncEffByGlobalAttribute(
        1, 0.02, GlobalAttributeType::DORM_SUM_LEVEL, data::building::RoomType::TRADING, RoomBuffType::TRADING_FUNDRAISING);

    // "trade_ord_spd&gold[000]": 物流规划·α: 进驻贸易站时，订单获取效率+5%，每有4条赤金生产线，则当前贸易站订单获取效率额外+15%
    buffs["trade_ord_spd&gold[000]"] = new IncEffByGlobalAttribute(
        0.05, 4, 0.15, GlobalAttributeType::GOLD_PROD_LINE_CNT, data::building::RoomType::TRADING, RoomBuffType::TRADING_ORDER_FLOW_VISUALIZATION);

    // "trade_ord_spd&gold[010]": 物流规划·β: 进驻贸易站时，订单获取效率+5%，每有2条赤金生产线，则当前贸易站订单获取效率额外+15%
    buffs["trade_ord_spd&gold[010]"] = new IncEffByGlobalAttribute(
        0.05, 2, 0.15, GlobalAttributeType::GOLD_PROD_LINE_CNT, data::building::RoomType::TRADING, RoomBuffType::TRADING_ORDER_FLOW_VISUALIZATION);

    // "trade_ord_spd&limit[000]": 订单管理·α: 进驻贸易站时，订单获取效率+10%，且订单上限+2
    buffs["trade_ord_spd&limit[000]"] = new BasicInc(0.1, 2, 0, CharCostModifierType::SELF, data::building::RoomType::TRADING,
                                                     RoomBuffType::TRADING_INC_EFF_AND_CAP);

    // "trade_ord_spd&limit[001]": 订单管理·β: 进驻贸易站时，订单获取效率+10%，且订单上限+4
    buffs["trade_ord_spd&limit[001]"] = new BasicInc(0.1, 4, 0, CharCostModifierType::SELF, data::building::RoomType::TRADING,
                                                     RoomBuffType::TRADING_INC_EFF_AND_CAP);

    // "trade_ord_spd_bd_n2[000]": “愿者上钩”: 进驻贸易站时，宿舍内每有1名干员则人间烟火+1，同时每有1点人间烟火，则订单获取效率+1%

    // "trade_ord_vodfox[000]": 低语: 进驻贸易站时，当前贸易站内其他干员提供的订单获取效率全部归零，且每人为自身+45%订单获取效率，同时全体心情每小时消耗+0.25
    buffs["trade_ord_vodfox[000]"] = new VodfoxTradeBuff();

    // "trade_ord_wt&cost[000]": 裁缝·α: 进驻贸易站时，小幅提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25

    // "trade_ord_wt&cost[001]": 手工艺品·α: 进驻贸易站时，小幅提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25

    // "trade_ord_wt&cost[010]": 裁缝·β: 进驻贸易站时，提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25

    // "trade_ord_wt&cost[011]": 手工艺品·β: 进驻贸易站时，提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25

    //  加高品质贵金属订单概率等效效率加成的计算过程：https://www.bilibili.com/video/BV1bo4y1y7ui
}
}
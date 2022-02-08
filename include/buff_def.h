#pragma once

#include "primitive_types.h"
#include "buff_model.h"

namespace albc
{
	using BuffMap = Dictionary<string, RoomBuff*>;
	static BuffMap& buffs = *new BuffMap;

	static void init_buffs()
	{
		static bool has_buffs_init_finished;
		if (has_buffs_init_finished)return;
		has_buffs_init_finished = true;

		// ============================ Manufacture Buffs ============================
		buffs["manu_prod_spd&power[000]"] = new IncEffByPowerPlantCnt(0.05, true);
		buffs["manu_prod_spd&power[010]"] = new IncEffByPowerPlantCnt(0.10, true);
		buffs["manu_prod_spd&power[020]"] = new IncEffByPowerPlantCnt(0.15, true);

		buffs["manu_prod_spd[000]"] = new BasicInc(0.15, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
		buffs["manu_prod_spd[001]"] = new BasicInc(0.15, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
		buffs["manu_prod_spd[002]"] = new BasicInc(0.15, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
		buffs["manu_prod_spd[010]"] = new BasicInc(0.25, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
		buffs["manu_prod_spd[011]"] = new BasicInc(0.25, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);
		buffs["manu_prod_spd[021]"] = new BasicInc(0.3, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

		buffs["manu_prod_spd_addition[030]"] = new IncEffOverTime(0.2, 0.01, 0.05, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_HOTHEAD);
		buffs["manu_prod_spd_addition[031]"] = new IncEffOverTime(0.2, 0.01, 0.05, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_HOTHEAD);
		buffs["manu_prod_spd_addition[040]"] = new IncEffOverTime(0.15, 0.02, 0.10, bm::RoomType::MANUFACTURE,  RoomBuffType::FACTORY_SLOWCOACH);
		buffs["manu_prod_spd_addition[041]"] = new IncEffOverTime(0.15, 0.02, 0.10, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_SLOWCOACH);

		buffs["manu_prod_spd&limit[000]"] = new BasicInc(0.1, 6, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_AND_CAP);
		buffs["manu_prod_spd&limit[001]"] = new BasicInc(0.1, 10, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_AND_CAP);

		buffs["manu_prod_limit&cost[000]"] = new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
		buffs["manu_prod_limit&cost[003]"] = new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
		buffs["manu_prod_limit&cost[0000]"] = new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
		buffs["manu_prod_limit&cost[001]"] = new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
		buffs["manu_prod_limit&cost[002]"] = new BasicInc(0, 8, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
		buffs["manu_prod_limit&cost[010]"] = new BasicInc(0, 10, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);
		buffs["manu_prod_limit&cost[020]"] = new BasicInc(0, 16, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_JUNKMAN);

		buffs["manu_formula_cost[000]"] = new BasicInc(0, 0, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_VLOG);
		buffs["manu_formula_cost[000]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

		buffs["manu_formula_limit[0000]"] = new BasicInc(0, 12, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_VLOG);
		buffs["manu_formula_limit[0000]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

		buffs["manu_formula_limit[010]"] = new BasicInc(0, 15, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_VLOG);
		buffs["manu_formula_limit[010]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

		buffs["manu_prod_spd&limit&cost[000]"] = new BasicInc(-0.05, 16, -0.15, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_CRAFTSMANSHIP_SPIRIT);
		buffs["manu_prod_spd&limit&cost[001]"] = new BasicInc(-0.05, 19, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_CRAFTSMANSHIP_SPIRIT);
		buffs["manu_prod_spd&limit&cost[010]"] = new BasicInc(0.25, -12, 0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_TROUBLE_MAKER);
		buffs["manu_prod_spd&limit&cost[011]"] = new BasicInc(0.25, -12, 0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_TROUBLE_MAKER);
		buffs["manu_prod_spd&limit&cost[020]"] = new BasicInc(-0.2, 17, -0.25, CharCostModifierType::SELF, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_TROUBLE_MAKER);
		buffs["manu_prod_spd_variable[000]"] = new IncEffByOtherCapInc(0, 0.02, 0.02); //TODO:优先级
		buffs["manu_prod_spd_variable2[000]"] = new IncEffByOtherEffInc(0.05, 0.05, 0.4, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_OTHER_EFF_INC);
		buffs["manu_prod_spd_variable3[000]"] = new IncEffByOtherCapInc(16, 0.01, 0.03);
		//"manu_prod_spd_bd_n1[000]"
		buffs["manu_prod_spd_bd[000]"] = new IncEffByGlobalAttribute(0.005, GlobalAttributeType::CHAIN_OF_THOUGHT, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_CHAIN_OF_THOUGHT);
		buffs["manu_prod_spd_bd[010]"] = new IncEffByGlobalAttribute(0.01, GlobalAttributeType::CHAIN_OF_THOUGHT, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_CHAIN_OF_THOUGHT);

		buffs["manu_formula_spd[000]"] = new BasicInc(0.3, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_RECORD);
		buffs["manu_formula_spd[000]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

		buffs["manu_formula_spd[010]"] = new BasicInc(0.3, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_RECORD);
		buffs["manu_formula_spd[010]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

		buffs["manu_formula_spd[020]"] = new BasicInc(0.35, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_RECORD);
		buffs["manu_formula_spd[020]"]->AddValidator(new ProdTypeSelector(ProdType::RECORD));

		buffs["manu_prod_spd&trade[000]"] = new IncEffByGlobalAttribute(0.2, GlobalAttributeType::TRADING_POST_CNT, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_TRADING_POST_CNT);
		buffs["manu_prod_spd&trade[000]"]->AddValidator(new ProdTypeSelector(ProdType::GOLD)); //条件

		buffs["manu_formula_spd[100]"] = new BasicInc(0.3, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_GOLD);
		buffs["manu_formula_spd[100]"]->AddValidator(new ProdTypeSelector(ProdType::GOLD));

		buffs["manu_formula_spd[101]"] = new BasicInc(0.35, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_GOLD);
		buffs["manu_formula_spd[101]"]->AddValidator(new ProdTypeSelector(ProdType::GOLD));

		buffs["manu_formula_spd[200]"] =new BasicInc(0.30, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
		buffs["manu_formula_spd[200]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

		buffs["manu_formula_spd[201]"] = new BasicInc(0.30, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
		buffs["manu_formula_spd[201]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

		buffs["manu_formula_spd[210]"] = new BasicInc(0.35, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
		buffs["manu_formula_spd[210]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

		buffs["manu_formula_spd[211]"] = new BasicInc(0.35, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
		buffs["manu_formula_spd[211]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

		buffs["manu_formula_spd[212]"] = new BasicInc(0.35, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
		buffs["manu_formula_spd[212]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

		buffs["manu_formula_spd[213]"] = new BasicInc(0.35, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ORIGINIUM);
		buffs["manu_formula_spd[213]"]->AddValidator(new ProdTypeSelector(ProdType::ORIGINIUM_SHARD));

		// "manu_cost_all[000]": 团队精神: 进驻制造站时，消除当前制造站内所有干员自身心情消耗的影响
		buffs["manu_cost_all[000]"] = new BasicInc(0, 0, 0, CharCostModifierType::ROOM_CLEAR_ALL, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_TRAM_SPIRIT);

		// "manu_prod_spd[003]": 磐蟹·豆豆: 进驻制造站时，生产力+15%
		buffs["manu_prod_spd[003]"] = new BasicInc(0.15, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

		// "manu_prod_spd[012]": 红松骑士团·β: 进驻制造站时，生产力+25%
		buffs["manu_prod_spd[012]"] = new BasicInc(0.25, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

		// "manu_prod_spd[020]": 咪波·制造型: 进驻制造站时，生产力+30%
		buffs["manu_prod_spd[020]"] = new BasicInc(0.30, 0, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_ALL);

		// "manu_prod_spd_bd_n1[000]": 超感: 进驻制造站时，宿舍内每有1名干员则感知信息+1，同时每1点感知信息转化为1点思维链环

		// "manu_skill_spd1[010]": 意识协议: 进驻制造站时，当前制造站内每个标准化类技能为自身+5%的生产力
		buffs["manu_skill_spd1[010]"] = new IncEffByStandardizationCnt(0.05, bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_STANDARDIZATION_CNT);

		//  ============================ Trading Post Buffs ============================
		buffs["trade_ord_spd[000]"] = new BasicInc(0.2, 0, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
		buffs["trade_ord_spd[001]"] = new BasicInc(0.3, 0, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
		buffs["trade_ord_spd[010]"] = new BasicInc(0.2, 0, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
		buffs["trade_ord_spd[011]"] = new BasicInc(0.3, 0, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
		buffs["trade_ord_spd[020]"] = new BasicInc(0.35, 0, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF);
		buffs["trade_ord_spd_variable2[000]"] = new IncEffByOtherEffInc(0.05, 0.05, 0.25, bm::RoomType::TRADING, RoomBuffType::TRADING_HEAVENLY_REWARD);
		buffs["trade_ord_spd_variable2[001]"] = new IncEffByOtherEffInc(0.05, 0.05, 0.35, bm::RoomType::TRADING, RoomBuffType::TRADING_HEAVENLY_REWARD);
		buffs["trade_ord_spd&limit[010]"] = new BasicInc(0.25, 1, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
		buffs["trade_ord_spd&limit[020]"] = new BasicInc(0.15, 2, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
		buffs["trade_ord_spd&limit[021]"] = new BasicInc(0.15, 4, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
		buffs["trade_ord_spd&limit[022]"] = new BasicInc(0.20, 4, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
		buffs["trade_ord_spd&limit[030]"] = new BasicInc(0.20, 0, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
		buffs["trade_ord_spd&limit[031]"] = new BasicInc(0.30, 1, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
		buffs["trade_ord_spd&limit[032]"] = new BasicInc(0.20, 0, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);
		buffs["trade_ord_spd&limit[033]"] = new BasicInc(0.30, 1, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);

		// "trade_ord_limit&cost[000]": 谈判: 进驻贸易站时，订单上限+5，心情每小时消耗-0.25
		buffs["trade_ord_limit&cost[000]"] = new BasicInc(0, 5, -0.25, CharCostModifierType::SELF, bm::RoomType::TRADING, RoomBuffType::TRADING_NEGOTIATION);

		// "trade_ord_limit&cost_P[000]": 醉翁之意·α: 当与德克萨斯在同一个贸易站时，心情每小时消耗-0.1，订单上限+2
		// "trade_ord_limit&cost_P[001]": 醉翁之意·β: 当与德克萨斯在同一个贸易站时，心情每小时消耗-0.1，订单上限+4
		// "trade_ord_limit&cost_P[010]": 默契: 当与能天使在同一个贸易站时，心情每小时消耗-0.3

		// 孑使用等效效率 计算过程：https://www.bilibili.com/video/BV1nK4y137Xh
		// "trade_ord_limit_count[000]": 市井之道: 进驻贸易站时，当前贸易站内其他干员提供的每10%订单获取效率使订单上限-1（订单最少为1），同时每有1笔订单就+4%订单获取效率
		//buffs["trade_ord_limit_count[000]"] = new 

		// "trade_ord_limit_diff[000]": 摊贩经济: 进驻贸易站时，当前订单数与订单上限每差1笔订单，则订单获取效率+4%
		// 假设贸易站空余10个订单数，初始加成为40%
		buffs["trade_ord_limit_diff[000]"] = new TradeOrdLimitDiffBuff();

		// "trade_ord_line_gold[000]": 订单流可视化·α: 进驻贸易站时，订单获取效率+5%，每有4条赤金生产线，则赤金生产线额外+2
		// "trade_ord_line_gold[010]": 订单流可视化·β: 进驻贸易站时，订单获取效率+5%，每有2条赤金生产线，则赤金生产线额外+2
		// "trade_ord_long[000]": 投资·α: 进驻贸易站后，如果下笔赤金订单交付数大于3，则其龙门币收益+250，心情每小时消耗-0.25
		// "trade_ord_long[010]": 投资·β: 进驻贸易站后，如果下笔赤金订单交付数大于3，则其龙门币收益+500，心情每小时消耗-0.25
		// "trade_ord_spd&cost[000]": 交际: 进驻贸易站时，订单获取效率+30%，心情每小时消耗-0.25
		buffs["trade_ord_spd&cost[000]"] = new BasicInc(0.3, 0, -0.25, CharCostModifierType::SELF, bm::RoomType::TRADING, RoomBuffType::TRADING_COMM);

		// "trade_ord_spd&cost_P[000]": 恩怨: 当与拉普兰德在同一个贸易站时，心情每小时消耗+0.3，订单获取效率+65%
		// "trade_ord_spd&dorm&lv[000]": 虔诚筹款·α: 进驻贸易站时，每间宿舍每级+1%获取效率
		buffs["trade_ord_spd&dorm&lv[000]"] = new IncEffByGlobalAttribute(0.01, GlobalAttributeType::DORM_SUM_LEVEL, bm::RoomType::TRADING, RoomBuffType::TRADING_FUNDRAISING);

		// "trade_ord_spd&dorm&lv[010]": 虔诚筹款·β: 进驻贸易站时，每间宿舍每级+2%获取效率
		buffs["trade_ord_spd&dorm&lv[000]"] = new IncEffByGlobalAttribute(0.02, GlobalAttributeType::DORM_SUM_LEVEL, bm::RoomType::TRADING, RoomBuffType::TRADING_FUNDRAISING);

		// "trade_ord_spd&gold[000]": 物流规划·α: 进驻贸易站时，订单获取效率+5%，每有4条赤金生产线，则当前贸易站订单获取效率额外+15%
		// "trade_ord_spd&gold[010]": 物流规划·β: 进驻贸易站时，订单获取效率+5%，每有2条赤金生产线，则当前贸易站订单获取效率额外+15%
		// "trade_ord_spd&limit[000]": 订单管理·α: 进驻贸易站时，订单获取效率+10%，且订单上限+2
		buffs["trade_ord_spd&limit[000]"] = new BasicInc(0.1, 2, 0, CharCostModifierType::SELF, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);

		// "trade_ord_spd&limit[001]": 订单管理·β: 进驻贸易站时，订单获取效率+10%，且订单上限+4
		buffs["trade_ord_spd&limit[001]"] = new BasicInc(0.1, 4, 0, CharCostModifierType::SELF, bm::RoomType::TRADING, RoomBuffType::TRADING_INC_EFF_AND_CAP);

		// "trade_ord_spd_bd_n2[000]": “愿者上钩”: 进驻贸易站时，宿舍内每有1名干员则人间烟火+1，同时每有1点人间烟火，则订单获取效率+1%
		// "trade_ord_vodfox[000]": 低语: 进驻贸易站时，当前贸易站内其他干员提供的订单获取效率全部归零，且每人为自身+45%订单获取效率，同时全体心情每小时消耗+0.25
		buffs["trade_ord_vodfox[000]"] = new VodfoxTradeBuff();

		// "trade_ord_wt&cost[000]": 裁缝·α: 进驻贸易站时，小幅提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25
		// "trade_ord_wt&cost[001]": 手工艺品·α: 进驻贸易站时，小幅提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25
		// "trade_ord_wt&cost[010]": 裁缝·β: 进驻贸易站时，提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25
		// "trade_ord_wt&cost[011]": 手工艺品·β: 进驻贸易站时，提升当前贸易站高品质贵金属订单的出现概率（工作时长影响概率），心情每小时消耗-0.25
		//  加高品质贵金属订单概率等效效率加成的计算过程：https://www.bilibili.com/video/BV1bo4y1y7ui

		// ============================ Control Center Buffs ============================
		// "control_allCost_condChar[000]": 浮生得闲: 当与阿进驻控制中枢一起工作时，控制中枢内所有干员心情每小时恢复+0.25
		// "control_clue_cost[000]": 神经质: 进驻控制中枢时，提升会客室内干员所属派系的线索倾向效果，但控制中枢内所有干员的心情每小时消耗+1.5
		// "control_clue_cost[010]": 至察: 进驻控制中枢时，小幅提升会客室内干员所属派系的线索倾向效果，但控制中枢内所有干员的心情每小时消耗+0.5
		// "control_clue_cost[011]": 断事如神: 进驻控制中枢时，小幅提升会客室内干员所属派系的线索倾向效果
		// "control_costToBD[000]": “山河远阔”: 进驻控制中枢时，当自身心情大于12时，人间烟火+15；当自身心情处于12以下时，感知信息+10
		// "control_facCostReset[000]": 杯莫停: 进驻控制中枢时，消除当前控制中枢内所有岁干员自身心情消耗的影响
		// "control_hire_spd[000]": 感染力: 进驻控制中枢时，人力办公室联络速度小于30%时（其中包含基础联络速度5%），则联络速度额外+20%（该加成全局效果唯一，不受其它加成影响）
		// "control_mp_aegir1[000]": 潮汐守望: 进驻控制中枢时，每有1个深海猎人干员进驻在宿舍以外的设施，则自身心情每小时消耗+0.5；反之则自身心情每小时恢复+0.5，如果进驻在宿舍内的深海猎人干员为满心情，则额外+0.5
		// "control_mp_aegir2[000]": 集群狩猎·α: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05；基建内深海猎人干员获得特殊加成（与部分技能有特殊叠加规则）
		// "control_mp_aegir2[010]": 集群狩猎·β: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05；基建内深海猎人干员获得特殊加成（与部分技能有特殊叠加规则）
		// "control_mp_bd[000]": 情报储备: 进驻控制中枢时，控制中枢内每有1名彩虹小队干员，则情报储备+1
		// "control_mp_bd[010]": 乌萨斯特饮: 进驻控制中枢时，控制中枢内每有1名乌萨斯学生自治团干员，则乌萨斯特饮+1
		// "control_mp_cost&bd1[000]": "不以物喜": 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05；当自身心情处于12以下时，人间烟火+15
		// "control_mp_cost&bd2[000]": "不以己悲": 进驻控制中枢时，自身心情每小时消耗+0.5；当自身心情大于12时，感知信息+10
		// "control_mp_cost&faction2[000]": 坚毅随和: 进驻控制中枢时，控制中枢内每个鲤氏侦探事务所干员可使控制中枢内所有干员的心情每小时恢复+0.05，同时该派系干员的心情每小时恢复额外+0.2
		// "control_mp_cost&faction[000]": 德才兼备: 进驻控制中枢时，控制中枢内每个龙门近卫局干员可使控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost&faction[020]": 学生会会长: 进驻控制中枢时，控制中枢内每个乌萨斯学生自治团干员可使控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost&faction[030]": 幕后指挥: 进驻控制中枢时，控制中枢内每个喀兰贸易干员可使控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost&faction[900]": 异格者: 进驻控制中枢时，控制中枢内每个异格干员可使控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost&faction[990]": 彩虹小队: 进驻控制中枢时，控制中枢内每个彩虹小队干员可使控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[000]": 左膀右臂: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[001]": S.W.E.E.P.: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[002]": 零食网络: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[003]": 清理协议: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[004]": 替身: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[005]": 必要责任: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[006]": 护卫: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_cost[007]": 小小的领袖: 进驻控制中枢时，控制中枢内所有干员的心情每小时恢复+0.05
		// "control_mp_psk[000]": 红松的骑士: 进驻控制中枢时，每个进驻在制造站的红松骑士团干员，作战记录类配方的生产力+10%，贵金属类配方的生产力-10%
		// "control_pow_bot[000]": 我寻思能行: 进驻控制中枢时，如果Lancet-2进驻在发电站，发电站额外+2（仅影响设施数量）
		// "control_prod_spd[000]": 最高权限: 进驻控制中枢时，所有制造站生产力+2%（同种效果取最高）
		// "control_token_prod_spd[000]": 超频: 进驻控制中枢时，如果有2台以上作业平台进驻在发电站，则所有制造站生产力+2%（同种效果取最高）
		// "control_tra_limit&spd[000]": 精密计算: 进驻控制中枢时，每个进驻在贸易站的喀兰贸易干员，订单获取效率-15%，订单上限+6
		// "control_tra_spd[000]": 合作协议: 进驻控制中枢时，所有贸易站订单效率+7%（同种效果取最高）
		// "control_tra_spd[010]": 大小姐: 进驻控制中枢时，所有贸易站订单效率+7%（同种效果取最高）
		// "control_upMeetingSpeed[000]": 世事洞明: 进驻控制中枢时，会客室线索搜集速度+25%

		// ============================ Dormitory Buffs ============================
		// "dorm_rec_all&oneself[000]": 慵懒: 进驻宿舍时，自身心情每小时恢复-0.1，该宿舍内所有干员的心情每小时恢复+0.2（同种效果取最高）
		// "dorm_rec_all&oneself[001]": 嗜睡: 进驻宿舍时，自身心情每小时恢复-0.1，该宿舍内所有干员的心情每小时恢复+0.25（同种效果取最高）
		// "dorm_rec_all&oneself[010]": 超脱: 进驻宿舍时，自身心情每小时恢复+0.55
		// "dorm_rec_all&oneself[011]": 挣脱: 进驻宿舍时，该宿舍内除自身以外所有干员的心情每小时恢复+0.1（同种效果取最高）
		// "dorm_rec_all&oneself[012]": 解脱: 进驻宿舍时，自身心情每小时恢复+0.55，该宿舍内所有干员的心情每小时恢复+0.1（同种效果取最高）
		// "dorm_rec_all&oneself[021]": 牧歌: 进驻宿舍时，自身心情每小时恢复+0.55，该宿舍内所有干员的心情每小时恢复+0.1（同种效果取最高）
		// "dorm_rec_all&oneself[042]": “归乡”: 进驻宿舍时，自身心情每小时恢复+0.55，该宿舍内所有干员的心情每小时恢复+0.1（同种效果取最高）
		// "dorm_rec_all[000]": 鼓舞: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.1（同种效果取最高）
		// "dorm_rec_all[010]": 小提琴独奏: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.15（同种效果取最高）
		// "dorm_rec_all[011]": 偶像: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.15（同种效果取最高）
		// "dorm_rec_all[012]": 沁人心脾: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.15（同种效果取最高）
		// "dorm_rec_all[013]": 领袖: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.15（同种效果取最高）
		// "dorm_rec_all[020]": 冬将军: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.2（同种效果取最高）
		// "dorm_rec_all[021]": 提灯女神: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.2（同种效果取最高）
		// "dorm_rec_all[022]": 狮心王: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.2（同种效果取最高）
		// "dorm_rec_bd_n1[000]": 梦境呓语: 进驻宿舍时，每1层梦境转化为1点感知信息
		// "dorm_rec_bd_n1_n2[000]": 睡前故事: 进驻宿舍时，该宿舍内所有干员的心情每小时恢复+0.1（同种效果取最高），同时当前宿舍每级提供1层梦境
		// "dorm_rec_oneself2[000]": “独处”: 进驻宿舍时，自身心情每小时恢复+0.7；如果该宿舍内有其他干员，则每人额外为自身心情每小时恢复+0.05
		// "dorm_rec_oneself[000]": 独处: 进驻宿舍时，自身心情每小时恢复+0.7
		// "dorm_rec_oneself[010]": 狂热: 进驻宿舍时，自身心情每小时恢复+0.85
		// "dorm_rec_oneself[011]": 麻烦回避者: 进驻宿舍时，自身心情每小时恢复+0.85
		// "dorm_rec_oneself[020]": 悲歌: 进驻宿舍时，自身心情每小时恢复+1
		// "dorm_rec_oneself[030]": 隐形的美食家: 进驻宿舍时，自身心情每小时恢复+0.75
		// "dorm_rec_single&oneself[000]": 活泼: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.2（同种效果取最高）；同时自身心情每小时恢复+0.4
		// "dorm_rec_single&oneself[001]": 探险家的热情: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.25（同种效果取最高）；同时自身心情每小时恢复+0.5
		// "dorm_rec_single&oneself[010]": 烘焙: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.3（同种效果取最高）；同时自身心情每小时恢复+0.3
		// "dorm_rec_single&oneself[011]": 烹饪: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.35（同种效果取最高）；同时自身心情每小时恢复+0.35
		// "dorm_rec_single&oneself[012]": Give me five: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.35（同种效果取最高）；同时自身心情每小时恢复+0.35
		// "dorm_rec_single&oneself[020]": 和谐: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.4（同种效果取最高）；同时自身心情每小时恢复+0.2
		// "dorm_rec_single&oneself[021]": 喀兰圣女: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.5（同种效果取最高）；同时自身心情每小时恢复+0.25
		// "dorm_rec_single&oneself[030]": 使徒: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.5（同种效果取最高）；同时自身心情每小时恢复+0.25
		// "dorm_rec_single[000]": 医疗服务: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.65（同种效果取最高）
		// "dorm_rec_single[001]": 疗养: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.65（同种效果取最高）
		// "dorm_rec_single[010]": 善解人意: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.55（同种效果取最高）
		// "dorm_rec_single[020]": 慈悲: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.75（同种效果取最高）
		// "dorm_rec_single[030]": 天启: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.7（同种效果取最高）
		// "dorm_rec_single[031]": 维多利亚文学: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.7（同种效果取最高）
		// "dorm_rec_single[032]": 心理疏导: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.7（同种效果取最高）
		// "dorm_rec_single_P[000]": 沏茶: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.55（同种效果取最高），如果目标是锡兰，则恢复效果额外+0.45
		// "dorm_rec_single_P[001]": 烤肉大师: 进驻宿舍时，使该宿舍内除自身以外心情未满的某个干员每小时恢复+0.55（同种效果取最高），如果目标是嘉维尔，则恢复效果额外+0.45
		// "dorm_rec_single_P[002]": 毒剂师之友: 进驻宿舍时，使该宿舍内心情未满的某个干员每小时恢复+0.55（同种效果取最高），如果目标是蓝毒，则恢复效果额外+0.45

		// ============================ HR Buffs ============================
		// "hire_spd&clue2[230]": 内幕: 进驻人力办公室时，人脉资源的联络速度+20%，每产生1次联络次数，增加黑钢国际线索的概率（工作时长和招募位影响概率）
		// "hire_spd&clue2[250]": 人望: 进驻人力办公室时，人脉资源的联络速度+20%，每产生1次联络次数，增加乌萨斯学生自治团线索的概率（工作时长和招募位影响概率）
		// "hire_spd&clue[010]": 天灾信使·α: 进驻人力办公室时，人脉资源的联络速度+30%
		// "hire_spd&clue[100]": 洞悉人心: 进驻人力办公室时，人脉资源的联络速度+35%，同时每个招募位（不包含初始招募位）额外+5%会客室线索搜集速度
		// "hire_spd&clue[101]": 好事之徒: 进驻人力办公室时，人脉资源的联络速度+35%，同时每个招募位（不包含初始招募位）额外+5%会客室线索搜集速度
		// "hire_spd[000]": 人事管理·α: 进驻人力办公室时，人脉资源的联络速度+20%
		// "hire_spd[010]": 天灾信使·α: 进驻人力办公室时，人脉资源的联络速度+30%
		// "hire_spd[011]": 天灾信使·β: 进驻人力办公室时，人脉资源的联络速度+45%
		// "hire_spd[020]": 心理学: 进驻人力办公室时，人脉资源的联络速度+40%
		// "hire_spd[030]": WRITER: 进驻人力办公室时，人脉资源的联络速度+40%
		// "hire_spd[031]": B-girl: 进驻人力办公室时，人脉资源的联络速度+40%
		// "hire_spd_bd_n1[000]": 追忆: 进驻人力办公室时，每1点记忆碎片转化为1点感知信息，心情耗尽时清空所有记忆碎片和自身累积的感知信息
		// "hire_spd_bd_n1_n1[100]": 巡游: 进驻人力办公室时，人脉资源的联络速度+20%，同时每个招募位（不包含初始招募位）+10点记忆碎片
		// "hire_spd_bd_n1_n1[200]": 救援队·灾后普查: 进驻人力办公室时，每个招募位（不包含初始招募位）+10点人间烟火
		// "hire_spd_blitz[000]": 语言学: 进驻人力办公室时，人脉资源的联络速度+20%，每1点情报储备额外+5%联络速度，每1瓶乌萨斯特饮额外+5%联络速度
		// "hire_spd_cost[010]": 天灾信使·α: 进驻人力办公室时，人脉资源的联络速度+30%
		// "hire_spd_cost[100]": 救援队·珠算: 进驻人力办公室时，人脉资源的联络速度+10%，心情每小时消耗-0.25
		// "hire_spd_cost[110]": 救援队·资源清点: 进驻人力办公室时，人脉资源的联络速度+20%，心情每小时消耗-0.25
		// "hire_spd_cost[200]": 准时下班: 进驻人力办公室时，人脉资源的联络速度+45%，心情每小时消耗+2

		// ============================ Meeting Room Buffs ============================
		// "meet_flag[010]": 哥伦比亚史: 进驻会客室后，每当新搜集到的线索不是莱茵生命时，则额外增加莱茵生命线索的出现概率（工作时长影响概率）
		// "meet_flag[040]": 耶拉冈德: 进驻会客室后，每当新搜集到的线索不是喀兰贸易时，则额外增加喀兰贸易线索的出现概率（工作时长影响概率）
		// "meet_flag[050]": 甄别: 进驻会客室后，每当新搜集到的线索不是乌萨斯学生自治团时，则额外增加乌萨斯学生自治团线索的出现概率（工作时长影响概率）
		// "meet_flag[060]": 插旗: 进驻会客室后，每当新搜集到的线索不是罗德岛制药时，则额外增加罗德岛制药线索的出现概率（工作时长影响概率）
		// "meet_spd&team[000]": 线索搜集·α: 进驻会客室时，线索搜集速度提升10%
		// "meet_spd&team[010]": 守望者: 进驻会客室时，线索搜集速度提升10%，且更容易获得莱茵生命线索（工作时长影响概率）
		// "meet_spd&team[020]": 信使·企鹅物流: 进驻会客室时，线索搜集速度提升10%，且更容易获得企鹅物流线索（工作时长影响概率）
		// "meet_spd&team[030]": 联络员: 进驻会客室时，线索搜集速度提升10%，且更容易获得黑钢国际线索（工作时长影响概率）
		// "meet_spd&team[031]": B.P.R.S: 进驻会客室时，线索搜集速度提升10%，且更容易获得黑钢国际线索（工作时长影响概率）
		// "meet_spd&team[040]": 讯使: 进驻会客室时，线索搜集速度提升10%，且更容易获得喀兰贸易线索（工作时长影响概率）
		// "meet_spd&team[041]": 雪境守望: 进驻会客室时，线索搜集速度提升10%，且更容易获得喀兰贸易线索（工作时长影响概率）
		// "meet_spd&team[050]": 军师: 进驻会客室时，线索搜集速度提升10%，且更容易获得乌萨斯学生自治团线索（工作时长影响概率）
		// "meet_spd&team[060]": 信使·罗德岛制药: 进驻会客室时，线索搜集速度提升10%，且更容易获得罗德岛制药线索（工作时长影响概率）
		// "meet_spd&team[070]": 传讯者: 进驻会客室时，线索搜集速度提升10%，且更容易获得格拉斯哥帮线索（工作时长影响概率）
		// "meet_spd&team[071]": 没落贵族: 进驻会客室时，线索搜集速度提升10%，且更容易获得格拉斯哥帮线索（工作时长影响概率）
		// "meet_spd&team[100]": 警司: 进驻会客室时，线索搜集速度提升25%
		// "meet_spd&team[110]": 星象学: 进驻会客室时，线索搜集速度提升25%
		// "meet_spd[000]": 线索搜集·α: 进驻会客室时，线索搜集速度提升10%
		// "meet_spd[0010]": 线索搜集·β: 进驻会客室时，线索搜集速度提升20%
		// "meet_spd[001]": 线索搜集·β: 进驻会客室时，线索搜集速度提升20%
		// "meet_spd[010]": 占卜: 进驻会客室时，线索搜集速度提升25%
		// "meet_spd[020]": 追踪者: 进驻会客室时，线索搜集速度提升25%
		// "meet_team[020]": 皇家探员（自称）: 进驻会客室时，更容易获得企鹅物流线索（工作时长影响概率）
		// "meet_team[050]": 秘密搜查: 进驻会客室时，更容易获得乌萨斯学生自治团线索（工作时长影响概率）
		// "meet_team[060]": 通讯员: 进驻会客室时，更容易获得罗德岛制药线索（工作时长影响概率）

		// ============================ Power plant Buffs ============================
		// "power_prod_spd_P[000]": “滴滴，启动！”: 进驻发电站时，野鬃所在的制造站生产力+5%
		// "power_rec_spd&cost[000]": 热情澎湃: 进驻发电站时，心情每小时消耗-0.52
		// "power_rec_spd[000]": 备用能源: 进驻发电站时，无人机充能速度+10%
		// "power_rec_spd[001]": 热能充能·α: 进驻发电站时，无人机每小时充能速度+10%
		// "power_rec_spd[002]": 光能充能·α: 进驻发电站时，无人机每小时充能速度+10%
		// "power_rec_spd[003]": 电磁充能·α: 进驻发电站时，无人机每小时充能速度+10%
		// "power_rec_spd[010]": 设备维护: 进驻发电站时，无人机每小时充能速度+15%
		// "power_rec_spd[011]": 清洁能源: 进驻发电站时，无人机每小时充能速度+15%
		// "power_rec_spd[013]": 高热充能: 进驻发电站时，无人机每小时充能速度+15%
		// "power_rec_spd[014]": 脉冲电弧·α: 进驻发电站时，无人机每小时充能速度+15%
		// "power_rec_spd[015]": 电磁充能·β: 进驻发电站时，无人机每小时充能速度+15%
		// "power_rec_spd[016]": 聚能: 进驻发电站时，无人机每小时充能速度+15%
		// "power_rec_spd[017]": 灯塔供能模块: 进驻发电站时，无人机每小时充能速度+15%
		// "power_rec_spd[020]": 静电场: 进驻发电站时，无人机每小时充能速度+20%
		// "power_rec_spd[021]": 脉冲电弧·β: 进驻发电站时，无人机每小时充能速度+20%
		// "power_rec_spd[022]": 热能充能·γ: 进驻发电站时，无人机每小时充能速度+20%

		// ============================ Training Room Buffs ============================
		// "train_cost&profession[140]": 工作狂: 进驻训练室协助狙击干员训练专精技能至1级时，心情每小时消耗+1
		// "train_cost&profession[340]": 索然无味: 进驻训练室协助狙击干员训练专精技能至3级时，心情每小时消耗+1
		// "train_cost&profession[360]": 变异: 进驻训练室协助辅助干员训练专精技能至3级时，心情每小时消耗+1
		// "train_spd&profession2[010]": 先锋专精·α: 进驻训练室协助位时，先锋干员的专精技能训练速度+30%
		// "train_spd&profession2[020]": 近卫专精·α: 进驻训练室协助位时，近卫干员的专精技能训练速度+30%
		// "train_spd&profession2[030]": 重装专精·α: 进驻训练室协助位时，重装干员的专精技能训练速度+30%
		// "train_spd&profession2[040]": 狙击专精·α: 进驻训练室协助位时，狙击干员的专精技能训练速度+30%
		// "train_spd&profession2[050]": 术师专精·α: 进驻训练室协助位时，术师干员的专精技能训练速度+30%
		// "train_spd&profession2[060]": 辅助专精·α: 进驻训练室协助位时，辅助干员的专精技能训练速度+30%
		// "train_spd&profession2[080]": 特种专精·α: 进驻训练室协助位时，特种干员的专精技能训练速度+30%
		// "train_spd&profession2[110]": 战术研习: 进驻训练室协助位时，先锋干员的专精技能训练速度+30%，如果本次训练专精技能至1级，则训练速度额外+45%
		// "train_spd&profession2[120]": 信影流: 进驻训练室协助位时，近卫干员的专精技能训练速度+30%，如果本次训练专精技能至1级，则训练速度额外+45%
		// "train_spd&profession2[130]": 极地生存: 进驻训练室协助位时，重装干员的专精技能训练速度+30%，如果本次训练专精技能至1级，则训练速度额外+45%
		// "train_spd&profession2[150]": 一知半解: 进驻训练室协助位时，术师干员的专精技能训练速度+30%，如果本次训练专精技能至1级，则训练速度额外+45%
		// "train_spd&profession2[180]": 陷阱对焦: 进驻训练室协助位时，特种干员的专精技能训练速度+30%，如果本次训练专精技能至1级，则训练速度额外+45%
		// "train_spd&profession2[220]": 苦行: 进驻训练室协助位时，近卫干员的专精技能训练速度+30%，如果本次训练专精技能至2级，则训练速度额外+45%
		// "train_spd&profession2[230]": 阵地经验: 进驻训练室协助位时，重装干员的专精技能训练速度+30%，如果本次训练专精技能至2级，则训练速度额外+45%
		// "train_spd&profession2[250]": 博览群书: 进驻训练室协助位时，术师干员的专精技能训练速度+30%，如果本次训练专精技能至2级，则训练速度额外+45%
		// "train_spd&profession2[320]": 尽心尽力: 进驻训练室协助位时，近卫干员的专精技能训练速度+30%，如果本次训练专精技能至3级，则训练速度额外+45%
		// "train_spd&profession2[440]": 以身作则: 进驻训练室协助位时，狙击干员的专精技能训练速度+30%，如果本次训练专精技能至1级，则训练速度额外+65%
		// "train_spd&profession2[640]": 伺机而动: 进驻训练室协助位时，狙击干员的专精技能训练速度+30%，如果本次训练专精技能至3级，则训练速度额外+65%
		// "train_spd&profession2[660]": 同化: 进驻训练室协助位时，辅助干员的专精技能训练速度+30%，如果本次训练专精技能至3级，则训练速度额外+65%
		// "train_spd&profession3[130]": 实战技巧：驭法铁卫: 进驻训练室协助位时，重装干员的专精技能训练速度+30%，如果训练位干员分支为驭法铁卫，则训练速度额外+45%
		// "train_spd&profession3[140]": 实战技巧：速射: 进驻训练室协助位时，狙击干员的专精技能训练速度+30%，如果训练位干员分支为速射手，则训练速度额外+45%
		// "train_spd&profession[010]": 先锋专精·α: 进驻训练室协助位时，先锋干员的专精技能训练速度+30%
		// "train_spd&profession[011]": 先锋专精·β: 进驻训练室协助位时，先锋干员的专精技能训练速度+50%
		// "train_spd&profession[020]": 近卫专精·α: 进驻训练室协助位时，近卫干员的专精技能训练速度+30%
		// "train_spd&profession[021]": 近卫专精·β: 进驻训练室协助位时，近卫干员的专精技能训练速度+50%
		// "train_spd&profession[030]": 重装专精·α: 进驻训练室协助位时，重装干员的专精技能训练速度+30%
		// "train_spd&profession[031]": 重装专精·β: 进驻训练室协助位时，重装干员的专精技能训练速度+50%
		// "train_spd&profession[040]": 狙击专精·α: 进驻训练室协助位时，狙击干员的专精技能训练速度+30%
		// "train_spd&profession[041]": 狙击专精·β: 进驻训练室协助位时，狙击干员的专精技能训练速度+50%
		// "train_spd&profession[050]": 术师专精·α: 进驻训练室协助位时，术师干员的专精技能训练速度+30%
		// "train_spd&profession[051]": 术师专精·β: 进驻训练室协助位时，术师干员的专精技能训练速度+50%
		// "train_spd&profession[060]": 辅助专精·α: 进驻训练室协助位时，辅助干员的专精技能训练速度+30%
		// "train_spd&profession[061]": 辅助专精·β: 进驻训练室协助位时，辅助干员的专精技能训练速度+50%
		// "train_spd&profession[070]": 医疗专精·α: 进驻训练室协助位时，医疗干员的专精技能训练速度+30%
		// "train_spd&profession[071]": 医疗专精·β: 进驻训练室协助位时，医疗干员的专精技能训练速度+50%
		// "train_spd&profession[080]": 特种专精·α: 进驻训练室协助位时，特种干员的专精技能训练速度+30%
		// "train_spd&profession[081]": 特种专精·β: 进驻训练室协助位时，特种干员的专精技能训练速度+50%
		// "train_spd&profession[110]": 入世: 进驻训练室协助位时，先锋干员的专精技能训练速度+60%
		// "train_spd&profession[120]": 剑术记忆: 进驻训练室协助位时，近卫干员的专精技能训练速度+60%
		// "train_spd&profession[130]": 般若: 进驻训练室协助位时，重装干员的专精技能训练速度+60%
		// "train_spd&profession[140]": 黑矢: 进驻训练室协助位时，狙击干员的专精技能训练速度+60%
		// "train_spd&profession[150]": 尽在掌握: 进驻训练室协助位时，术师干员的专精技能训练速度+60%
		// "train_spd&profession[160]": 共鸣: 进驻训练室协助位时，辅助干员的专精技能训练速度+60%
		// "train_spd&profession[170]": 精准手术: 进驻训练室协助位时，医疗干员的专精技能训练速度+60%
		// "train_spd&profession[180]": 假面魅影: 进驻训练室协助位时，特种干员的专精技能训练速度+60%
		// "train_spd[0000]": 教官: 进驻训练室协助位时，干员的专精技能训练速度+25%
		// "train_spd[000]": 教官: 进驻训练室协助位时，干员的专精技能训练速度+25%
		// "train_spd_tag[000]": 骑士训练: 进驻训练室协助位时，基建内（不包含副手）每名骑士干员为当前干员的专精技能训练速度+5%（最多生效5名）
		// "train_spd_tag[010]": 崇高准则: 进驻训练室协助位时，基建内（不包含副手）每名骑士干员为当前干员的专精技能训练速度+5%（最多生效8名）

		// ============================ Workshop Buffs ============================
		// "workshop_formula_bonus1[000]": 因果: 进驻加工站时，累积40点因果必定产出一次副产品
		// "workshop_formula_bonus2[000]": 业报: 进驻加工站时，累积80点业报必定产出一次副产品
		// "workshop_formula_cost2[000]": 热心修补匠: 进驻加工站加工精英材料时，副产品的产出概率提升40%，同时心情消耗为8以上的配方全部-4心情消耗
		// "workshop_formula_cost3[000]": 放空: 进驻加工站加工技巧概要时，心情消耗为2的配方全部-1心情消耗
		// "workshop_formula_cost3[100]": 一丝不苟: 进驻加工站加工精英材料时，心情消耗为2的配方全部-1心情消耗
		// "workshop_formula_cost3[110]": 灵感: 进驻加工站加工精英材料时，心情消耗为4的配方全部-1心情消耗
		// "workshop_formula_cost3[111]": 熔铸: 进驻加工站加工精英材料时，心情消耗为4的配方全部-1心情消耗
		// "workshop_formula_cost3[120]": 极简实用主义: 进驻加工站加工精英材料时，心情消耗为4的配方全部-2心情消耗
		// "workshop_formula_cost3[200]": 便携蓄电池: 进驻加工站加工基建材料时，心情消耗为2的配方全部-1心情消耗
		// "workshop_formula_cost3[220]": 选矿学: 进驻加工站加工基建材料时，心情消耗为4的配方全部-2心情消耗
		// "workshop_formula_cost3[300]": 淡泊: 进驻加工站加工芯片时，心情消耗为2的配方全部-1心情消耗
		// "workshop_formula_cost4[000]": 执著: 进驻加工站加工精英材料时，相应配方的心情消耗恒定为2
		// "workshop_formula_cost[000]": 龙腾式无人机: 进驻加工站加工任意类材料时，心情消耗为4以上的配方全部-2心情消耗
		// "workshop_formula_cost[010]": 肆无忌惮: 进驻加工站加工精英材料时，相应配方全部+2心情消耗
		// "workshop_formula_device[000]": DIY·装置: 进驻加工站加工装置类材料时，副产品的产出概率提升90%
		// "workshop_formula_device[020]": DIY·异铁: 进驻加工站加工异铁类材料时，副产品的产出概率提升90%
		// "workshop_formula_device[030]": DIY·聚酸酯: 进驻加工站加工聚酸酯类材料时，副产品的产出概率提升90%
		// "workshop_formula_device[050]": DIY·源岩: 进驻加工站加工源岩类材料时，副产品的产出概率提升90%
		// "workshop_formula_dorm[000]": “物尽其用”: 进驻加工站加工精英材料时，宿舍内每有1名心情4以下的干员，副产品的产出概率提升5%
		// "workshop_formula_dorm[001]": “人善其事”: 进驻加工站加工精英材料时，宿舍内每有1名心情20以下的干员，副产品的产出概率提升5%
		// "workshop_formula_drop[020]": 缓蚀技术: 进驻加工站加工精英材料产出T3品质的副产品时，副产品必定为异铁组
		// "workshop_formula_free[000]": 精打细算: 进驻加工站加工精英材料时，减免龙门币消耗
		// "workshop_formula_frost[000]": 机械工程: 进驻加工站加工任意类材料时，副产品的产出概率提升50%，每1点情报储备额外提升5%；如果乌萨斯特饮达到4瓶，则额外提升15%
		// "workshop_formula_probability[000]": 技巧理论: 进驻加工站加工技巧概要时，副产品的产出概率提升70%
		// "workshop_formula_probability[010]": 兵者诡道: 进驻加工站加工技巧概要时，副产品的产出概率提升80%
		// "workshop_formula_probability[020]": 登峰造极: 进驻加工站加工技巧概要时，副产品的产出概率提升80%
		// "workshop_formula_probability[030]": 适应力: 进驻加工站加工技巧概要时，副产品的产出概率提升75%
		// "workshop_formula_probability[100]": 营养学: 进驻加工站加工精英材料时，副产品的产出概率提升70%
		// "workshop_formula_probability[110]": 药理学·α: 进驻加工站加工精英材料时，副产品的产出概率提升75%
		// "workshop_formula_probability[111]": 毒理学·α: 进驻加工站加工精英材料时，副产品的产出概率提升75%
		// "workshop_formula_probability[112]": 爆破材料学·α: 进驻加工站加工精英材料时，副产品的产出概率提升75%
		// "workshop_formula_probability[113]": 腐蚀科学·α: 进驻加工站加工精英材料时，副产品的产出概率提升75%
		// "workshop_formula_probability[114]": 草药学·α: 进驻加工站加工精英材料时，副产品的产出概率提升75%
		// "workshop_formula_probability[119]": 工业设计: 进驻加工站加工精英材料时，副产品的产出概率提升75%
		// "workshop_formula_probability[120]": 药理学·β: 进驻加工站加工精英材料时，副产品的产出概率提升80%
		// "workshop_formula_probability[121]": 毒理学·β: 进驻加工站加工精英材料时，副产品的产出概率提升80%
		// "workshop_formula_probability[122]": 爆破材料学·β: 进驻加工站加工精英材料时，副产品的产出概率提升80%
		// "workshop_formula_probability[123]": 腐蚀科学·β: 进驻加工站加工精英材料时，副产品的产出概率提升80%
		// "workshop_formula_probability[124]": 草药学·β: 进驻加工站加工精英材料时，副产品的产出概率提升80%
		// "workshop_formula_probability[129]": 学者: 进驻加工站加工精英材料时，副产品的产出概率提升80%
		// "workshop_formula_probability[160]": 稀有金属辨识: 进驻加工站加工精英材料时，副产品的产出概率提升100%
		// "workshop_formula_probability[200]": 工程学: 进驻加工站加工基建材料时，副产品的产出概率提升70%
		// "workshop_formula_probability[210]": 斩铁裂钢: 进驻加工站加工基建材料时，副产品的产出概率提升80%
		// "workshop_formula_probability[220]": 结构力学: 进驻加工站加工基建材料时，副产品的产出概率提升75%
		// "workshop_formula_probability[300]": 特训记录: 进驻加工站加工芯片时，副产品的产出概率提升70%
		// "workshop_formula_probability[310]": 训练有素: 进驻加工站加工芯片时，副产品的产出概率提升80%
		// "workshop_formula_recovery[000]": “爆炸艺术”: 进驻加工站加工任意类材料时，每2次加工没有产出副产品，则恢复自身一次心情，恢复量为对应配方所需消耗的心情（加工结束后统一结算）
		// "workshop_proc_probability[000]": 专注·α: 进驻加工站加工任意类材料时，副产品的产出概率提升40%
		// "workshop_proc_probability[010]": 能工巧匠: 进驻加工站加工任意类材料时，副产品的产出概率提升50%
		// "workshop_proc_probability[011]": 多功能测绘仪: 进驻加工站加工任意类材料时，副产品的产出概率提升50%
		// "workshop_proc_probability[012]": “炼金术”: 进驻加工站加工任意类材料时，副产品的产出概率提升50%
		// "workshop_proc_probability[020]": 专注·β: 进驻加工站加工任意类材料时，副产品的产出概率提升60%
		// "workshop_proc_probability[021]": 老当益壮: 进驻加工站加工任意类材料时，副产品的产出概率提升60%
		// "workshop_proc_probability[030]": 咪波·加工型: 进驻加工站加工任意类材料时，副产品的产出概率提升65%
		// "workshop_proc_probability[040]": 未知技术: 进驻加工站加工任意类材料时，副产品的产出概率提升70%
	}
}

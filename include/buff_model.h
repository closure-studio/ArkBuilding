#pragma once

#include <cstdarg>
#include <algorithm>
#include "attributes_util.h"
#include "bit_ops.h"
#include "building_data_model.h"
#include "log_util.h"
#include "primitive_types.h"
#include "util.h"
#include "mem_util.h"
#include "buff_primitives.h"

namespace albc
{
	/**
	 * @brief 描述Buff的作用范围类型, 即Buff效果由哪些因素决定
	 */
	enum class ModifierScopeType : unsigned char
	{
		INDEPENDENT,				//效果始终不变, 构造实例时初始化
		DEPEND_ON_ROOM, //效果受全局变量影响, 进入新房间时初始化
		DEPEND_ON_OTHER_CHAR		//效果受同一房间中角色影响, 每次计算时初始化
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
		RoomBuff *owner;					  //指向房间buff
		//bool enabled{};						  //是否生效
		ModifierScope scope;				  //作用范围
		RoomAttributeModifier room_mod;		  //房间属性修改
		RoomFinalAttributeModifier final_mod; //房间最终属性修改
		CharacterCostModifier cost_mod;		  //角色心情消耗修改

		ModifierApplier(RoomBuff *owner) : owner(owner)
		{
		}

		[[nodiscard]] bool NeedUpdateScope(const ModifierScopeData &data) const;

		void UpdateScope(const ModifierScopeData &data);
	};

    /**
     * @brief 房间buff
     */
	class RoomBuff
	{
	public:
		int owner_inst_id = 0;							  //拥有该Buff的干员的实例Id
		string owner_char_id{};							  //拥有该Buff的角色Id
		string buff_id{};								  // Buff的Id
		RoomBuffType inner_type{RoomBuffType::UNDEFINED}; //内部类型
		Vector<RoomBuffTargetValidator *> validators;	  //作用范围验证器
		bm::RoomType room_type{bm::RoomType::NONE};		  //作用房间类型
		int sort_id = 0;								  //排序id
		double duration = 86400;						  //持续时间，由干员的心情决定
		ModifierApplier applier;

		RoomBuff() : applier(this) {}

		explicit RoomBuff(bm::RoomType room_type, RoomBuffType inner_type)
			: inner_type(inner_type),
			  room_type(room_type),
			  applier(this)
		{
		}

		virtual ~RoomBuff() = default;

		RoomBuff(const RoomBuff &src) = default;
		RoomBuff &operator=(const RoomBuff &rhs) = default;
		RoomBuff(RoomBuff &&src) noexcept = default;
		RoomBuff &operator=(RoomBuff &&src) noexcept = default;

		virtual RoomBuff *Clone() = 0;

		virtual bool ValidateTarget(const RoomModel *room)
		{
			return std::all_of(validators.begin(), validators.end(), [room](RoomBuffTargetValidator *validator) -> bool
							   { return validator->validate(room); });
		}

		virtual void OnScopeUpdate(const ModifierScopeData &data) {}

		virtual RoomBuff *AddValidator(RoomBuffTargetValidator *validator){
			validators.push_back(validator);
			return this;
		}
	};

    /**
     * @brief 依据模板类型参数复制Buff, 实现虚函数Clone()
     */
	template <typename TDerived>
	class CloneableRoomBuff : public RoomBuff
	{
	public:
		// inherit constructor
		using RoomBuff::RoomBuff;

        RoomBuff *Clone() final
		{
			return mem::aligned_new<TDerived>(static_cast<const TDerived&>(*this));
		}
	};

    /**
     * @brief 最基础的加成，直接增减房间属性
     */
	class BasicInc : public CloneableRoomBuff<BasicInc>
	{
	public:
		BasicInc(const double eff_delta, const int cap_delta, const bm::RoomType room_type_1,
				 const RoomBuffType inner_type) : CloneableRoomBuff(room_type_1, inner_type) // 指定房间的效率, 房间容量, 内部类型(用于处理某些同类buff互斥)
		{
			RoomAttributeModifier::init(applier.room_mod, this, inner_type,
										eff_delta, cap_delta); // 初始化房间属性修改器
		}

		BasicInc(const double eff_delta, const int cap_delta, const double cost_mod,
				 const CharCostModifierType cost_mod_type, const bm::RoomType room_type_1,
				 const RoomBuffType inner_type) : CloneableRoomBuff(room_type_1, inner_type)
		{
			RoomAttributeModifier::init(applier.room_mod, this, inner_type,
										eff_delta, cap_delta);
			CharacterCostModifier::init(applier.cost_mod, this, cost_mod_type,
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
		IncEffOverTime(const double base_eff_delta, const double eff_inc_per_hour,
					   const double max_extra_eff_inc,
					   const bm::RoomType room_type_1, const RoomBuffType inner_type)
			: CloneableRoomBuff(room_type_1, inner_type),
			  base_eff_delta_(base_eff_delta),
			  eff_delta_per_hour_(eff_inc_per_hour),
			  max_extra_eff_inc_(max_extra_eff_inc)
		{
			RoomAttributeModifier::init(applier.room_mod, this, inner_type,
										base_eff_delta_, 0, eff_delta_per_hour_,
										max_extra_eff_inc_);
		}

	protected:
		double base_eff_delta_;
		double eff_delta_per_hour_;
		double max_extra_eff_inc_;
	};

    /**
     * @brief 自动化: 由发电站数量增加房间属性
     */
	class IncEffByPowerPlantCnt : public CloneableRoomBuff<IncEffByPowerPlantCnt>
	{
	public:
		IncEffByPowerPlantCnt(const double addition_per_power_plant,
							  const bool clear_others) : CloneableRoomBuff(bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_POWER_PLANT),
														 addition_per_power_plant_(addition_per_power_plant),
														 clear_others_(clear_others)
		{
			applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
		}

		void OnScopeUpdate(const ModifierScopeData &data) override
		{
			const int power_plant_count = read_attribute_as_int(data.room->global_attributes,
																GlobalAttributeType::POWER_PLANT_CNT);

			if (clear_others_)
			{
				RoomFinalAttributeModifier::init(
					applier.final_mod,
					this,
					inner_type,
					1 + power_plant_count * addition_per_power_plant_,
					0,
					0,
					INFINITY,
					RoomFinalAttributeModifierType::OVERRIDE_AND_CANCEL_ALL,
					0);
			}
			else
			{
				RoomAttributeModifier::init(
					applier.room_mod,
					this,
					inner_type,
					1 + power_plant_count * addition_per_power_plant_);
			}
		}

	protected:
		double addition_per_power_plant_;
		bool clear_others_;
	};

    /**
     * @brief 根据房间内其他干员的效率增加，提供效率增加
     */
	class IncEffByOtherEffInc : public CloneableRoomBuff<IncEffByOtherEffInc>
	{
	public:
		IncEffByOtherEffInc(const double unit_addition, const double unit_factor,
							const double max_extra_addition, const bm::RoomType room_type_1,
							const RoomBuffType inner_type)
			: CloneableRoomBuff(room_type_1, inner_type)
		{
			RoomFinalAttributeModifier::init(
				applier.final_mod, this, inner_type, 0, 0, 0, max_extra_addition_,
				RoomFinalAttributeModifierType::ADDITIONAL, 2);
		}

	protected:
		double max_extra_addition_;
	};

    /**
     * @brief 根据房间内其他干员的容量增加，提供效率增加
     */
	class IncEffByOtherCapInc : public CloneableRoomBuff<IncEffByOtherCapInc>
	{
	public:
		IncEffByOtherCapInc(const int threshold, const double below_addition_per_limit,
							const double above_addition_per_limit) : CloneableRoomBuff(bm::RoomType::MANUFACTURE, RoomBuffType::FACTORY_INC_EFF_BY_CAP_ADDITION),
																	 threshold_(threshold),
																	 below_addition_(below_addition_per_limit),
																	 above_addition_(above_addition_per_limit)
		{
			applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
		}

		void OnScopeUpdate(const ModifierScopeData &data) override
		{
			double eff_delta = 0;
			UNROLL_LOOP(kRoomMaxBuffSlots)
			for (UInt32 i = 0; i < data.room->n_buff; ++i)
			{
				const auto &applier = data.room->buffs[i]->applier;
				int cap_delta;
				if (!applier.room_mod.IsValid() || (cap_delta = applier.room_mod.cap_delta) <= 0)
					continue;

				eff_delta += cap_delta >= threshold_ ? cap_delta * above_addition_ : cap_delta * below_addition_;
			}
			RoomAttributeModifier::init(applier.room_mod, this, inner_type, eff_delta);
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
		inline IncEffByGlobalAttribute(const double addition_per_unit,
									   const GlobalAttributeType global_attribute_type,
									   const bm::RoomType room_type_1,
									   const RoomBuffType inner_type) : CloneableRoomBuff(room_type_1, inner_type),
																		addition_per_unit_(addition_per_unit),
																		global_attribute_type_(global_attribute_type)
		{
			applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
		}

		void OnScopeUpdate(const ModifierScopeData &data) override
		{
			RoomAttributeModifier::init(
				applier.room_mod,
				this,
				inner_type,
				addition_per_unit_ * read_attribute(data.room->global_attributes, global_attribute_type_));
		}

	protected:
		double addition_per_unit_;
		GlobalAttributeType global_attribute_type_;
	};

    /**
     * @brief 根据标准化数量增加房间属性
     */
	class IncEffByStandardizationCnt : public CloneableRoomBuff<IncEffByStandardizationCnt>
	{
	public:
		inline IncEffByStandardizationCnt(const double addition_per_unit,
										  const bm::RoomType room_type_1,
										  const RoomBuffType inner_type) : CloneableRoomBuff(room_type_1, inner_type),
																			addition_per_unit_(addition_per_unit)
		{
			applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
		}

		void OnScopeUpdate(const ModifierScopeData &data) override
		{
			double addition = 0.;

			UNROLL_LOOP(kRoomMaxBuffSlots)
			for (UInt32 i = 0; i < data.room->n_buff; ++i)
			{
				if (data.room->buffs[i]->inner_type == RoomBuffType::FACTORY_STANDARDIZATION)
				{
					addition += addition_per_unit_;
				}
			}
			
			RoomAttributeModifier::init(
				applier.room_mod,
				this,
				inner_type,
				addition);
		}

	protected:
		double addition_per_unit_;

	};

	/**
	 * @brief 低语: 进驻贸易站时，当前贸易站内其他干员提供的订单获取效率全部归零，且每人为自身+45%订单获取效率，同时全体心情每小时消耗+0.25
	 */
	class VodfoxTradeBuff : public CloneableRoomBuff<VodfoxTradeBuff>
	{
	public:
		inline VodfoxTradeBuff() 
			: CloneableRoomBuff(bm::RoomType::TRADING, RoomBuffType::TRADING_WHISPERS)
		{
			applier.scope.type = ModifierScopeType::DEPEND_ON_ROOM;
		}

		void OnScopeUpdate(const ModifierScopeData &data) override
		{
			RoomFinalAttributeModifier::init(
				applier.final_mod,
				this,
				inner_type,
				(data.room->max_slot_count - 1) * 0.45,
				0.,
				0.,
				INFINITY,
				RoomFinalAttributeModifierType::OVERRIDE_AND_CANCEL_ALL);

			CharacterCostModifier::init(
				applier.cost_mod,
				this,
				CharCostModifierType::ROOM_ALL,
				0.25);
		}
	};

	/**
	 * @brief trade_ord_limit_diff: 摊贩经济: 进驻贸易站时，当前订单数与订单上限每差1笔订单，则订单获取效率+4%
	 */
	class TradeOrdLimitDiffBuff : public CloneableRoomBuff<TradeOrdLimitDiffBuff>
	{
	public:
		inline TradeOrdLimitDiffBuff() 
			: CloneableRoomBuff(bm::RoomType::TRADING, RoomBuffType::TRADING_STREET_ECO)
		{
			applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
		}

		void OnScopeUpdate(const ModifierScopeData &data) override
		{
			double additional = data.room->room_attributes.base_prop_cap * 0.04;

			// 考虑其他干员带来的订单容量加成
			UNROLL_LOOP(kRoomMaxBuffSlots)
			for (UInt32 i = 0; i < data.room->n_buff; ++i)
			{
				const auto &applier = data.room->buffs[i]->applier;
				if (!applier.room_mod.IsValid())
					continue;

				additional += applier.room_mod.cap_delta * 0.04;
			}
			//LOG_D << "TradeOrdLimitDiffBuff additional: " << additional << std::endl;

			RoomAttributeModifier::init(
				applier.room_mod,
				this,
				inner_type,
				additional);
		}
	};

	/**
	 * @brief "trade_ord_limit_count[000]": 市井之道: 进驻贸易站时，当前贸易站内其他干员提供的每10%订单获取效率使订单上限-1（订单最少为1），同时每有1笔订单就+4%订单获取效率
	 */
	class TradeOrdLimitCountBuff : public CloneableRoomBuff<TradeOrdLimitCountBuff>
	{
	public:
		inline TradeOrdLimitCountBuff() 
			: CloneableRoomBuff(bm::RoomType::TRADING, RoomBuffType::TRADING_BASIC_NEEDS)
		{
			applier.scope.type = ModifierScopeType::DEPEND_ON_OTHER_CHAR;
		}

		void OnScopeUpdate(const ModifierScopeData &data) override
		{
			
		}
	};

	inline bool
	ModifierApplier::NeedUpdateScope(const ModifierScopeData &data) const
	{
		switch (this->scope.type)
		{
		case ModifierScopeType::INDEPENDENT:
			return false;

		case ModifierScopeType::DEPEND_ON_ROOM:
			return this->scope.data.room != data.room;

		case ModifierScopeType::DEPEND_ON_OTHER_CHAR:
			return true;

		default: // should not reach here
			assert(false);
			return false;
		}
	}

	inline void
    ModifierApplier::UpdateScope(const ModifierScopeData &data)
	{
		this->scope.data = data;
		owner->OnScopeUpdate(data);
	};
}

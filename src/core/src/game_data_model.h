#pragma once
#include "json_util.h"

namespace albc::data
{
	enum class EvolvePhase
	{
		PHASE_0 = 0,
		PHASE_1 = 1,
		PHASE_2 = 2,
		PHASE_3 = 3
	};


	struct UnlockCondition
	{
		EvolvePhase phase = EvolvePhase::PHASE_0;
		int level = 0;

		explicit UnlockCondition(const Json::Value& json) :
			phase(util::json_val_as_enum<EvolvePhase>(json["phase"])),
			level(json["level"].asInt())
		{ }

        UnlockCondition() = default;

		[[nodiscard]] constexpr bool Check(const EvolvePhase phase_val, const int level_val) const
		{
			return phase_val >= this->phase && level_val >= this->level;
		}
	};
}
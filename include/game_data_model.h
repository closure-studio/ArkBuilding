#pragma once
#include "json_util.h"

namespace albc
{
	enum class EvolvePhase
	{
		PHASE_0 = 0,
		PHASE_1 = 1,
		PHASE_2 = 2,
		PHASE_3 = 3,
		E_NUM,
	};


	struct UnlockCondition
	{
		EvolvePhase phase;
		int level;

		explicit UnlockCondition(const Json::Value& json) :
			phase(json_val_as_enum<EvolvePhase>(json["phase"])),
			level(json["level"].asInt())
		{ }

		[[nodiscard]] constexpr bool Check(const EvolvePhase phase, const int level) const
		{
			return phase >= this->phase && level >= this->level;
		}
	};
}
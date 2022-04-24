#pragma once
#include "util_json.h"

namespace albc::data
{
	enum class EvolvePhase
	{
        NONE = -1,
        PHASE_0 = 0,
		PHASE_1 = 1,
		PHASE_2 = 2,
		PHASE_3 = 3,
	};


	struct UnlockCondition
	{
		EvolvePhase phase = EvolvePhase::PHASE_0;
		int level = 0;

		explicit UnlockCondition(const Json::Value& json);

        UnlockCondition() = default;

		[[nodiscard]] constexpr bool Check(EvolvePhase phase_val, int level_val) const
        {
            return phase_val >= this->phase && level_val >= this->level;
        }
	};
}
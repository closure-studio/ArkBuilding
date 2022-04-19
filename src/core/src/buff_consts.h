// buff consts
#pragma once
#include <cstddef>

namespace albc::model::buff
{
static constexpr size_t kRoomMaxBuffSlots = 10;
static constexpr size_t kRoomMaxOperators = 5;
static constexpr size_t kOperatorMaxBuffs = 4;
static constexpr size_t kAlgOperatorSize = 512;
static constexpr size_t kFuncPiecewiseMaxSegmentCount = 5;

static constexpr double kAlgDefaultDuration = 3600 * 16;
} // namespace albc
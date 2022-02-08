#pragma once
#include "primitive_types.h"
#include "buff_model.h"
#include "building_data_model.h"

namespace albc
{
    class OperatorModel // 表明一个干员, 包括其属性、buff
	{
	public:
		int inst_id;
		string char_id;
		bm::RoomType room_type_mask; // 可以放置的房间类型, 位掩码
		Vector<RoomBuff*> buffs; // 所有buff

		OperatorModel(const int inst_id, string& char_id, const bm::RoomType room_type_mask) :
			inst_id(inst_id),
			char_id(std::move(char_id)),
			room_type_mask(room_type_mask)
		{
		}
	};
}
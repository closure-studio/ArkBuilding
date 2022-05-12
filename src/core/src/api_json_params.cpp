//
// Created by Nonary on 2022/4/24.
//
#include "api_json_params.h"
#include "data_building.h"
#include "model_buff_primitives.h"

namespace albc::api
{
JsonInCharStruct::JsonInCharStruct(const Json::Value &val)
    : name(val.get(kName, "").asString()), id(val.get(kCharId, "").asString()),
      phase(val.get(kPhase, -1).asInt()),
      level(val.get(kLevel, -1).asInt()),
      morale(val.get(kMorale, 24).asDouble()),
      skills(util::json_val_as_vector<std::string>(
          val.get(kSkills, Json::Value(Json::arrayValue)),
          util::json_cast<std::string>))
{
}
JsonInRoomAttributeFields::JsonInRoomAttributeFields(const Json::Value &val)
    : prod_cnt(val.get(kProdCnt, 0).asInt()),
      base_prod_cap(val.get(kBaseProdCap, 10).asInt()),
      base_char_cost(val.get(kBaseCharCost, 1).asDouble()),
      base_prod_eff(val.get(kBaseProdEff, 1).asDouble())
{
}
JsonInRoomStruct::JsonInRoomStruct(const Json::Value &val)
    : type((data::building::RoomType)
               util::parse_enum_string(
                   val.get(kType, "").asString(),
                   JsonRoomType::NONE)),
      slot_count(val.get(kSlotCount, 0).asInt()),
      attributes(val.get(kAttributes, Json::Value(Json::objectValue))),
      level(val.get(kLevel, 1).asInt())
{
    const auto shared_prod_type_str = val.get(kSharedProdType, "").asString();
    switch (type)
    {
    case data::building::RoomType::MANUFACTURE:
        prod_type =
            (model::buff::ProdType) util::parse_enum_string(shared_prod_type_str,
                JsonManufactureProdType::NONE);
        break;

    case data::building::RoomType::TRADING:
        order_type =
            (model::buff::OrderType) util::parse_enum_string(shared_prod_type_str,
                JsonTradingOrderType::NONE);
        break;

    case data::building::RoomType::POWER:
    case data::building::RoomType::DORMITORY:
        break;

    default:
        LOG_E("Unsupported room type: ", util::enum_to_string(type),
              ", Raw Json Value: \n", val.toStyledString());
        break;
    }
}
JsonInParams::JsonInParams(const Json::Value &val)
    : model_time_limit(val.get(kModelTimeLimit, 3600 * 16).asInt()),
      solve_time_limit(val.get(kSolveTimeLimit, 60).asInt()),
      gen_sol_details(val.get(kGenSolDetails, false).asBool()),
      gen_lp_file(val.get(kGenLpFile, false).asBool()),
      chars(util::json_val_as_dictionary<JsonInCharStruct>(
          val.get(kChars, Json::Value(Json::objectValue)))),
      rooms(util::json_val_as_dictionary<JsonInRoomStruct>(
          val.get(kRooms, Json::Value(Json::objectValue))))
{
}
JsonOutRoomStruct::operator Json::Value() const
{
    Json::Value val;
    val[kScore] = score;
    val[kDuration] = duration;
    val[kChars] = util::json_val_from_vector<std::string>(chars);
    return val;
}
JsonOutParams::operator Json::Value() const
{
    Json::Value val;
    val[kRooms] = util::json_val_from_dictionary<JsonOutRoomStruct>(rooms, util::to_json_cast<JsonOutRoomStruct>);
    val[kErrors] = static_cast<Json::Value>(errors);
    return val;
}
JsonOutErrorStruct::operator Json::Value() const
{
    Json::Value val;
    val[kChars] = util::json_val_from_dictionary<std::string>(chars);
    val[kRooms] = util::json_val_from_dictionary<std::string>(rooms);
    val[kError] = util::json_val_from_vector<std::string>(errors);
    return val;
}
}
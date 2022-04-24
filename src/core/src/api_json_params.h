#pragma once
#include "util_json.h"
#include "data_building.h"
#include "model_buff_primitives.h"
#define ALBC_API_JSON_KEY(name, key) constexpr static const char* name = key;
namespace albc::api
{
enum class JsonRoomType;
}

template <>
struct magic_enum::customize::enum_range<albc::api::JsonRoomType>
{
    static constexpr bool is_flags = true;
    static constexpr int min = 0;
    static constexpr int max = 4095;
};

namespace albc::api
{

enum class JsonRoomType
{
    NONE = (int)data::building::RoomType::NONE,
    MANUFACTURE = (int)data::building::RoomType::MANUFACTURE,
    TRADING = (int)data::building::RoomType::TRADING,
};

enum class JsonManufactureProdType
{
    NONE = (int)model::buff::ProdType::UNDEFINED,
    GOLD = (int)model::buff::ProdType::GOLD,
    SHARD = (int)model::buff::ProdType::ORIGINIUM_SHARD,
    RECORD = (int)model::buff::ProdType::RECORD,
    CHIP = (int)model::buff::ProdType::CHIP,
};

enum class JsonTradingOrderType
{
    NONE = (int)model::buff::OrderType::UNDEFINED,
    GOLD = (int)model::buff::OrderType::GOLD,
    SHARD = (int)model::buff::OrderType::ORUNDUM,
};

struct JsonInCharStruct
{
    std::string name;           ALBC_API_JSON_KEY(kName,    "name");
    std::string id;             ALBC_API_JSON_KEY(kCharId,  "id");
    int phase;                  ALBC_API_JSON_KEY(kPhase,   "phase");
    int level;                  ALBC_API_JSON_KEY(kLevel,   "level");
    double morale;              ALBC_API_JSON_KEY(kMorale,  "morale");
    Vector<std::string> skills; ALBC_API_JSON_KEY(kSkills,  "skills");

    explicit JsonInCharStruct(const Json::Value& val);
};

struct JsonInRoomAttributeFields
{
    int prod_cnt;               ALBC_API_JSON_KEY(kProdCnt, "prodCnt");
    int base_prod_cap;          ALBC_API_JSON_KEY(kBaseProdCap, "baseProdCap");
    double base_char_cost;      ALBC_API_JSON_KEY(kBaseCharCost, "baseCharCost");
    double base_prod_eff;       ALBC_API_JSON_KEY(kBaseProdEff, "baseProdEff");

    explicit JsonInRoomAttributeFields(const Json::Value& val);
};

struct JsonInRoomStruct
{
    data::building::RoomType type;          ALBC_API_JSON_KEY(kType, "type");

                                            ALBC_API_JSON_KEY(kSharedProdType, "prodType");
    model::buff::ProdType prod_type;        // shared prod type, does not exist in json
    model::buff::OrderType order_type;      // shared order type, does not exist in json

    int slot_count;                         ALBC_API_JSON_KEY(kSlotCount, "slots");
    JsonInRoomAttributeFields attributes;   ALBC_API_JSON_KEY(kAttributes, "attributes");

    explicit JsonInRoomStruct(const Json::Value& val);
};

struct JsonInParams
{
    int model_time_limit;                                 ALBC_API_JSON_KEY(kModelTimeLimit, "modelTimeLimit");
    int solve_time_limit;                                 ALBC_API_JSON_KEY(kSolveTimeLimit, "solveTimeLimit");
    bool gen_sol_details;                                 ALBC_API_JSON_KEY(kGenSolDetails, "genSolDetails");
    bool gen_lp_file;                                     ALBC_API_JSON_KEY(kGenLpFile, "genLpFile");
    Dictionary<std::string, JsonInCharStruct> chars;      ALBC_API_JSON_KEY(kChars, "chars");
    Dictionary<std::string, JsonInRoomStruct> rooms;      ALBC_API_JSON_KEY(kRooms, "rooms");

    explicit JsonInParams(const Json::Value& val);
};

struct JsonOutRoomStruct
{
    double score = 0;                                    ALBC_API_JSON_KEY(kScore, "score");
    double duration = 0;                                 ALBC_API_JSON_KEY(kDuration, "duration");
    Vector<std::string> chars;                           ALBC_API_JSON_KEY(kChars, "chars");

    JsonOutRoomStruct() = default;
    explicit operator Json::Value() const;
};

struct JsonOutErrorStruct
{
    Dictionary<std::string, std::string> chars;          ALBC_API_JSON_KEY(kChars, "chars");
    Dictionary<std::string, std::string> rooms;          ALBC_API_JSON_KEY(kRooms, "rooms");
    Vector<std::string> errors;                          ALBC_API_JSON_KEY(kError, "errors");

    JsonOutErrorStruct() = default;
    explicit operator Json::Value() const;
};

struct JsonOutParams
{
    Dictionary<std::string, JsonOutRoomStruct> rooms;    ALBC_API_JSON_KEY(kRooms, "rooms");
    JsonOutErrorStruct errors;                           ALBC_API_JSON_KEY(kErrors, "errors");

    JsonOutParams() = default;
    explicit operator Json::Value() const;
};
}
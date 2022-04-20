#pragma once
#include "api_util.h"
#include "albc_config.h"
#include "primitive_types.h"
#include "albc/albc.h"
#include "game_data_tables.h"
#include "locale_util.h"

#include "worker.h"
#include "worker_params.h"
#include "algorithm.h"
#include "algorithm_consts.h"

#include <numeric>

namespace albc ALBC_PUBLIC_NAMESPACE
{

inline static std::shared_ptr<data::building::BuildingData> global_building_data;
inline static std::mutex global_building_data_mutex;

class String::Impl : public std::string
{
  public:
    Impl() = default;

    explicit Impl(const char* str) : std::string(util::ToTargetLocale(str))
    {

    }

    Impl(const Impl& other) = default;
};

template <typename T>
struct [[maybe_unused]] ICollectionVectorImpl : public ICollection<T>, public Vector<T>
{
  protected:
    [[nodiscard]] T* begin_ptr() const { return const_cast<T*>(this->data()); }
    [[nodiscard]] T* end_ptr() const { return const_cast<T*>(this->data() + this->size()); }

  public:
    using Vector<T>::Vector;
    using typename ICollection<T>::const_iterator;
    using typename ICollection<T>::iterator;

    explicit ICollectionVectorImpl(const ICollection<T>* other)
    {
        for (auto const& item : *other)
            this->push_back(item);
    }

    [[nodiscard]] const_iterator begin() const noexcept override
    {
        return const_iterator(this->begin_ptr());
    }
    [[nodiscard]] const_iterator end() const noexcept override
    {
        return const_iterator(this->end_ptr());
    }
    iterator begin() noexcept override
    {
        return iterator(this->begin_ptr());
    }
    iterator end() noexcept override
    {
        return iterator(this->end_ptr());
    }

};

class [[maybe_unused]] CharQueryImpl : public ICharQuery
{
  public:
    data::game::SkillLookupTable::CharQueryItem item;

    [[nodiscard]] bool IsValid() const noexcept override
    {
        return item.HasContent();
    }
    [[nodiscard]] String Id() const noexcept override
    {
        return String { item.id.c_str() };
    }
    [[nodiscard]] String Name() const noexcept override
    {
        if (!data::game::CharacterLookupTable::IsInitialized())
            return String {};

        return String { data::game::CharacterLookupTable::IdToName(item.id).c_str() };
    }
    [[nodiscard]] int Phase() const noexcept override
    {
        return static_cast<int>(item.phase);
    }
    [[nodiscard]] int Level() const noexcept override
    {
        return item.level;
    }
};

class ResultImpl: public IResult
{
  public:
    int status;
    ICollectionVectorImpl<IRoomResult*>*rooms;

    ResultImpl(int status_val, ICollectionVectorImpl<IRoomResult*>* rooms_val)
        : status(status_val), rooms(rooms_val)
    {
    }

    [[nodiscard]] int GetStatus() const noexcept override
    {
        return status;
    }
    [[nodiscard]] ICollection<IRoomResult *>* GetRoomDetails() const noexcept override
    {
        return rooms;
    }
    ~ResultImpl() override
    {
        mem::free_ptr_vector(*rooms);
        delete rooms;
    }
};

class RoomResultImpl: public IRoomResult
{
    String id_;
    ICollectionVectorImpl<String>* char_identifiers_;
    double estimated_score_;
    double duration_;
    String readable_info_;

  public:
    RoomResultImpl(
        String id_val,
        ICollectionVectorImpl<String>* char_identifiers_val,
        double estimated_score_val,
        double duration_val,
        String readable_info_val)
        : id_(std::move(id_val)),
          char_identifiers_(char_identifiers_val),
          estimated_score_(estimated_score_val),
          duration_(duration_val),
          readable_info_(std::move(readable_info_val))
    {
    }

    [[nodiscard]] ICollection<String> *GetCharacterIdentifiers() const noexcept override
    {
        return char_identifiers_;
    }
    [[nodiscard]] double GetScore() const noexcept override
    {
        return estimated_score_;
    }
    [[nodiscard]] double GetDuration() const noexcept override
    {
        return duration_;
    }
    [[nodiscard]] String GetIdentifier() const noexcept override
    {
        return id_;
    }
    [[nodiscard]] String GetReadableInfo() const noexcept override
    {
        return readable_info_;
    }
    ~RoomResultImpl() noexcept override
    {
        delete char_identifiers_;
    }
};

class Character::Impl
{
    Vector<std::string> skill_ids_;
    Vector<std::string> skill_names_;

    static bool QueryChar(const std::shared_ptr<data::game::SkillLookupTable>& table,
                          data::game::SkillLookupTable::CharQueryItem& result,
                          const Vector<std::string>&skill_keys, const std::string& char_id)
    {
        result = table->QueryCharWithBuffList(skill_keys, char_id);
        if (result.HasContent())
            return true;

        for (auto const&skill_key : skill_keys)
        {
            result = table->QueryCharWithBuff(skill_key, char_id);
            if (result.HasContent())
                return true;
        }
        return false;
    }

  public:
    enum class CharParamsType
    {
        ID,
        NAME,
        SKILL
    };

    CharParamsType params_type = CharParamsType::SKILL;
    bool sufficient_level_cond = false;

    std::string identifier;
    std::string resolved_char_id;
    Vector<std::string> resolved_skill_ids;
    data::EvolvePhase phase = data::EvolvePhase::PHASE_0;
    int level = 0;
    double morale = 24;

    void AssignGameDataId(const std::string &id)
    {
        if (id.empty())
            throw std::invalid_argument("id cannot be empty");

        if (!identifier.empty())
            throw std::invalid_argument("identifier is already set" + identifier);

        identifier = id;
        params_type = CharParamsType::ID;
    }

    void AssignName(const std::string &name)
    {
        if (name.empty())
            throw std::invalid_argument("name cannot be empty");

        if (!identifier.empty())
            throw std::invalid_argument("identifier is already set: " + identifier);

        identifier = name;
        params_type = CharParamsType::NAME;
    }

    void SetLevel(int phase_val, int level_val)
    {
        if (phase_val < 0 || phase_val > 2 || level_val < 0)
        {
            throw std::invalid_argument("invalid arguments");
        }

        phase = static_cast<data::EvolvePhase>(phase_val);
        level = level_val;
        sufficient_level_cond = true;
    }

    void AddSkillByName(const std::string &name)
    {
        skill_names_.push_back(name);
    }

    void AddSkillByGameDataId(const std::string &id)
    {
        skill_ids_.push_back(id);
    }

    void SetMorale(double morale_val)
    {
        if (morale_val < 0 || morale_val > 24)
        {
            throw std::invalid_argument("invalid arguments");
        }
        morale = morale_val;
    }

    [[nodiscard]] bool CheckPrepared() const
    {
        return !resolved_skill_ids.empty() || (!resolved_char_id.empty() && sufficient_level_cond);
    }

    bool Prepare()
    {
        if (identifier.empty())
        {
            LOG_E("identifier is empty");
            return false;
        }

        if (CheckPrepared())
        {
            LOG_W(identifier," is already prepared");
            return true;
        }

        auto skill_lookup_table = data::game::SkillLookupTable::instance();
        auto character_table = data::game::CharacterTable::instance();
        auto character_lookup_table = data::game::CharacterLookupTable::instance();
        bool has_skill_lookup_table = skill_lookup_table != nullptr;
        bool has_character_table = character_table != nullptr;
        bool has_character_lookup_table = character_lookup_table != nullptr;

        // 尝试解析干员ID。
        std::string id;
        bool id_resolve_fail = false;
        bool sufficient_id_cond = false;
        switch (params_type)
        {
        case CharParamsType::ID:
            if (!has_character_table)
            {
                LOG_W(identifier, ":CharacterTable is not initialized, character identifier provided has no effect");
                id_resolve_fail = true;
                break;
            }

            id_resolve_fail = character_table->find(identifier) == character_table->end();
            break;

        case CharParamsType::NAME:
            if (!has_character_lookup_table)
            {
                LOG_W(identifier, ":CharacterLookupTable is not initialized, character identifier provided has no effect");
                id_resolve_fail = true;
                break;
            }

            id = character_lookup_table->NameToId(identifier);
            if (id.empty())
                id = character_lookup_table->AppellationToId(identifier);

            id_resolve_fail = id.empty();
            break;

        case CharParamsType::SKILL:
            break;

        default:
            ALBC_UNREACHABLE();
        }

        if (id_resolve_fail)
        {
            LOG_W("Character not found: ", identifier, ". Params type is: ", util::enum_to_string(params_type));
        }
        else
        {
            resolved_char_id = id;
            sufficient_id_cond = true;
        }

        // 解析技能。
        // 1. ID解析成功，且设置等级，则使用building_data.json中定义的干员技能
        // 2. ID和等级未全部提供或无法解析，或只解析了ID的情况下，尝试使用技能反推并补足前两项条件，若无法解析，则直接使用技能参与运算。
        // 如果未提供技能，且ID和等级未全部设置，则解析失败
        bool sufficient_skill_cond = (!skill_ids_.empty() || !skill_names_.empty()) && has_skill_lookup_table;
        if (!has_skill_lookup_table)
        {
            LOG_W(identifier, ": SkillLookupTable is not initialized. Skills provided has no effect.");
        }

        // 已设置技能，使用技能条件
        if (sufficient_skill_cond)
        {
            // 统一使用技能ID解析
            for (const auto& skill_name : skill_names_)
            {
                std::string skill_id = skill_lookup_table->NameToId(skill_name);
                if (skill_id.empty())
                {
                    LOG_W("Skill name not found: ", skill_name, ". Skipping.");
                    continue;
                }

                resolved_skill_ids.push_back(skill_id);
            }

            for (const auto& skill_id : skill_ids_)
            {
                if (!skill_lookup_table->HasId(skill_id))
                {
                    LOG_W("Skill id not found: ", skill_id, ". Skipping.");
                    continue;
                }

                resolved_skill_ids.push_back(skill_id);
            }
        }

        // 尝试使用技能反推并补足前两项条件
        if (sufficient_skill_cond && (!sufficient_level_cond || !sufficient_id_cond))
        {
            if (data::game::SkillLookupTable::CharQueryItem query;
                (!resolved_char_id.empty() && QueryChar(skill_lookup_table,query, resolved_skill_ids, resolved_char_id))
                || QueryChar(skill_lookup_table,query, resolved_skill_ids, ""))
            {
                if (!sufficient_id_cond)
                {
                    resolved_char_id = query.id;
                    sufficient_id_cond = true;
                }

                if (!sufficient_level_cond)
                {
                    level = query.level;
                    phase = query.phase;
                    sufficient_level_cond = true;
                }
                LOG_D("Character resolved by skill: ", resolved_char_id);
            }
        }

        // 如果等级条件无法被解析，则使用角色初始等级状态
        if (sufficient_id_cond && !sufficient_level_cond)
        {
            level = 1;
            phase = data::EvolvePhase::PHASE_0;
            sufficient_level_cond = true;
        }

        bool ok = CheckPrepared();
        if (!ok)
        {
            LOG_W("Character not resolved: ", identifier,
                  ", params type: ", albc::util::enum_to_string(params_type),
                  ", has_skill_lookup_table: ", has_skill_lookup_table,
                  ", has_character_lookup_table: ", has_character_lookup_table,
                  ", has_character_table", has_character_table,
                  ", sufficient_level_cond: ", sufficient_level_cond,
                  ", sufficient_id_cond: ", sufficient_id_cond,
                  ", sufficient_skill_cond: ", sufficient_skill_cond,
                  ", resolved_char_id: ", resolved_char_id,
                  ", level: ", level,
                  ", phase: ", util::enum_to_string(phase),
                  ", skill_ids: {", util::Join(skill_ids_.begin(), skill_ids_.end(), ", "),
                  "}, skill_names: {", util::Join(skill_names_.begin(), skill_names_.end(), ", "),
                  "}");
        }
        return ok;
    }

    void EnsurePrepared() const
    {
        if (!CheckPrepared())
            throw std::runtime_error("Character not prepared: " + identifier);
    }
};

class Room::Impl
{
  public:
    data::building::RoomType type = data::building::RoomType::NONE;
    Array<double, util::enum_size<AlbcRoomParamType>::value> params{};
    std::string identifier;

    [[nodiscard]] bool Prepare() const
    {
        bool type_valid;
        switch (type)
        {
        case data::building::RoomType::MANUFACTURE:
        case data::building::RoomType::TRADING:
            type_valid = true;
            break;

        default:
            type_valid = false;
            break;
        }
        if (!type_valid)
        {
            throw std::invalid_argument("invalid or unsupported room type");
        }

        return true;
    }

    void EnsurePrepared() const
    {
        if (type == data::building::RoomType::NONE)
        {
            throw std::invalid_argument("room type not set: " + identifier);
        }
    }
};

class Model::Impl
{
  private:
    enum class ModelCreateType
    {
        FROM_EMPTY,
        FROM_JSON,
    };

    std::shared_ptr<data::building::BuildingData> building_data_;
    std::unique_ptr<data::player::PlayerDataModel> player_data_;
    Vector<Character*> characters_;
    Vector<Room*> rooms_;
    ModelCreateType create_type_;
    std::unordered_map<std::string, std::string> resolved_char_id_to_orig_identifier_map_;

  public:
    explicit Impl(const Json::Value &player_data_json)
          : player_data_(mem::make_unique_nothrow<data::player::PlayerDataModel>(player_data_json))
    {
        if (!global_building_data)
        {
            throw std::invalid_argument("global building data is not set");
        }

        building_data_ = global_building_data;
        create_type_ = ModelCreateType::FROM_JSON;
    }

    Array<double, util::enum_size<AlbcModelParamType>::value> model_parameters{};

    static Impl* CreateFromFile(const char *player_data_path)
    {
        return new Impl(util::read_json_from_file(player_data_path));
    }

    static Impl* CreateFromJson(const char* player_data_json)
    {
        return new Impl(util::read_json_from_char_array(player_data_json));
    }

    Impl()
    {
        if (!global_building_data)
        {
            throw std::invalid_argument("global building data is not set");
        }

        building_data_ = global_building_data;
        player_data_ = mem::make_unique_nothrow<data::player::PlayerDataModel>();
        create_type_ = ModelCreateType::FROM_EMPTY;
    }

    ~Impl()
    {
        LOG_D("Destroying model. Freeing ", characters_.size(), " characters and ", rooms_.size(), " rooms.");
        mem::free_ptr_vector(characters_);
        mem::free_ptr_vector(rooms_);
        LOG_D("Model destroyed.");
    }

    void AddCharacter(Character *character)
    {
        characters_.push_back(character);
        auto& char_data = *character->impl();
        if (char_data.CheckPrepared() && char_data.resolved_char_id != char_data.identifier)
        {
            resolved_char_id_to_orig_identifier_map_[char_data.resolved_char_id] = char_data.identifier;
        }
    }

    void AddRoom(Room *room)
    {
        rooms_.push_back(room);
    }

    void RemoveCharacter(Character *character)
    {
        characters_.erase(std::remove(characters_.begin(), characters_.end(), character), characters_.end());
        auto& char_data = *character->impl();
        if (char_data.CheckPrepared() && char_data.resolved_char_id != char_data.identifier)
        {
            resolved_char_id_to_orig_identifier_map_.erase(char_data.resolved_char_id);
        }
    }

    void RemoveRoom(Room *room)
    {
        rooms_.erase(std::remove(rooms_.begin(), rooms_.end(), room), rooms_.end());
    }

    void EnsurePrepared() const
    {
        for (const auto& room : rooms_)
        {
            room->impl()->EnsurePrepared();
        }

        for (const auto& character : characters_)
        {
            character->impl()->EnsurePrepared();
        }
    }

    [[nodiscard]] worker::WorkerParams CreateWorkerParams() const
    {
        if (create_type_ == ModelCreateType::FROM_JSON)
        {
            return {*player_data_, *building_data_};
        }

        EnsurePrepared();
        worker::CustomPackedInput input;
        for (const auto& room : rooms_)
        {
            switch (room->impl()->type)
            {
            case data::building::RoomType::MANUFACTURE:
            case data::building::RoomType::TRADING:
                break;

            default:
                LOG_E("Unsupported room type: ", util::enum_to_string(room->impl()->type),
                      " of room ", room->impl()->identifier, ", skipping.");
                continue;
            }

            worker::CustomRoomData room_data;
            room_data.identifier = room->impl()->identifier;
            room_data.type = room->impl()->type;
            room_data.max_slot_cnt = util::read_attribute_as_int(room->impl()->params, ALBC_ROOM_PARAM_SLOT_COUNT);
            room_data.room_attributes.prod_cnt = util::read_attribute_as_int(room->impl()->params, ALBC_ROOM_PARAM_PRODUCT_COUNT);
            room_data.room_attributes.base_prod_eff = 1; // TODO: read from params
            room_data.room_attributes.base_prod_cap = 10; // TODO: read from params
            room_data.room_attributes.prod_type = util::read_attribute_as_enum<model::buff::ProdType>(room->impl()->params, ALBC_ROOM_PARAM_PRODUCT_TYPE);
            room_data.room_attributes.order_type = util::read_attribute_as_enum<model::buff::OrderType>(room->impl()->params, ALBC_ROOM_PARAM_ORDER_TYPE);

            input.rooms.push_back(room_data);
            LOG_D("Added room: ", room_data.identifier,
                  ", RT", util::enum_to_string(room_data.type),
                  ", PT: ", util::enum_to_string(room_data.room_attributes.prod_type),
                  ", OT: ", util::enum_to_string(room_data.room_attributes.order_type));
        }

        for (const auto& character : characters_)
        {
            auto& character_data = input.characters.emplace_back();
            const auto& char_data = *character->impl();
            character_data.identifier = char_data.identifier;

            character_data.is_regular_character =
                !char_data.resolved_char_id.empty() && char_data.sufficient_level_cond;

            character_data.phase = char_data.phase;
            character_data.level = char_data.level;
            character_data.resolved_char_id = char_data.resolved_char_id;
            character_data.resolved_skill_ids = char_data.resolved_skill_ids;
            character_data.morale = char_data.morale;

            std::string skill_ids_dbg_str = util::Join(character_data.resolved_skill_ids.begin(), character_data.resolved_skill_ids.end(), ", ");
            LOG_D("Added character: ", character_data.identifier,
                  ", is_regular_character: ", character_data.is_regular_character,
                  ", phase: ", util::enum_to_string(character_data.phase),
                  ", level: ", character_data.level,
                  ", resolved_char_id: ", character_data.resolved_char_id,
                  ", resolved_skill_ids: ", skill_ids_dbg_str);
        }

        return { input, *global_building_data };
    }

    [[nodiscard]] IResult *GetResult() const
    {
        using namespace worker;
        using namespace algorithm;

        WorkerParams params = CreateWorkerParams();
        LOG_D("Creating worker with params: ", params.GetOperators().size(), " operators, ");
        const auto sc = SCOPE_TIMER_WITH_TRACE("Solving");
        Vector<model::buff::RoomModel *> all_rooms;
        const auto &manu_rooms = mem::unwrap_ptr_vector(params.GetRoomsOfType(data::building::RoomType::MANUFACTURE));
        const auto &trade_rooms = mem::unwrap_ptr_vector(params.GetRoomsOfType(data::building::RoomType::TRADING));

        all_rooms.insert(all_rooms.end(), manu_rooms.begin(), manu_rooms.end());
        all_rooms.insert(all_rooms.end(), trade_rooms.begin(), trade_rooms.end());

        AlbcSolverParameters sp;
        sp.model_time_limit = util::read_attribute(model_parameters, ALBC_MODEL_PARAM_DURATION);
        sp.solve_time_limit = util::read_attribute(model_parameters, ALBC_MODEL_PARAM_SOLVE_TIME_LIMIT);

        if (sp.model_time_limit <= 0) sp.model_time_limit = kDefaultModelTimeLimit;
        if (sp.solve_time_limit <= 0) sp.solve_time_limit = kDefaultSolveTimeLimit;

        MultiRoomIntegerProgramming alg_all(all_rooms, params.GetOperators(), sp);
        AlgorithmResult alg_result;
        alg_all.Run(alg_result);
        auto result = new ResultImpl(0, new ICollectionVectorImpl<IRoomResult*>());
        for (const auto& alg_room_result: alg_result.rooms)
        {
            auto ops = new ICollectionVectorImpl<String>();
            for (const auto* op: alg_room_result.solution.operators)
            {
                if(op)
                {
                    auto it = resolved_char_id_to_orig_identifier_map_.find(op->char_id);
                    if (it != resolved_char_id_to_orig_identifier_map_.end())
                    {
                        ops->emplace_back(it->second.c_str());
                    }
                    else
                    {
                        ops->emplace_back(op->char_id.c_str());
                    }
                }
            }
            auto* room_result = new RoomResultImpl(
                String(alg_room_result.room->id.c_str()),
                ops,
                alg_room_result.solution.productivity,
                alg_room_result.solution.duration,
                String(alg_room_result.room->to_string()
                           .append("\n")
                           .append(alg_room_result.solution.ToString()).c_str()));

            result->rooms->push_back(room_result);
        }
        return result;
    }
};

}
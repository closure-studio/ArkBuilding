#pragma once
#include "albc_config.h"
#include "primitive_types.h"
#include "albc/albc.h"
#include "game_data_tables.h"
#include "locale_util.h"

#include "worker.h"
#include "worker_params.h"

namespace albc ALBC_PUBLIC_NAMESPACE
{

enum class CharResolveType {
    BY_ID,
    BY_NAME,
    BY_SKILL
};

inline static std::shared_ptr<albc::bm::BuildingData> global_building_data;
inline static std::mutex global_building_data_mutex;

struct String::Impl : public string
{
    using string::string;
};

template <typename T>
struct [[maybe_unused]] IEnumerableVectorImpl: public Vector<T>, public IEnumerable<T>
{
    using Vector<T>::Vector;
    typename IEnumerable<T>::ConstIterType begin() const noexcept override
    {
        return typename IEnumerable<T>::ConstIterType(this->data());
    }

    typename IEnumerable<T>::ConstIterType end() const noexcept override
    {
        return typename IEnumerable<T>::ConstIterType(this->data() + this->size());
    }

    // non-const version
    typename IEnumerable<T>::IterType begin() noexcept override
    {
        return typename IEnumerable<T>::IterType(this->data());
    }

    // non-const version
    typename IEnumerable<T>::IterType end() noexcept override
    {
        return typename IEnumerable<T>::IterType(this->data() + this->size());
    }
};


struct Character::Impl
{
  public:
    string identifier;
    string resolved_char_id;
    Vector<string> resolved_skill_ids;
    EvolvePhase phase = EvolvePhase::PHASE_0;
    int level = 0;
    double morale = 24;
    Vector<string> skill_ids;
    Vector<string> skill_names;
    CharResolveType resolve_type = CharResolveType::BY_SKILL;
    bool level_provided = false;

    void AssignGameDataId(const std::string &id)
    {
        if (id.empty())
            throw std::invalid_argument("id cannot be empty");

        if (!identifier.empty())
            throw std::invalid_argument("identifier is already set" + identifier);

        identifier = id;
        resolve_type = CharResolveType::BY_ID;
    }

    void AssignName(const std::string &name)
    {
        if (name.empty())
            throw std::invalid_argument("name cannot be empty");

        if (!identifier.empty())
            throw std::invalid_argument("identifier is already set: " + identifier);

        identifier = name;
        resolve_type = CharResolveType::BY_NAME;
    }

    void SetLevel(int phase_val, int level_val)
    {
        if (phase_val < 0 || phase_val > 2 || level_val < 0)
        {
            throw std::invalid_argument("invalid arguments");
        }

        phase = static_cast<EvolvePhase>(phase_val);
        level = level_val;
        level_provided = true;
    }

    void AddSkillByName(const std::string &name)
    {
        skill_names.push_back(name);
    }

    void AddSkillByGameDataId(const std::string &id)
    {
        skill_ids.push_back(id);
    }

    void SetMorale(double morale_val)
    {
        if (morale_val < 0 || morale_val > 24)
        {
            throw std::invalid_argument("invalid arguments");
        }
        morale = morale_val;
    }

    bool IsPrepared()
    {
        return !resolved_skill_ids.empty() || !resolved_char_id.empty();
    }

    bool Prepare()
    {
        if (identifier.empty())
            throw std::invalid_argument("identifier is not set");

        if (IsPrepared())
            throw std::invalid_argument("This character is already prepared");

        // 尝试解析干员ID，如果失败，将切换为解析技能模式
        string id;
        bool id_resolve_fail = false;
        bool sufficient_id_cond = false;
        switch (resolve_type)
        {
        case CharResolveType::BY_ID:
            if (!CharacterTable::IsInitialized())
                throw std::runtime_error("CharacterTable is not initialized");

            if (CharacterTable::instance()->find(identifier) == CharacterTable::instance()->end())
            {
                id_resolve_fail = true;
            }
            break;

        case CharResolveType::BY_NAME:
            if (!CharacterLookupTable::IsInitialized())
                throw std::runtime_error("CharacterLookupTable is not initialized");

            id = CharacterLookupTable::instance()->NameToId(identifier);
            if (id.empty())
                id = CharacterLookupTable::instance()->AppellationToId(identifier);

            if (!id.empty())
            {
                id_resolve_fail = true;
            }
            break;

        case CharResolveType::BY_SKILL:
            break;

        default:
            ALBC_UNREACHABLE();
        }

        if (resolve_type != CharResolveType::BY_SKILL && id_resolve_fail)
        {
            LOG_W("Character not found: ", id, ". Switching to skill-resolving mode.");
            resolve_type = CharResolveType::BY_SKILL;
        }
        else
        {
            resolved_char_id = id;
            sufficient_id_cond = true;
        }

        // 解析技能
        // 1. ID解析成功，且设置等级，则使用building_data.json中定义的干员技能
        // 2. 任意情况下，只要设置了技能名称或技能ID，则直接使用设置的技能
        // 如果未提供技能，且ID和等级未全部设置，则解析失败
        bool sufficient_skill_cond = !skill_ids.empty() || !skill_names.empty();
        if (!sufficient_skill_cond && (!sufficient_id_cond || !level_provided))
        {
            LOG_W("Insufficient arguments for character resolving. Skipping: ", identifier);
            LOG_W("Resolve type: ", albc::util::to_string(resolve_type),
                  ", level_provided: ", level_provided,
                  ", sufficient_id_cond: ", sufficient_id_cond,
                  ", sufficient_skill_cond: ", sufficient_skill_cond);
            return false;
        }

        // 已设置技能，使用技能条件
        if (sufficient_skill_cond)
        {
            // 统一使用技能ID解析
            for (const auto& skill_name : skill_names)
            {
                string skill_id = SkillLookupTable::instance()->NameToId(skill_name);
                if (skill_id.empty())
                {
                    LOG_W("Skill name not found: ", toOSCharset(skill_name), ". Skipping.");
                    continue;
                }

                resolved_skill_ids.push_back(skill_id);
            }

            for (const auto& skill_id : skill_ids)
            {
                if (!SkillLookupTable::instance()->HasId(skill_id))
                {
                    LOG_W("Skill id not found: ", skill_id, ". Skipping.");
                    continue;
                }

                resolved_skill_ids.push_back(skill_id);
            }
        }

        // 如果名称/ID没有设置，尝试从技能反推
        if (!sufficient_id_cond)
        {
            auto query = SkillLookupTable::instance()->QueryCharWithBuffList(resolved_skill_ids);
            if (!query.HasContent())
            {
                // 从单个buff依次反推
                for (const auto& skill_id : resolved_skill_ids) {
                    query = SkillLookupTable::instance()->QueryCharWithBuff(skill_id);
                    if (query.HasContent())
                    {
                        break;
                    }
                }
            }

            if (query.HasContent())
            {
                resolved_char_id = query.id;
                sufficient_id_cond = true;
                LOG_I("Character resolved by skill: ", resolved_char_id);
            }
        }

        return !resolved_skill_ids.empty() || !resolved_char_id.empty();
    }
};

struct Room::Impl
{
  public:
    bm::RoomType type = bm::RoomType::NONE;
    Array<double, enum_size<AlbcRoomParamType>::value> params{};
    std::string identifier;

    bool Prepare()
    {
        bool type_valid;
        switch (type)
        {
        case bm::RoomType::MANUFACTURE:
        case bm::RoomType::TRADING:
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
};

class Model::Impl
{
  private:
    std::shared_ptr<bm::BuildingData> building_data_;
    std::unique_ptr<PlayerDataModel> player_data_;
    Vector<Character*> characters_;
    Vector<Room*> rooms_;

    Impl(const Json::Value &game_data_json, const Json::Value &player_data_json)
        : building_data_(make_shared_nothrow<bm::BuildingData>(game_data_json)),
          player_data_(make_unique_nothrow<PlayerDataModel>(player_data_json))
    {
    }

  public:
    Array<double, enum_size<AlbcRoomParamType>::value> room_parameters{};

    Impl(const char *game_data_path, const char *player_data_path)
        : Impl(read_json_from_file(game_data_path), read_json_from_file(player_data_path))
    {
    }

    Impl()
    {
        if (!global_building_data)
        {
            throw std::invalid_argument("global building data is not set");
        }

        building_data_ = global_building_data;
        player_data_ = make_unique_nothrow<PlayerDataModel>();
    }

    ~Impl()
    {
        free_ptr_vector(characters_);
        free_ptr_vector(rooms_);
    }

    void AddCharacter(Character *character)
    {
        characters_.push_back(character);
    }

    void AddRoom(Room *room)
    {
        rooms_.push_back(room);
    }

    void RemoveCharacter(Character *character)
    {
        characters_.erase(std::remove(characters_.begin(), characters_.end(), character), characters_.end());
    }

    void RemoveRoom(Room *room)
    {
        rooms_.erase(std::remove(rooms_.begin(), rooms_.end(), room), rooms_.end());
    }

    void Populate()
    {
        for (auto &character : characters_)
        {
        }

        for (auto &room : rooms_)
        {
        }
    }

    [[nodiscard]] worker::WorkerParams CreateWorkerParams() const
    {
        return {*player_data_, *building_data_};
    }
    IResult *GetResult()
    {
        // TODO
        throw std::runtime_error("not implemented");
    }
};

}
#pragma once
#include <utility>

#include "albc_types.h"
#include "data_building.h"
#include "data_character_meta_table.h"
#include "data_character_resolver.h"
#include "data_game.h"
#include "model_buff_primitives.h"

namespace albc::algorithm::iface
{
struct CustomCharacterData
{
    std::string identifier;
    std::string resolved_char_id;
    std::string sp_char_group;
    bool is_regular_character = false;
    double morale = 0;
    data::EvolvePhase phase = data::EvolvePhase::PHASE_0;
    int level = -1;
    Vector<std::string> resolved_skill_ids;
};

struct CustomRoomData
{
    std::string identifier;
    data::building::RoomType type = data::building::RoomType::NONE;
    int max_slot_cnt = 0;
    int level = 1;
    model::buff::RoomAttributeFields room_attributes;
};

struct CustomGlobalData
{
    model::buff::GlobalAttributeFields global_attributes{};
};

class CustomPackedInput
{
  public:
    Vector<CustomCharacterData> characters;
    Vector<CustomRoomData> rooms;
};

class CustomCharacter
{
    const std::shared_ptr<data::game::ICharacterResolver> character_resolver_;
    const std::shared_ptr<data::game::CharacterMetaTable> character_meta_table_;
    Vector<std::string> skill_ids_;
    Vector<std::string> skill_names_;
    Vector<std::string> skill_icons_;
    data::game::CharIdentifierType id_resolve_type_ = data::game::CharIdentifierType::NONE;
    std::string id_for_resolve_;
    std::string identifier_;
    data::EvolvePhase phase_ = data::EvolvePhase::PHASE_0;
    int level_ = -1;
    double morale_ = 0;

  public:
    explicit CustomCharacter(std::shared_ptr<data::game::ICharacterResolver> character_resolver,
                             std::shared_ptr<data::game::CharacterMetaTable> character_meta_table)
        : character_resolver_(std::move(character_resolver)), character_meta_table_(std::move(character_meta_table))
    {
    }

    void SetIdentifier(const std::string &identifier)
    {
        identifier_ = identifier;
    }

    void SetIdResolveCond(const std::string &id_val, data::game::CharIdentifierType id_type_val)
    {
        id_for_resolve_ = id_val;
        id_resolve_type_ = id_type_val;
    }

    void SetLevelCond(data::EvolvePhase phase_val, int level_val)
    {
        phase_ = phase_val;
        level_ = level_val;
    }

    void AddSkillByName(const std::string &skill_name)
    {
        skill_names_.push_back(skill_name);
    }

    void AddSkillById(const std::string &skill_id)
    {
        skill_ids_.push_back(skill_id);
    }

    void AddSkillByIcon(const std::string &skill_icon)
    {
        skill_icons_.push_back(skill_icon);
    }

    void SetMorale(double morale_val)
    {
        morale_ = morale_val;
    }

    [[nodiscard]] std::optional<CustomCharacterData> GenerateCharacterData() const
    {
        data::game::CharResolveParams params(id_for_resolve_,
                                             id_resolve_type_,
                                             skill_ids_,
                                             skill_names_,
                                             skill_icons_,
                                             phase_,
                                             level_);

        data::game::CharResolveResult result;
        if (!character_resolver_->ResolveCharacter(params, result))
        {
            return std::nullopt;
        }

        CustomCharacterData data;
        data.sp_char_group = character_meta_table_->TryGetSpGroup(result.char_id);
        data.identifier = !identifier_.empty() ? identifier_ : result.char_id;
        data.is_regular_character = !result.char_id.empty() && result.HasSufficientLevelCond();
        data.phase = result.phase;
        data.level = result.level;
        data.resolved_char_id = result.char_id;
        data.resolved_skill_ids = result.skill_ids;
        data.morale = morale_;
        return data;
    }
};

class CustomRoom
{
    data::building::RoomType type_ = data::building::RoomType::NONE;
    int max_slot_cnt_ = 0;
    int level_ = 1;
    std::string identifier_;

  public:
    model::buff::RoomAttributeFields room_attributes;
    CustomRoom() = default;

    void SetType(data::building::RoomType type_val)
    {
        type_ = type_val;
    }

    void SetMaxSlotCnt(int max_slot_cnt)
    {
        max_slot_cnt_ = max_slot_cnt;
    }

    void SetLevel(int level)
    {
        level_ = level;
    }

    void SetIdentifier(const std::string &identifier)
    {
        identifier_ = identifier;
    }

    [[nodiscard]] std::optional<CustomRoomData> GenerateRoomData() const
    {
        CustomRoomData data;
        data.identifier = identifier_;
        data.type = type_;
        data.max_slot_cnt = max_slot_cnt_;
        data.level = level_;
        data.room_attributes = room_attributes;
        return data;
    }
};
} // namespace albc::algorithm::iface
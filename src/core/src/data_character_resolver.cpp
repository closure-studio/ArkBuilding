//
// Created by Nonary on 2022/4/24.
//
#include "data_character_resolver.h"

namespace albc::data::game
{
CharResolveParams::CharResolveParams(
    const std::string& identifier_val,
    CharIdentifierType id_type,
    const Vector<std::string>& skill_ids_val,
    const Vector<std::string>& skill_names_val,
    EvolvePhase phase_val,
    int level_val,
    bool warn_if_not_found_val)
    : phase_(phase_val),
      level_(level_val),
      phase_and_level_provided_(level_val != -1),
      identifier_(identifier_val),
      identifier_type_(id_type),
      skill_ids_(skill_ids_val),
      skill_names_(skill_names_val),
      warn_on_failure_(warn_if_not_found_val)
{}
bool CharacterResolver::QueryChar(const ISkillLookupTable &table, ISkillLookupTable::CharQueryItem &result,
                                  const Vector<std::string> &skill_keys, const std::string &char_id)
{
    result = table.QueryCharWithBuffList(skill_keys, char_id);
    if (result.HasContent())
        return true;

    for (auto const&skill_key : skill_keys)
    {
        result = table.QueryCharWithBuff(skill_key, char_id);
        if (result.HasContent())
            return true;
    }
    return false;
}
bool CharacterResolver::ResolveCharacter(const CharResolveParams &params, CharResolveResult &result) const
{
    CharResolveResult tmp_result;

    // 尝试解析干员ID。
    std::string id;
    bool id_resolve_fail = false;
    bool sufficient_id_cond = false;
    switch (params.identifier_type_)
    {
    case CharIdentifierType::ID:

        id_resolve_fail = !character_lookup_table_->HasId(params.identifier_);
        break;

    case CharIdentifierType::NAME:

        id = character_lookup_table_->NameToId(params.identifier_);
        if (id.empty())
            id = character_lookup_table_->AppellationToId(params.identifier_);

        id_resolve_fail = id.empty();
        break;

    case CharIdentifierType::NONE:
        id_resolve_fail = false;
        break;
    }

    if (id_resolve_fail)
    {
        LOG_W("Character not found: ", params.identifier_, ". Params type is: ", util::enum_to_string(params.identifier_type_));
    }
    else if (params.identifier_type_ != CharIdentifierType::NONE)
    {
        tmp_result.char_id = id;
        sufficient_id_cond = true;
    }

    // 解析技能。
    // 1. ID解析成功，且设置等级，则使用building_data.json中定义的干员技能
    // 2. ID和等级未全部提供或无法解析，或只解析了ID的情况下，尝试使用技能反推并补足前两项条件，若无法解析，则直接使用技能参与运算。
    // 如果未提供技能，且ID和等级未全部设置，则解析失败
    bool sufficient_skill_cond = (!params.skill_ids_.empty() || !params.skill_names_.empty());

    // 已设置技能，使用技能条件
    if (sufficient_skill_cond)
    {
        // 统一使用技能ID解析
        for (const auto& skill_name : params.skill_names_)
        {
            std::string skill_id = skill_lookup_table_->NameToId(skill_name);
            if (skill_id.empty())
            {
                LOG_W("Skill name not found: ", skill_name, ". Skipping.");
                continue;
            }

            tmp_result.skill_ids.push_back(skill_id);
        }

        for (const auto& skill_id : params.skill_ids_)
        {
            if (!skill_lookup_table_->HasId(skill_id))
            {
                LOG_W("Skill id not found: ", skill_id, ". Skipping.");
                continue;
            }

            tmp_result.skill_ids.push_back(skill_id);
        }
    }
    bool sufficient_level_cond = params.phase_and_level_provided_;
    if (params.phase_and_level_provided_)
    {
        tmp_result.phase = params.phase_;
        tmp_result.level = params.level_;
    }

    // 尝试使用技能反推并补足前两项条件
    if (sufficient_skill_cond && (!sufficient_level_cond || !sufficient_id_cond))
    {
        if (ISkillLookupTable::CharQueryItem query;
            (!tmp_result.char_id.empty() && QueryChar(*skill_lookup_table_,query, tmp_result.skill_ids, tmp_result.char_id))
            || QueryChar(*skill_lookup_table_,query, tmp_result.skill_ids, ""))
        {
            if (!sufficient_id_cond)
            {
                tmp_result.char_id = query.id;
                sufficient_id_cond = true;
            }

            if (!sufficient_level_cond)
            {
                tmp_result.level = query.level;
                tmp_result.phase = query.phase;
                sufficient_level_cond = true;
            }
            LOG_D("Character resolved by skill: ", query.id);
        }
    }

    // 如果等级条件无法被解析，则使用角色初始等级状态
    if (sufficient_id_cond && !sufficient_level_cond)
    {
        tmp_result.level = 1;
        tmp_result.phase = data::EvolvePhase::PHASE_0;
        sufficient_level_cond = true;
    }

    bool ok = !tmp_result.skill_ids.empty() || (!tmp_result.char_id.empty() && sufficient_level_cond);
    if (ok)
    {
        result = std::move(tmp_result);
    }
    else if (params.warn_on_failure_)
    {
        LOG_W("Character not resolved: ", params.identifier_,
              ", id type: ", albc::util::enum_to_string(params.identifier_type_),
              ", sufficient_level_cond: ", sufficient_level_cond,
              ", sufficient_id_cond: ", sufficient_id_cond,
              ", sufficient_skill_cond: ", sufficient_skill_cond,
              ", resolved_char_id: ", tmp_result.char_id,
              ", level: ", tmp_result.level,
              ", phase: ", util::enum_to_string(tmp_result.phase),
              ", skill_ids: {", util::Join(params.skill_ids_.begin(), params.skill_ids_.end(), ", "),
              "}, skill_names: {", util::Join(params.skill_names_.begin(), params.skill_names_.end(), ", "),
              "}");
    }
    return ok;
}
}
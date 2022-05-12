#pragma once
#include "data_skill_lookup_table.h"

namespace albc::data::game
{
enum class CharIdentifierType
{
    NONE,
    ID,
    NAME
};

class CharResolveParams
{
    friend class CharacterResolver;
    EvolvePhase phase_;
    int level_;
    bool phase_and_level_provided_;
    const std::string& identifier_;
    CharIdentifierType identifier_type_;
    const Vector<std::string>& skill_ids_;
    const Vector<std::string>& skill_names_;
    const Vector<std::string>& skill_icons_;
    bool warn_on_failure_;

  public:
    CharResolveParams(
        const std::string& identifier_val,
        CharIdentifierType id_type,
        const Vector<std::string>& skill_ids_val,
        const Vector<std::string>& skill_names_val,
        const Vector<std::string>& skill_icons_val,
        EvolvePhase phase_val = EvolvePhase::PHASE_0,
        int level_val = -1,
        bool warn_if_not_found_val = true);
};

struct CharResolveResult
{
    std::string char_id;
    Vector<std::string> skill_ids;
    EvolvePhase phase = EvolvePhase::PHASE_0;
    int level = -1;

    [[nodiscard]] constexpr bool HasSufficientLevelCond() const
    {
        return level != -1;
    }
};

class ICharacterResolver
{
  public:
    [[nodiscard]] virtual bool ResolveCharacter(const CharResolveParams& params, CharResolveResult& result) const = 0;
    virtual ~ICharacterResolver() = default;
};

class CharacterResolver : public ICharacterResolver
{
    std::shared_ptr<ISkillLookupTable> skill_lookup_table_;
    std::shared_ptr<ICharacterLookupTable> character_lookup_table_;

    static bool QueryChar(const ISkillLookupTable& table,
                          ISkillLookupTable::CharQueryEntry & result,
                          const Vector<std::string>&skill_keys, const std::string& char_id);

  public:
    explicit CharacterResolver(std::shared_ptr<ISkillLookupTable> skill_lookup_table,
                      std::shared_ptr<ICharacterLookupTable> character_lookup_table)
        : skill_lookup_table_(std::move(skill_lookup_table)),
          character_lookup_table_(std::move(character_lookup_table))
    {}

    [[nodiscard]] bool ResolveCharacter(const CharResolveParams& params, CharResolveResult& result) const override;
};

}

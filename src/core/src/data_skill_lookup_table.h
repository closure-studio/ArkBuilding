//
// Created by Nonary on 2022/4/21.
//
#pragma once
#include "data_character_lookup_table.h"
#include "data_game.h"
#include "util_log.h"
#include "albc_types.h"
#include "util.h"
namespace albc::data::game
{
class ISkillLookupTable
{
  public:
    struct CharQueryItem
    {
        std::string id;
        albc::data::EvolvePhase phase = albc::data::EvolvePhase::PHASE_0;
        int level = -1;

        static CharQueryItem Empty();
        bool operator==(const CharQueryItem &other) const;
        bool operator!=(const CharQueryItem &other) const;

        // 当该角色查询的解锁条件（精英阶段和等级）低于目标角色查询时，认为该角色查询可以被目标查询覆写
        [[nodiscard]] bool CanBeOverwritten(const CharQueryItem &other) const;
        [[nodiscard]] bool HasContent() const;

        [[nodiscard]] std::string to_string() const;
    };

    [[nodiscard]] virtual bool HasId(const std::string &id) const = 0;

    [[nodiscard]] virtual bool HasName(const std::string &name) const = 0;

    [[nodiscard]] virtual std::string NameToId(const std::string &name) const = 0;

    [[nodiscard]] virtual std::string IdToName(const std::string &id) const = 0;

    [[nodiscard]] virtual CharQueryItem QueryCharWithBuffList(const albc::Vector<std::string> &keys,
                                                              const std::string &char_key) const = 0;

    [[nodiscard]] virtual CharQueryItem QueryCharWithBuff(const std::string &buff_key,
                                                          const std::string &char_key) const = 0;
};

class SkillLookupTable : public albc::data::game::ISkillLookupTable
{
  public:
    [[nodiscard]] bool HasId(const std::string &id) const override;

    [[nodiscard]] bool HasName(const std::string &name) const override;

    [[nodiscard]] std::string NameToId(const std::string &name) const override;

    [[nodiscard]] std::string IdToName(const std::string &id) const override;

    [[nodiscard]] CharQueryItem QueryCharWithBuffList(const Vector<std::string> &keys,
                                                      const std::string &char_key) const override;

    [[nodiscard]] CharQueryItem QueryCharWithBuff(const std::string &buff_key,
                                                  const std::string &char_key) const override;

    explicit SkillLookupTable(std::shared_ptr<building::BuildingData> building_data,
                              std::shared_ptr<ICharacterLookupTable> char_lookup_table);

  private:
    Dictionary<std::string, std::string> name_to_id_;
    Dictionary<std::string, std::string> id_to_name_;

    // （字符串集合） -> 查询结果
    using CompositeHashKey = size_t;

    struct MapItem
    {
        bool is_terminal = true;
        bool has_item = false;
        CharQueryItem char_query;

        [[nodiscard]] bool IsValid() const;

        void TryAssignOrOverwrite(const CharQueryItem &char_query_val);
    };

    using BuffToCharMap = std::unordered_map<CompositeHashKey, MapItem>;

    // 多buff/单buff/干员+多buff/干员+单buff/到干员的查询表
    // buff_id(s) / buff_name(s) -> CharQueryItem
    // char_id/name + buff_id(s) / char_id/name + buff_name(s)-> CharQueryItem
    // id和名字共用一个map（因为不会冲突）
    // 不支持模糊查询
    BuffToCharMap query_map_;
    static constexpr std::string_view kSingleBuffHashKey = "single_buff";
    static constexpr std::string_view kMultiBuffHashKey = "multi_buff";
    static constexpr std::string_view kCharHashKey = "with_char_key";

    static void InsertQueryItem(BuffToCharMap &target, const CharQueryItem &query, const Vector<std::string> &buff_keys,
                                const std::string &char_key = {});

    static CompositeHashKey HashStringCollection(const Vector<std::string> &list);

    static CompositeHashKey HashString(const std::string_view &str);

    static CompositeHashKey HashMultiBuff(const Vector<std::string> &buff_keys, const std::string &char_key = {});

    static CompositeHashKey HashSingleBuff(const std::string &buff_key, const std::string &char_key = {});

    static void InsertMultiBuffLookupItem(BuffToCharMap &target, const CharQueryItem &query,
                                          const Vector<std::string> &buff_keys, const std::string &char_key = {});

    static void InsertSingleBuffLookupItem(BuffToCharMap &target, const CharQueryItem &query,
                                           const std::string &buff_key, const std::string &char_key = {});

    static void CleanupBuffLookupMap(BuffToCharMap &target, UInt32 &valid_count);
};

} // namespace albc::data::game
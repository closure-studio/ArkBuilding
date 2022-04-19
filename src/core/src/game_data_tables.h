#pragma once

#include "primitive_types.h"
#include "building_data_model.h"

namespace albc::data::game
{
template <typename TGet, typename TInit, typename TStore = TGet>
class SingletonDB
{
  public:
    static std::shared_ptr<TGet> instance()
    {
        return instance_;
    }

    static void Init(const TInit &init_data)
    {
        std::scoped_lock lk(init_mutex_);
        std::shared_ptr<TGet> ptr(new TStore(init_data));
        instance_.swap(ptr);
    }

    static bool IsInitialized()
    {
        return instance_ != nullptr;
    }

  protected:
    inline static std::shared_ptr<TGet> instance_;
    inline static std::mutex init_mutex_;
    SingletonDB() = default;
};

class CharacterData
{
  public:
    std::string name;
    std::string appellation;

    explicit CharacterData(const Json::Value &json)
        : name(json["name"].asString()), appellation(json["appellation"].asString())
    {
    }
};
class CharacterTable : public mem::PtrDictionary<std::string, CharacterData>, public SingletonDB<CharacterTable, Json::Value>
{
  protected:
    friend class SingletonDB<CharacterTable, Json::Value>;
    explicit CharacterTable(const Json::Value &json);
};
class CharacterLookupTable : public SingletonDB<CharacterLookupTable, CharacterTable>
{
  public:
    [[nodiscard]] std::string NameToId(const std::string &name) const;

    [[nodiscard]] std::string AppellationToId(const std::string &appellation) const;

    [[nodiscard]] static std::string IdToName(const std::string &id);

    [[nodiscard]] static std::string IdToAppellation(const std::string &id);

  protected:
    friend class SingletonDB<CharacterLookupTable, CharacterTable>;
    Dictionary<std::string, std::string> name_to_id_;
    Dictionary<std::string, std::string> appellation_to_id_;

    explicit CharacterLookupTable(const CharacterTable &character_table);
};
class SkillLookupTable : public SingletonDB<SkillLookupTable, data::building::BuildingData>
{
  public:
    struct CharQueryItem
    {
        std::string id;
        EvolvePhase phase = EvolvePhase::PHASE_0;
        int level = -1;

        static CharQueryItem Empty();

        bool operator==(const CharQueryItem &other) const;

        bool operator!=(const CharQueryItem &other) const;

        // 当该角色查询的解锁条件（精英阶段和等级）低于目标角色查询时，认为该角色查询可以被目标查询覆写
        [[nodiscard]] bool CanBeOverwritten(const CharQueryItem &other) const;

        [[nodiscard]] bool HasContent() const;

        [[nodiscard]] std::string to_string() const;
    };

    [[nodiscard]] bool HasId(const std::string& id) const;

    [[nodiscard]] bool HasName(const std::string& name) const;

    [[nodiscard]] std::string NameToId(const std::string &name) const;

    [[nodiscard]] std::string IdToName(const std::string &id) const;

    [[nodiscard]] CharQueryItem QueryCharWithBuffList(const Vector<std::string> &keys, const std::string& char_key = {}) const;

    [[nodiscard]] CharQueryItem QueryCharWithBuff(const std::string& buff_key, const std::string& char_key = {}) const;

  protected:
    friend class SingletonDB<SkillLookupTable, data::building::BuildingData>;
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

    explicit SkillLookupTable(const data::building::BuildingData &building_data);

    static void InsertQueryItem(BuffToCharMap & target, const CharQueryItem& query,
                                const Vector<std::string>&buff_keys, const std::string& char_key = {});

    static CompositeHashKey HashStringCollection(const Vector<std::string>& list);

    static CompositeHashKey HashString(const std::string_view& str);

    static CompositeHashKey HashMultiBuff(const Vector<std::string> &buff_keys, const std::string&char_key = {});

    static CompositeHashKey HashSingleBuff(const std::string& buff_key, const std::string&char_key = {});

    static void InsertMultiBuffLookupItem(BuffToCharMap &target, const CharQueryItem &query,
                                          const Vector<std::string> &buff_keys, const std::string&char_key = {});

    static void InsertSingleBuffLookupItem(BuffToCharMap &target, const CharQueryItem &query,
                                           const std::string &buff_key, const std::string&char_key = {});

    static void CleanupBuffLookupMap(BuffToCharMap &target, UInt32& valid_count);
};
} // namespace albc
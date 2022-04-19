#pragma once
#include "buff_consts.h"
#include "building_data_model.h"
#include "json_util.h"
#include "primitive_types.h"

#include <numeric>

namespace albc
{
template <typename TGet, typename TInit> class SingletonDB
{
  public:
    static std::shared_ptr<TGet> instance()
    {
        return instance_;
    }

    static void init(const TInit &init_data)
    {
        std::scoped_lock lk(init_mutex_);
        instance_ = std::shared_ptr<TGet>(new TGet(init_data));
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
    string name;
    string appellation;

    explicit CharacterData(const Json::Value &json)
        : name(json["name"].asString()), appellation(json["appellation"].asString())
    {
    }
};
class CharacterTable : public PtrDictionary<string, CharacterData>, public SingletonDB<CharacterTable, Json::Value>
{
  protected:
    explicit CharacterTable(const Json::Value &json)
        : PtrDictionary<string, CharacterData>(json_val_as_ptr_dictionary<CharacterData>(json))
    {
    }
};
class CharacterLookupTable : public SingletonDB<CharacterLookupTable, Json::Value>
{
  public:
    [[nodiscard]] string NameToId(const string &name) const
    {
        auto it = name_to_id_.find(name);
        return it == name_to_id_.end() ? "" : it->second;
    }

    [[nodiscard]] string AppellationToId(const string &appellation) const
    {
        auto it = appellation_to_id_.find(appellation);
        return it == appellation_to_id_.end() ? "" : it->second;
    }

    [[nodiscard]] static string IdToName(const string &id)
    {
        if (!CharacterTable::IsInitialized())
            return "";

        auto it = CharacterTable::instance()->find(id);
        return it == CharacterTable::instance()->end() ? "" : it->second->name;
    }

    [[nodiscard]] static string IdToAppellation(const string &id)
    {
        if (!CharacterTable::IsInitialized())
            return "";

        auto it = CharacterTable::instance()->find(id);
        return it == CharacterTable::instance()->end() ? "" : it->second->appellation;
    }

  protected:
    Dictionary<string, string> name_to_id_;
    Dictionary<string, string> appellation_to_id_;

    explicit CharacterLookupTable(const CharacterTable &character_table)
    {
        for (auto &[char_id, character] : character_table)
        {
            name_to_id_[character->name] = char_id;
            if (!character->appellation.empty())
                appellation_to_id_[character->appellation] = char_id;
        }
    }
};
class SkillLookupTable : public SingletonDB<SkillLookupTable, bm::BuildingData>
{
  public:
    struct CharQueryItem
    {
        string id;
        EvolvePhase phase = EvolvePhase::PHASE_0;
        int level = -1;

        static CharQueryItem Empty()
        {
            return {};
        }

        bool operator==(const CharQueryItem &other) const
        {
            return id == other.id && phase == other.phase && level == other.level;
        }

        bool operator!=(const CharQueryItem &other) const
        {
            return !(*this == other);
        }

        // 当该角色查询的解锁条件（精英阶段和等级）低于目标角色查询时，认为该角色查询可以被目标查询覆写
        [[nodiscard]] bool CanBeOverwritten(const CharQueryItem &other) const
        {
            return id == other.id && phase <= other.phase && level <= other.level;
        }

        [[nodiscard]] bool HasContent() const
        {
            return !id.empty();
        }

        [[nodiscard]] string to_string() const
        {
            char buf[256];
            char *p = buf;
            size_t s = sizeof(buf);
            append_snprintf(p, s, "id:%s, phase:%s, level:%d", id.c_str(), albc::util::to_string(phase), level);
            return buf;
        }
    };

    [[nodiscard]] bool HasId(const string& id) const
    {
        return id_to_name_.find(id) != id_to_name_.end();
    }

    [[nodiscard]] bool HasName(const string& name) const
    {
        return name_to_id_.find(name) != name_to_id_.end();
    }

    [[nodiscard]] string NameToId(const string &name) const
    {
        auto it = name_to_id_.find(name);
        return it == name_to_id_.end() ? "" : it->second;
    }

    [[nodiscard]] string IdToName(const string &id) const
    {
        auto it = id_to_name_.find(id);
        return it == id_to_name_.end() ? "" : it->second;
    }

    [[nodiscard]] CharQueryItem QueryCharWithBuffList(const Vector<string> &keys, bool is_name = false) const
    {
        CompositeHashKey key;
        std::sort(keys.begin(), keys.end());
        std::copy_n(keys.begin(), std::min(keys.size(), key.size()), key.begin());
        const auto &map = is_name ? char_query_multi_buff_id_map_ : char_query_multi_buff_name_map_;
        auto it = map.find(key);
        return it != map.end() && it->second.IsValid()
                   ? it->second.char_query
                   : CharQueryItem::Empty();
    }

    [[nodiscard]] CharQueryItem QueryCharWithBuffIdentifier(const string &key, bool is_name = false) const
    {
        const auto &map = is_name ? char_query_buff_id_map_ : char_query_buff_name_map_;
        auto it = map.find(key);
        return it != map.end() && it->second.IsValid()
                   ? it->second.char_query
                   : CharQueryItem::Empty();
    }

  protected:
    Dictionary<string, string> name_to_id_;
    Dictionary<string, string> id_to_name_;

    using CompositeHashKey = Array<string, kOperatorMaxBuffs>;

    struct MapItem
    {
        bool is_terminal = true;
        bool has_item = false;
        CharQueryItem char_query;

        [[nodiscard]] bool IsValid() const
        {
            return has_item && is_terminal;
        }

        void TryAssignOrOverwrite(const CharQueryItem &char_query_val)
        {
            if (!char_query_val.HasContent())
                return;

            // 当不能被覆写（既与目标查询不兼容）时，认为该条目存在冲突，不再表明某个独立的角色查询
            is_terminal &= is_terminal && (!has_item || char_query.CanBeOverwritten(char_query_val));
            has_item = true;
            char_query = char_query_val;
        }
    };

    struct Hash
    {
        size_t operator()(const CompositeHashKey &key) const
        {
            return std::accumulate(key.begin(), key.end(), 0,
                                   [](size_t seed, const string &str) { return seed ^ std::hash<string>()(str); });
        }
    };

    using MultiBuffToCharMap = std::unordered_map<CompositeHashKey, MapItem, Hash>;
    using SingleBuffToCharMap = std::unordered_map<string, MapItem>;

    // 多buff/单buff到干员的查询表
    MultiBuffToCharMap char_query_multi_buff_id_map_;
    MultiBuffToCharMap char_query_multi_buff_name_map_;
    SingleBuffToCharMap char_query_buff_id_map_;
    SingleBuffToCharMap char_query_buff_name_map_;

    explicit SkillLookupTable(const bm::BuildingData &building_data)
    {
        for (const auto &[id, buff] : building_data.buffs)
        {
            name_to_id_[buff->buff_name] = id;
            id_to_name_[id] = buff->buff_name;
        }

        for (const auto &[char_id, character] : building_data.chars)
        {
            // 用于生成该干员在所有可能的等级条件下的技能组合
            EvolvePhase cur_phase = EvolvePhase::PHASE_0;
            int cur_level = 1;
            Vector<string> current;
            Vector<string> current_names;
            Vector<std::pair<bm::BuildingBuffCharSlot*, bm::SlotItem>> buff_cond_nodes;
            // 同在一个Slot中的buff，当更高级的生效时需要替换掉低级的
            Dictionary<bm::BuildingBuffCharSlot*, bm::SlotItem> buff_cond_slots;

            for (const auto &slot : character->buff_char) {
                for (const auto &slot_item : slot->buff_data) {
                    buff_cond_nodes.push_back({slot.get(), slot_item});
                }
            }

            // 添加边界条件
            bm::SlotItem boundary;
            boundary.buff_id = "";
            boundary.cond.phase = EvolvePhase::PHASE_3;
            boundary.cond.level = INFINITY;
            buff_cond_nodes.push_back({nullptr, boundary});

            // 根据升级条件升序排序buff，得到角色从低级到高级依此解锁的技能顺序
            std::sort(buff_cond_nodes.begin(), buff_cond_nodes.end(),
                      [](const std::pair<bm::BuildingBuffCharSlot*, bm::SlotItem>& lhs,
                         const std::pair<bm::BuildingBuffCharSlot*, bm::SlotItem>& rhs)
                      { return lhs.second.cond.phase < rhs.second.cond.phase
                               && lhs.second.cond.level < rhs.second.cond.level; });

            // 生成buff组合
            for (const auto &[slot, slot_item] : buff_cond_nodes) {
                if (!slot_item.cond.Check(cur_phase, cur_level))
                {
                    if (current.empty())
                        continue;

                    CharQueryItem query{char_id, cur_phase, cur_level};
                    InsertMultiBuffLookupItem(char_query_multi_buff_id_map_, query, current);
                    InsertMultiBuffLookupItem(char_query_multi_buff_id_map_, query, current_names);
                    for (const auto &id : current)
                        InsertSingleBuffLookupItem(char_query_buff_id_map_, query, id);

                    for (const auto &name : current_names)
                        InsertSingleBuffLookupItem(char_query_buff_id_map_, query, name);
                }

                // 跳过边界条件
                if (slot == nullptr || slot_item.buff_id.empty())
                    break;

                auto slot_it = buff_cond_slots.find(slot);
                if (slot_it == buff_cond_slots.end())
                {
                    buff_cond_slots.insert({slot, slot_item});
                    current.push_back(slot_item.buff_id);
                    current_names.push_back(id_to_name_[slot_item.buff_id]);
                }
                else
                {
                    auto perv = slot_it->second;
                    slot_it->second = slot_item;
                    std::replace(current.begin(), current.end(), perv.buff_id, slot_item.buff_id);
                    std::replace(current_names.begin(), current_names.end(), id_to_name_[perv.buff_id], id_to_name_[slot_item.buff_id]);
                }
            }
        }
    }

    static void InsertMultiBuffLookupItem(MultiBuffToCharMap &target, const CharQueryItem &query,
                                          const Vector<string> &identifiers)
    {
        if (!query.HasContent())
            throw std::invalid_argument("InsertMultiBuffLookupItem(): char_id is empty");

        CompositeHashKey key;
        std::sort(identifiers.begin(), identifiers.end());
        std::copy_n(identifiers.begin(), std::min(identifiers.size(), key.size()), key.begin());
        target[key].TryAssignOrOverwrite(query);
    }

    static void InsertSingleBuffLookupItem(SingleBuffToCharMap &target, const CharQueryItem &query,
                                           const string &identifier)
    {
        if (!query.HasContent())
            throw std::invalid_argument("InsertSingleBuffLookupItem(): char_id is empty");

        target[identifier].TryAssignOrOverwrite(query);
    }
};
} // namespace albc
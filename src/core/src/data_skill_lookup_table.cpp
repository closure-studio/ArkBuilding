//
// Created by Nonary on 2022/4/24.
//
#include "data_skill_lookup_table.h"

namespace albc::data::game
{

ISkillLookupTable::CharQueryItem ISkillLookupTable::CharQueryItem::Empty()
{
    return {};
}
bool ISkillLookupTable::CharQueryItem::operator==(const ISkillLookupTable::CharQueryItem &other) const
{
    return id == other.id && phase == other.phase && level == other.level;
}
bool ISkillLookupTable::CharQueryItem::operator!=(const ISkillLookupTable::CharQueryItem &other) const
{
    return !(*this == other);
}
bool ISkillLookupTable::CharQueryItem::CanBeOverwritten(const ISkillLookupTable::CharQueryItem &other) const
{
    return id == other.id && phase <= other.phase && level <= other.level;
}
bool ISkillLookupTable::CharQueryItem::HasContent() const
{
    return !id.empty();
}
std::string ISkillLookupTable::CharQueryItem::to_string() const
{
    char buf[256];
    char *p = buf;
    size_t s = sizeof(buf);
    util::append_snprintf(p, s, "id:%s, phase:%s, level:%d", id.c_str(), util::enum_to_string(phase), level);
    return buf;
}
bool SkillLookupTable::HasId(const std::string &id) const
{
    return id_to_name_.find(id) != id_to_name_.end();
}
bool SkillLookupTable::HasName(const std::string &name) const
{
    return name_to_id_.find(name) != name_to_id_.end();
}
std::string SkillLookupTable::NameToId(const std::string &name) const
{
    auto it = name_to_id_.find(name);
    return it == name_to_id_.end() ? "" : it->second;
}
std::string SkillLookupTable::IdToName(const std::string &id) const
{
    auto it = id_to_name_.find(id);
    return it == id_to_name_.end() ? "" : it->second;
}
ISkillLookupTable::CharQueryItem SkillLookupTable::QueryCharWithBuffList(const Vector<std::string> &keys,
                                                                         const std::string &char_key) const
{
    if (keys.empty())
        return CharQueryItem::Empty();

    CompositeHashKey key = HashMultiBuff(keys, char_key);
    const auto &map = query_map_;
    auto it = map.find(key);
    return it != map.end() && it->second.IsValid() ? it->second.char_query : CharQueryItem::Empty();
}
ISkillLookupTable::CharQueryItem SkillLookupTable::QueryCharWithBuff(const std::string &buff_key,
                                                                     const std::string &char_key) const
{
    if (buff_key.empty())
        return CharQueryItem::Empty();

    CompositeHashKey key = HashSingleBuff(buff_key, char_key);
    const auto &map = query_map_;
    auto it = map.find(key);
    return it != map.end() && it->second.IsValid() ? it->second.char_query : CharQueryItem::Empty();
}
SkillLookupTable::SkillLookupTable(std::shared_ptr<building::BuildingData> building_data,
                                   std::shared_ptr<ICharacterLookupTable> char_lookup_table)
{
    for (const auto &[id, buff] : building_data->buffs)
    {
        name_to_id_[buff->buff_name] = id;
        id_to_name_[id] = buff->buff_name;
    }

    for (const auto &[char_id, character] : building_data->chars)
    {
        // 用于生成该干员在所有可能的等级条件下的技能组合
        EvolvePhase cur_phase = EvolvePhase::PHASE_0;
        int cur_level = 1;
        Vector<std::string> current;
        Vector<std::string> current_names;
        Vector<std::pair<building::BuildingBuffCharSlot *, building::SlotItem>> buff_cond_nodes;
        // 同在一个Slot中的buff，当更高级的生效时需要替换掉低级的
        Dictionary<building::BuildingBuffCharSlot *, building::SlotItem> buff_cond_slots;

        for (const auto &slot : character->buff_char)
        {
            for (const auto &slot_item : slot->buff_data)
            {
                buff_cond_nodes.push_back({slot.get(), slot_item});
            }
        }

        // 添加边界条件
        {
            auto &[slot, boundary] = buff_cond_nodes.emplace_back();
            slot = nullptr;
            boundary.buff_id = "";
            boundary.cond.phase = EvolvePhase::PHASE_3;
            boundary.cond.level = INT32_MAX;
        }

        // 根据升级条件升序排序buff，得到角色从低级到高级依此解锁的技能顺序
        std::sort(buff_cond_nodes.begin(), buff_cond_nodes.end(), [](const auto &lhs, const auto &rhs) {
          return lhs.second.cond.phase < rhs.second.cond.phase && lhs.second.cond.level < rhs.second.cond.level;
        });

        // 生成buff组合
        for (const auto &[slot, slot_item] : buff_cond_nodes)
        {
            if (!slot_item.cond.Check(cur_phase, cur_level))
            {
                if (current.empty())
                    continue;

                CharQueryItem query{char_id, cur_phase, cur_level};
                InsertQueryItem(query_map_, query, current);
                InsertQueryItem(query_map_, query, current_names);
                InsertQueryItem(query_map_, query, current, char_id);
                InsertQueryItem(query_map_, query, current_names, char_id);

                std::string char_name = char_lookup_table->IdToName(char_id);
                InsertQueryItem(query_map_, query, current, char_name);
                InsertQueryItem(query_map_, query, current_names, char_name);

                cur_phase = slot_item.cond.phase;
                cur_level = slot_item.cond.level;
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
                std::replace(current_names.begin(), current_names.end(), id_to_name_[perv.buff_id],
                             id_to_name_[slot_item.buff_id]);
            }
        }
    }

    // 删除无效条目并输出统计信息
    UInt32 multi_lookup_count = 0;
    CleanupBuffLookupMap(query_map_, multi_lookup_count);

    LOG_I("Successfully loaded: ", multi_lookup_count, " multi buff lookup items. ");
}
void SkillLookupTable::InsertQueryItem(
    std::unordered_map<SkillLookupTable::CompositeHashKey, SkillLookupTable::MapItem> &target,
    const ISkillLookupTable::CharQueryItem &query, const Vector<std::string> &buff_keys, const std::string &char_key)
{
    InsertMultiBuffLookupItem(target, query, buff_keys, char_key);
    for (const auto &id : buff_keys)
        InsertSingleBuffLookupItem(target, query, id, char_key);
}
SkillLookupTable::CompositeHashKey SkillLookupTable::HashStringCollection(const Vector<std::string> &list)
{
    return std::accumulate(list.begin(), list.end(), 0,
                           [](size_t seed, const std::string &str) { return seed ^ HashString(str); });
}
SkillLookupTable::CompositeHashKey SkillLookupTable::HashString(const std::string_view &str)
{
    return std::hash<std::string_view>{}(str);
}
SkillLookupTable::CompositeHashKey SkillLookupTable::HashMultiBuff(const Vector<std::string> &buff_keys,
                                                                   const std::string &char_key)
{
    CompositeHashKey key = HashStringCollection(buff_keys);
    key ^= HashString(kMultiBuffHashKey);
    if (!char_key.empty())
    {
        key ^= HashString(char_key);
        key ^= HashString(kCharHashKey);
    }
    return key;
}
SkillLookupTable::CompositeHashKey SkillLookupTable::HashSingleBuff(const std::string &buff_key,
                                                                    const std::string &char_key)
{
    CompositeHashKey key = HashString(buff_key);
    key ^= HashString(kSingleBuffHashKey);
    if (!char_key.empty())
    {
        key ^= HashString(char_key);
        key ^= HashString(kCharHashKey);
    }
    return key;
}
void SkillLookupTable::InsertMultiBuffLookupItem(
    std::unordered_map<SkillLookupTable::CompositeHashKey, SkillLookupTable::MapItem> &target,
    const ISkillLookupTable::CharQueryItem &query, const Vector<std::string> &buff_keys, const std::string &char_key)
{
    if (!query.HasContent())
        throw std::invalid_argument("InsertMultiBuffLookupItem(): char_id is empty");

    CompositeHashKey key = HashMultiBuff(buff_keys, char_key);
    target[key].TryAssignOrOverwrite(query);
}
void SkillLookupTable::InsertSingleBuffLookupItem(
    std::unordered_map<SkillLookupTable::CompositeHashKey, SkillLookupTable::MapItem> &target,
    const ISkillLookupTable::CharQueryItem &query, const std::string &buff_key, const std::string &char_key)
{
    if (!query.HasContent())
        throw std::invalid_argument("InsertSingleBuffLookupItem(): char_id is empty");

    CompositeHashKey key = HashSingleBuff(buff_key, char_key);
    target[key].TryAssignOrOverwrite(query);
}
void SkillLookupTable::CleanupBuffLookupMap(
    std::unordered_map<SkillLookupTable::CompositeHashKey, SkillLookupTable::MapItem> &target, UInt32 &valid_count)
{
    for (auto it = target.begin(); it != target.end();)
    {
        if (!it->second.IsValid())
        {
            it = target.erase(it);
            continue;
        }

        ++valid_count;
        ++it;
    }
}
bool SkillLookupTable::MapItem::IsValid() const
{
    return has_item && is_terminal;
}
void SkillLookupTable::MapItem::TryAssignOrOverwrite(const ISkillLookupTable::CharQueryItem &char_query_val)
{
    if (!char_query_val.HasContent())
        return;

    // 当不能被覆写（既与目标查询不兼容）时，认为该条目存在冲突，不再表明某个独立的角色查询
    is_terminal &= is_terminal && (!has_item || char_query.CanBeOverwritten(char_query_val));
    has_item = true;
    char_query = char_query_val;
}
}
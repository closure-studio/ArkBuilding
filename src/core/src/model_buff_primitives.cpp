//
// Created by Nonary on 2022/4/24.
//
#include "model_buff_primitives.h"

namespace albc::model::buff
{
std::string RoomAttributeModifier::to_string() const
{
    char buf[256];
    char *p = buf;
    size_t size = sizeof buf;
    util::append_snprintf(p, size, "[%-35s]dE:%3.f%%, dC:%2d, A:%3.f%%, mE:%3.f%%", util::enum_to_string(buff_type).data(),
                          eff_delta * 100, cap_delta, eff_inc_per_hour * 100, max_extra_eff_delta * 100);

    return buf;
}
std::string CharacterCostModifier::to_string() const
{
    char buf[256];
    char *p = buf;
    size_t size = sizeof(buf);
    util::append_snprintf(p, size, "[%-15s]dC:%3.f%%", util::enum_to_string(type).data(), value * 100);

    return buf;
}
std::string RoomFinalAttributeModifier::to_string() const
{
    char buf[256];
    char *p = buf;
    size_t size = sizeof buf;
    util::append_snprintf(p, size, "[%-35s]dE:%3.f%%, dC:%2d, A:%3.f%%, mE:%3.f%%, [%-12s]kE%3.f%%",
                          util::enum_to_string(buff_type).data(), eff_delta * 100, cap_delta, eff_inc_per_hour * 100,
                          max_extra_eff_delta * 100, util::enum_to_string(final_mod_type).data(), eff_scale * 100);

    return buf;
}

std::string RoomModel::to_string() const
{
    using namespace util;

    char buf[8192];
    char *p = buf;
    size_t size = sizeof buf;

    append_snprintf(p, size, "[RoomModel]\n");
    append_snprintf(p, size, "\t- Type    :%s\n", enum_to_string(type).data());
    append_snprintf(p, size, "\t- ID      :%s\n", id.data());
    append_snprintf(p, size, "\t- SlotCnt :%d\n", max_slot_count);
    append_snprintf(p, size, "[GlobalAttributes]\n");

    std::string global_attr;
    UInt32 attr_type = 0;
    for (const auto attr : global_attributes)
    {
        append_snprintf(p, size, "\t- %-20s: %3.f\n",
                        enum_to_string(static_cast<GlobalAttributeType>(attr_type)).data(), attr);
        ++attr_type;
    }

    append_snprintf(p, size, "[RoomAttributes]\n");
    append_snprintf(p, size, "\t- Prod    :%s\n", enum_to_string(room_attributes.prod_type).data());
    append_snprintf(p, size, "\t- Order   :%s\n", enum_to_string(room_attributes.order_type).data());
    append_snprintf(p, size, "\t- BaseEff :%3.f%%\n", room_attributes.base_prod_eff * 100);
    append_snprintf(p, size, "\t- BaseCap :%d\n", room_attributes.base_prod_cap);
    append_snprintf(p, size, "\t- BaseCost:%3.f%%\n", room_attributes.base_char_cost * 100);
    append_snprintf(p, size, "\t- ProdCnt :%d\n", room_attributes.prod_cnt);
    return buf;
}
bool RoomBuffTargetValidator::validate(const RoomModel *)
{
    return true;
}
ProdTypeSelector::ProdTypeSelector(ProdType prod_type) : prod_type_(prod_type)
{
}
bool ProdTypeSelector::validate(const RoomModel *room)
{
    return room->room_attributes.prod_type == prod_type_;
}
} // namespace albc::model::buff
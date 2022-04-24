#pragma once

#include "model_buff.h"
#include "albc_types.h"
#include "util_time.h"

namespace albc::model::buff
{
void init_buffs(Dictionary<std::string, RoomBuff *> &buffs);

class BuffMap : public Dictionary<std::string, RoomBuff *>
{
  public:
    static std::shared_ptr<const Dictionary<std::string, RoomBuff *>> instance();

    virtual ~BuffMap();

  private:
    BuffMap();
};
} // namespace albc::buff_map
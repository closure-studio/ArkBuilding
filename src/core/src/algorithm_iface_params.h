//
// Created by Nonary on 2022/2/15.
//
#pragma once
#include "algorithm_iface_custom.h"
#include "data_building.h"
#include "data_player.h"
#include "data_player_building.h"
#include "model_buff_primitives.h"
#include "model_operator.h"

#include <bitset>
#include <unordered_set>

namespace albc::algorithm::iface
{

using PlayerBuildingRoomMap = Array<mem::PtrVector<model::buff::RoomModel>, data::building::kRoomTypeCount>;

model::buff::GlobalAttributeFields GlobalAttributeFactory(const data::player::PlayerBuilding &building);

std::unique_ptr<model::buff::RoomModel> RoomFactory(
    const std::string &id, const data::player::PlayerBuildingManufacture &manufacture_room, int level);

std::unique_ptr<model::buff::RoomModel> RoomFactory(const std::string &id,
                                                           const data::player::PlayerBuildingTrading &trading_room,
                                                           int level);

std::unique_ptr<model::buff::RoomModel> RoomFactory(const CustomRoomData &room_data);

[[maybe_unused]] void GenTestModePlayerData(data::player::PlayerDataModel &player_data,
                                                   const data::building::BuildingData &building_data);

class AlgorithmParams
{
  public:
    AlgorithmParams(const data::player::PlayerDataModel &player_data, const data::building::BuildingData &building_data);

    AlgorithmParams(const CustomPackedInput &custom_input, const data::building::BuildingData &building_data);

    [[maybe_unused]] void UpdateGlobalAttributes(const model::buff::GlobalAttributeFields &global_attr) const;

    [[nodiscard]] const mem::PtrVector<model::buff::RoomModel> &GetRoomsOfType(data::building::RoomType type) const;

    [[nodiscard]] const mem::PtrVector<model::OperatorModel> &GetOperators() const
    {
        return operators_;
    }

  private:
    PlayerBuildingRoomMap rooms_map_;
    mem::PtrVector<model::OperatorModel> operators_;

    [[nodiscard]] static int GetRoomTypeIndex(data::building::RoomType type);

    void AddRoom(data::building::RoomType type, std::unique_ptr<model::buff::RoomModel> &&room);
};
} // namespace albc::algorithm::iface
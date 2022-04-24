#pragma once
#include "api_di.h"
#include "api_util.h"
#include "albc_config.h"
#include "albc_types.h"
#include "albc/albc.h"
#include "util_locale.h"

#include "algorithm_iface.h"
#include "algorithm_iface_params.h"
#include "algorithm.h"
#include "algorithm_consts.h"

#include <numeric>

namespace albc
{

class String::Impl : public std::string
{
  public:
    Impl() = default;

    explicit Impl(const char* str);

    Impl(const Impl& other) = default;
};

template <typename T>
struct [[maybe_unused]] ICollectionVectorImpl : public ICollection<T>, public Vector<T>
{
  protected:
    [[nodiscard]] T* begin_ptr() const { return const_cast<T*>(this->data()); }
    [[nodiscard]] T* end_ptr() const { return const_cast<T*>(this->data() + this->size()); }

  public:
    using Vector<T>::Vector;
    using typename ICollection<T>::const_iterator;
    using typename ICollection<T>::iterator;

    explicit ICollectionVectorImpl(const ICollection<T>* other)
    {
        for (auto const& item : *other)
            this->push_back(item);
    }

    [[nodiscard]] const_iterator begin() const noexcept override
    {
        return const_iterator(this->begin_ptr());
    }
    [[nodiscard]] const_iterator end() const noexcept override
    {
        return const_iterator(this->end_ptr());
    }
    iterator begin() noexcept override
    {
        return iterator(this->begin_ptr());
    }
    iterator end() noexcept override
    {
        return iterator(this->end_ptr());
    }

};

class [[maybe_unused]] CharQueryImpl : public ICharQuery
{
  public:
    data::game::ISkillLookupTable::CharQueryItem item;
    String name;

    [[nodiscard]] bool IsValid() const noexcept override;
    [[nodiscard]] String Id() const noexcept override;
    [[nodiscard]] String Name() const noexcept override;
    [[nodiscard]] int Phase() const noexcept override;
    [[nodiscard]] int Level() const noexcept override;
};

class ResultImpl: public IResult
{
  public:
    int status;
    ICollectionVectorImpl<IRoomResult*>*rooms;

    ResultImpl(int status_val, ICollectionVectorImpl<IRoomResult*>* rooms_val);

    [[nodiscard]] int GetStatus() const noexcept override;
    [[nodiscard]] ICollection<IRoomResult *>* GetRoomDetails() const noexcept override;
    ~ResultImpl() override;
};

class RoomResultImpl: public IRoomResult
{
    String id_;
    ICollectionVectorImpl<String>* char_identifiers_;
    double estimated_score_;
    double duration_;
    String readable_info_;

  public:
    RoomResultImpl(
        String id_val,
        ICollectionVectorImpl<String>* char_identifiers_val,
        double estimated_score_val,
        double duration_val,
        String readable_info_val);

    [[nodiscard]] ICollection<String> *GetCharacterIdentifiers() const noexcept override;
    [[nodiscard]] double GetScore() const noexcept override;
    [[nodiscard]] double GetDuration() const noexcept override;
    [[nodiscard]] String GetIdentifier() const noexcept override;
    [[nodiscard]] String GetReadableInfo() const noexcept override;
    ~RoomResultImpl() noexcept override;
};

class Character::Impl
{
    algorithm::iface::CustomCharacter character_ {api::di::Resolve<data::game::ICharacterResolver>(),
                                       api::di::Resolve<data::game::CharacterMetaTable>()};

    std::optional<algorithm::iface::CustomCharacterData> character_data_;
    std::string cached_identifier_;
  public:
    void SetIdentifier(const std::string& identifier);

    void AssignGameDataId(const std::string &id);

    void AssignName(const std::string &name);

    void SetLevel(int phase_val, int level_val);

    void AddSkillByName(const std::string &name);

    void AddSkillByGameDataId(const std::string &id);

    void SetMorale(double morale_val);

    [[nodiscard]] bool CheckPrepared() const;

    bool Prepare();

    [[nodiscard]] std::string GetIdentifier() const;

    [[nodiscard]] std::optional<algorithm::iface::CustomCharacterData> GetCharacterData() const;

    void EnsurePrepared() const;
};

class Room::Impl
{
    algorithm::iface::CustomRoom room_{};
    std::optional<algorithm::iface::CustomRoomData> room_data_;
    std::string cached_identifier_;
  public:
    void SetIdentifier(const std::string& identifier);

    void SetType(const data::building::RoomType& type);

    void SetDblParam(AlbcRoomParamType type, double value);

    [[nodiscard]] bool Prepare();

    [[nodiscard]] std::string GetIdentifier() const;

    [[nodiscard]] std::optional<algorithm::iface::CustomRoomData> GetRoomData() const;

    void EnsurePrepared() const;
};

class Model::Impl
{
  private:
    enum class ModelCreateType
    {
        FROM_EMPTY,
        FROM_JSON,
    };

    std::shared_ptr<data::building::BuildingData> building_data_ = api::di::Resolve<data::building::BuildingData>();
    std::unique_ptr<data::player::PlayerDataModel> player_data_;
    Vector<Character*> characters_;
    Vector<Room*> rooms_;
    ModelCreateType create_type_;

  public:
    Array<double, util::enum_size<AlbcModelParamType>::value> model_parameters{};

    explicit Impl(const Json::Value &player_data_json);

    static Impl* CreateFromFile(const char *player_data_path);

    static Impl* CreateFromJson(const char* player_data_json);

    Impl();

    ~Impl();

    void AddCharacter(Character *character);

    void AddRoom(Room *room);

    void RemoveCharacter(Character *character);

    void RemoveRoom(Room *room);

    void EnsurePrepared() const;

    [[nodiscard]] algorithm::iface::AlgorithmParams CreateAlgParams() const;

    [[nodiscard]] IResult *GetResult() const;
};

}
//
// Created by Nonary on 2022/4/24.
//
#include "api_impl.h"

namespace albc
{

String::Impl::Impl(const char *str) : std::string(util::ToTargetLocale(str))
{

}
bool CharQueryImpl::IsValid() const noexcept
{
    return item.HasContent();
}
String CharQueryImpl::Id() const noexcept
{
    return String { item.id.c_str() };
}
String CharQueryImpl::Name() const noexcept
{
    return name;
}
int CharQueryImpl::Phase() const noexcept
{
    return static_cast<int>(item.phase);
}
int CharQueryImpl::Level() const noexcept
{
    return item.level;
}
ResultImpl::ResultImpl(int status_val, ICollectionVectorImpl<IRoomResult *> *rooms_val)
    : status(status_val), rooms(rooms_val)
{
}
int ResultImpl::GetStatus() const noexcept
{
    return status;
}
ICollection<IRoomResult *> *ResultImpl::GetRoomDetails() const noexcept
{
    return rooms;
}
ResultImpl::~ResultImpl()
{
    mem::free_ptr_vector(*rooms);
    delete rooms;
}
RoomResultImpl::RoomResultImpl(String id_val, ICollectionVectorImpl<String> *char_identifiers_val,
                               double estimated_score_val, double duration_val, String readable_info_val)
    : id_(std::move(id_val)),
      char_identifiers_(char_identifiers_val),
      estimated_score_(estimated_score_val),
      duration_(duration_val),
      readable_info_(std::move(readable_info_val))
{
}
ICollection<String> *RoomResultImpl::GetCharacterIdentifiers() const noexcept
{
    return char_identifiers_;
}
double RoomResultImpl::GetScore() const noexcept
{
    return estimated_score_;
}
double RoomResultImpl::GetDuration() const noexcept
{
    return duration_;
}
String RoomResultImpl::GetIdentifier() const noexcept
{
    return id_;
}
String RoomResultImpl::GetReadableInfo() const noexcept
{
    return readable_info_;
}
RoomResultImpl::~RoomResultImpl() noexcept
{
    delete char_identifiers_;
}
void Character::Impl::SetIdentifier(const std::string &identifier)
{
    if (identifier.empty())
        throw std::invalid_argument("identifier cannot be empty");

    character_.SetIdentifier(identifier);
    cached_identifier_ = identifier;
}
void Character::Impl::AssignGameDataId(const std::string &id)
{
    if (id.empty())
        throw std::invalid_argument("id cannot be empty");

    character_.SetIdResolveCond(id, data::game::CharIdentifierType::ID);
}
void Character::Impl::AssignName(const std::string &name)
{
    if (name.empty())
        throw std::invalid_argument("name cannot be empty");

    character_.SetIdResolveCond(name, data::game::CharIdentifierType::NAME);
}
void Character::Impl::SetLevel(int phase_val, int level_val)
{
    if (phase_val < 0 || phase_val > 2 || level_val < 0)
    {
        throw std::invalid_argument("invalid arguments");
    }

    character_.SetLevelCond((data::EvolvePhase) phase_val, level_val);
}
void Character::Impl::AddSkillByName(const std::string &name)
{
    if (name.empty())
        throw std::invalid_argument("name cannot be empty");

    character_.AddSkillByName(name);
}
void Character::Impl::AddSkillByGameDataId(const std::string &id)
{
    if (id.empty())
        throw std::invalid_argument("id cannot be empty");

    character_.AddSkillById(id);
}
void Character::Impl::SetMorale(double morale_val)
{
    if (morale_val < 0 || morale_val > 24)
        throw std::invalid_argument("invalid arguments");

    character_.SetMorale(morale_val);
}
bool Character::Impl::CheckPrepared() const
{
    return character_data_.has_value();
}
bool Character::Impl::Prepare()
{
    character_data_ = character_.GenerateCharacterData();
    return character_data_.has_value();
}
std::string Character::Impl::GetIdentifier() const
{
    return cached_identifier_;
}
std::optional<algorithm::iface::CustomCharacterData> Character::Impl::GetCharacterData() const
{
    return character_data_;
}
void Character::Impl::EnsurePrepared() const
{
    if (!CheckPrepared())
        throw std::runtime_error("Character not prepared: " + cached_identifier_);
}
void Room::Impl::SetIdentifier(const std::string &identifier)
{
    if (identifier.empty())
        throw std::invalid_argument("identifier cannot be empty");

    room_.SetIdentifier(identifier);
    cached_identifier_ = identifier;
}
void Room::Impl::SetType(const data::building::RoomType &type)
{
    room_.SetType(type);
}
void Room::Impl::SetDblParam(const AlbcRoomParamType type, const double value)
{
    switch (type)
    {
    case ALBC_ROOM_PARAM_SLOT_COUNT:
        room_.SetMaxSlotCnt((int)value);
        break;
    case ALBC_ROOM_PARAM_PRODUCT_TYPE:
        room_.room_attributes.prod_type = (model::buff::ProdType)value;
        break;
    case ALBC_ROOM_PARAM_ORDER_TYPE:
        room_.room_attributes.order_type = (model::buff::OrderType)value;
        break;
    case ALBC_ROOM_PARAM_PRODUCT_COUNT:
        room_.room_attributes.prod_cnt = (int)value;
        break;
    case ALBC_ROOM_PARAM_BASE_PRODUCT_CAP:
        room_.room_attributes.base_prod_cap = (int)value;
        break;
    case ALBC_ROOM_PARAM_BASE_CHAR_COST:
        room_.room_attributes.base_char_cost = value;
        break;
    case ALBC_ROOM_PARAM_BASE_PROD_EFF:
        room_.room_attributes.base_prod_eff = value;
        break;
    default:
        throw std::invalid_argument("invalid argument: " + std::string(util::enum_to_string(type)));
    }
}
bool Room::Impl::Prepare()
{
    room_data_ = room_.GenerateRoomData();
    return room_data_.has_value();
}
std::string Room::Impl::GetIdentifier() const
{
    return cached_identifier_;
}
std::optional<algorithm::iface::CustomRoomData> Room::Impl::GetRoomData() const
{
    return room_data_;
}
void Room::Impl::EnsurePrepared() const
{
    if (!room_data_.has_value())
        throw std::runtime_error("Room not prepared: " + cached_identifier_);
}
Model::Impl::Impl(const Json::Value &player_data_json)
    : player_data_(mem::make_unique_nothrow<data::player::PlayerDataModel>(player_data_json))
{
    create_type_ = ModelCreateType::FROM_JSON;
}
Model::Impl *Model::Impl::CreateFromFile(const char *player_data_path)
{
    return new Impl(util::read_json_from_file(player_data_path));
}
Model::Impl *Model::Impl::CreateFromJson(const char *player_data_json)
{
    return new Impl(util::read_json_from_char_array(player_data_json));
}
Model::Impl::Impl()
{
    player_data_ = nullptr;
    create_type_ = ModelCreateType::FROM_EMPTY;
}
Model::Impl::~Impl()
{
    LOG_D("Destroying model. Freeing ", characters_.size(), " characters and ", rooms_.size(), " rooms.");
    mem::free_ptr_vector(characters_);
    mem::free_ptr_vector(rooms_);
    LOG_D("Model destroyed.");
}
void Model::Impl::AddCharacter(Character *character)
{
    characters_.push_back(character);
}
void Model::Impl::AddRoom(Room *room)
{
    rooms_.push_back(room);
}
void Model::Impl::RemoveCharacter(Character *character)
{
    characters_.erase(std::remove(characters_.begin(), characters_.end(), character), characters_.end());
}
void Model::Impl::RemoveRoom(Room *room)
{
    rooms_.erase(std::remove(rooms_.begin(), rooms_.end(), room), rooms_.end());
}
void Model::Impl::EnsurePrepared() const
{
    for (const auto& room : rooms_)
    {
        room->impl()->EnsurePrepared();
    }

    for (const auto& character : characters_)
    {
        character->impl()->EnsurePrepared();
    }
}
algorithm::iface::AlgorithmParams Model::Impl::CreateAlgParams() const
{
    if (create_type_ == ModelCreateType::FROM_JSON)
    {
        return {*player_data_, *building_data_};
    }
    EnsurePrepared();
    algorithm::iface::CustomPackedInput input;
    for (const auto& room : rooms_)
    {
        input.rooms.push_back(room->impl()->GetRoomData().value());
    }

    for (const auto& character : characters_)
    {
        input.characters.push_back(character->impl()->GetCharacterData().value());
    }

    return { input, *building_data_ };
}
IResult *Model::Impl::GetResult() const
{
    using namespace algorithm::iface;
    using namespace algorithm;

    AlgorithmParams params = CreateAlgParams();
    LOG_D("Creating algorithm params: ", params.GetOperators().size(), " operators, ");
    const auto sc = SCOPE_TIMER_WITH_TRACE("Solving");
    const auto i_runner = api::di::Resolve<algorithm::iface::IRunner>();
    AlbcSolverParameters sp;
    sp.model_time_limit = util::read_attribute(model_parameters, ALBC_MODEL_PARAM_DURATION);
    sp.solve_time_limit = util::read_attribute(model_parameters, ALBC_MODEL_PARAM_SOLVE_TIME_LIMIT);

    if (sp.model_time_limit <= 0) sp.model_time_limit = kDefaultModelTimeLimit;
    if (sp.solve_time_limit <= 0) sp.solve_time_limit = kDefaultSolveTimeLimit;
    AlgorithmResult alg_result;
    i_runner->Run(params, sp, alg_result);

    auto result = new ResultImpl(0, new ICollectionVectorImpl<IRoomResult*>());
    for (const auto& alg_room_result: alg_result.rooms)
    {
        auto ops = new ICollectionVectorImpl<String>();
        for (const auto* op: alg_room_result.solution.operators)
        {
            if(op)
            {
                ops->emplace_back(op->identifier.c_str());
            }
        }
        auto* room_result = new RoomResultImpl(
            String(alg_room_result.room->id.c_str()),
            ops,
            alg_room_result.solution.productivity,
            alg_room_result.solution.duration,
            String(alg_room_result.room->to_string()
                       .append("\n")
                       .append(alg_room_result.solution.ToString()).c_str()));

        result->rooms->push_back(room_result);
    }
    return result;
}
}
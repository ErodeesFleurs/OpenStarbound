import std;
import star.string;
import star.format;
import star.hash;
import star.json;
import star.json_extra;
import star.strong_typedef;
import star.data_stream;
import star.uuid;
import star.vector;
import star.celestial_coordinate;
import star.bi_map;
import star.game_types;
import star.warping;
import star.data_stream_extra;
import star.lexical_cast;

namespace Star {

extern EnumMap<WarpMode> const WarpModeNames {
  {WarpMode::None, "None"},
  {WarpMode::BeamOnly, "BeamOnly"},
  {WarpMode::DeployOnly, "DeployOnly"},
  {WarpMode::BeamOrDeploy, "BeamOrDeploy"}
};

template <>
ClientShipWorldId DataStream::read<ClientShipWorldId>() {
  return ClientShipWorldId(read<Uuid>());
}

template <>
ClientCustomWorldId DataStream::read<ClientCustomWorldId>() {
  auto uuid = read<Uuid>();
  auto name = read<String>();
  return ClientCustomWorldId(std::move(uuid), std::move(name));
}

template <>
WarpToPlayer DataStream::read<WarpToPlayer>() {
  return WarpToPlayer(read<Uuid>());
}

InstanceWorldId::InstanceWorldId() {}

InstanceWorldId::InstanceWorldId(String instance, std::optional<Uuid> uuid, std::optional<float> level)
  : instance(std::move(instance)), uuid(std::move(uuid)), level(std::move(level)) {}

bool InstanceWorldId::operator==(InstanceWorldId const& rhs) const {
  return std::tie(instance, uuid, level) == std::tie(rhs.instance, rhs.uuid, rhs.level);
}

bool InstanceWorldId::operator<(InstanceWorldId const& rhs) const {
  return std::tie(instance, uuid, rhs.level) < std::tie(rhs.instance, rhs.uuid, rhs.level);
}

std::size_t hash<InstanceWorldId>::operator()(InstanceWorldId const& id) const {
  return hashOf(id.instance, id.uuid, id.level);
}

DataStream& operator>>(DataStream& ds, InstanceWorldId& instanceWorldId) {
  ds >> instanceWorldId.instance;
  ds >> instanceWorldId.uuid;
  ds >> instanceWorldId.level;
  return ds;
}

DataStream& operator<<(DataStream& ds, InstanceWorldId const& instanceWorldId) {
  ds << instanceWorldId.instance;
  ds << instanceWorldId.uuid;
  ds << instanceWorldId.level;
  return ds;
}


ClientCustomWorldId::ClientCustomWorldId(Uuid uuid, String name)
  : uuid(std::move(uuid)), name(std::move(name)) {}

bool ClientCustomWorldId::operator==(ClientCustomWorldId const& rhs) const {
  return std::tie(uuid, name) == std::tie(rhs.uuid, rhs.name);
}

bool ClientCustomWorldId::operator<(ClientCustomWorldId const& rhs) const {
  return std::tie(uuid, name) < std::tie(rhs.uuid, rhs.name);
}

std::size_t hash<ClientCustomWorldId>::operator()(ClientCustomWorldId const& id) const {
  return hashOf(id.uuid, id.name);
}

DataStream& operator>>(DataStream& ds, ClientCustomWorldId& clientWorldId) {
  ds >> clientWorldId.uuid;
  ds >> clientWorldId.name;
  return ds;
}

DataStream& operator<<(DataStream& ds, ClientCustomWorldId const& clientWorldId) {
  ds << clientWorldId.uuid;
  ds << clientWorldId.name;
  return ds;
}

String printWorldId(WorldId const& worldId) {
  if (auto instanceWorldId = std::get_if<InstanceWorldId>(&worldId)) {
    if (instanceWorldId->level && *instanceWorldId->level < 0.0f)
      throw StarException::format("InstanceWorldId level component cannot be negative");

    String uuidPart = instanceWorldId->uuid ? instanceWorldId->uuid->hex() : "-";
    String levelPart = instanceWorldId->level ? toString(*instanceWorldId->level) : "-";
    return strf("InstanceWorld:{}:{}:{}", instanceWorldId->instance, uuidPart, levelPart);
  } else if (auto celestialWorldId = std::get_if<CelestialWorldId>(&worldId)) {
    return strf("CelestialWorld:{}", *celestialWorldId);
  } else if (auto clientShipWorldId = std::get_if<ClientShipWorldId>(&worldId)) {
    return strf("ClientShipWorld:{}", clientShipWorldId->hex());
  } else if (auto customWorldId = std::get_if<CustomWorldId>(&worldId)) {
    return strf("CustomWorld:{}", *customWorldId);
  } else if (auto clientCustomWorldId = std::get_if<ClientCustomWorldId>(&worldId)) {
    return strf("ClientCustomWorld:{}:{}", clientCustomWorldId->uuid.hex(), clientCustomWorldId->name);
  } else {
    return "Nowhere";
  }
}

WorldId parseWorldId(String const& printedId) {
  if (printedId.empty())
    return WorldId();

  auto parts = printedId.split(':', 1);
  String const& type = parts.at(0);

  if (type.equalsIgnoreCase("InstanceWorld")) {
    auto rest = parts.at(1).split(":", 2);
    if (rest.size() == 0 || rest.size() > 3)
      throw StarException::format("Wrong number of parts in InstanceWorldId");
    auto getOptPart = [](String part) -> std::optional<String> {
      if (part.empty() || part == "-")
        return {};
      return part;
    };

    InstanceWorldId instanceWorldId{rest.at(0), {}, {}};
    if (rest.size() > 1) {
      if (auto uuid = getOptPart(rest.at(1)))
        instanceWorldId.uuid = Uuid(*uuid);
      if (rest.size() > 2) {
        if (auto level = getOptPart(rest.at(2))) {
          instanceWorldId.level = lexicalCast<float>(*level);
          if (*instanceWorldId.level < 0)
            throw StarException::format("InstanceWorldId level component cannot be negative");
        }
      }
    }

    return instanceWorldId;
  } else if (type.equalsIgnoreCase("CelestialWorld")) {
    return CelestialWorldId(CelestialCoordinate(parts.at(1)));
  } else if (type.equalsIgnoreCase("ClientShipWorld")) {
    return ClientShipWorldId(Uuid(parts.at(1)));
  } else if (type.equalsIgnoreCase("CustomWorld")) {
    return CustomWorldId(parts.at(1));
  } else if (type.equalsIgnoreCase("ClientCustomWorld")) {
    auto rest = parts.at(1).split(":", 1);
    if (rest.size() <= 1 || rest.size() > 2)
      throw StarException::format("Wrong number of parts in ClientCustomWorldId");
    
    return ClientCustomWorldId(Uuid(rest.at(0)),rest.at(1));
  } else if (type.equalsIgnoreCase("Nowhere")) {
    return {};
  } else {
    throw StarException::format("Improper WorldId type '{}'", type);
  }
}

std::ostream& operator<<(std::ostream& os, CelestialWorldId const& worldId) {
  os << (CelestialCoordinate)worldId;
  return os;
}


std::ostream& operator<<(std::ostream& os, ClientShipWorldId const& worldId) {
  os << ((Uuid)worldId).hex();
  return os;
}

std::ostream& operator<<(std::ostream& os, InstanceWorldId const& worldId) {
  os << printWorldId(worldId);
  return os;
}

std::ostream& operator<<(std::ostream& os, CustomWorldId const& worldId) {
  os << (String)worldId;
  return os;
}

std::ostream& operator<<(std::ostream& os, ClientCustomWorldId const& worldId) {
  os << printWorldId(worldId);
  return os;
}

std::ostream& operator<<(std::ostream& os, WorldId const& worldId) {
  os << printWorldId(worldId);
  return os;
}

Json spawnTargetToJson(SpawnTarget spawnTarget) {
  if (std::holds_alternative<SpawnTargetUniqueEntity>(spawnTarget))
    return std::get<SpawnTargetUniqueEntity>(spawnTarget);
  else if (std::holds_alternative<SpawnTargetPosition>(spawnTarget))
    return jsonFromVec2F(std::get<SpawnTargetPosition>(spawnTarget));
  else if (std::holds_alternative<SpawnTargetX>(spawnTarget))
    return Json(std::get<SpawnTargetX>(spawnTarget));
  else
    return Json();
}

SpawnTarget spawnTargetFromJson(Json v) {
  if (v.isNull())
    return {};
  else if (v.isType(Json::Type::String))
    return SpawnTargetUniqueEntity(v.toString());
  else if (v.isType(Json::Type::Float))
    return SpawnTargetX(v.toFloat());
  else
    return SpawnTargetPosition(jsonToVec2F(v));
}

String printSpawnTarget(SpawnTarget spawnTarget) {
  if (auto str = std::get_if<SpawnTargetUniqueEntity>(&spawnTarget))
    return *str;
  else if (auto pos = std::get_if<SpawnTargetPosition>(&spawnTarget))
    return strf("{}.{}", (*pos)[0], (*pos)[1]);
  else if (auto x = std::get_if<SpawnTargetX>(&spawnTarget))
    return toString(x->t);
  else
    return "";
}

WarpToWorld::WarpToWorld() {}

WarpToWorld::WarpToWorld(WorldId world, SpawnTarget target) : world(std::move(world)), target(std::move(target)) {}

WarpToWorld::WarpToWorld(Json v) {
  if (v) {
    world = parseWorldId(v.getString("world"));
    target = spawnTargetFromJson(v.get("target"));
  }
}

bool WarpToWorld::operator==(WarpToWorld const& rhs) const {
  return std::tie(world, target) == std::tie(rhs.world, rhs.target);
}

WarpToWorld::operator bool() const {
  return !std::holds_alternative<std::monostate>(world);
}

Json WarpToWorld::toJson() const {
  return JsonObject{{"world", printWorldId(world)}, {"target", spawnTargetToJson(target)}};
}

WarpAction parseWarpAction(String const& warpString) {
  if (warpString.equalsIgnoreCase("Return")) {
    return WarpAlias::Return;
  } else if (warpString.equalsIgnoreCase("OrbitedWorld")) {
    return WarpAlias::OrbitedWorld;
  } else if (warpString.equalsIgnoreCase("OwnShip")) {
    return WarpAlias::OwnShip;
  } else if (warpString.beginsWith("Player:", String::CaseSensitivity::CaseInsensitive)) {
    auto parts = warpString.split(':', 1);
    return WarpToPlayer(Uuid(parts.at(1)));
  } else {
    auto parts = warpString.split('=', 1);
    auto world = parseWorldId(parts.at(0));
    SpawnTarget target;
    if (parts.size() == 2) {
      auto const& targetPart = parts.at(1);
      if (targetPart.regexMatch("\\d+.\\d+")) {
        auto pos = targetPart.split(".", 1);
        target = SpawnTargetPosition(Vec2F(lexicalCast<int>(pos.at(0)), lexicalCast<int>(pos.at(1))));
      } else if (targetPart.regexMatch("\\d+")) {
        target = SpawnTargetX(lexicalCast<int>(targetPart));
      } else {
        target = SpawnTargetUniqueEntity(targetPart);
      }
    }
    return WarpToWorld(world, target);
  }
}

String printWarpAction(WarpAction const& warpAction) {
  if (auto warpAlias = std::get_if<WarpAlias>(&warpAction)) {
    if (*warpAlias == WarpAlias::Return)
      return "Return";
    else if (*warpAlias == WarpAlias::OrbitedWorld)
      return "OrbitedWorld";
    else if (*warpAlias == WarpAlias::OwnShip)
      return "OwnShip";
  } else if (auto warpToPlayer = std::get_if<WarpToPlayer>(&warpAction)) {
    return strf("Player:{}", warpToPlayer->hex());
  } else if (auto warpToWorld = std::get_if<WarpToWorld>(&warpAction)) {
    auto toWorldString = printWorldId(warpToWorld->world);
    if (!std::holds_alternative<std::monostate>(warpToWorld->target))
      toWorldString = strf("{}={}", toWorldString, printSpawnTarget(warpToWorld->target));
    return toWorldString;
  }

  return "UnknownWarpAction";
}

JsonObject warpActionToJson(WarpAction const& warpAction) {
  if (auto warpAlias = std::get_if<WarpAlias>(&warpAction)) {
    auto out = JsonObject{
      {"actionKind", "Alias"}
    };
    if (*warpAlias == WarpAlias::Return)
      out.set("actionAlias","Return");
    else if (*warpAlias == WarpAlias::OrbitedWorld)
      out.set("actionAlias","OrbitedWorld");
    else if (*warpAlias == WarpAlias::OwnShip)
      out.set("actionAlias","OwnShip");
    return out;
  } else if (auto warpToPlayer = std::get_if<WarpToPlayer>(&warpAction)) {
    return JsonObject{
      {"actionKind", "Player"},
      {"uuid", warpToPlayer->hex()}
    };
  } else if (auto warpToWorld = std::get_if<WarpToWorld>(&warpAction)) {
    auto out = JsonObject{
      {"actionKind", "World"},
      {"worldId", printWorldId(warpToWorld->world)}
    };
    if (!std::holds_alternative<std::monostate>(warpToWorld->target))
      out.set("spawnTarget", spawnTargetToJson(warpToWorld->target));
    return out;
  }

  return JsonObject{
      {"actionKind", "UnknownWarpAction"}
  };
}

DataStream& operator>>(DataStream& ds, WarpToWorld& warpToWorld) {
  ds >> warpToWorld.world;
  ds >> warpToWorld.target;
  return ds;
}

DataStream& operator<<(DataStream& ds, WarpToWorld const& warpToWorld) {
  ds << warpToWorld.world;
  ds << warpToWorld.target;
  return ds;
}

}

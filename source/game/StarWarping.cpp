#include "StarWarping.hpp"
#include "StarDataStreamExtra.hpp"
#include "StarJsonExtra.hpp"
#include "StarLexicalCast.hpp"

import std;

namespace Star {

EnumMap<WarpMode> WarpModeNames{
  {WarpMode::None, "None"},
  {WarpMode::BeamOnly, "BeamOnly"},
  {WarpMode::DeployOnly, "DeployOnly"},
  {WarpMode::BeamOrDeploy, "BeamOrDeploy"}};

InstanceWorldId::InstanceWorldId() = default;

InstanceWorldId::InstanceWorldId(String instance, std::optional<Uuid> uuid, std::optional<float> level)
    : instance(std::move(instance)), uuid(std::move(uuid)), level(std::move(level)) {}

auto InstanceWorldId::operator==(InstanceWorldId const& rhs) const -> bool {
  return tie(instance, uuid, level) == tie(rhs.instance, rhs.uuid, rhs.level);
}

auto InstanceWorldId::operator<(InstanceWorldId const& rhs) const -> bool {
  return tie(instance, uuid, rhs.level) < tie(rhs.instance, rhs.uuid, rhs.level);
}

auto hash<InstanceWorldId>::operator()(InstanceWorldId const& id) const -> size_t {
  return hashOf(id.instance, id.uuid, id.level);
}

auto operator>>(DataStream& ds, InstanceWorldId& instanceWorldId) -> DataStream& {
  ds >> instanceWorldId.instance;
  ds >> instanceWorldId.uuid;
  ds >> instanceWorldId.level;
  return ds;
}

auto operator<<(DataStream& ds, InstanceWorldId const& instanceWorldId) -> DataStream& {
  ds << instanceWorldId.instance;
  ds << instanceWorldId.uuid;
  ds << instanceWorldId.level;
  return ds;
}

auto printWorldId(WorldId const& worldId) -> String {
  if (auto instanceWorldId = worldId.ptr<InstanceWorldId>()) {
    if (instanceWorldId->level && *instanceWorldId->level < 0.0f)
      throw StarException::format("InstanceWorldId level component cannot be negative");

    String uuidPart = instanceWorldId->uuid ? instanceWorldId->uuid->hex() : "-";
    String levelPart = instanceWorldId->level ? toString(*instanceWorldId->level) : "-";
    return strf("InstanceWorld:{}:{}:{}", instanceWorldId->instance, uuidPart, levelPart);
  } else if (auto celestialWorldId = worldId.ptr<CelestialWorldId>()) {
    return strf("CelestialWorld:{}", *celestialWorldId);
  } else if (auto clientShipWorldId = worldId.ptr<ClientShipWorldId>()) {
    return strf("ClientShipWorld:{}", clientShipWorldId->get().hex());
  } else {
    return "Nowhere";
  }
}

auto parseWorldId(String const& printedId) -> WorldId {
  if (printedId.empty())
    return {};

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

    InstanceWorldId instanceWorldId{rest.at(0)};
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
  } else if (type.equalsIgnoreCase("Nowhere")) {
    return {};
  } else {
    throw StarException::format("Improper WorldId type '{}'", type);
  }
}

auto operator<<(std::ostream& os, CelestialWorldId const& worldId) -> std::ostream& {
  os << (CelestialCoordinate)worldId;
  return os;
}

auto operator<<(std::ostream& os, ClientShipWorldId const& worldId) -> std::ostream& {
  os << ((Uuid)worldId).hex();
  return os;
}

auto operator<<(std::ostream& os, InstanceWorldId const& worldId) -> std::ostream& {
  os << printWorldId(worldId);
  return os;
}

auto operator<<(std::ostream& os, WorldId const& worldId) -> std::ostream& {
  os << printWorldId(worldId);
  return os;
}

auto spawnTargetToJson(SpawnTarget spawnTarget) -> Json {
  if (spawnTarget.is<SpawnTargetUniqueEntity>())
    return spawnTarget.get<SpawnTargetUniqueEntity>().get();
  else if (spawnTarget.is<SpawnTargetPosition>())
    return jsonFromVec2F(spawnTarget.get<SpawnTargetPosition>().get());
  else if (spawnTarget.is<SpawnTargetX>())
    return {spawnTarget.get<SpawnTargetX>().get()};
  else
    return {};
}

auto spawnTargetFromJson(Json v) -> SpawnTarget {
  if (v.isNull())
    return {};
  else if (v.isType(Json::Type::String))
    return SpawnTargetUniqueEntity(v.toString());
  else if (v.isType(Json::Type::Float))
    return SpawnTargetX(v.toFloat());
  else
    return SpawnTargetPosition(jsonToVec2F(v));
}

auto printSpawnTarget(SpawnTarget spawnTarget) -> String {
  if (auto str = spawnTarget.ptr<SpawnTargetUniqueEntity>())
    return str->get();
  else if (auto pos = spawnTarget.ptr<SpawnTargetPosition>())
    return strf("{}.{}", (pos->get())[0], (pos->get())[1]);
  else if (auto x = spawnTarget.ptr<SpawnTargetX>())
    return toString(x->get());
  else
    return "";
}

WarpToWorld::WarpToWorld() = default;

WarpToWorld::WarpToWorld(WorldId world, SpawnTarget target) : world(std::move(world)), target(std::move(target)) {}

WarpToWorld::WarpToWorld(Json v) {
  if (v) {
    world = parseWorldId(v.getString("world"));
    target = spawnTargetFromJson(v.get("target"));
  }
}

auto WarpToWorld::operator==(WarpToWorld const& rhs) const -> bool {
  return tie(world, target) == tie(rhs.world, rhs.target);
}

WarpToWorld::operator bool() const {
  return (bool)world;
}

auto WarpToWorld::toJson() const -> Json {
  return JsonObject{{"world", printWorldId(world)}, {"target", spawnTargetToJson(target)}};
}

auto parseWarpAction(String const& warpString) -> WarpAction {
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

auto printWarpAction(WarpAction const& warpAction) -> String {
  if (auto warpAlias = warpAction.ptr<WarpAlias>()) {
    if (*warpAlias == WarpAlias::Return)
      return "Return";
    else if (*warpAlias == WarpAlias::OrbitedWorld)
      return "OrbitedWorld";
    else if (*warpAlias == WarpAlias::OwnShip)
      return "OwnShip";
  } else if (auto warpToPlayer = warpAction.ptr<WarpToPlayer>()) {
    return strf("Player:{}", warpToPlayer->get().hex());
  } else if (auto warpToWorld = warpAction.ptr<WarpToWorld>()) {
    auto toWorldString = printWorldId(warpToWorld->world);
    if (auto spawnTarget = warpToWorld->target)
      toWorldString = strf("{}={}", toWorldString, printSpawnTarget(spawnTarget));
    return toWorldString;
  }

  return "UnknownWarpAction";
}

auto warpActionToJson(WarpAction const& warpAction) -> JsonObject {
  if (auto warpAlias = warpAction.ptr<WarpAlias>()) {
    auto out = JsonObject{
      {"actionKind", "Alias"}};
    if (*warpAlias == WarpAlias::Return)
      out.set("actionAlias", "Return");
    else if (*warpAlias == WarpAlias::OrbitedWorld)
      out.set("actionAlias", "OrbitedWorld");
    else if (*warpAlias == WarpAlias::OwnShip)
      out.set("actionAlias", "OwnShip");
    return out;
  } else if (auto warpToPlayer = warpAction.ptr<WarpToPlayer>()) {
    return JsonObject{
      {"actionKind", "Player"},
      {"uuid", warpToPlayer->get().hex()}};
  } else if (auto warpToWorld = warpAction.ptr<WarpToWorld>()) {
    auto out = JsonObject{
      {"actionKind", "World"},
      {"worldId", printWorldId(warpToWorld->world)}};
    if (auto spawnTarget = warpToWorld->target)
      out.set("spawnTarget", spawnTargetToJson(spawnTarget));
    return out;
  }

  return JsonObject{
    {"actionKind", "UnknownWarpAction"}};
}

auto operator>>(DataStream& ds, WarpToWorld& warpToWorld) -> DataStream& {
  ds >> warpToWorld.world;
  ds >> warpToWorld.target;
  return ds;
}

auto operator<<(DataStream& ds, WarpToWorld const& warpToWorld) -> DataStream& {
  ds << warpToWorld.world;
  ds << warpToWorld.target;
  return ds;
}

}// namespace Star

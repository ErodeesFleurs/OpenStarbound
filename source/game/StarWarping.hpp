#pragma once

#include "StarBiMap.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarStrongTypedef.hpp"
#include "StarUuid.hpp"

import std;

namespace Star {

enum class WarpMode : std::uint8_t {
  None,
  BeamOnly,
  DeployOnly,
  BeamOrDeploy
};
extern EnumMap<WarpMode> WarpModeNames;

struct InstanceWorldId {
  String instance;
  std::optional<Uuid> uuid;
  std::optional<float> level;

  InstanceWorldId();
  InstanceWorldId(String instance, std::optional<Uuid> uuid = {}, std::optional<float> level = {});

  auto operator==(InstanceWorldId const& rhs) const -> bool;
  auto operator<(InstanceWorldId const& rhs) const -> bool;
};

template <>
struct hash<InstanceWorldId> {
  auto operator()(InstanceWorldId const& id) const -> std::size_t;
};

auto operator>>(DataStream& ds, InstanceWorldId& missionWorldId) -> DataStream&;
auto operator<<(DataStream& ds, InstanceWorldId const& missionWorldId) -> DataStream&;

using CelestialWorldId = StrongTypedef<CelestialCoordinate>;
using ClientShipWorldId = StrongTypedef<Uuid>;
using WorldId = MVariant<CelestialWorldId, ClientShipWorldId, InstanceWorldId>;

auto printWorldId(WorldId const& worldId) -> String;
auto parseWorldId(String const& printedId) -> WorldId;

// Same as outputting printWorldId
auto operator<<(std::ostream& os, CelestialWorldId const& worldId) -> std::ostream&;
auto operator<<(std::ostream& os, ClientShipWorldId const& worldId) -> std::ostream&;
auto operator<<(std::ostream& os, InstanceWorldId const& worldId) -> std::ostream&;
auto operator<<(std::ostream& os, WorldId const& worldId) -> std::ostream&;

using SpawnTargetUniqueEntity = StrongTypedef<String>;
using SpawnTargetPosition = StrongTypedef<Vec2F>;
using SpawnTargetX = StrongTypedefBuiltin<float>;
using SpawnTarget = MVariant<SpawnTargetUniqueEntity, SpawnTargetPosition, SpawnTargetX>;

auto spawnTargetToJson(SpawnTarget spawnTarget) -> Json;
auto spawnTargetFromJson(Json v) -> SpawnTarget;

auto printSpawnTarget(SpawnTarget spawnTarget) -> String;

struct WarpToWorld {
  WarpToWorld();
  explicit WarpToWorld(WorldId world, SpawnTarget spawn = {});
  explicit WarpToWorld(Json v);

  WorldId world;
  SpawnTarget target;

  auto operator==(WarpToWorld const& rhs) const -> bool;
  explicit operator bool() const;

  [[nodiscard]] auto toJson() const -> Json;
};

using WarpToPlayer = StrongTypedef<Uuid>;

enum class WarpAlias {
  Return,
  OrbitedWorld,
  OwnShip
};

using WarpAction = MVariant<WarpToWorld, WarpToPlayer, WarpAlias>;

auto parseWarpAction(String const& warpString) -> WarpAction;
auto printWarpAction(WarpAction const& warpAction) -> String;
auto warpActionToJson(WarpAction const& warpAction) -> JsonObject;

auto operator>>(DataStream& ds, WarpToWorld& warpToWorld) -> DataStream&;
auto operator<<(DataStream& ds, WarpToWorld const& warpToWorld) -> DataStream&;

}// namespace Star

template <>
struct std::formatter<Star::CelestialWorldId> : Star::ostream_formatter {};
template <>
struct std::formatter<Star::ClientShipWorldId> : Star::ostream_formatter {};
template <>
struct std::formatter<Star::InstanceWorldId> : Star::ostream_formatter {};
template <>
struct std::formatter<Star::WorldId> : Star::ostream_formatter {};
template <>
struct std::formatter<Star::WarpToWorld> : Star::ostream_formatter {};

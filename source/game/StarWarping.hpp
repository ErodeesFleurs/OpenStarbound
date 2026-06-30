#pragma once

#include "StarStrongTypedef.hpp"
#include "StarUuid.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarGameTypes.hpp"

namespace Star {

enum class WarpMode : uint8_t {
  None,
  BeamOnly,
  DeployOnly,
  BeamOrDeploy
};
extern EnumMap<WarpMode> WarpModeNames;

struct InstanceWorldId {
  String instance;
  Maybe<Uuid> uuid;
  Maybe<float> level;

  InstanceWorldId() = default;
  InstanceWorldId(String instance, Maybe<Uuid> uuid = {}, Maybe<float> level = {});

  bool operator==(InstanceWorldId const& other) const;
  [[nodiscard]] bool operator<(InstanceWorldId const& other) const;
};

template <>
struct hash<InstanceWorldId> {
  [[nodiscard]] size_t operator()(InstanceWorldId const& id) const;
};

DataStream& operator>>(DataStream& ds, InstanceWorldId& missionWorldId);
DataStream& operator<<(DataStream& ds, InstanceWorldId const& missionWorldId);

using CelestialWorldId = StrongTypedef<CelestialCoordinate, struct CelestialWorldIdTag>;
using ClientShipWorldId = StrongTypedef<Uuid, struct ClientShipWorldIdTag>;
using WorldId = MVariant<CelestialWorldId, ClientShipWorldId, InstanceWorldId>;

[[nodiscard]] String printWorldId(WorldId const& worldId);
[[nodiscard]] WorldId parseWorldId(String const& printedId);

// Same as outputting printWorldId
std::ostream& operator<<(std::ostream& os, CelestialWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, ClientShipWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, InstanceWorldId const& worldId);
std::ostream& operator<<(std::ostream& os, WorldId const& worldId);

using SpawnTargetUniqueEntity = StrongTypedef<String, struct SpawnTargetUniqueEntityTag>;
using SpawnTargetPosition = StrongTypedef<Vec2F, struct SpawnTargetPositionTag>;
using SpawnTargetX = StrongTypedefBuiltin<float, struct SpawnTargetXTag>;
using SpawnTarget = MVariant<SpawnTargetUniqueEntity, SpawnTargetPosition, SpawnTargetX>;

[[nodiscard]] Json spawnTargetToJson(SpawnTarget spawnTarget);
[[nodiscard]] SpawnTarget spawnTargetFromJson(Json v);

[[nodiscard]] String printSpawnTarget(SpawnTarget spawnTarget);

struct WarpToWorld {
  WarpToWorld() = default;
  explicit WarpToWorld(WorldId world, SpawnTarget spawn = {});
  explicit WarpToWorld(Json v);

  WorldId world;
  SpawnTarget target;

  [[nodiscard]] bool operator==(WarpToWorld const& rhs) const;
  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] Json toJson() const;
};

using WarpToPlayer = StrongTypedef<Uuid, struct WarpToPlayerTag>;

enum class WarpAlias {
  Return,
  OrbitedWorld,
  OwnShip
};

using WarpAction = MVariant<WarpToWorld, WarpToPlayer, WarpAlias>;

[[nodiscard]] WarpAction parseWarpAction(String const& warpString);
[[nodiscard]] String printWarpAction(WarpAction const& warpAction);
[[nodiscard]] JsonObject warpActionToJson(WarpAction const& warpAction);

DataStream& operator>>(DataStream& ds, WarpToWorld& warpToWorld);
DataStream& operator<<(DataStream& ds, WarpToWorld const& warpToWorld);

}

template <> struct std::formatter<Star::CelestialWorldId> : Star::OstreamFormatter {};
template <> struct std::formatter<Star::ClientShipWorldId> : Star::OstreamFormatter {};
template <> struct std::formatter<Star::InstanceWorldId> : Star::OstreamFormatter {};
template <> struct std::formatter<Star::WorldId> : Star::OstreamFormatter {};
template <> struct std::formatter<Star::WarpToWorld> : Star::OstreamFormatter {};

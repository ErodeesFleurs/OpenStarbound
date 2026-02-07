#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarSkyParameters.hpp"
#include "StarUuid.hpp"
#include "StarWarping.hpp"

import std;

namespace Star {

class Clock;
class SystemWorld;
class SystemObject;
class SystemWorldServer;

struct SystemObjectConfig;
// STAR_CLASS(Celestial);
// STAR_CLASS(ClientContext);
// STAR_CLASS(SystemClientShip);
// STAR_CLASS(SystemObject);

// STAR_STRUCT(SystemObjectConfig);

struct CelestialOrbit {
  static auto fromJson(Json const& json) -> CelestialOrbit;
  [[nodiscard]] auto toJson() const -> Json;

  CelestialCoordinate target;
  int direction;
  double enterTime;
  Vec2F enterPosition;

  void write(DataStream& ds) const;
  void read(DataStream& ds);

  auto operator==(CelestialOrbit const& rhs) const -> bool;
};
auto operator>>(DataStream& ds, CelestialOrbit& orbit) -> DataStream&;
auto operator<<(DataStream& ds, CelestialOrbit const& orbit) -> DataStream&;

// in transit, at a planet, orbiting a planet,, at a system object, or at a vector position
using SystemLocation = MVariant<CelestialCoordinate, CelestialOrbit, Uuid, Vec2F>;
auto jsonFromSystemLocation(SystemLocation const& location) -> Json;
auto jsonToSystemLocation(Json const& json) -> SystemLocation;

struct SystemWorldConfig {
  static auto fromJson(Json const& config) -> SystemWorldConfig;

  float starGravitationalConstant;
  float planetGravitationalConstant;

  Map<unsigned, float> planetSizes;
  float emptyOrbitSize;
  float unvisitablePlanetSize;
  StringMap<float> floatingDungeonWorldSizes;

  float starSize;
  Vec2F planetaryOrbitPadding;
  Vec2F satelliteOrbitPadding;

  Vec2F arrivalRange;

  float objectSpawnPadding;
  float clientObjectSpawnPadding;
  Vec2F objectSpawnInterval;
  double objectSpawnCycle;
  float minObjectOrbitTime;

  float asteroidBeamDistance;

  SkyParameters emptySkyParameters;
};

class SystemWorld {
public:
  SystemWorld(ConstPtr<Clock> universeClock, Ptr<CelestialDatabase> celestialDatabase);

  virtual ~SystemWorld() = default;

  [[nodiscard]] auto systemConfig() const -> SystemWorldConfig const&;
  [[nodiscard]] auto time() const -> double;
  [[nodiscard]] auto location() const -> Vec3I;
  [[nodiscard]] auto planets() const -> List<CelestialCoordinate>;

  [[nodiscard]] auto coordinateSeed(CelestialCoordinate const& coord, String const& seedMix) const -> std::uint64_t;
  [[nodiscard]] auto planetOrbitDistance(CelestialCoordinate const& coord) const -> float;
  // assumes circular orbit
  [[nodiscard]] auto orbitInterval(float distance, bool isMoon) const -> float;
  [[nodiscard]] auto orbitPosition(CelestialOrbit const& orbit) const -> Vec2F;
  [[nodiscard]] auto clusterSize(CelestialCoordinate const& planet) const -> float;
  [[nodiscard]] auto planetSize(CelestialCoordinate const& planet) const -> float;
  [[nodiscard]] auto planetPosition(CelestialCoordinate const& planet) const -> Vec2F;
  [[nodiscard]] auto systemLocationPosition(SystemLocation const& position) const -> std::optional<Vec2F>;
  [[nodiscard]] auto randomArrivalPosition() const -> Vec2F;
  [[nodiscard]] auto objectWarpAction(Uuid const& uuid) const -> std::optional<WarpAction>;

  [[nodiscard]] virtual auto objects() const -> List<Ptr<SystemObject>> = 0;
  [[nodiscard]] virtual auto objectKeys() const -> List<Uuid> = 0;
  [[nodiscard]] virtual auto getObject(Uuid const& uuid) const -> Ptr<SystemObject> = 0;

  [[nodiscard]] auto systemObjectConfig(String const& name, Uuid const& uuid) const -> SystemObjectConfig;
  static auto systemObjectTypeConfig(String const& typeName) -> Json;

protected:
  Vec3I m_location;
  Ptr<CelestialDatabase> m_celestialDatabase;

private:
  ConstPtr<Clock> m_universeClock;
  SystemWorldConfig m_config;
};

struct SystemObjectConfig {
  String name;

  bool moving;
  float speed;
  float orbitDistance;
  float lifeTime;

  // permanent system objects may only have a solar orbit and can never be removed
  bool permanent;

  WarpAction warpAction;
  std::optional<float> threatLevel;
  SkyParameters skyParameters;
  StringMap<String> generatedParameters;
  JsonObject parameters;
};

class SystemObject {
public:
  SystemObject(SystemObjectConfig config, Uuid uuid, Vec2F const& position, JsonObject parameters = {});
  SystemObject(SystemObjectConfig config, Uuid uuid, Vec2F const& position, double spawnTime, JsonObject parameters = {});
  SystemObject(SystemWorld* system, Json const& diskStore);

  void init();

  [[nodiscard]] auto uuid() const -> Uuid;
  [[nodiscard]] auto name() const -> String;
  [[nodiscard]] auto permanent() const -> bool;
  [[nodiscard]] auto position() const -> Vec2F;

  [[nodiscard]] auto warpAction() const -> WarpAction;
  [[nodiscard]] auto threatLevel() const -> std::optional<float>;
  [[nodiscard]] auto skyParameters() const -> SkyParameters;
  [[nodiscard]] auto parameters() const -> JsonObject;

  [[nodiscard]] auto shouldDestroy() const -> bool;

  void enterOrbit(CelestialCoordinate const& target, Vec2F const& targetPosition, double time);
  [[nodiscard]] auto orbitTarget() const -> std::optional<CelestialCoordinate>;
  [[nodiscard]] auto orbit() const -> std::optional<CelestialOrbit>;

  void clientUpdate(float dt);
  void serverUpdate(SystemWorldServer* system, float dt);

  auto writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t>;
  void readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules = {});

  [[nodiscard]] auto netStore() const -> ByteArray;
  [[nodiscard]] auto diskStore() const -> Json;

private:
  void setPosition(Vec2F const& position);

  SystemObjectConfig m_config;
  Uuid m_uuid;
  double m_spawnTime;
  JsonObject m_parameters;

  std::optional<CelestialCoordinate> m_approach;

  bool m_shouldDestroy;

  NetElementTopGroup m_netGroup;
  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;
  NetElementData<std::optional<CelestialOrbit>> m_orbit;
};

class SystemClientShip {
public:
  SystemClientShip(SystemWorld* world, Uuid uuid, float speed, SystemLocation const& position);
  SystemClientShip(SystemWorld* world, Uuid uuid, SystemLocation const& position);

  auto uuid() const -> Uuid;
  auto position() const -> Vec2F;
  auto systemLocation() const -> SystemLocation;
  auto destination() const -> SystemLocation;
  void setDestination(SystemLocation const& destination);
  void setSpeed(float speed);
  void startFlying();

  auto flying() const -> bool;

  // update is only called on master
  void clientUpdate(float dt);
  void serverUpdate(SystemWorld* system, float dt);

  auto writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules = {}) -> std::pair<ByteArray, std::uint64_t>;
  void readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules = {});

  auto netStore() const -> ByteArray;

private:
  struct ClientShipConfig {
    float orbitDistance;
    float departTime;
    float spaceDepartTime;
  };

  void setPosition(Vec2F const& position);

  Uuid m_uuid;

  ClientShipConfig m_config;
  float m_departTimer;
  float m_speed;

  std::optional<CelestialOrbit> m_orbit;

  NetElementTopGroup m_netGroup;
  NetElementData<SystemLocation> m_systemLocation;
  NetElementData<SystemLocation> m_destination;
  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;
};

}// namespace Star

#pragma once

#include "StarCelestialDatabase.hpp"
#include "StarCelestialParameters.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarUuid.hpp"
#include "StarWarping.hpp"
#include "StarSkyParameters.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarAssets.hpp"
#include "StarNameGenerator.hpp"

namespace Star {

class Clock;
using ClockConstPtr = SharedPtr<Clock const>;
class SystemWorldServer;
using SystemWorldServerPtr = SharedPtr<SystemWorldServer>;
class SystemClientShip;
using SystemClientShipPtr = SharedPtr<SystemClientShip>;
class SystemObject;
using SystemObjectPtr = SharedPtr<SystemObject>;
struct SystemObjectConfig;

struct CelestialOrbit {
  [[nodiscard]] static CelestialOrbit fromJson(Json const& json);
  [[nodiscard]] Json toJson() const;

  CelestialCoordinate target;
  int direction;
  double enterTime;
  Vec2F enterPosition;

  void write(DataStream& ds) const;
  void read(DataStream& ds);

  bool operator==(CelestialOrbit const& rhs) const;
};
DataStream& operator>>(DataStream& ds, CelestialOrbit& orbit);
DataStream& operator<<(DataStream& ds, CelestialOrbit const& orbit);

// in transit, at a planet, orbiting a planet,, at a system object, or at a vector position
using SystemLocation = MVariant<CelestialCoordinate, CelestialOrbit, Uuid, Vec2F>;
[[nodiscard]] Json jsonFromSystemLocation(SystemLocation const& location);
[[nodiscard]] SystemLocation jsonToSystemLocation(Json const& json);

struct SystemWorldConfig {
  [[nodiscard]] static SystemWorldConfig fromJson(Json const& config);

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
  SystemWorld(AssetsConstPtr assets, ClockConstPtr universeClock, CelestialDatabasePtr celestialDatabase, PatternedNameGeneratorConstPtr nameGenerator);

  virtual ~SystemWorld() = default;

  [[nodiscard]] AssetsConstPtr assets() const;
  [[nodiscard]] PatternedNameGeneratorConstPtr nameGenerator() const;
  [[nodiscard]] SystemWorldConfig const& systemConfig() const;
  [[nodiscard]] double time() const;
  [[nodiscard]] Vec3I location() const;
  [[nodiscard]] List<CelestialCoordinate> planets() const;

  [[nodiscard]] uint64_t coordinateSeed(CelestialCoordinate const& coord, String const& seedMix) const;
  [[nodiscard]] float planetOrbitDistance(CelestialCoordinate const& coord) const;
  // assumes circular orbit
  [[nodiscard]] float orbitInterval(float distance, bool isMoon) const;
  [[nodiscard]] Vec2F orbitPosition(CelestialOrbit const& orbit) const;
  [[nodiscard]] float clusterSize(CelestialCoordinate const& planet) const;
  [[nodiscard]] float planetSize(CelestialCoordinate const& planet) const;
  [[nodiscard]] Vec2F planetPosition(CelestialCoordinate const& planet) const;
  [[nodiscard]] Maybe<Vec2F> systemLocationPosition(SystemLocation const& position) const;
  [[nodiscard]] Vec2F randomArrivalPosition() const;
  [[nodiscard]] Maybe<WarpAction> objectWarpAction(Uuid const& uuid) const;

  [[nodiscard]] virtual List<SystemObjectPtr> objects() const = 0;
  [[nodiscard]] virtual List<Uuid> objectKeys() const = 0;
  [[nodiscard]] virtual SystemObjectPtr getObject(Uuid const& uuid) const = 0;

  [[nodiscard]] SystemObjectConfig systemObjectConfig(String const& name, Uuid const& uuid) const;
  [[nodiscard]] static Json systemObjectTypeConfig(AssetsConstPtr assets, String const& typeName);

protected:
  Vec3I m_location;
  CelestialDatabasePtr m_celestialDatabase;

private:
  AssetsConstPtr m_assets;
  ClockConstPtr m_universeClock;
  PatternedNameGeneratorConstPtr m_nameGenerator;
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
  Maybe<float> threatLevel;
  SkyParameters skyParameters;
  StringMap<String> generatedParameters;
  JsonObject parameters;
};

class SystemObject {
public:
  SystemObject(SystemObjectConfig config, Uuid uuid, Vec2F const& position, JsonObject parameters = {});
  SystemObject(SystemObjectConfig config, Uuid uuid, Vec2F const& position, double spawnTime, PatternedNameGeneratorConstPtr nameGenerator, JsonObject parameters = {});
  SystemObject(SystemWorld& system, Json const& diskStore);

  void init();

  [[nodiscard]] Uuid uuid() const;
  [[nodiscard]] String name() const;
  [[nodiscard]] bool permanent() const;
  [[nodiscard]] Vec2F position() const;

  [[nodiscard]] WarpAction warpAction() const;
  [[nodiscard]] Maybe<float> threatLevel() const;
  [[nodiscard]] SkyParameters skyParameters() const;
  [[nodiscard]] JsonObject parameters() const;

  [[nodiscard]] bool shouldDestroy() const;

  void enterOrbit(CelestialCoordinate const& target, Vec2F const& targetPosition, double time);
  [[nodiscard]] Maybe<CelestialCoordinate> orbitTarget() const;
  [[nodiscard]] Maybe<CelestialOrbit> orbit() const;

  void clientUpdate(float dt);
  void serverUpdate(SystemWorldServer& system, float dt);

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion, NetCompatibilityRules rules = {});
  void readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules = {});

  [[nodiscard]] ByteArray netStore() const;
  [[nodiscard]] Json diskStore() const;
private:

  void setPosition(Vec2F const& position);

  SystemObjectConfig m_config;
  Uuid m_uuid;
  double m_spawnTime = 0.0;
  JsonObject m_parameters;

  Maybe<CelestialCoordinate> m_approach;

  bool m_shouldDestroy;

  NetElementTopGroup m_netGroup;
  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;
  NetElementData<Maybe<CelestialOrbit>> m_orbit;
};

class SystemClientShip {
public:
  SystemClientShip(SystemWorld& world, Uuid uuid, float speed, SystemLocation const& position);
  SystemClientShip(SystemWorld& world, Uuid uuid, SystemLocation const& position);

  [[nodiscard]] Uuid uuid() const;
  [[nodiscard]] Vec2F position() const;
  [[nodiscard]] SystemLocation systemLocation() const;
  [[nodiscard]] SystemLocation destination() const;
  void setDestination(SystemLocation const& destination);
  void setSpeed(float speed);
  void startFlying();

  [[nodiscard]] bool flying() const;

  // update is only called on master
  void clientUpdate(float dt);
  void serverUpdate(SystemWorld& system, float dt);

  [[nodiscard]] pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion, NetCompatibilityRules rules = {});
  void readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules = {});

  [[nodiscard]] ByteArray netStore() const;
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

  Maybe<CelestialOrbit> m_orbit;

  NetElementTopGroup m_netGroup;
  NetElementData<SystemLocation> m_systemLocation;
  NetElementData<SystemLocation> m_destination;
  NetElementFloat m_xPosition;
  NetElementFloat m_yPosition;
};

}

module;
import std;
import star.string;
import star.map;
import star.json;
import star.net_compatibility;
import star.byte_array;
import star.vector;
import star.variant;
import star.game_types;
import star.color;
import star.bi_map;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarStrongTypedef.hpp"
import star.either;

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.uuid;
import star.warping;
import star.sky_parameters;

import star.net_element_basic_fields;
import star.net_element_float_fields;
import star.net_element_system;

namespace Star {

class CelestialDatabase;
using CelestialDatabasePtr = std::shared_ptr<CelestialDatabase>;
using CelestialDatabaseConstPtr = std::shared_ptr<const CelestialDatabase>;
using CelestialDatabaseWeakPtr = std::weak_ptr<CelestialDatabase>;
using CelestialDatabaseConstWeakPtr = std::weak_ptr<const CelestialDatabase>;
using CelestialDatabaseUPtr = std::unique_ptr<CelestialDatabase>;
using CelestialDatabaseConstUPtr = std::unique_ptr<const CelestialDatabase>;
class Celestial;
using CelestialPtr = std::shared_ptr<Celestial>;
using CelestialConstPtr = std::shared_ptr<const Celestial>;
using CelestialWeakPtr = std::weak_ptr<Celestial>;
using CelestialConstWeakPtr = std::weak_ptr<const Celestial>;
using CelestialUPtr = std::unique_ptr<Celestial>;
using CelestialConstUPtr = std::unique_ptr<const Celestial>;
class Clock;
using ClockPtr = std::shared_ptr<Clock>;
using ClockConstPtr = std::shared_ptr<const Clock>;
using ClockWeakPtr = std::weak_ptr<Clock>;
using ClockConstWeakPtr = std::weak_ptr<const Clock>;
using ClockUPtr = std::unique_ptr<Clock>;
using ClockConstUPtr = std::unique_ptr<const Clock>;
class ClientContext;
using ClientContextPtr = std::shared_ptr<ClientContext>;
using ClientContextConstPtr = std::shared_ptr<const ClientContext>;
using ClientContextWeakPtr = std::weak_ptr<ClientContext>;
using ClientContextConstWeakPtr = std::weak_ptr<const ClientContext>;
using ClientContextUPtr = std::unique_ptr<ClientContext>;
using ClientContextConstUPtr = std::unique_ptr<const ClientContext>;
class SystemWorld;
using SystemWorldPtr = std::shared_ptr<SystemWorld>;
using SystemWorldConstPtr = std::shared_ptr<const SystemWorld>;
using SystemWorldWeakPtr = std::weak_ptr<SystemWorld>;
using SystemWorldConstWeakPtr = std::weak_ptr<const SystemWorld>;
using SystemWorldUPtr = std::unique_ptr<SystemWorld>;
using SystemWorldConstUPtr = std::unique_ptr<const SystemWorld>;
class SystemWorldServer;
using SystemWorldServerPtr = std::shared_ptr<SystemWorldServer>;
using SystemWorldServerConstPtr = std::shared_ptr<const SystemWorldServer>;
using SystemWorldServerWeakPtr = std::weak_ptr<SystemWorldServer>;
using SystemWorldServerConstWeakPtr = std::weak_ptr<const SystemWorldServer>;
using SystemWorldServerUPtr = std::unique_ptr<SystemWorldServer>;
using SystemWorldServerConstUPtr = std::unique_ptr<const SystemWorldServer>;
class SystemClientShip;
using SystemClientShipPtr = std::shared_ptr<SystemClientShip>;
using SystemClientShipConstPtr = std::shared_ptr<const SystemClientShip>;
using SystemClientShipWeakPtr = std::weak_ptr<SystemClientShip>;
using SystemClientShipConstWeakPtr = std::weak_ptr<const SystemClientShip>;
using SystemClientShipUPtr = std::unique_ptr<SystemClientShip>;
using SystemClientShipConstUPtr = std::unique_ptr<const SystemClientShip>;
class SystemObject;
using SystemObjectPtr = std::shared_ptr<SystemObject>;
using SystemObjectConstPtr = std::shared_ptr<const SystemObject>;
using SystemObjectWeakPtr = std::weak_ptr<SystemObject>;
using SystemObjectConstWeakPtr = std::weak_ptr<const SystemObject>;
using SystemObjectUPtr = std::unique_ptr<SystemObject>;
using SystemObjectConstUPtr = std::unique_ptr<const SystemObject>;

struct SystemObjectConfig;
using SystemObjectConfigPtr = std::shared_ptr<SystemObjectConfig>;
using SystemObjectConfigConstPtr = std::shared_ptr<const SystemObjectConfig>;
using SystemObjectConfigWeakPtr = std::weak_ptr<SystemObjectConfig>;
using SystemObjectConfigConstWeakPtr = std::weak_ptr<const SystemObjectConfig>;
using SystemObjectConfigUPtr = std::unique_ptr<SystemObjectConfig>;
using SystemObjectConfigConstUPtr = std::unique_ptr<const SystemObjectConfig>;

struct CelestialOrbit {
  friend DataStream& operator>>(DataStream& ds, CelestialOrbit& orbit);
  friend DataStream& operator<<(DataStream& ds, CelestialOrbit const& orbit);
  static CelestialOrbit fromJson(Json const& json);
  Json toJson() const;

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
typedef MVariant<CelestialCoordinate, CelestialOrbit, Uuid, Vec2F> SystemLocation;
Json jsonFromSystemLocation(SystemLocation const& location);
SystemLocation jsonToSystemLocation(Json const& json);

struct SystemWorldConfig {
  static SystemWorldConfig fromJson(Json const& config);

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
  SystemWorld(ClockConstPtr universeClock, CelestialDatabasePtr celestialDatabase);

  virtual ~SystemWorld() = default;

  SystemWorldConfig const& systemConfig() const;
  double time() const;
  Vec3I location() const;
  List<CelestialCoordinate> planets() const;

  std::uint64_t coordinateSeed(CelestialCoordinate const& coord, String const& seedMix) const;
  float planetOrbitDistance(CelestialCoordinate const& coord) const;
  // assumes circular orbit
  float orbitInterval(float distance, bool isMoon) const;
  Vec2F orbitPosition(CelestialOrbit const& orbit) const;
  float clusterSize(CelestialCoordinate const& planet) const;
  float planetSize(CelestialCoordinate const& planet) const;
  Vec2F planetPosition(CelestialCoordinate const& planet) const;
  Maybe<Vec2F> systemLocationPosition(SystemLocation const& position) const;
  Vec2F randomArrivalPosition() const;
  Maybe<WarpAction> objectWarpAction(Uuid const& uuid) const;

  virtual List<SystemObjectPtr> objects() const = 0;
  virtual List<Uuid> objectKeys() const = 0;
  virtual SystemObjectPtr getObject(Uuid const& uuid) const = 0;

  SystemObjectConfig systemObjectConfig(String const& name, Uuid const& uuid) const;
  static Json systemObjectTypeConfig(String const& typeName);

protected:
  Vec3I m_location;
  CelestialDatabasePtr m_celestialDatabase;

private:
  ClockConstPtr m_universeClock;
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
  SystemObject(SystemObjectConfig config, Uuid uuid, Vec2F const& position, double spawnTime, JsonObject parameters = {});
  SystemObject(SystemWorld* system, Json const& diskStore);

  void init();

  Uuid uuid() const;
  String name() const;
  bool permanent() const;
  Vec2F position() const;

  WarpAction warpAction() const;
  Maybe<float> threatLevel() const;
  SkyParameters skyParameters() const;
  JsonObject parameters() const;

  bool shouldDestroy() const;

  void enterOrbit(CelestialCoordinate const& target, Vec2F const& targetPosition, double time);
  Maybe<CelestialCoordinate> orbitTarget() const;
  Maybe<CelestialOrbit> orbit() const;

  void clientUpdate(float dt);
  void serverUpdate(SystemWorldServer* system, float dt);

  std::pair<ByteArray, std::uint64_t> writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules = {});
  void readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules = {});

  ByteArray netStore() const;
  Json diskStore() const;
private:

  void setPosition(Vec2F const& position);

  SystemObjectConfig m_config;
  Uuid m_uuid;
  double m_spawnTime;
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
  SystemClientShip(SystemWorld* world, Uuid uuid, float speed, SystemLocation const& position);
  SystemClientShip(SystemWorld* world, Uuid uuid, SystemLocation const& position);

  Uuid uuid() const;
  Vec2F position() const;
  SystemLocation systemLocation() const;
  SystemLocation destination() const;
  void setDestination(SystemLocation const& destination);
  void setSpeed(float speed);
  void startFlying();

  bool flying() const;

  // update is only called on master
  void clientUpdate(float dt);
  void serverUpdate(SystemWorld* system, float dt);

  std::pair<ByteArray, std::uint64_t> writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules = {});
  void readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules = {});

  ByteArray netStore() const;
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

export module star.system_world;

export namespace Star {
  using ::Star::CelestialDatabase;
  using ::Star::CelestialDatabasePtr;
  using ::Star::CelestialDatabaseConstPtr;
  using ::Star::CelestialDatabaseWeakPtr;
  using ::Star::CelestialDatabaseConstWeakPtr;
  using ::Star::CelestialDatabaseUPtr;
  using ::Star::CelestialDatabaseConstUPtr;
  using ::Star::Celestial;
  using ::Star::CelestialPtr;
  using ::Star::CelestialConstPtr;
  using ::Star::CelestialWeakPtr;
  using ::Star::CelestialConstWeakPtr;
  using ::Star::CelestialUPtr;
  using ::Star::CelestialConstUPtr;
  using ::Star::Clock;
  using ::Star::ClockPtr;
  using ::Star::ClockConstPtr;
  using ::Star::ClockWeakPtr;
  using ::Star::ClockConstWeakPtr;
  using ::Star::ClockUPtr;
  using ::Star::ClockConstUPtr;
  using ::Star::ClientContext;
  using ::Star::ClientContextPtr;
  using ::Star::ClientContextConstPtr;
  using ::Star::ClientContextWeakPtr;
  using ::Star::ClientContextConstWeakPtr;
  using ::Star::ClientContextUPtr;
  using ::Star::ClientContextConstUPtr;
  using ::Star::SystemWorld;
  using ::Star::SystemWorldPtr;
  using ::Star::SystemWorldConstPtr;
  using ::Star::SystemWorldWeakPtr;
  using ::Star::SystemWorldConstWeakPtr;
  using ::Star::SystemWorldUPtr;
  using ::Star::SystemWorldConstUPtr;
  using ::Star::SystemWorldServer;
  using ::Star::SystemWorldServerPtr;
  using ::Star::SystemWorldServerConstPtr;
  using ::Star::SystemWorldServerWeakPtr;
  using ::Star::SystemWorldServerConstWeakPtr;
  using ::Star::SystemWorldServerUPtr;
  using ::Star::SystemWorldServerConstUPtr;
  using ::Star::SystemClientShip;
  using ::Star::SystemClientShipPtr;
  using ::Star::SystemClientShipConstPtr;
  using ::Star::SystemClientShipWeakPtr;
  using ::Star::SystemClientShipConstWeakPtr;
  using ::Star::SystemClientShipUPtr;
  using ::Star::SystemClientShipConstUPtr;
  using ::Star::SystemObject;
  using ::Star::SystemObjectPtr;
  using ::Star::SystemObjectConstPtr;
  using ::Star::SystemObjectWeakPtr;
  using ::Star::SystemObjectConstWeakPtr;
  using ::Star::SystemObjectUPtr;
  using ::Star::SystemObjectConstUPtr;
  using ::Star::SystemObjectConfig;
  using ::Star::SystemObjectConfigPtr;
  using ::Star::SystemObjectConfigConstPtr;
  using ::Star::SystemObjectConfigWeakPtr;
  using ::Star::SystemObjectConfigConstWeakPtr;
  using ::Star::SystemObjectConfigUPtr;
  using ::Star::SystemObjectConfigConstUPtr;
  using ::Star::CelestialOrbit;
  using ::Star::SystemLocation;
  using ::Star::jsonFromSystemLocation;
  using ::Star::jsonToSystemLocation;
  using ::Star::SystemWorldConfig;
}

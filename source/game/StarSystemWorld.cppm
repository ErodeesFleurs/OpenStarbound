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
import star.strong_typedef;
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
class Celestial;
class Clock;
class ClientContext;
class SystemWorldServer;
class SystemObject;

struct SystemObjectConfig;

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

class Root;
class SystemWorld {
  Root& m_root;
public:
  Root& root() const { return m_root; }
public:
  SystemWorld(Root& root, std::shared_ptr<Clock const> universeClock, std::shared_ptr<CelestialDatabase> celestialDatabase);

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
  std::optional<Vec2F> systemLocationPosition(SystemLocation const& position) const;
  Vec2F randomArrivalPosition() const;
  std::optional<WarpAction> objectWarpAction(Uuid const& uuid) const;

  virtual List<std::shared_ptr<SystemObject>> objects() const = 0;
  virtual List<Uuid> objectKeys() const = 0;
  virtual std::shared_ptr<SystemObject> getObject(Uuid const& uuid) const = 0;

  SystemObjectConfig systemObjectConfig(String const& name, Uuid const& uuid) const;
  static Json systemObjectTypeConfig(Root& root, String const& typeName);

protected:
  Vec3I m_location;
  std::shared_ptr<CelestialDatabase> m_celestialDatabase;

private:
  std::shared_ptr<Clock const> m_universeClock;
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
  Root& m_root;
  Root& root() const { return m_root; }
public:
  SystemObject(Root& root, SystemObjectConfig config, Uuid uuid, Vec2F const& position, JsonObject parameters = {});
  SystemObject(Root& root, SystemObjectConfig config, Uuid uuid, Vec2F const& position, double spawnTime, JsonObject parameters = {});
  SystemObject(Root& root, SystemWorld* system, Json const& diskStore);

  void init();

  Uuid uuid() const;
  String name() const;
  bool permanent() const;
  Vec2F position() const;

  WarpAction warpAction() const;
  std::optional<float> threatLevel() const;
  SkyParameters skyParameters() const;
  JsonObject parameters() const;

  bool shouldDestroy() const;

  void enterOrbit(CelestialCoordinate const& target, Vec2F const& targetPosition, double time);
  std::optional<CelestialCoordinate> orbitTarget() const;
  std::optional<CelestialOrbit> orbit() const;

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

  std::optional<CelestialOrbit> m_orbit;

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
  using ::Star::Celestial;
  using ::Star::Clock;
  using ::Star::ClientContext;
  using ::Star::SystemWorld;
  using ::Star::SystemWorldServer;
  using ::Star::SystemClientShip;
  using ::Star::SystemObject;
  using ::Star::SystemObjectConfig;
  using ::Star::CelestialOrbit;
  using ::Star::SystemLocation;
  using ::Star::jsonFromSystemLocation;
  using ::Star::jsonToSystemLocation;
  using ::Star::SystemWorldConfig;
}

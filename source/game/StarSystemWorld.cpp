#include "StarSystemWorld.hpp"
#include "StarCasting.hpp"
#include "StarCelestialDatabase.hpp"
#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarNameGenerator.hpp"// IWYU pragma: export
#include "StarRoot.hpp"
#include "StarSystemWorldServer.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

auto CelestialOrbit::fromJson(Json const& json) -> CelestialOrbit {
  return CelestialOrbit{
    .target = CelestialCoordinate(json.get("target")),
    .direction = (int)json.getInt("direction"),
    .enterTime = json.getDouble("enterTime"),
    .enterPosition = jsonToVec2F(json.get("enterPosition"))};
}

auto CelestialOrbit::toJson() const -> Json {
  JsonObject json;
  json.set("target", target.toJson());
  json.set("direction", direction);
  json.set("enterTime", enterTime);
  json.set("enterPosition", jsonFromVec2F(enterPosition));
  return json;
}

void CelestialOrbit::write(DataStream& ds) const {
  ds.write(target);
  ds.write(direction);
  ds.write(enterTime);
  ds.write(enterPosition);
}

void CelestialOrbit::read(DataStream& ds) {
  target = ds.read<CelestialCoordinate>();
  direction = ds.read<int>();
  enterTime = ds.read<double>();
  enterPosition = ds.read<Vec2F>();
}

auto CelestialOrbit::operator==(CelestialOrbit const& rhs) const -> bool {
  return target == rhs.target
    && direction == rhs.direction
    && enterTime == rhs.enterTime
    && enterPosition == rhs.enterPosition;
}

auto operator>>(DataStream& ds, CelestialOrbit& orbit) -> DataStream& {
  orbit.read(ds);
  return ds;
}

auto operator<<(DataStream& ds, CelestialOrbit const& orbit) -> DataStream& {
  orbit.write(ds);
  return ds;
}

auto jsonToSystemLocation(Json const& json) -> SystemLocation {
  if (auto location = json.optArray()) {
    if (location->get(0).type() == Json::Type::String) {
      String type = location->get(0).toString();
      if (type == "coordinate")
        return CelestialCoordinate(location->get(1));
      else if (type == "orbit")
        return CelestialOrbit::fromJson(location->get(1));
      else if (type == "object")
        return Uuid(location->get(1).toString());
    } else if (auto position = jsonToMaybe<Vec2F>(*location, jsonToVec2F)) {
      return *position;
    }
  }
  return {};
}

auto jsonFromSystemLocation(SystemLocation const& location) -> Json {
  if (auto coordinate = location.maybe<CelestialCoordinate>())
    return JsonArray{"coordinate", coordinate->toJson()};
  else if (auto orbit = location.maybe<CelestialOrbit>())
    return JsonArray{"orbit", orbit->toJson()};
  else if (auto uuid = location.maybe<Uuid>())
    return JsonArray{"object", uuid->hex()};
  else
    return jsonFromMaybe<Vec2F>(location.maybe<Vec2F>(), jsonFromVec2F);
}

auto SystemWorldConfig::fromJson(Json const& json) -> SystemWorldConfig {
  SystemWorldConfig config;
  config.starGravitationalConstant = json.getFloat("starGravitationalConstant");
  config.planetGravitationalConstant = json.getFloat("planetGravitationalConstant");

  for (auto size : json.getArray("planetSizes"))
    config.planetSizes.set(size.getUInt(0), size.getFloat(1));
  config.emptyOrbitSize = json.getFloat("emptyOrbitSize");
  config.unvisitablePlanetSize = json.getFloat("unvisitablePlanetSize");
  for (auto p : json.getObject("floatingDungeonWorldSizes"))
    config.floatingDungeonWorldSizes.set(p.first, p.second.toFloat());

  config.starSize = json.getFloat("starSize");
  config.planetaryOrbitPadding = jsonToVec2F(json.get("planetaryOrbitPadding"));
  config.satelliteOrbitPadding = jsonToVec2F(json.get("satelliteOrbitPadding"));

  config.arrivalRange = jsonToVec2F(json.get("arrivalRange"));

  config.objectSpawnPadding = json.getFloat("objectSpawnPadding");
  config.clientObjectSpawnPadding = json.getFloat("clientObjectSpawnPadding");
  config.objectSpawnInterval = jsonToVec2F(json.get("objectSpawnInterval"));
  config.objectSpawnCycle = json.getDouble("objectSpawnCycle");
  config.minObjectOrbitTime = json.getFloat("minObjectOrbitTime");

  config.asteroidBeamDistance = json.getFloat("asteroidBeamDistance");

  config.emptySkyParameters = SkyParameters(json.get("emptySkyParameters"));
  return config;
}

SystemWorld::SystemWorld(ConstPtr<Clock> universeClock, Ptr<CelestialDatabase> celestialDatabase)
    : m_celestialDatabase(std::move(celestialDatabase)), m_universeClock(std::move(universeClock)) {
  m_config = SystemWorldConfig::fromJson(Root::singleton().assets()->json("/systemworld.config"));
}

auto SystemWorld::systemConfig() const -> SystemWorldConfig const& {
  return m_config;
}

auto SystemWorld::time() const -> double {
  return m_universeClock->time();
}

auto SystemWorld::location() const -> Vec3I {
  return m_location;
}

auto SystemWorld::planets() const -> List<CelestialCoordinate> {
  return m_celestialDatabase->children(CelestialCoordinate(m_location));
}

auto SystemWorld::coordinateSeed(CelestialCoordinate const& coordinate, String const& seedMix) const -> uint64_t {
  auto satellite = coordinate.isSatelliteBody() ? coordinate.orbitNumber() : 0;
  auto planet = coordinate.isSatelliteBody() ? coordinate.parent().orbitNumber() : (coordinate.isPlanetaryBody() && coordinate.orbitNumber()) || false;
  return staticRandomU64(coordinate.location()[0], coordinate.location()[1], coordinate.location()[2], planet, satellite, seedMix);
}

auto SystemWorld::planetOrbitDistance(CelestialCoordinate const& coordinate) const -> float {
  RandomSource random(coordinateSeed(coordinate, "PlanetOrbitDistance"));

  if (coordinate.isSystem() || coordinate.isNull())
    return 0;

  float distance = planetSize(coordinate.parent()) / 2.0;
  for (int i = 0; i < coordinate.orbitNumber(); i++) {
    if (i > 0 && i < coordinate.orbitNumber())
      distance += clusterSize(coordinate.parent().child(i));

    if (coordinate.isPlanetaryBody())
      distance += random.randf(m_config.planetaryOrbitPadding[0], m_config.planetaryOrbitPadding[1]);
    else if (coordinate.isSatelliteBody())
      distance += random.randf(m_config.satelliteOrbitPadding[0], m_config.satelliteOrbitPadding[1]);
  }

  distance += clusterSize(coordinate) / 2.0;

  return distance;
}

auto SystemWorld::orbitInterval(float distance, bool isMoon) const -> float {
  float gravityConstant = isMoon ? m_config.planetGravitationalConstant : m_config.starGravitationalConstant;
  float speed = std::sqrt(gravityConstant / distance);
  return (distance * 2 * Constants::pi) / speed;
}

auto SystemWorld::orbitPosition(CelestialOrbit const& orbit) const -> Vec2F {
  Vec2F targetPosition = orbit.target.isPlanetaryBody() || orbit.target.isSatelliteBody() ? planetPosition(orbit.target) : Vec2F(0, 0);
  float distance = orbit.enterPosition.magnitude();
  float interval = orbitInterval(distance, false);

  float timeOffset = std::fmod(time() - orbit.enterTime, interval) / interval;
  float angle = (orbit.enterPosition * -1).angle() + (orbit.direction * timeOffset * (Constants::pi * 2));
  return targetPosition + Vec2F::withAngle(angle, distance);
}

auto SystemWorld::clusterSize(CelestialCoordinate const& coordinate) const -> float {
  if (coordinate.isPlanetaryBody() && m_celestialDatabase->childOrbits(coordinate.parent()).contains(coordinate.orbitNumber())) {
    auto childOrbits = m_celestialDatabase->childOrbits(coordinate).sorted();
    if (childOrbits.size() > 0) {
      CelestialCoordinate outer = coordinate.child(childOrbits.get(childOrbits.size() - 1));
      return (planetOrbitDistance(outer) * 2) + planetSize(outer);
    } else {
      return planetSize(coordinate);
    }
  } else {
    return planetSize(coordinate);
  }
};

auto SystemWorld::planetSize(CelestialCoordinate const& coordinate) const -> float {
  if (coordinate.isNull())
    return 0;

  if (coordinate.isSystem())
    return m_config.starSize;

  if (!m_celestialDatabase->childOrbits(coordinate.parent()).contains(coordinate.orbitNumber()))
    return m_config.emptyOrbitSize;

  if (auto parameters = m_celestialDatabase->parameters(coordinate)) {
    if (auto visitableParameters = parameters->visitableParameters()) {
      float size = 0;
      if (is<FloatingDungeonWorldParameters>(visitableParameters)) {
        if (auto s = m_config.floatingDungeonWorldSizes.maybe(visitableParameters->typeName))
          return *s;
      }
      for (auto s : m_config.planetSizes) {
        if (visitableParameters->worldSize[0] >= s.first)
          size = s.second;
        else
          break;
      }
      return size;
    }
  }
  return m_config.unvisitablePlanetSize;
}

auto SystemWorld::planetPosition(CelestialCoordinate const& coordinate) const -> Vec2F {
  if (coordinate.isNull() || coordinate.isSystem())
    return {0.0, 0.0};

  RandomSource random(coordinateSeed(coordinate, "PlanetSystemPosition"));

  Vec2F parentPosition = planetPosition(coordinate.parent());
  float distance = planetOrbitDistance(coordinate);
  float interval = orbitInterval(distance, coordinate.isSatelliteBody());

  double start = random.randf();
  double offset = (std::fmod(m_universeClock->time(), interval) / interval);
  int direction = random.randf() > 0.5 ? 1 : -1;
  float angle = (start + direction * offset) * (Constants::pi * 2);

  return parentPosition + Vec2F(std::cos(angle), std::sin(angle)) * distance;
}

auto SystemWorld::systemObjectConfig(String const& name, Uuid const& uuid) const -> SystemObjectConfig {
  RandomSource rand(staticRandomU64(uuid.hex()));

  SystemObjectConfig object;
  auto config = systemObjectTypeConfig(name);
  auto orbitRange = jsonToVec2F(config.get("orbitRange"));
  auto lifeTimeRange = jsonToVec2F(config.get("lifeTime"));

  object.name = name;

  object.moving = config.getBool("moving");
  object.speed = config.getFloat("speed");
  object.orbitDistance = Random::randf(orbitRange[0], orbitRange[1]);
  object.lifeTime = Random::randf(lifeTimeRange[0], lifeTimeRange[1]);

  object.permanent = config.getBool("permanent", false);

  object.warpAction = parseWarpAction(config.getString("warpAction"));
  object.threatLevel = config.optFloat("threatLevel");
  object.skyParameters = SkyParameters(config.get("skyParameters"));
  object.parameters = config.getObject("parameters");

  if (config.contains("generatedParameters")) {
    for (auto p : config.getObject("generatedParameters"))
      object.generatedParameters[p.first] = p.second.toString();
  }

  return object;
}

auto SystemWorld::systemObjectTypeConfig(String const& name) -> Json {
  return Root::singleton().assets()->json(strf("/system_objects.config:{}", name));
}

auto SystemWorld::systemLocationPosition(SystemLocation const& location) const -> std::optional<Vec2F> {
  if (auto coordinate = location.maybe<CelestialCoordinate>()) {
    return planetPosition(*coordinate);
  } else if (auto orbit = location.maybe<CelestialOrbit>()) {
    return orbitPosition(*orbit);
  } else if (auto objectUuid = location.maybe<Uuid>()) {
    if (auto object = getObject(*objectUuid))
      return object->position();
  } else if (auto position = location.maybe<Vec2F>()) {
    return Vec2F(*position);
  }
  return {};
}

auto SystemWorld::randomArrivalPosition() const -> Vec2F {
  RandomSource rand;
  float range = m_config.arrivalRange[0] + (rand.randf() * (m_config.arrivalRange[1] - m_config.arrivalRange[0]));
  float angle = rand.randf() * Constants::pi * 2;
  return Vec2F::withAngle(angle, range);
}

auto SystemWorld::objectWarpAction(Uuid const& uuid) const -> std::optional<WarpAction> {
  if (auto object = getObject(uuid)) {
    WarpAction warpAction = object->warpAction();
    if (auto warpToWorld = warpAction.ptr<WarpToWorld>()) {
      if (auto instanceWorldId = warpToWorld->world.ptr<InstanceWorldId>()) {
        instanceWorldId->uuid = object->uuid();
        if (auto parameters = m_celestialDatabase->parameters(CelestialCoordinate(m_location))) {
          std::optional<float> systemThreatLevel = parameters->getParameter("spaceThreatLevel").optFloat();
          instanceWorldId->level = object->threatLevel() ? object->threatLevel() : systemThreatLevel;
        } else {
          return {};
        }
      }
    }
    return warpAction;
  } else {
    return {};
  }
}

SystemObject::SystemObject(SystemObjectConfig config, Uuid uuid, Vec2F const& position, JsonObject parameters)
    : m_config(std::move(config)), m_uuid(std::move(uuid)), m_spawnTime(0.0f), m_parameters(std::move(parameters)) {
  setPosition(position);
  init();
}

SystemObject::SystemObject(SystemObjectConfig config, Uuid uuid, Vec2F const& position, double spawnTime, JsonObject parameters)
    : m_config(std::move(config)), m_uuid(std::move(uuid)), m_spawnTime(std::move(spawnTime)), m_parameters(std::move(parameters)) {
  setPosition(position);
  for (auto p : m_config.generatedParameters) {
    if (!m_parameters.contains(p.first))
      m_parameters[p.first] = Root::singleton().nameGenerator()->generateName(p.second);
  }
  init();
}

SystemObject::SystemObject(SystemWorld* system, Json const& diskStore) {
  m_uuid = Uuid(diskStore.getString("uuid"));
  auto name = diskStore.getString("name");
  m_config = system->systemObjectConfig(name, m_uuid);
  m_parameters = diskStore.getObject("parameters", {});

  m_orbit.set(jsonToMaybe<CelestialOrbit>(diskStore.get("orbit"), [](Json const& json) -> CelestialOrbit {
    return CelestialOrbit::fromJson(json);
  }));

  m_spawnTime = diskStore.getDouble("spawnTime");

  setPosition(jsonToVec2F(diskStore.get("position")));

  init();
}

void SystemObject::init() {
  m_shouldDestroy = false;

  m_xPosition.setInterpolator(lerp<float, float>);
  m_yPosition.setInterpolator(lerp<float, float>);

  m_netGroup.addNetElement(&m_xPosition);
  m_netGroup.addNetElement(&m_yPosition);
  m_netGroup.addNetElement(&m_orbit);
}

auto SystemObject::uuid() const -> Uuid {
  return m_uuid;
}

auto SystemObject::name() const -> String {
  return m_config.name;
}

auto SystemObject::permanent() const -> bool {
  return m_config.permanent;
}

auto SystemObject::position() const -> Vec2F {
  return {m_xPosition.get(), m_yPosition.get()};
}

auto SystemObject::warpAction() const -> WarpAction {
  return m_config.warpAction;
}

auto SystemObject::threatLevel() const -> std::optional<float> {
  return m_config.threatLevel;
}

auto SystemObject::skyParameters() const -> SkyParameters {
  return m_config.skyParameters;
}

auto SystemObject::parameters() const -> JsonObject {
  return jsonMerge(m_config.parameters, m_parameters).toObject();
}

auto SystemObject::shouldDestroy() const -> bool {
  return m_shouldDestroy;
}

void SystemObject::enterOrbit(CelestialCoordinate const& target, Vec2F const& targetPosition, double time) {
  int direction = Random::randf() > 0.5 ? 1 : -1;// random direction
  m_orbit.set(CelestialOrbit{.target = target, .direction = direction, .enterTime = time, .enterPosition = targetPosition - position()});
  m_approach.reset();
}

auto SystemObject::orbitTarget() const -> std::optional<CelestialCoordinate> {
  if (m_orbit.get())
    return m_orbit.get()->target;
  else
    return {};
}

auto SystemObject::orbit() const -> std::optional<CelestialOrbit> {
  return m_orbit.get();
}

void SystemObject::clientUpdate(float dt) {
  m_netGroup.tickNetInterpolation(dt);
}

void SystemObject::serverUpdate(SystemWorldServer* system, float dt) {
  if (!m_config.permanent && m_spawnTime > 0.0 && system->time() > m_spawnTime + m_config.lifeTime)
    m_shouldDestroy = true;

  if (m_orbit.get()) {
    setPosition(system->orbitPosition(*m_orbit.get()));
  } else if (m_config.permanent || !m_config.moving) {
    // permanent locations always have a solar orbit
    enterOrbit(CelestialCoordinate(system->location()), {0.0, 0.0}, system->time());
  } else if (m_approach && !m_approach->isNull()) {

    if (system->shipsAtLocation(m_uuid).size() > 0)
      return;

    if (m_approach->isPlanetaryBody()) {
      auto approach = system->planetPosition(*m_approach);
      auto toApproach = (approach - position());
      auto pos = position();
      setPosition(pos + toApproach.normalized() * m_config.speed * dt);

      if ((approach - position()).magnitude() < system->planetSize(*m_approach) + m_config.orbitDistance)
        enterOrbit(*m_approach, approach, system->time());
    } else {
      enterOrbit(*m_approach, {0.0, 0.0}, system->time());
    }
  } else {
    auto planets = system->planets().filtered([system](CelestialCoordinate const& p) -> bool {
      auto objectsAtPlanet = system->objects().filtered([p](Ptr<SystemObject> const& o) -> bool { return o->orbitTarget() == p; });
      return objectsAtPlanet.size() == 0;
    });

    if (planets.size() > 0)
      m_approach = Random::randFrom(planets);
  }
}

auto SystemObject::writeNetState(uint64_t fromVersion, NetCompatibilityRules rules) -> std::pair<ByteArray, uint64_t> {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void SystemObject::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

auto SystemObject::netStore() const -> ByteArray {
  DataStreamBuffer ds;
  ds.write(m_uuid);
  ds.write(m_config.name);
  ds.write(position());
  ds.write(m_parameters);
  return ds.takeData();
}

auto SystemObject::diskStore() const -> Json {
  JsonObject store;
  store.set("uuid", m_uuid.hex());
  store.set("name", m_config.name);
  store.set("orbit", jsonFromMaybe<CelestialOrbit>(m_orbit.get(), [](CelestialOrbit const& o) -> Json {
              return o.toJson();
            }));
  store.set("spawnTime", m_spawnTime);
  store.set("position", jsonFromVec2F(position()));
  store.set("parameters", m_parameters);
  return store;
}

void SystemObject::setPosition(Vec2F const& position) {
  m_xPosition.set(position[0]);
  m_yPosition.set(position[1]);
}

SystemClientShip::SystemClientShip(SystemWorld* system, Uuid uuid, float speed, SystemLocation const& location)
    : m_uuid(std::move(uuid)) {
  m_systemLocation.set(location);
  setPosition(system->systemLocationPosition(location).value_or(Vec2F{0, 0}));

  // temporary
  auto shipConfig = Root::singleton().assets()->json("/systemworld.config:clientShip");
  m_config = ClientShipConfig{
    .orbitDistance = shipConfig.getFloat("orbitDistance"),
    .departTime = shipConfig.getFloat("departTime"),
    .spaceDepartTime = shipConfig.getFloat("spaceDepartTime")};
  m_speed = speed;
  m_departTimer = 0.0f;

  // systemLocation should not be interpolated
  // if it's stale it can point to a removed system object
  m_netGroup.addNetElement(&m_systemLocation, false);
  m_netGroup.addNetElement(&m_destination);

  m_netGroup.addNetElement(&m_xPosition);
  m_netGroup.addNetElement(&m_yPosition);
  m_netGroup.enableNetInterpolation();

  m_xPosition.setInterpolator(lerp<float, float>);
  m_yPosition.setInterpolator(lerp<float, float>);
}

SystemClientShip::SystemClientShip(SystemWorld* system, Uuid uuid, SystemLocation const& location)
    : SystemClientShip(system, uuid, 0.0f, location) {}

auto SystemClientShip::uuid() const -> Uuid {
  return m_uuid;
}

auto SystemClientShip::position() const -> Vec2F {
  return {m_xPosition.get(), m_yPosition.get()};
}

auto SystemClientShip::systemLocation() const -> SystemLocation {
  return m_systemLocation.get();
}

auto SystemClientShip::destination() const -> SystemLocation {
  return m_destination.get();
}

void SystemClientShip::setDestination(SystemLocation const& destination) {
  auto location = m_systemLocation.get();
  if (location.is<CelestialCoordinate>() || location.is<Uuid>())
    m_departTimer = m_config.departTime;
  else if (m_destination.get().empty())
    m_departTimer = m_config.spaceDepartTime;
  m_destination.set(destination);
  m_systemLocation.set({});
}

void SystemClientShip::setSpeed(float speed) {
  m_speed = speed;
}

auto SystemClientShip::flying() const -> bool {
  return m_systemLocation.get().empty();
}

void SystemClientShip::clientUpdate(float dt) {
  m_netGroup.tickNetInterpolation(dt);
}

void SystemClientShip::serverUpdate(SystemWorld* system, float dt) {
  // if destination is an orbit we haven't started orbiting yet, update the time
  if (auto orbit = m_destination.get().maybe<CelestialOrbit>())
    orbit->enterTime = system->time();

  auto nearPlanetOrbit = [this, system](CelestialCoordinate const& planet) -> CelestialOrbit {
    Vec2F toShip = system->planetPosition(planet) - position();
    return CelestialOrbit{
      .target = planet,
      .direction = 1,
      .enterTime = system->time(),
      .enterPosition = Vec2F::withAngle(toShip.angle(), system->planetSize(planet) / 2.0 + m_config.orbitDistance)};
  };

  if (auto coordinate = m_systemLocation.get().maybe<CelestialCoordinate>()) {
    if (!m_orbit || m_orbit->target != *coordinate)
      m_orbit = nearPlanetOrbit(*coordinate);
  } else if (m_systemLocation.get().empty()) {
    m_departTimer = std::max(0.0f, m_departTimer - dt);
    if (m_departTimer > 0.0f)
      return;

    if (auto coordinate = m_destination.get().maybe<CelestialCoordinate>()) {
      if (!m_orbit || m_orbit->target != *coordinate)
        m_orbit = nearPlanetOrbit(*coordinate);
    } else {
      m_orbit.reset();
    }

    Vec2F pos = position();
    Vec2F destination;
    if (m_orbit) {
      m_orbit->enterTime = system->time();
      destination = system->orbitPosition(*m_orbit);
    } else {
      destination = system->systemLocationPosition(m_destination.get()).value_or(pos);
    }

    auto toTarget = destination - pos;
    pos += toTarget.normalized() * (m_speed * dt);

    if (destination == pos || (destination - pos).normalized() * toTarget.normalized() < 0) {
      m_systemLocation.set(m_destination.get());
      m_destination.set({});
    } else {
      setPosition(pos);
      return;
    }
  }

  if (m_orbit) {
    setPosition(system->systemLocationPosition(*m_orbit).value());
  } else {
    setPosition(system->systemLocationPosition(m_systemLocation.get()).value());
  }
}

auto SystemClientShip::writeNetState(std::uint64_t fromVersion, NetCompatibilityRules rules) -> std::pair<ByteArray, std::uint64_t> {
  return m_netGroup.writeNetState(fromVersion, rules);
}

void SystemClientShip::readNetState(ByteArray data, float interpolationTime, NetCompatibilityRules rules) {
  m_netGroup.readNetState(data, interpolationTime, rules);
}

auto SystemClientShip::netStore() const -> ByteArray {
  DataStreamBuffer ds;
  ds.write(m_uuid);
  ds.write(m_systemLocation.get());
  return ds.takeData();
}

void SystemClientShip::setPosition(Vec2F const& position) {
  m_xPosition.set(position[0]);
  m_yPosition.set(position[1]);
}

}// namespace Star

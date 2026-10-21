import std;
import star.string;
import star.map;
import star.format;
import star.json;
import star.json_extra;
import star.strong_typedef;
import star.data_stream;
import star.vector;
import star.game_types;
import star.color;
import star.bi_map;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.either;
import star.lexical_cast;

import star.uuid;
import star.celestial_coordinate;
import star.warping;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.sky_parameters;
import star.system_world;

import star.player_universe_map;

namespace Star {

template<> Json jsonFromBookmarkTarget<OrbitTarget>(OrbitTarget const& target) {
  if (auto uuid = std::get_if<Uuid>(&target))
    return uuid->hex();
  else
    return std::get<CelestialCoordinate>(target).toJson();
};
template<> OrbitTarget jsonToBookmarkTarget<OrbitTarget>(Json const& json) {
  if (json.type() == Json::Type::String)
    return Uuid(json.toString());
  else
    return CelestialCoordinate(json);
};

template<> Json jsonFromBookmarkTarget<TeleportTarget>(TeleportTarget const& target) {
  return JsonArray{printWorldId(target.first), spawnTargetToJson(target.second)};
}
template<> TeleportTarget jsonToBookmarkTarget<TeleportTarget>(Json const& target) {
  return {parseWorldId(target.get(0).toString()), spawnTargetFromJson(target.get(1))};
}

template <typename T>
Bookmark<T> Bookmark<T>::fromJson(Json const& json) {
  Bookmark<T> bookmark;
  bookmark.target = jsonToBookmarkTarget<T>(json.get("target"));
  bookmark.targetName = json.getString("targetName");
  bookmark.bookmarkName = json.getString("bookmarkName");
  bookmark.icon = json.getString("icon");
  return bookmark;
}

template <typename T>
Json Bookmark<T>::toJson() const {
  JsonObject result;
  result["target"] = jsonFromBookmarkTarget<T>(target);
  result["targetName"] = targetName;
  result["bookmarkName"] = bookmarkName;
  result["icon"] = icon;
  return result;
}

PlayerUniverseMap::PlayerUniverseMap() : PlayerUniverseMap(Json{}) {}

PlayerUniverseMap::PlayerUniverseMap(Json const& json) {
  if (auto maps = json.optObject()) {
    for (auto p : *maps)
      m_universeMaps.set(Uuid(p.first), UniverseMap::fromJson(p.second));
  }
}

Json PlayerUniverseMap::toJson() const {
  JsonObject json;
  for (auto p : m_universeMaps)
    json.set(p.first.hex(), p.second.toJson());
  return json;
}

List<std::pair<Vec3I, OrbitBookmark>> PlayerUniverseMap::orbitBookmarks() const {
  if (!m_serverUuid.has_value())
    return {};

  List<std::pair<Vec3I, OrbitBookmark>> bookmarks;
  for (auto p : universeMap().systems) {
    bookmarks.appendAll(p.second.bookmarks.values().transformed([&p](OrbitBookmark const& b) {
      return std::pair<Vec3I, OrbitBookmark>(p.first, b);
    }));
  }
  return bookmarks;
}

bool PlayerUniverseMap::addOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark) {
  if (system.isNull())
    throw StarException("Cannot add orbit bookmark to null system");

  return m_universeMaps[m_serverUuid.value()].systems[system.location()].bookmarks.add(std::move(bookmark));
}

bool PlayerUniverseMap::removeOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark) {
  if (system.isNull())
    throw StarException("Cannot remove orbit bookmark from null system");

  return m_universeMaps[m_serverUuid.value()].systems[system.location()].bookmarks.remove(bookmark);
}

List<TeleportBookmark> PlayerUniverseMap::teleportBookmarks() const {
  return universeMap().teleportBookmarks.values();
}

bool PlayerUniverseMap::addTeleportBookmark(TeleportBookmark bookmark) {
  return m_universeMaps[m_serverUuid.value()].teleportBookmarks.add(std::move(bookmark));
}

bool PlayerUniverseMap::removeTeleportBookmark(TeleportBookmark const& bookmark) {
  return m_universeMaps[m_serverUuid.value()].teleportBookmarks.remove(bookmark);
}

void PlayerUniverseMap::invalidateWarpAction(WarpAction const& warpAction) {
  if (auto warpToWorld = std::get_if<WarpToWorld>(&warpAction))
    removeTeleportBookmark({ {warpToWorld->world, warpToWorld->target}, "", "", ""});
}

std::optional<OrbitBookmark> PlayerUniverseMap::worldBookmark(CelestialCoordinate const& world) const {
  if (auto systemMap = universeMap().systems.ptr(world.location())) {
    for (auto& bookmark : systemMap->bookmarks) {
      if (auto coordinate = std::get_if<CelestialCoordinate>(&bookmark.target); coordinate && *coordinate == world)
        return bookmark;
    }
  }
  return {};
}

List<OrbitBookmark> PlayerUniverseMap::systemBookmarks(CelestialCoordinate const& system) const {
  if (auto systemMap = universeMap().systems.ptr(system.location()))
    return systemMap->bookmarks.values();
  return {};
}

List<OrbitBookmark> PlayerUniverseMap::planetBookmarks(CelestialCoordinate const& planet) const {
  if (auto systemMap = universeMap().systems.ptr(planet.location())) {
    return systemMap->bookmarks.values().filtered([planet](OrbitBookmark const& bookmark) {
      if (auto coordinate = std::get_if<CelestialCoordinate>(&bookmark.target))
        return coordinate->planet().orbitNumber() == planet.planet().orbitNumber();
      return false;
    });
  }
  return {};
}

bool PlayerUniverseMap::isMapped(CelestialCoordinate const& coordinate) {
  if (coordinate.isNull())
    return false;

  auto& universeMap = m_universeMaps[m_serverUuid.value()];
  if (auto systemMap = universeMap.systems.ptr(coordinate.location()))
    return coordinate.isSystem() || systemMap->mappedPlanets.contains(coordinate.planet());
  else
    return false;
}

HashMap<Uuid, PlayerUniverseMap::MappedObject> PlayerUniverseMap::mappedObjects(CelestialCoordinate const& system) {
  auto& universeMap = m_universeMaps[m_serverUuid.value()];
  if (auto systemMap = universeMap.systems.ptr(system.location()))
    return systemMap->mappedObjects;
  else
    return {};
}

void PlayerUniverseMap::addMappedCoordinate(CelestialCoordinate const& coordinate) {
  if (coordinate.isNull())
    return;

  auto& universeMap = m_universeMaps[m_serverUuid.value()];
  auto& systemMap = universeMap.systems[coordinate.location()];
  if (!coordinate.isSystem())
    systemMap.mappedPlanets.add(coordinate.planet());
}

void PlayerUniverseMap::addMappedObject(CelestialCoordinate const& system, Uuid const& uuid, String const& typeName, std::optional<CelestialOrbit> const& orbit, JsonObject parameters) {
  MappedObject object {
    typeName,
    orbit,
    parameters
  };
  auto& universeMap = m_universeMaps[m_serverUuid.value()];
  universeMap.systems[system.location()].mappedObjects.set(uuid, object);
}

void PlayerUniverseMap::removeMappedObject(CelestialCoordinate const& system, Uuid const& uuid) {
  auto& universeMap = m_universeMaps[m_serverUuid.value()];
  if (auto systemMap = universeMap.systems.ptr(system.location()))
    systemMap->mappedObjects.remove(uuid);
}

void PlayerUniverseMap::filterMappedObjects(CelestialCoordinate const& system, List<Uuid> const& allowed) {
  auto& universeMap = m_universeMaps[m_serverUuid.value()];
  if (auto systemMap = universeMap.systems.ptr(system.location())) {
    auto& objects = systemMap->mappedObjects;
    for (auto& uuid : objects.keys()) {
      if (!allowed.contains(uuid))
        objects.remove(uuid);
    }
  }
}

void PlayerUniverseMap::setServerUuid(std::optional<Uuid> serverUuid) {
  m_serverUuid = std::move(serverUuid);
  if (m_serverUuid && !m_universeMaps.contains(*m_serverUuid))
    m_universeMaps.set(*m_serverUuid, UniverseMap());
}

PlayerUniverseMap::SystemMap PlayerUniverseMap::SystemMap::fromJson(Json const& json) {
  SystemMap map;

  for (auto m : json.getArray("mappedPlanets"))
    map.mappedPlanets.add(CelestialCoordinate(m));

  for (auto o : json.getObject("mappedObjects")) {
    MappedObject object;
    object.typeName = o.second.getString("typeName");
    object.orbit = jsonToOptional<CelestialOrbit>(o.second.get("orbit"), [](Json const& o ) {
        return CelestialOrbit::fromJson(o);
      });
    object.parameters = o.second.getObject("parameters", {});
    map.mappedObjects.set(Uuid(o.first), object);
  }

  for (auto b : json.getArray("bookmarks"))
    map.bookmarks.add(OrbitBookmark::fromJson(b));

  return map;
}

Json PlayerUniverseMap::SystemMap::toJson() const {
  JsonObject json;

  JsonArray planets;
  for (auto m : mappedPlanets)
    planets.append(m.toJson());
  json.set("mappedPlanets", planets);

  JsonObject objects;
  for (auto o : mappedObjects) {
    JsonObject object;
    objects.set(o.first.hex(), JsonObject{
      {"typeName", o.second.typeName},
      {"orbit", jsonFromOptional<CelestialOrbit>(
        o.second.orbit, [](CelestialOrbit const& orbit){ return orbit.toJson(); })},
      {"parameters", o.second.parameters}
    });
  }
  json.set("mappedObjects", objects);

  json.set("bookmarks", bookmarks.values().transformed([](OrbitBookmark const& b) {
      return b.toJson();
    }));

  return json;
}

PlayerUniverseMap::UniverseMap PlayerUniverseMap::UniverseMap::fromJson(Json const& json) {
  UniverseMap map;

  for (auto s : json.getArray("systems")) {
    Vec3I location = jsonToVec3I(s.get(0));
    map.systems.set(location, SystemMap::fromJson(s.get(1)));
  }

  for (auto bookmark : json.getArray("teleportBookmarks").transformed(&TeleportBookmark::fromJson))
    map.teleportBookmarks.add(bookmark);

  return map;
}

Json PlayerUniverseMap::UniverseMap::toJson() const {
  JsonObject json;

  JsonArray s;
  for (auto p : systems) {
    s.append(JsonArray{jsonFromVec3I(p.first), p.second.toJson()});
  }
  json.set("systems", s);

  JsonArray bookmarks = teleportBookmarks.values().transformed([](TeleportBookmark const& b) {
      return b.toJson();
    });
  json.set("teleportBookmarks", bookmarks);

  return json;
}

PlayerUniverseMap::UniverseMap const& PlayerUniverseMap::universeMap() const {
  if (!m_serverUuid.has_value())
    throw StarException("Cannot get universe map of null server uuid");

  return m_universeMaps.get(*m_serverUuid);
}

}

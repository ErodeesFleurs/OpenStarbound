#include "StarPlayerUniverseMap.hpp"

#include "StarJsonExtra.hpp"

import std;

namespace Star {

template <>
auto jsonFromBookmarkTarget<OrbitTarget>(OrbitTarget const& target) -> Json {
  if (auto uuid = target.maybe<Uuid>())
    return uuid->hex();
  else
    return target.get<CelestialCoordinate>().toJson();
};
template <>
auto jsonToBookmarkTarget<OrbitTarget>(Json const& json) -> OrbitTarget {
  if (json.type() == Json::Type::String)
    return Uuid(json.toString());
  else
    return CelestialCoordinate(json);
};

template <>
auto jsonFromBookmarkTarget<TeleportTarget>(TeleportTarget const& target) -> Json {
  return JsonArray{printWorldId(target.first), spawnTargetToJson(target.second)};
}
template <>
auto jsonToBookmarkTarget<TeleportTarget>(Json const& target) -> TeleportTarget {
  return {parseWorldId(target.get(0).toString()), spawnTargetFromJson(target.get(1))};
}

template <typename T>
auto Bookmark<T>::fromJson(Json const& json) -> Bookmark<T> {
  Bookmark<T> bookmark;
  bookmark.target = jsonToBookmarkTarget<T>(json.get("target"));
  bookmark.targetName = json.getString("targetName");
  bookmark.bookmarkName = json.getString("bookmarkName");
  bookmark.icon = json.getString("icon");
  return bookmark;
}

template <typename T>
auto Bookmark<T>::toJson() const -> Json {
  JsonObject result;
  result["target"] = jsonFromBookmarkTarget<T>(target);
  result["targetName"] = targetName;
  result["bookmarkName"] = bookmarkName;
  result["icon"] = icon;
  return result;
}

PlayerUniverseMap::PlayerUniverseMap(Json const& json) {
  if (auto maps = json.optObject()) {
    for (auto p : *maps)
      m_universeMaps.set(Uuid(p.first), UniverseMap::fromJson(p.second));
  }
}

auto PlayerUniverseMap::toJson() const -> Json {
  JsonObject json;
  for (auto p : m_universeMaps)
    json.set(p.first.hex(), p.second.toJson());
  return json;
}

auto PlayerUniverseMap::orbitBookmarks() const -> List<std::pair<Vec3I, OrbitBookmark>> {
  if (!m_serverUuid)
    return {};

  List<std::pair<Vec3I, OrbitBookmark>> bookmarks;
  for (auto p : universeMap().systems) {
    bookmarks.appendAll(p.second.bookmarks.values().transformed([&p](OrbitBookmark const& b) -> std::pair<Vec3I, OrbitBookmark> {
      return {p.first, b};
    }));
  }
  return bookmarks;
}

auto PlayerUniverseMap::addOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark) -> bool {
  if (system.isNull())
    throw StarException("Cannot add orbit bookmark to null system");

  return m_universeMaps[*m_serverUuid].systems[system.location()].bookmarks.add(std::move(bookmark));
}

auto PlayerUniverseMap::removeOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark) -> bool {
  if (system.isNull())
    throw StarException("Cannot remove orbit bookmark from null system");

  return m_universeMaps[*m_serverUuid].systems[system.location()].bookmarks.remove(bookmark);
}

auto PlayerUniverseMap::teleportBookmarks() const -> List<TeleportBookmark> {
  return universeMap().teleportBookmarks.values();
}

auto PlayerUniverseMap::addTeleportBookmark(TeleportBookmark bookmark) -> bool {
  return m_universeMaps[*m_serverUuid].teleportBookmarks.add(std::move(bookmark));
}

auto PlayerUniverseMap::removeTeleportBookmark(TeleportBookmark const& bookmark) -> bool {
  return m_universeMaps[*m_serverUuid].teleportBookmarks.remove(bookmark);
}

void PlayerUniverseMap::invalidateWarpAction(WarpAction const& warpAction) {
  if (auto warpToWorld = warpAction.maybe<WarpToWorld>())
    removeTeleportBookmark({{warpToWorld->world, warpToWorld->target}, "", "", ""});
}

auto PlayerUniverseMap::worldBookmark(CelestialCoordinate const& world) const -> std::optional<OrbitBookmark> {
  if (auto systemMap = universeMap().systems.ptr(world.location())) {
    for (auto& bookmark : systemMap->bookmarks) {
      if (bookmark.target == world)
        return bookmark;
    }
  }
  return {};
}

auto PlayerUniverseMap::systemBookmarks(CelestialCoordinate const& system) const -> List<OrbitBookmark> {
  if (auto systemMap = universeMap().systems.ptr(system.location()))
    return systemMap->bookmarks.values();
  return {};
}

auto PlayerUniverseMap::planetBookmarks(CelestialCoordinate const& planet) const -> List<OrbitBookmark> {
  if (auto systemMap = universeMap().systems.ptr(planet.location())) {
    return systemMap->bookmarks.values().filtered([planet](OrbitBookmark const& bookmark) -> bool {
      if (auto coordinate = bookmark.target.maybe<CelestialCoordinate>())
        return coordinate->planet().orbitNumber() == planet.planet().orbitNumber();
      return false;
    });
  }
  return {};
}

auto PlayerUniverseMap::isMapped(CelestialCoordinate const& coordinate) -> bool {
  if (coordinate.isNull())
    return false;

  auto& universeMap = m_universeMaps[*m_serverUuid];
  if (auto systemMap = universeMap.systems.ptr(coordinate.location()))
    return coordinate.isSystem() || systemMap->mappedPlanets.contains(coordinate.planet());
  else
    return false;
}

auto PlayerUniverseMap::mappedObjects(CelestialCoordinate const& system) -> HashMap<Uuid, PlayerUniverseMap::MappedObject> {
  auto& universeMap = m_universeMaps[*m_serverUuid];
  if (auto systemMap = universeMap.systems.ptr(system.location()))
    return systemMap->mappedObjects;
  else
    return {};
}

void PlayerUniverseMap::addMappedCoordinate(CelestialCoordinate const& coordinate) {
  if (coordinate.isNull())
    return;

  auto& universeMap = m_universeMaps[*m_serverUuid];
  auto& systemMap = universeMap.systems[coordinate.location()];
  if (!coordinate.isSystem())
    systemMap.mappedPlanets.add(coordinate.planet());
}

void PlayerUniverseMap::addMappedObject(CelestialCoordinate const& system, Uuid const& uuid, String const& typeName, std::optional<CelestialOrbit> const& orbit, JsonObject parameters) {
  MappedObject object{
    .typeName = typeName,
    .orbit = orbit,
    .parameters = parameters};
  auto& universeMap = m_universeMaps[*m_serverUuid];
  universeMap.systems[system.location()].mappedObjects.set(uuid, object);
}

void PlayerUniverseMap::removeMappedObject(CelestialCoordinate const& system, Uuid const& uuid) {
  auto& universeMap = m_universeMaps[*m_serverUuid];
  if (auto systemMap = universeMap.systems.ptr(system.location()))
    systemMap->mappedObjects.remove(uuid);
}

void PlayerUniverseMap::filterMappedObjects(CelestialCoordinate const& system, List<Uuid> const& allowed) {
  auto& universeMap = m_universeMaps[*m_serverUuid];
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

auto PlayerUniverseMap::SystemMap::fromJson(Json const& json) -> PlayerUniverseMap::SystemMap {
  SystemMap map;

  for (auto m : json.getArray("mappedPlanets"))
    map.mappedPlanets.add(CelestialCoordinate(m));

  for (auto o : json.getObject("mappedObjects")) {
    MappedObject object;
    object.typeName = o.second.getString("typeName");
    object.orbit = jsonToMaybe<CelestialOrbit>(o.second.get("orbit"), [](Json const& o) -> CelestialOrbit {
      return CelestialOrbit::fromJson(o);
    });
    object.parameters = o.second.getObject("parameters", {});
    map.mappedObjects.set(Uuid(o.first), object);
  }

  for (auto b : json.getArray("bookmarks"))
    map.bookmarks.add(OrbitBookmark::fromJson(b));

  return map;
}

auto PlayerUniverseMap::SystemMap::toJson() const -> Json {
  JsonObject json;

  JsonArray planets;
  for (auto m : mappedPlanets)
    planets.append(m.toJson());
  json.set("mappedPlanets", planets);

  JsonObject objects;
  for (auto o : mappedObjects) {
    JsonObject object;
    objects.set(o.first.hex(), JsonObject{{"typeName", o.second.typeName}, {"orbit", jsonFromMaybe<CelestialOrbit>(o.second.orbit, [](CelestialOrbit const& orbit) -> Json { return orbit.toJson(); })}, {"parameters", o.second.parameters}});
  }
  json.set("mappedObjects", objects);

  json.set("bookmarks", bookmarks.values().transformed([](OrbitBookmark const& b) -> Json {
    return b.toJson();
  }));

  return json;
}

auto PlayerUniverseMap::UniverseMap::fromJson(Json const& json) -> PlayerUniverseMap::UniverseMap {
  UniverseMap map;

  for (auto s : json.getArray("systems")) {
    Vec3I location = jsonToVec3I(s.get(0));
    map.systems.set(location, SystemMap::fromJson(s.get(1)));
  }

  for (auto bookmark : json.getArray("teleportBookmarks").transformed(&TeleportBookmark::fromJson))
    map.teleportBookmarks.add(bookmark);

  return map;
}

auto PlayerUniverseMap::UniverseMap::toJson() const -> Json {
  JsonObject json;

  JsonArray s;
  for (auto p : systems) {
    s.append(JsonArray{jsonFromVec3I(p.first), p.second.toJson()});
  }
  json.set("systems", s);

  JsonArray bookmarks = teleportBookmarks.values().transformed([](TeleportBookmark const& b) -> Json {
    return b.toJson();
  });
  json.set("teleportBookmarks", bookmarks);

  return json;
}

auto PlayerUniverseMap::universeMap() const -> PlayerUniverseMap::UniverseMap const& {
  if (!m_serverUuid)
    throw StarException("Cannot get universe map of null server uuid");

  return m_universeMaps.get(*m_serverUuid);
}

}// namespace Star

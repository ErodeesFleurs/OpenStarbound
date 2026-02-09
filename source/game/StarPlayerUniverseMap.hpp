#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarJson.hpp"
#include "StarSystemWorld.hpp"
#include "StarWarping.hpp"

import std;

namespace Star {

template <typename T>
auto jsonFromBookmarkTarget(T const& target) -> Json;

template <typename T>
auto jsonToBookmarkTarget(Json const& json) -> T;

// Bookmark<T> requires T to implement jsonToBookmarkTarget<T> and jsonFromBookmarkTarget<T>
// also operator== and operator!=
template <typename T>
struct Bookmark {
  T target;
  String targetName;
  String bookmarkName;
  String icon;

  static auto fromJson(Json const& json) -> Bookmark;
  [[nodiscard]] auto toJson() const -> Json;

  auto operator==(Bookmark<T> const& rhs) const -> bool;
  auto operator!=(Bookmark<T> const& rhs) const -> bool;
  auto operator<(Bookmark<T> const& rhs) const -> bool;
};

using OrbitTarget = Variant<CelestialCoordinate, Uuid>;
using TeleportTarget = std::pair<WorldId, SpawnTarget>;

using OrbitBookmark = Bookmark<OrbitTarget>;
using TeleportBookmark = Bookmark<TeleportTarget>;

class PlayerUniverseMap {
public:
  struct MappedObject {
    String typeName;
    std::optional<CelestialOrbit> orbit;
    JsonObject parameters;
  };

  PlayerUniverseMap(Json const& json = {});

  [[nodiscard]] auto toJson() const -> Json;

  // pair of system location and bookmark, not all orbit bookmarks include the system
  [[nodiscard]] auto orbitBookmarks() const -> List<std::pair<Vec3I, OrbitBookmark>>;
  auto addOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark) -> bool;
  auto removeOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark) -> bool;

  [[nodiscard]] auto teleportBookmarks() const -> List<TeleportBookmark>;
  auto addTeleportBookmark(TeleportBookmark bookmark) -> bool;
  auto removeTeleportBookmark(TeleportBookmark const& bookmark) -> bool;
  void invalidateWarpAction(WarpAction const& bookmark);

  [[nodiscard]] auto worldBookmark(CelestialCoordinate const& world) const -> std::optional<OrbitBookmark>;
  [[nodiscard]] auto systemBookmarks(CelestialCoordinate const& system) const -> List<OrbitBookmark>;
  [[nodiscard]] auto planetBookmarks(CelestialCoordinate const& planet) const -> List<OrbitBookmark>;

  auto isMapped(CelestialCoordinate const& coordinate) -> bool;
  auto mappedObjects(CelestialCoordinate const& system) -> HashMap<Uuid, MappedObject>;

  void addMappedCoordinate(CelestialCoordinate const& coordinate);
  void addMappedObject(CelestialCoordinate const& system, Uuid const& uuid, String const& typeName, std::optional<CelestialOrbit> const& orbit = {}, JsonObject parameters = {});
  void removeMappedObject(CelestialCoordinate const& system, Uuid const& uuid);
  void filterMappedObjects(CelestialCoordinate const& system, List<Uuid> const& allowed);

  void setServerUuid(std::optional<Uuid> serverUuid);

private:
  struct SystemMap {
    Set<CelestialCoordinate> mappedPlanets;
    HashMap<Uuid, MappedObject> mappedObjects;
    Set<OrbitBookmark> bookmarks;

    static auto fromJson(Json const& json) -> SystemMap;
    [[nodiscard]] auto toJson() const -> Json;
  };
  struct UniverseMap {
    HashMap<Vec3I, SystemMap> systems;
    Set<TeleportBookmark> teleportBookmarks;

    static auto fromJson(Json const& json) -> UniverseMap;
    [[nodiscard]] auto toJson() const -> Json;
  };

  [[nodiscard]] auto universeMap() const -> UniverseMap const&;
  auto universeMap() -> UniverseMap&;

  std::optional<Uuid> m_serverUuid;
  HashMap<Uuid, UniverseMap> m_universeMaps;
};

template <typename T>
auto Bookmark<T>::operator==(Bookmark<T> const& rhs) const -> bool {
  return target == rhs.target;
}

template <typename T>
auto Bookmark<T>::operator!=(Bookmark<T> const& rhs) const -> bool {
  return target != rhs.target;
}

template <typename T>
auto Bookmark<T>::operator<(Bookmark<T> const& rhs) const -> bool {
  return target < rhs.target;
}

}// namespace Star

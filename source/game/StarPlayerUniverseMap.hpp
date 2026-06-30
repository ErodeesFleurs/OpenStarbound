#pragma once

#include "StarJson.hpp"
#include "StarWarping.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarSystemWorld.hpp"

namespace Star {

class PlayerUniverseMap;
using PlayerUniverseMapPtr = SharedPtr<PlayerUniverseMap>;

template <typename T>
[[nodiscard]] Json jsonFromBookmarkTarget(T const& target);

template <typename T>
[[nodiscard]] T jsonToBookmarkTarget(Json const& json);

// Bookmark<T> requires T to implement jsonToBookmarkTarget<T> and jsonFromBookmarkTarget<T>
// also operator== and operator!=
template<typename T>
struct Bookmark {
  T target;
  String targetName;
  String bookmarkName;
  String icon;

  [[nodiscard]] static Bookmark fromJson(Json const& json);
  [[nodiscard]] Json toJson() const;

  [[nodiscard]] bool operator==(Bookmark<T> const& rhs) const;
  [[nodiscard]] bool operator!=(Bookmark<T> const& rhs) const;
  [[nodiscard]] bool operator<(Bookmark<T> const& rhs) const;
};

using OrbitTarget = Variant<CelestialCoordinate, Uuid>;
using TeleportTarget = pair<WorldId, SpawnTarget>;

using OrbitBookmark = Bookmark<OrbitTarget>;
using TeleportBookmark = Bookmark<TeleportTarget>;


class PlayerUniverseMap {
public:
  struct MappedObject {
    String typeName;
    Maybe<CelestialOrbit> orbit;
    JsonObject parameters;
  };

  PlayerUniverseMap(Json const& json = {});

  [[nodiscard]] Json toJson() const;

  // pair of system location and bookmark, not all orbit bookmarks include the system
  [[nodiscard]] List<pair<Vec3I, OrbitBookmark>> orbitBookmarks() const;
  [[nodiscard]] bool addOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark);
  [[nodiscard]] bool removeOrbitBookmark(CelestialCoordinate const& system, OrbitBookmark const& bookmark);

  [[nodiscard]] List<TeleportBookmark> teleportBookmarks() const;
  [[nodiscard]] bool addTeleportBookmark(TeleportBookmark bookmark);
  [[nodiscard]] bool removeTeleportBookmark(TeleportBookmark const& bookmark);
  void invalidateWarpAction(WarpAction const& bookmark);

  [[nodiscard]] Maybe<OrbitBookmark> worldBookmark(CelestialCoordinate const& world) const;
  [[nodiscard]] List<OrbitBookmark> systemBookmarks(CelestialCoordinate const& system) const;
  [[nodiscard]] List<OrbitBookmark> planetBookmarks(CelestialCoordinate const& planet) const;

  [[nodiscard]] bool isMapped(CelestialCoordinate const& coordinate);
  [[nodiscard]] HashMap<Uuid, MappedObject> mappedObjects(CelestialCoordinate const& system);

  void addMappedCoordinate(CelestialCoordinate const& coordinate);
  void addMappedObject(CelestialCoordinate const& system, Uuid const& uuid, String const& typeName, Maybe<CelestialOrbit> const& orbit = {}, JsonObject parameters = {});
  void removeMappedObject(CelestialCoordinate const& system, Uuid const& uuid);
  void filterMappedObjects(CelestialCoordinate const& system, List<Uuid> const& allowed);

  void setServerUuid(Maybe<Uuid> serverUuid);

private:
  struct SystemMap {
    Set<CelestialCoordinate> mappedPlanets;
    HashMap<Uuid, MappedObject> mappedObjects;
    Set<OrbitBookmark> bookmarks;

    [[nodiscard]] static SystemMap fromJson(Json const& json);
    [[nodiscard]] Json toJson() const;
  };
  struct UniverseMap {
    HashMap<Vec3I, SystemMap> systems;
    Set<TeleportBookmark> teleportBookmarks;

    [[nodiscard]] static UniverseMap fromJson(Json const& json);
    [[nodiscard]] Json toJson() const;
  };

  [[nodiscard]] UniverseMap const& universeMap() const;
  [[nodiscard]] UniverseMap& universeMap();

  Maybe<Uuid> m_serverUuid;
  HashMap<Uuid, UniverseMap> m_universeMaps;
};

template <typename T>
[[nodiscard]] bool Bookmark<T>::operator==(Bookmark<T> const& rhs) const {
  return target == rhs.target;
}

template <typename T>
[[nodiscard]] bool Bookmark<T>::operator!=(Bookmark<T> const& rhs) const {
  return target != rhs.target;
}

template <typename T>
[[nodiscard]] bool Bookmark<T>::operator<(Bookmark<T> const& rhs) const {
  return target < rhs.target;
}

}

#pragma once

#include "StarConfig.hpp"
#include "StarEntity.hpp"
#include "StarException.hpp"
#include "StarSpatialHash2D.hpp"

import std;

namespace Star {

class TileEntity;
class InteractiveEntity;

using EntityMapException = ExceptionDerived<"EntityMapException">;

// Class used by WorldServer and WorldClient to store entites organized in a
// spatial hash.  Provides convenient ways of querying entities based on
// different selection criteria.
//
// Several of the methods in EntityMap take callbacks or filters that will be
// called while iterating over internal structures.  They are all designed so
// that adding new entities is safe to do from the callback, but removing
// entities is never safe to do from any callback function.
class EntityMap {
public:
  static float const SpatialHashSectorSize;
  static int const MaximumEntityBoundBox;

  // beginIdSpace and endIdSpace is the *inclusive* range for new enittyIds.
  EntityMap(Vec2U const& worldSize, EntityId beginIdSpace, EntityId endIdSpace);

  // Get the next free id in the entity id space.
  auto reserveEntityId() -> EntityId;
  // Or a specific one, can fail.
  auto maybeReserveEntityId(EntityId entityId) -> std::optional<EntityId>;
  // If it doesn't matter that we don't get the one want
  auto reserveEntityId(EntityId entityId) -> EntityId;

  // Add an entity to this EntityMap.  The entity must already be initialized
  // and have a unique EntityId returned by reserveEntityId.
  void addEntity(Ptr<Entity> entity);
  auto removeEntity(EntityId entityId) -> Ptr<Entity>;

  [[nodiscard]] auto size() const -> std::size_t;
  [[nodiscard]] auto entityIds() const -> List<EntityId>;

  // Iterates through the entity map optionally in the given order, updating
  // the spatial information for each entity along the way.
  void updateAllEntities(EntityCallback const& callback = {}, std::function<bool(Ptr<Entity> const&, Ptr<Entity> const&)> sortOrder = {});

  // If the given unique entity is in this map, then return its entity id
  [[nodiscard]] auto uniqueEntityId(String const& uniqueId) const -> EntityId;

  [[nodiscard]] auto entity(EntityId entityId) const -> Ptr<Entity>;
  [[nodiscard]] auto uniqueEntity(String const& uniqueId) const -> Ptr<Entity>;

  // Queries entities based on metaBoundBox
  [[nodiscard]] auto entityQuery(RectF const& boundBox, EntityFilter const& filter = {}) const -> List<Ptr<Entity>>;

  // A fuzzy query of the entities at this position, sorted by closeness.
  [[nodiscard]] auto entitiesAt(Vec2F const& pos, EntityFilter const& filter = {}) const -> List<Ptr<Entity>>;

  [[nodiscard]] auto entitiesAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> const& filter = {}) const -> List<Ptr<TileEntity>>;

  // Sort of a fuzzy line intersection test.  Tests if a given line intersects
  // the bounding box of any entities, and returns them.
  [[nodiscard]] auto entityLineQuery(Vec2F const& begin, Vec2F const& end, EntityFilter const& filter = {}) const -> List<Ptr<Entity>>;

  // Callback versions of query functions.
  void forEachEntity(RectF const& boundBox, EntityCallback const& callback) const;
  void forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback const& callback) const;
  // Returns tile-based entities that occupy the given tile position.
  void forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> const& callback) const;

  // Iterate through all the entities, optionally in the given sort order.
  void forAllEntities(EntityCallback const& callback, std::function<bool(Ptr<Entity> const&, Ptr<Entity> const&)> sortOrder = {}) const;

  // Stops searching when filter returns true, and returns the entity which
  // caused it.
  [[nodiscard]] auto findEntity(RectF const& boundBox, EntityFilter const& filter) const -> Ptr<Entity>;
  [[nodiscard]] auto findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter const& filter) const -> Ptr<Entity>;
  [[nodiscard]] auto findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> const& filter) const -> Ptr<Entity>;

  // Closest entity that satisfies the given selector, if given.
  [[nodiscard]] auto closestEntity(Vec2F const& center, float radius, EntityFilter const& filter = {}) const -> Ptr<Entity>;

  // Returns interactive entity that is near the given world position
  [[nodiscard]] auto interactiveEntityNear(Vec2F const& pos, float maxRadius = 1.5f) const -> Ptr<InteractiveEntity>;

  // Whether or not any tile entity occupies this tile
  [[nodiscard]] auto tileIsOccupied(Vec2I const& pos, bool includeEphemeral = false) const -> bool;

  // Intersects any entity's collision area
  [[nodiscard]] auto spaceIsOccupied(RectF const& rect, bool includeEphemeral = false) const -> bool;

  // Convenience template methods that filter based on dynamic cast and deal
  // with pointers to a derived entity type.

  template <typename EntityT>
  auto get(EntityId entityId) const -> std::shared_ptr<EntityT>;

  template <typename EntityT>
  auto getUnique(String const& uniqueId) const -> std::shared_ptr<EntityT>;

  template <typename EntityT>
  auto query(RectF const& boundBox, EntityFilterOf<EntityT> const& filter = {}) const -> List<std::shared_ptr<EntityT>>;

  template <typename EntityT>
  auto all(EntityFilterOf<EntityT> const& filter = {}) const -> List<std::shared_ptr<EntityT>>;

  template <typename EntityT>
  auto lineQuery(Vec2F const& begin, Vec2F const& end, EntityFilterOf<EntityT> const& filter = {}) const -> List<std::shared_ptr<EntityT>>;

  template <typename EntityT>
  auto closest(Vec2F const& center, float radius, EntityFilterOf<EntityT> const& filter = {}) const -> std::shared_ptr<EntityT>;

  template <typename EntityT>
  auto atTile(Vec2I const& pos) const -> List<std::shared_ptr<EntityT>>;

private:
  using SpatialMap = SpatialHash2D<EntityId, float, Ptr<Entity>>;

  WorldGeometry m_geometry;

  SpatialMap m_spatialMap;
  BiHashMap<String, EntityId> m_uniqueMap;

  EntityId m_nextId;
  EntityId m_beginIdSpace;
  EntityId m_endIdSpace;

  List<SpatialMap::Entry const*> m_entrySortBuffer;
};

template <typename EntityT>
auto EntityMap::get(EntityId entityId) const -> std::shared_ptr<EntityT> {
  return as<EntityT>(entity(entityId));
}

template <typename EntityT>
auto EntityMap::getUnique(String const& uniqueId) const -> std::shared_ptr<EntityT> {
  return as<EntityT>(uniqueEntity(uniqueId));
}

template <typename EntityT>
auto EntityMap::query(RectF const& boundBox, EntityFilterOf<EntityT> const& filter) const -> List<std::shared_ptr<EntityT>> {
  List<std::shared_ptr<EntityT>> entities;
  for (auto const& entity : entityQuery(boundBox, entityTypeFilter(filter)))
    entities.append(as<EntityT>(entity));

  return entities;
}

template <typename EntityT>
auto EntityMap::all(EntityFilterOf<EntityT> const& filter) const -> List<std::shared_ptr<EntityT>> {
  List<std::shared_ptr<EntityT>> entities;
  forAllEntities([&](Ptr<Entity> const& entity) -> auto {
    if (auto e = as<EntityT>(entity)) {
      if (!filter || filter(e))
        entities.append(e);
    }
  });

  return entities;
}

template <typename EntityT>
auto EntityMap::lineQuery(Vec2F const& begin, Vec2F const& end, EntityFilterOf<EntityT> const& filter) const -> List<std::shared_ptr<EntityT>> {
  List<std::shared_ptr<EntityT>> entities;
  for (auto const& entity : entityLineQuery(begin, end, entityTypeFilter(filter)))
    entities.append(as<EntityT>(entity));

  return entities;
}

template <typename EntityT>
auto EntityMap::closest(Vec2F const& center, float radius, EntityFilterOf<EntityT> const& filter) const -> std::shared_ptr<EntityT> {
  return as<EntityT>(closestEntity(center, radius, entityTypeFilter(filter)));
}

template <typename EntityT>
auto EntityMap::atTile(Vec2I const& pos) const -> List<std::shared_ptr<EntityT>> {
  List<std::shared_ptr<EntityT>> list;
  forEachEntityAtTile(pos, [&](Ptr<TileEntity> const& entity) -> auto {
    if (auto e = as<EntityT>(entity))
      list.append(std::move(e));
    return false;
  });
  return list;
}

}// namespace Star

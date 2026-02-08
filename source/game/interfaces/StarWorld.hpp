#pragma once

#include "StarCollisionBlock.hpp"
#include "StarConfig.hpp"
#include "StarForceRegions.hpp"
#include "StarInteractionTypes.hpp"
#include "StarLuaRoot.hpp"
#include "StarRpcPromise.hpp"
#include "StarTileEntity.hpp"
#include "StarTileModification.hpp"
#include "StarWorldGeometry.hpp"

import std;

namespace Star {

using WorldAction = std::function<void(World*)>;

class World {
public:
  virtual ~World() = default;

  // Will remain constant throughout the life of the world.
  [[nodiscard]] virtual auto connection() const -> ConnectionId = 0;
  [[nodiscard]] virtual auto geometry() const -> WorldGeometry = 0;

  // Update frame counter.  Returns the frame that is *currently* being
  // updated, not the *last* frame, so during the first call to update(), this
  // would return 1
  [[nodiscard]] virtual auto currentStep() const -> std::uint64_t = 0;

  // All methods that take int parameters wrap around or clamp so that all int
  // values are valid world indexes.

  [[nodiscard]] virtual auto material(Vec2I const& position, TileLayer layer) const -> MaterialId = 0;
  [[nodiscard]] virtual auto materialHueShift(Vec2I const& position, TileLayer layer) const -> MaterialHue = 0;
  [[nodiscard]] virtual auto mod(Vec2I const& position, TileLayer layer) const -> ModId = 0;
  [[nodiscard]] virtual auto modHueShift(Vec2I const& position, TileLayer layer) const -> MaterialHue = 0;
  [[nodiscard]] virtual auto colorVariant(Vec2I const& position, TileLayer layer) const -> MaterialColorVariant = 0;
  [[nodiscard]] virtual auto liquidLevel(Vec2I const& pos) const -> LiquidLevel = 0;
  [[nodiscard]] virtual auto liquidLevel(RectF const& region) const -> LiquidLevel = 0;

  // Tests a tile modification list and returns the ones that are valid.
  [[nodiscard]] virtual auto validTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) const -> TileModificationList = 0;
  // Apply a list of tile modifications in the best order to apply as many
  // possible, and returns the modifications that could not be applied.
  virtual auto applyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) -> TileModificationList = 0;
  // Swap existing tiles for ones defined in the modification list,
  // and returns the modifications that could not be applied.
  virtual auto replaceTiles(TileModificationList const& modificationList, TileDamage const& tileDamage, bool applyDamage = false) -> TileModificationList = 0;
  // If an applied damage would destroy a tile
  [[nodiscard]] virtual auto damageWouldDestroy(Vec2I const& pos, TileLayer layer, TileDamage const& tileDamage) const -> bool = 0;

  [[nodiscard]] virtual auto isTileProtected(Vec2I const& pos) const -> bool = 0;

  [[nodiscard]] virtual auto entity(EntityId entityId) const -> Ptr<Entity> = 0;
  // *If* the entity is initialized immediately and locally, then will use the
  // passed in pointer directly and initialize it, and entity will have a valid
  // id in this world and be ready for use.  This is always the case on the
  // server, but not *always* the case on the client.
  virtual void addEntity(Ptr<Entity> const& entity, EntityId entityId = NullEntityId) = 0;

  [[nodiscard]] virtual auto closestEntity(Vec2F const& center, float radius, EntityFilter selector = {}) const -> Ptr<Entity> = 0;

  virtual void forAllEntities(EntityCallback entityCallback) const = 0;

  // Query here is a fuzzy query based on metaBoundBox
  virtual void forEachEntity(RectF const& boundBox, EntityCallback entityCallback) const = 0;
  // Fuzzy metaBoundBox query for intersecting the given line.
  virtual void forEachEntityLine(Vec2F const& begin, Vec2F const& end, EntityCallback entityCallback) const = 0;
  // Performs action for all entities that occupies the given tile position
  // (only entity types laid out in the tile grid).
  virtual void forEachEntityAtTile(Vec2I const& pos, EntityCallbackOf<TileEntity> entityCallback) const = 0;

  // Like forEachEntity, but stops scanning when entityFilter returns true, and
  // returns the EntityPtr found, otherwise returns a null pointer.
  [[nodiscard]] virtual auto findEntity(RectF const& boundBox, EntityFilter entityFilter) const -> Ptr<Entity> = 0;
  [[nodiscard]] virtual auto findEntityLine(Vec2F const& begin, Vec2F const& end, EntityFilter entityFilter) const -> Ptr<Entity> = 0;
  [[nodiscard]] virtual auto findEntityAtTile(Vec2I const& pos, EntityFilterOf<TileEntity> entityFilter) const -> Ptr<Entity> = 0;

  // Is the given tile layer and position occupied by an entity or block?
  [[nodiscard]] virtual auto tileIsOccupied(Vec2I const& pos, TileLayer layer, bool includeEphemeral = false, bool checkCollision = false) const -> bool = 0;

  // Returns the collision kind of a tile.
  [[nodiscard]] virtual auto tileCollisionKind(Vec2I const& pos) const -> CollisionKind = 0;

  // Iterate over the collision block for each tile in the region.  Collision
  // polys for tiles can extend to a maximum of 1 tile outside of the natural
  // tile bounds.
  virtual void forEachCollisionBlock(RectI const& region, std::function<void(CollisionBlock const&)> const& iterator) const = 0;

  // Is there some connectable tile / tile based entity in this position?  If
  // tilesOnly is true, only checks to see whether that tile is a connectable
  // material.
  [[nodiscard]] virtual auto isTileConnectable(Vec2I const& pos, TileLayer layer, bool tilesOnly = false) const -> bool = 0;

  // Returns whether or not a given point is inside any colliding tile.  If
  // collisionSet is Dynamic or Static, then does not intersect with platforms.
  [[nodiscard]] virtual auto pointTileCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool = 0;

  // Returns whether line intersects with any colliding tiles.
  [[nodiscard]] virtual auto lineTileCollision(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool = 0;
  [[nodiscard]] virtual auto lineTileCollisionPoint(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const -> std::optional<std::pair<Vec2F, Vec2I>> = 0;

  // Returns a list of all the collidable tiles along the given line.
  [[nodiscard]] virtual auto collidingTilesAlongLine(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet, int maxSize = -1, bool includeEdges = true) const -> List<Vec2I> = 0;

  // Returns whether the given rect contains any colliding tiles.
  [[nodiscard]] virtual auto rectTileCollision(RectI const& region, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool = 0;

  // Damage multiple tiles, avoiding duplication (objects or plants that occupy
  // more than one tile
  // position are only damaged once)
  virtual auto damageTiles(List<Vec2I> const& tilePositions, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, std::optional<EntityId> sourceEntity = {}) -> TileDamageResult = 0;

  [[nodiscard]] virtual auto getInteractiveInRange(Vec2F const& targetPosition, Vec2F const& sourcePosition, float maxRange) const -> Ptr<InteractiveEntity> = 0;
  // Can the target entity be reached from the given position within the given radius?
  [[nodiscard]] virtual auto canReachEntity(Vec2F const& position, float radius, EntityId targetEntity, bool preferInteractive = true) const -> bool = 0;
  virtual auto interact(InteractRequest const& request) -> RpcPromise<InteractAction> = 0;

  [[nodiscard]] virtual auto gravity(Vec2F const& pos) const -> float = 0;
  [[nodiscard]] virtual auto windLevel(Vec2F const& pos) const -> float = 0;
  [[nodiscard]] virtual auto lightLevel(Vec2F const& pos) const -> float = 0;
  [[nodiscard]] virtual auto breathable(Vec2F const& pos) const -> bool = 0;
  [[nodiscard]] virtual auto threatLevel() const -> float = 0;
  [[nodiscard]] virtual auto environmentStatusEffects(Vec2F const& pos) const -> StringList = 0;
  [[nodiscard]] virtual auto weatherStatusEffects(Vec2F const& pos) const -> StringList = 0;
  [[nodiscard]] virtual auto exposedToWeather(Vec2F const& pos) const -> bool = 0;
  [[nodiscard]] virtual auto isUnderground(Vec2F const& pos) const -> bool = 0;
  [[nodiscard]] virtual auto disableDeathDrops() const -> bool = 0;
  [[nodiscard]] virtual auto forceRegions() const -> List<PhysicsForceRegion> = 0;

  // Gets / sets world-wide properties
  [[nodiscard]] virtual auto getProperty(String const& propertyName, Json const& def = {}) const -> Json = 0;
  virtual void setProperty(String const& propertyName, Json const& property) = 0;

  virtual void timer(float delay, WorldAction worldAction) = 0;
  [[nodiscard]] virtual auto epochTime() const -> double = 0;
  [[nodiscard]] virtual auto day() const -> std::uint32_t = 0;
  [[nodiscard]] virtual auto dayLength() const -> float = 0;
  [[nodiscard]] virtual auto timeOfDay() const -> float = 0;

  virtual auto luaRoot() -> Ptr<LuaRoot> = 0;

  // Locate a unique entity, if the target is local, the promise will be
  // finished before being returned.  If the unique entity is not found, the
  // promise will fail.
  virtual auto findUniqueEntity(String const& uniqueEntityId) -> RpcPromise<Vec2F> = 0;

  // Send a message to a local or remote scripted entity.  If the target is
  // local, the promise will be finished before being returned.  Entity id can
  // either be EntityId or a uniqueId.
  virtual auto sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) -> RpcPromise<Json> = 0;

  // Helper non-virtual methods.

  [[nodiscard]] auto isServer() const -> bool;
  [[nodiscard]] auto isClient() const -> bool;

  [[nodiscard]] auto entityQuery(RectF const& boundBox, EntityFilter selector = {}) const -> List<Ptr<Entity>>;
  [[nodiscard]] auto entityLineQuery(Vec2F const& begin, Vec2F const& end, EntityFilter selector = {}) const -> List<Ptr<Entity>>;

  [[nodiscard]] auto entitiesAtTile(Vec2I const& pos, EntityFilter filter = EntityFilter()) const -> List<Ptr<TileEntity>>;

  // Find tiles near the given point that are not occupied (according to
  // tileIsOccupied)
  [[nodiscard]] auto findEmptyTiles(Vec2I pos, unsigned maxDist = 5, size_t maxAmount = 1, bool excludeEphemeral = false) const -> List<Vec2I>;

  // Do tile modification that only uses a single tile.
  [[nodiscard]] auto canModifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap) const -> bool;
  auto modifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap) -> bool;

  auto damageTile(Vec2I const& tilePosition, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, std::optional<EntityId> sourceEntity = {}) -> TileDamageResult;

  // Returns closest entity for which lineCollision between the given center
  // position and the entity position returns false.
  [[nodiscard]] auto closestEntityInSight(Vec2F const& center, float radius, CollisionSet const& collisionSet = DefaultCollisionSet, EntityFilter selector = {}) const -> Ptr<Entity>;

  // Returns whether point collides with any collision geometry.
  [[nodiscard]] auto pointCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool;

  // Returns first point along line that collides with any collision geometry, along
  // with the normal of the intersected line, if any.
  [[nodiscard]] auto lineCollision(Line2F const& line, CollisionSet const& collisionSet = DefaultCollisionSet) const -> std::optional<std::pair<Vec2F, std::optional<Vec2F>>>;

  // Returns whether poly collides with any collision geometry.
  [[nodiscard]] auto polyCollision(PolyF const& poly, CollisionSet const& collisionSet = DefaultCollisionSet) const -> bool;

  // Helper template methods.  Only queries entities of the given template
  // type, and casts them to the appropriate pointer type.

  template <typename EntityT>
  auto get(EntityId entityId) const -> std::shared_ptr<EntityT>;

  template <typename EntityT>
  auto query(RectF const& boundBox, EntityFilterOf<EntityT> selector = {}) const -> List<std::shared_ptr<EntityT>>;

  template <typename EntityT>
  auto closest(Vec2F const& center, float radius, EntityFilterOf<EntityT> selector = {}) const -> std::shared_ptr<EntityT>;

  template <typename EntityT>
  auto closestInSight(Vec2F const& center, float radius, CollisionSet const& collisionSet, EntityFilterOf<EntityT> selector = {}) const -> std::shared_ptr<EntityT>;

  template <typename EntityT>
  auto lineQuery(Vec2F const& begin, Vec2F const& end, EntityFilterOf<EntityT> selector = {}) const -> List<std::shared_ptr<EntityT>>;

  template <typename EntityT>
  auto atTile(Vec2I const& pos) const -> List<std::shared_ptr<EntityT>>;
};

template <typename EntityT>
auto World::get(EntityId entityId) const -> std::shared_ptr<EntityT> {
  return as<EntityT>(entity(entityId));
}

template <typename EntityT>
auto World::query(RectF const& boundBox, EntityFilterOf<EntityT> selector) const -> List<std::shared_ptr<EntityT>> {
  List<std::shared_ptr<EntityT>> list;
  forEachEntity(boundBox, [&](Ptr<Entity> const& entity) -> auto {
    if (auto e = as<EntityT>(entity)) {
      if (!selector || selector(e))
        list.append(std::move(e));
    }
  });

  return list;
}

template <typename EntityT>
auto World::closest(Vec2F const& center, float radius, EntityFilterOf<EntityT> selector) const -> std::shared_ptr<EntityT> {
  return as<EntityT>(closestEntity(center, radius, entityTypeFilter<EntityT>(selector)));
}

template <typename EntityT>
auto World::closestInSight(
  Vec2F const& center, float radius, CollisionSet const& collisionSet, EntityFilterOf<EntityT> selector) const -> std::shared_ptr<EntityT> {
  return as<EntityT>(closestEntityInSight(center, radius, collisionSet, entityTypeFilter<EntityT>(selector)));
}

template <typename EntityT>
auto World::lineQuery(
  Vec2F const& begin, Vec2F const& end, EntityFilterOf<EntityT> selector) const -> List<std::shared_ptr<EntityT>> {
  List<std::shared_ptr<EntityT>> list;
  forEachEntityLine(begin, end, [&](Ptr<Entity> entity) -> auto {
    if (auto e = as<EntityT>(std::move(entity))) {
      if (!selector || selector(e))
        list.append(std::move(e));
    }
  });

  return list;
}

template <typename EntityT>
auto World::atTile(Vec2I const& pos) const -> List<std::shared_ptr<EntityT>> {
  List<std::shared_ptr<EntityT>> list;
  forEachEntityAtTile(pos, [&](Ptr<TileEntity> const& entity) -> auto {
    if (auto e = as<EntityT>(entity))
      list.append(std::move(e));
  });
  return list;
}
}// namespace Star

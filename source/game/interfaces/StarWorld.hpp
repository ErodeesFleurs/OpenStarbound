#pragma once

#include "StarAssets.hpp"
#include "StarCollisionBlock.hpp"
#include "StarEffectSourceDatabase.hpp"
#include "StarEntityWorldInterface.hpp"
#include "StarForceRegions.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarInteractionTypes.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLuaRoot.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarParticleDatabase.hpp"
#include "StarPlantDatabase.hpp"
#include "StarRpcPromise.hpp"
#include "StarStatusEffectDatabase.hpp"
#include "StarStoredFunctions.hpp"
#include "StarTechDatabase.hpp"
#include "StarTileEntity.hpp"
#include "StarTileModification.hpp"
#include "StarTileWorldInterface.hpp"
#include "StarTreasure.hpp"
#include "StarWorldGeometry.hpp"

namespace Star {

class World;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class ProjectileDatabase;
using ProjectileDatabaseConstPtr = SharedPtr<ProjectileDatabase const>;
class TileEntity;
class ScriptedEntity;
class BehaviorDatabase;
using BehaviorDatabaseConstPtr = SharedPtr<BehaviorDatabase const>;

using WorldAction = function<void(World*)>;

class World : public virtual TileWorldInterface, public virtual EntityWorldInterface {
public:
  virtual ~World() = default;

  // World metadata
  [[nodiscard]] virtual ConnectionId connection() const = 0;
  [[nodiscard]] virtual WorldGeometry geometry() const = 0;
  [[nodiscard]] virtual uint64_t currentStep() const = 0;
  [[nodiscard]] virtual AssetsConstPtr const& assets() const = 0;
  [[nodiscard]] virtual ItemDatabaseConstPtr const& itemDatabase() const = 0;
  [[nodiscard]] virtual ObjectDatabaseConstPtr const& objectDatabase() const = 0;
  [[nodiscard]] virtual MaterialDatabaseConstPtr const& materialDatabase() const = 0;
  [[nodiscard]] virtual LiquidsDatabaseConstPtr const& liquidsDatabase() const = 0;
  [[nodiscard]] virtual EffectSourceDatabaseConstPtr const& effectSourceDatabase() const = 0;
  [[nodiscard]] virtual ParticleDatabaseConstPtr const& particleDatabase() const = 0;
  [[nodiscard]] virtual ProjectileDatabaseConstPtr const& projectileDatabase() const = 0;
  [[nodiscard]] virtual TechDatabaseConstPtr const& techDatabase() const = 0;
  [[nodiscard]] virtual StatusEffectDatabaseConstPtr const& statusEffectDatabase() const = 0;
  [[nodiscard]] virtual PlantDatabaseConstPtr const& plantDatabase() const = 0;
  [[nodiscard]] virtual TreasureDatabaseConstPtr const& treasureDatabase() const = 0;
  [[nodiscard]] virtual ImageMetadataDatabaseConstPtr const& imageMetadataDatabase() const = 0;
  [[nodiscard]] virtual FunctionDatabaseConstPtr const& functionDatabase() const = 0;
  [[nodiscard]] virtual BehaviorDatabaseConstPtr const& behaviorDatabase() const = 0;

  // Environment
  [[nodiscard]] virtual float gravity(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual float windLevel(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual float lightLevel(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual bool breathable(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual float threatLevel() const = 0;
  [[nodiscard]] virtual StringList environmentStatusEffects(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual StringList weatherStatusEffects(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual bool exposedToWeather(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual bool isUnderground(Vec2F const& pos) const = 0;
  [[nodiscard]] virtual bool disableDeathDrops() const = 0;
  [[nodiscard]] virtual List<PhysicsForceRegion> forceRegions() const = 0;

  // Properties / messaging
  [[nodiscard]] virtual Json getProperty(String const& propertyName, Json const& def = {}) const = 0;
  virtual void setProperty(String const& propertyName, Json const& property) = 0;

  virtual void timer(float delay, WorldAction worldAction) = 0;
  [[nodiscard]] virtual double epochTime() const = 0;
  [[nodiscard]] virtual uint32_t day() const = 0;
  [[nodiscard]] virtual float dayLength() const = 0;
  [[nodiscard]] virtual float timeOfDay() const = 0;

  [[nodiscard]] virtual LuaRootPtr luaRoot() = 0;

  [[nodiscard]] virtual RpcPromise<Vec2F> findUniqueEntity(String const& uniqueEntityId) = 0;
  [[nodiscard]] virtual RpcPromise<Json> sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) = 0;

  // Helper non-virtual methods.

  [[nodiscard]] bool isServer() const;
  [[nodiscard]] bool isClient() const;

  [[nodiscard]] List<EntityPtr> entityQuery(RectF const& boundBox, EntityFilter selector = {}) const;
  [[nodiscard]] List<EntityPtr> entityLineQuery(Vec2F const& begin, Vec2F const& end, EntityFilter selector = {}) const;

  [[nodiscard]] List<TileEntityPtr> entitiesAtTile(Vec2I const& pos, EntityFilter filter = EntityFilter()) const;

  // Find tiles near the given point that are not occupied (according to
  // tileIsOccupied)
  [[nodiscard]] List<Vec2I> findEmptyTiles(Vec2I pos, unsigned maxDist = 5, size_t maxAmount = 1, bool excludeEphemeral = false) const;

  // Do tile modification that only uses a single tile.
  [[nodiscard]] bool canModifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap) const;
  [[nodiscard]] bool modifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap);

  [[nodiscard]] TileDamageResult damageTile(Vec2I const& tilePosition, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, Maybe<EntityId> sourceEntity = {});

  // Returns closest entity for which lineCollision between the given center
  // position and the entity position returns false.
  [[nodiscard]] EntityPtr closestEntityInSight(Vec2F const& center, float radius, CollisionSet const& collisionSet = DefaultCollisionSet, EntityFilter selector = {}) const;

  // Returns whether point collides with any collision geometry.
  [[nodiscard]] bool pointCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const;

  // Returns first point along line that collides with any collision geometry, along
  // with the normal of the intersected line, if any.
  [[nodiscard]] Maybe<pair<Vec2F, Maybe<Vec2F>>> lineCollision(Line2F const& line, CollisionSet const& collisionSet = DefaultCollisionSet) const;

  // Returns whether poly collides with any collision geometry.
  [[nodiscard]] bool polyCollision(PolyF const& poly, CollisionSet const& collisionSet = DefaultCollisionSet) const;

  // Helper template methods.  Only queries entities of the given template
  // type, and casts them to the appropriate pointer type.

  template <typename EntityT>
  [[nodiscard]] SharedPtr<EntityT> get(EntityId entityId) const;

  template <typename EntityT>
  [[nodiscard]] List<SharedPtr<EntityT>> query(RectF const& boundBox, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  [[nodiscard]] SharedPtr<EntityT> closest(Vec2F const& center, float radius, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  [[nodiscard]] SharedPtr<EntityT> closestInSight(Vec2F const& center, float radius, CollisionSet const& collisionSet, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  [[nodiscard]] List<SharedPtr<EntityT>> lineQuery(Vec2F const& begin, Vec2F const& end, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  [[nodiscard]] List<SharedPtr<EntityT>> atTile(Vec2I const& pos) const;
};

template <typename EntityT>
SharedPtr<EntityT> World::get(EntityId entityId) const {
  return as<EntityT>(entity(entityId));
}

template <typename EntityT>
List<SharedPtr<EntityT>> World::query(RectF const& boundBox, EntityFilterOf<EntityT> selector) const {
  List<SharedPtr<EntityT>> list;
  forEachEntity(boundBox, [&](EntityPtr const& entity) {
    if (auto typedEntity = as<EntityT>(entity)) {
      if (!selector || selector(typedEntity))
        list.append(std::move(typedEntity));
    }
  });

  return list;
}

template <typename EntityT>
SharedPtr<EntityT> World::closest(Vec2F const& center, float radius, EntityFilterOf<EntityT> selector) const {
  return as<EntityT>(closestEntity(center, radius, entityTypeFilter<EntityT>(selector)));
}

template <typename EntityT>
SharedPtr<EntityT> World::closestInSight(
  Vec2F const& center, float radius, CollisionSet const& collisionSet, EntityFilterOf<EntityT> selector) const {
  return as<EntityT>(closestEntityInSight(center, radius, collisionSet, entityTypeFilter<EntityT>(selector)));
}

template <typename EntityT>
List<SharedPtr<EntityT>> World::lineQuery(
  Vec2F const& begin, Vec2F const& end, EntityFilterOf<EntityT> selector) const {
  List<SharedPtr<EntityT>> list;
  forEachEntityLine(begin, end, [&](EntityPtr entity) {
    if (auto typedEntity = as<EntityT>(std::move(entity))) {
      if (!selector || selector(typedEntity))
        list.append(std::move(typedEntity));
    }
  });

  return list;
}

template <typename EntityT>
List<SharedPtr<EntityT>> World::atTile(Vec2I const& pos) const {
  List<SharedPtr<EntityT>> list;
  forEachEntityAtTile(pos, [&](TileEntityPtr const& entity) {
    if (auto typedEntity = as<EntityT>(entity))
      list.append(std::move(typedEntity));
  });
  return list;
}
}// namespace Star

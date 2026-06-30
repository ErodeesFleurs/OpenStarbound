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
  virtual ConnectionId connection() const = 0;
  virtual WorldGeometry geometry() const = 0;
  virtual uint64_t currentStep() const = 0;
  virtual AssetsConstPtr assets() const = 0;
  virtual ItemDatabaseConstPtr itemDatabase() const = 0;
  virtual ObjectDatabaseConstPtr objectDatabase() const = 0;
  virtual MaterialDatabaseConstPtr materialDatabase() const = 0;
  virtual LiquidsDatabaseConstPtr liquidsDatabase() const = 0;
  virtual EffectSourceDatabaseConstPtr effectSourceDatabase() const = 0;
  virtual ParticleDatabaseConstPtr particleDatabase() const = 0;
  virtual ProjectileDatabaseConstPtr projectileDatabase() const = 0;
  virtual TechDatabaseConstPtr techDatabase() const = 0;
  virtual StatusEffectDatabaseConstPtr statusEffectDatabase() const = 0;
  virtual PlantDatabaseConstPtr plantDatabase() const = 0;
  virtual TreasureDatabaseConstPtr treasureDatabase() const = 0;
  virtual ImageMetadataDatabaseConstPtr imageMetadataDatabase() const = 0;
  virtual FunctionDatabaseConstPtr functionDatabase() const = 0;
  virtual BehaviorDatabaseConstPtr behaviorDatabase() const = 0;

  // Environment
  virtual float gravity(Vec2F const& pos) const = 0;
  virtual float windLevel(Vec2F const& pos) const = 0;
  virtual float lightLevel(Vec2F const& pos) const = 0;
  virtual bool breathable(Vec2F const& pos) const = 0;
  virtual float threatLevel() const = 0;
  virtual StringList environmentStatusEffects(Vec2F const& pos) const = 0;
  virtual StringList weatherStatusEffects(Vec2F const& pos) const = 0;
  virtual bool exposedToWeather(Vec2F const& pos) const = 0;
  virtual bool isUnderground(Vec2F const& pos) const = 0;
  virtual bool disableDeathDrops() const = 0;
  virtual List<PhysicsForceRegion> forceRegions() const = 0;

  // Properties / messaging
  virtual Json getProperty(String const& propertyName, Json const& def = {}) const = 0;
  virtual void setProperty(String const& propertyName, Json const& property) = 0;

  virtual void timer(float delay, WorldAction worldAction) = 0;
  virtual double epochTime() const = 0;
  virtual uint32_t day() const = 0;
  virtual float dayLength() const = 0;
  virtual float timeOfDay() const = 0;

  virtual LuaRootPtr luaRoot() = 0;

  virtual RpcPromise<Vec2F> findUniqueEntity(String const& uniqueEntityId) = 0;
  virtual RpcPromise<Json> sendEntityMessage(Variant<EntityId, String> const& entity, String const& message, JsonArray const& args = {}) = 0;

  // Helper non-virtual methods.

  bool isServer() const;
  bool isClient() const;

  List<EntityPtr> entityQuery(RectF const& boundBox, EntityFilter selector = {}) const;
  List<EntityPtr> entityLineQuery(Vec2F const& begin, Vec2F const& end, EntityFilter selector = {}) const;

  List<TileEntityPtr> entitiesAtTile(Vec2I const& pos, EntityFilter filter = EntityFilter()) const;

  // Find tiles near the given point that are not occupied (according to
  // tileIsOccupied)
  List<Vec2I> findEmptyTiles(Vec2I pos, unsigned maxDist = 5, size_t maxAmount = 1, bool excludeEphemeral = false) const;

  // Do tile modification that only uses a single tile.
  bool canModifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap) const;
  bool modifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap);

  TileDamageResult damageTile(Vec2I const& tilePosition, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, Maybe<EntityId> sourceEntity = {});

  // Returns closest entity for which lineCollision between the given center
  // position and the entity position returns false.
  EntityPtr closestEntityInSight(Vec2F const& center, float radius, CollisionSet const& collisionSet = DefaultCollisionSet, EntityFilter selector = {}) const;

  // Returns whether point collides with any collision geometry.
  bool pointCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const;

  // Returns first point along line that collides with any collision geometry, along
  // with the normal of the intersected line, if any.
  Maybe<pair<Vec2F, Maybe<Vec2F>>> lineCollision(Line2F const& line, CollisionSet const& collisionSet = DefaultCollisionSet) const;

  // Returns whether poly collides with any collision geometry.
  bool polyCollision(PolyF const& poly, CollisionSet const& collisionSet = DefaultCollisionSet) const;

  // Helper template methods.  Only queries entities of the given template
  // type, and casts them to the appropriate pointer type.

  template <typename EntityT>
  SharedPtr<EntityT> get(EntityId entityId) const;

  template <typename EntityT>
  List<SharedPtr<EntityT>> query(RectF const& boundBox, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  SharedPtr<EntityT> closest(Vec2F const& center, float radius, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  SharedPtr<EntityT> closestInSight(Vec2F const& center, float radius, CollisionSet const& collisionSet, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  List<SharedPtr<EntityT>> lineQuery(Vec2F const& begin, Vec2F const& end, EntityFilterOf<EntityT> selector = {}) const;

  template <typename EntityT>
  List<SharedPtr<EntityT>> atTile(Vec2I const& pos) const;
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

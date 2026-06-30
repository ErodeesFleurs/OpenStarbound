#pragma once

#include "StarWorld.hpp"

namespace Star {

// Minimal World stub for unit testing.  All pure virtual methods return safe
// defaults: false for bool, zero for numeric types, empty for containers,
// nullptr for pointers, {} for Maybe.
//
// Override specific methods in derived test classes as needed.
class WorldStub : public World {
public:
  ConnectionId connection() const override { return ConnectionId(0); }
  WorldGeometry geometry() const override { return WorldGeometry(Vec2U(4096, 4096)); }
  uint64_t currentStep() const override { return 0; }

  MaterialId material(Vec2I const&, TileLayer) const override { return EmptyMaterialId; }
  std::tuple<MaterialId, ModId> materialAndMod(Vec2I const&, TileLayer) const override { return {}; }
  MaterialHue materialHueShift(Vec2I const&, TileLayer) const override { return MaterialHue(); }
  ModId mod(Vec2I const&, TileLayer) const override { return NullModId; }
  MaterialHue modHueShift(Vec2I const&, TileLayer) const override { return MaterialHue(); }
  MaterialColorVariant colorVariant(Vec2I const&, TileLayer) const override { return DefaultMaterialColorVariant; }
  LiquidLevel liquidLevel(Vec2I const&) const override { return {}; }
  LiquidLevel liquidLevel(RectF const&) const override { return {}; }

  TileModificationList validTileModifications(TileModificationList const&, bool) const override { return {}; }
  TileModificationList applyTileModifications(TileModificationList const&, bool) override { return {}; }
  TileModificationList replaceTiles(TileModificationList const&, TileDamage const&, bool) override { return {}; }
  bool damageWouldDestroy(Vec2I const&, TileLayer, TileDamage const&) const override { return false; }
  bool isTileProtected(Vec2I const&) const override { return false; }

  EntityPtr entity(EntityId) const override { return {}; }
  void addEntity(EntityPtr const&, EntityId) override {}

  EntityPtr closestEntity(Vec2F const&, float, EntityFilter) const override { return {}; }
  void forAllEntities(EntityCallback) const override {}
  void forEachEntity(RectF const&, EntityCallback) const override {}
  void forEachEntityLine(Vec2F const&, Vec2F const&, EntityCallback) const override {}
  void forEachEntityAtTile(Vec2I const&, EntityCallbackOf<TileEntity>) const override {}
  EntityPtr findEntity(RectF const&, EntityFilter) const override { return {}; }
  EntityPtr findEntityLine(Vec2F const&, Vec2F const&, EntityFilter) const override { return {}; }
  EntityPtr findEntityAtTile(Vec2I const&, EntityFilterOf<TileEntity>) const override { return {}; }

  bool tileIsOccupied(Vec2I const&, TileLayer, bool, bool) const override { return false; }
  CollisionKind tileCollisionKind(Vec2I const&) const override { return CollisionKind::Null; }
  void forEachCollisionBlock(RectI const&, function<void(CollisionBlock const&)> const&) const override {}
  bool isTileConnectable(Vec2I const&, TileLayer, bool) const override { return false; }
  bool pointTileCollision(Vec2F const&, CollisionSet const&) const override { return false; }
  bool lineTileCollision(Vec2F const&, Vec2F const&, CollisionSet const&) const override { return false; }
  Maybe<pair<Vec2F, Vec2I>> lineTileCollisionPoint(Vec2F const&, Vec2F const&, CollisionSet const&) const override { return {}; }
  List<Vec2I> collidingTilesAlongLine(Vec2F const&, Vec2F const&, CollisionSet const&, int, bool) const override { return {}; }
  bool rectTileCollision(RectI const&, CollisionSet const&) const override { return false; }
  TileDamageResult damageTiles(List<Vec2I> const&, TileLayer, Vec2F const&, TileDamage const&, Maybe<EntityId>) override { return {}; }

  InteractiveEntityPtr getInteractiveInRange(Vec2F const&, Vec2F const&, float) const override { return {}; }
  bool canReachEntity(Vec2F const&, float, EntityId, bool) const override { return false; }
  RpcPromise<InteractAction> interact(InteractRequest const&) override { return RpcPromise<InteractAction>(); }

  float gravity(Vec2F const&) const override { return 80.0f; }
  float windLevel(Vec2F const&) const override { return 0.0f; }
  float lightLevel(Vec2F const&) const override { return 1.0f; }
  bool breathable(Vec2F const&) const override { return true; }
  float threatLevel() const override { return 0.0f; }
  StringList environmentStatusEffects(Vec2F const&) const override { return {}; }
  StringList weatherStatusEffects(Vec2F const&) const override { return {}; }
  bool exposedToWeather(Vec2F const&) const override { return false; }
  bool isUnderground(Vec2F const&) const override { return false; }
  bool disableDeathDrops() const override { return false; }
  List<PhysicsForceRegion> forceRegions() const override { return {}; }

  Json getProperty(String const&, Json const& def) const override { return def; }
  void setProperty(String const&, Json const&) override {}

  void timer(float, WorldAction) override {}
  double epochTime() const override { return 0.0; }
  uint32_t day() const override { return 0; }
  float dayLength() const override { return 1200.0f; }
  float timeOfDay() const override { return 0.5f; }

  LuaRootPtr luaRoot() override { return {}; }

  RpcPromise<Vec2F> findUniqueEntity(String const&) override { return RpcPromise<Vec2F>(); }
  RpcPromise<Json> sendEntityMessage(Variant<EntityId, String> const&, String const&, JsonArray const&) override { return RpcPromise<Json>(); }
};

}

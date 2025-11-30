#pragma once

// ECS ItemDrop Adapter for OpenStarbound
// This adapter implements the ItemDrop entity using ECS components

#include "StarEntityAdapter.hpp"
#include "StarItemDescriptor.hpp"
#include "StarDrawable.hpp"
#include "StarGameTimers.hpp"

namespace Star {

// Forward declare
STAR_CLASS(Item);

namespace ECS {

// ItemDrop-specific components

// Stores the item data for an item drop
struct ItemDropDataComponent {
  ItemDescriptor itemDescriptor;
  bool eternal = false;
  
  // Drop mode
  enum class Mode { Intangible, Available, Taken, Dead };
  Mode mode = Mode::Intangible;
  
  // Owner when taken
  EntityId owningEntity = NullEntityId;
  
  // Timers and state
  float intangibleTime = 0.0f;
  float dropAge = 0.0f;
  float takenTimer = 0.0f;
  float ageItemsTimer = 0.0f;
  
  // Pickup settings
  float pickupDistance = 1.5f;
  float combineChance = 0.5f;
  float combineRadius = 2.0f;
};

// ItemDrop adapter that wraps ECS entity
class ItemDropAdapter : public EntityAdapter {
public:
  // Creates a drop at the given position with randomized velocity
  static shared_ptr<ItemDropAdapter> createRandomizedDrop(
    World* ecsWorld, ItemPtr const& item, Vec2F const& position, bool eternal = false);
  static shared_ptr<ItemDropAdapter> createRandomizedDrop(
    World* ecsWorld, ItemDescriptor const& itemDescriptor, Vec2F const& position, bool eternal = false);
  
  // Create a drop and throw in the given direction
  static shared_ptr<ItemDropAdapter> throwDrop(
    World* ecsWorld, ItemPtr const& item, Vec2F const& position, 
    Vec2F const& velocity, Vec2F const& direction, bool eternal = false);
  static shared_ptr<ItemDropAdapter> throwDrop(
    World* ecsWorld, ItemDescriptor const& itemDescriptor, Vec2F const& position,
    Vec2F const& velocity, Vec2F const& direction, bool eternal = false);
  
  // Construct from existing ECS entity
  ItemDropAdapter(World* ecsWorld, Entity ecsEntity);
  
  // Construct new item drop
  ItemDropAdapter(World* ecsWorld, Entity ecsEntity, ItemPtr item);
  ItemDropAdapter(World* ecsWorld, Entity ecsEntity, ItemDescriptor const& itemDescriptor);
  
  // Serialization
  Json diskStore() const;
  ByteArray netStore(NetCompatibilityRules rules = {}) const;
  
  // Entity interface
  EntityType entityType() const override;
  
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  Vec2F position() const override;
  RectF metaBoundBox() const override;
  RectF collisionArea() const override;
  
  bool ephemeral() const override;
  
  bool shouldDestroy() const override;
  
  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderCallback) override;
  void renderLightSources(RenderCallback* renderCallback) override;
  
  // ItemDrop-specific methods
  ItemPtr item() const;
  
  void setEternal(bool eternal);
  void setIntangibleTime(float intangibleTime);
  
  // Mark this drop as taken by the given entity
  ItemPtr takeBy(EntityId entityId, float timeOffset = 0.0f);
  
  // Mark this drop as taken, disappear immediately
  ItemPtr take();
  
  // Item is not taken and is not intangible
  bool canTake() const;
  
  void setPosition(Vec2F const& position);
  
  Vec2F velocity() const;
  void setVelocity(Vec2F const& velocity);

private:
  void setupComponents(ItemDescriptor const& itemDescriptor, bool eternal);
  void updateCollisionPoly();
  void updateTaken(bool master, float dt);
  
  // Cached item (loaded from descriptor when needed)
  mutable ItemPtr m_cachedItem;
  
  // Configuration from root
  Json m_config;
  RectF m_defaultBoundBox;
  float m_afterTakenLife = 1.0f;
  float m_overheadTime = 1.0f;
  float m_velocityApproach = 50.0f;
  float m_overheadApproach = 20.0f;
  Vec2F m_overheadOffset = {0.0f, 2.0f};
  double m_ageItemsEvery = 10.0;
};

using ItemDropAdapterPtr = shared_ptr<ItemDropAdapter>;

} // namespace ECS
} // namespace Star

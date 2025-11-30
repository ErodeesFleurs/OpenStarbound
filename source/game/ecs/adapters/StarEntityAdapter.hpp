#pragma once

// Base ECS Entity Adapter for OpenStarbound
// This adapter bridges the existing Entity interface with the new ECS architecture.
// It allows gradual migration of entity types without breaking existing code.

#include "ecs/StarEcs.hpp"
#include "ecs/components/StarGameComponents.hpp"
#include "interfaces/StarEntity.hpp"

namespace Star {
namespace ECS {

// Forward declarations
class AdapterWorld;

// Base adapter class that wraps an ECS entity to implement the Entity interface
// Subclasses implement specific entity types (ItemDrop, Monster, Player, etc.)
class EntityAdapter : public virtual Entity {
public:
  EntityAdapter(World* ecsWorld, Entity ecsEntity);
  virtual ~EntityAdapter();
  
  // Access to ECS internals
  World* ecsWorld() const { return m_ecsWorld; }
  Entity ecsEntity() const { return m_ecsEntity; }
  
  // Entity interface implementation
  void init(Star::World* world, EntityId entityId, EntityMode mode) override;
  void uninit() override;
  
  pair<ByteArray, uint64_t> writeNetState(uint64_t fromVersion = 0, NetCompatibilityRules rules = {}) override;
  void readNetState(ByteArray data, float interpolationTime = 0.0f, NetCompatibilityRules rules = {}) override;
  
  void enableInterpolation(float extrapolationHint) override;
  void disableInterpolation() override;
  
  Vec2F position() const override;
  RectF metaBoundBox() const override;
  RectF collisionArea() const override;
  
  bool ephemeral() const override;
  ClientEntityMode clientEntityMode() const override;
  bool masterOnly() const override;
  
  String name() const override;
  String description() const override;
  
  List<LightSource> lightSources() const override;
  List<DamageSource> damageSources() const override;
  
  Maybe<HitType> queryHit(DamageSource const& source) const override;
  Maybe<PolyF> hitPoly() const override;
  List<DamageNotification> applyDamage(DamageRequest const& damage) override;
  List<DamageNotification> selfDamageNotifications() override;
  
  void hitOther(EntityId targetEntityId, DamageRequest const& damageRequest) override;
  void damagedOther(DamageNotification const& damage) override;
  
  bool shouldDestroy() const override;
  void destroy(RenderCallback* renderCallback) override;
  
  Maybe<Json> receiveMessage(ConnectionId sendingConnection, String const& message, JsonArray const& args) override;
  
  void update(float dt, uint64_t currentStep) override;
  void render(RenderCallback* renderer) override;
  void renderLightSources(RenderCallback* renderer) override;

protected:
  // Helper methods for component access
  template<typename T>
  T* getComponent() {
    return m_ecsWorld->getComponent<T>(m_ecsEntity);
  }
  
  template<typename T>
  T const* getComponent() const {
    return m_ecsWorld->getComponent<T>(m_ecsEntity);
  }
  
  template<typename T>
  bool hasComponent() const {
    return m_ecsWorld->hasComponent<T>(m_ecsEntity);
  }
  
  template<typename T, typename... Args>
  T& addComponent(Args&&... args) {
    return m_ecsWorld->addComponent<T>(m_ecsEntity, std::forward<Args>(args)...);
  }
  
  template<typename T>
  void removeComponent() {
    m_ecsWorld->removeComponent<T>(m_ecsEntity);
  }
  
  // Mark network state as dirty
  void markNetworkDirty();
  
  World* m_ecsWorld;
  Entity m_ecsEntity;
};

// Helper function to create adapters
template<typename AdapterType, typename... Args>
shared_ptr<AdapterType> makeAdapter(World* ecsWorld, Args&&... args) {
  Entity entity = ecsWorld->createEntity();
  return make_shared<AdapterType>(ecsWorld, entity, std::forward<Args>(args)...);
}

} // namespace ECS
} // namespace Star

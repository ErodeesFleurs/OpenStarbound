#pragma once

#include "ecs/StarEcs.hpp"
#include "StarGameComponents.hpp"

namespace Star {
namespace ECS {

// Damage system - processes damage between entities
// Priority: 80 (runs after movement but before rendering)
class DamageSystem : public System {
public:
  DamageSystem();
  
  void update(float dt) override;
  int priority() const override { return 80; }

private:
  // Check if damage source can hit damage receiver
  bool canDamage(Entity source, DamageSourceComponent const& sourceComp,
                 Entity target, DamageReceiverComponent const& targetComp) const;
  
  // Calculate damage amount after applying modifiers
  float calculateDamage(DamageSource const& source, DamageReceiverComponent const& receiver) const;
  
  // Apply damage to an entity's health
  void applyDamage(Entity entity, HealthComponent& health, float damage);
  
  // Check for damage collision between entities
  bool checkDamageCollision(TransformComponent const& sourceTransform, BoundsComponent const& sourceBounds,
                           TransformComponent const& targetTransform, DamageReceiverComponent const& targetReceiver) const;
};

} // namespace ECS
} // namespace Star

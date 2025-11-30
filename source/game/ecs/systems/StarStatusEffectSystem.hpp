#pragma once

#include "ecs/StarEcs.hpp"
#include "StarGameComponents.hpp"

namespace Star {
namespace ECS {

// Status effect system - manages status effects on entities
// Priority: 70 (runs after damage but before rendering)
class StatusEffectSystem : public System {
public:
  StatusEffectSystem();
  
  void update(float dt) override;
  int priority() const override { return 70; }

private:
  // Update effect durations and remove expired effects
  void updateEffectDurations(StatusEffectsComponent& status, float dt);
  
  // Apply effect modifications to entity stats
  void applyStatModifications(Entity entity, StatusEffectsComponent const& status);
  
  // Update resources (health regen, energy regen, etc.)
  void updateResources(Entity entity, HealthComponent* health, EnergyComponent* energy, float dt);
};

} // namespace ECS
} // namespace Star

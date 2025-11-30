#include "StarStatusEffectSystem.hpp"

namespace Star {
namespace ECS {

StatusEffectSystem::StatusEffectSystem() = default;

void StatusEffectSystem::update(float dt) {
  // Process all entities with status effects
  for (auto [entity, status] : m_world->view<StatusEffectsComponent>()) {
    // Update effect durations
    updateEffectDurations(status, dt);
    
    // Apply stat modifications from effects
    applyStatModifications(entity, status);
  }
  
  // Update resources for all entities with health/energy
  for (auto [entity, health] : m_world->view<HealthComponent>()) {
    EnergyComponent* energy = m_world->getComponent<EnergyComponent>(entity);
    StatusEffectsComponent* status = m_world->getComponent<StatusEffectsComponent>(entity);
    
    // Only update if has status effects or energy component
    if (status || energy) {
      updateResources(entity, &health, energy, dt);
    }
  }
}

void StatusEffectSystem::updateEffectDurations(StatusEffectsComponent& status, float dt) {
  // Update ephemeral effects and remove expired ones
  auto it = status.ephemeralEffects.begin();
  while (it != status.ephemeralEffects.end()) {
    if (it->duration) {
      *it->duration -= dt;
      if (*it->duration <= 0.0f) {
        it = status.ephemeralEffects.erase(it);
        continue;
      }
    }
    ++it;
  }
}

void StatusEffectSystem::applyStatModifications(Entity entity, StatusEffectsComponent const& status) {
  // Apply stat modifications from persistent effects
  // This would typically modify stats stored in other components
  // For now, this is a placeholder for future stat system integration
  
  // Example: Iterate through persistent effects and apply their modifiers
  for (auto const& effect : status.persistentEffects) {
    // Apply effect based on type
    // This requires integration with the stat system
  }
}

void StatusEffectSystem::updateResources(Entity entity, HealthComponent* health, EnergyComponent* energy, float dt) {
  // Health regeneration
  if (health && !health->dead && health->currentHealth < health->maxHealth) {
    // Base health regen could be modified by status effects
    // For now, no automatic health regen
  }
  
  // Energy regeneration
  if (energy && !energy->locked && energy->currentEnergy < energy->maxEnergy) {
    // Check regen block
    if (energy->regenBlockPercent < 1.0f) {
      float effectiveRegenRate = energy->regenRate * (1.0f - energy->regenBlockPercent);
      energy->currentEnergy += effectiveRegenRate * dt;
      
      // Clamp to max
      if (energy->currentEnergy > energy->maxEnergy) {
        energy->currentEnergy = energy->maxEnergy;
      }
    }
    
    // Decay regen block over time
    if (energy->regenBlockPercent > 0.0f) {
      energy->regenBlockPercent -= dt;
      if (energy->regenBlockPercent < 0.0f) {
        energy->regenBlockPercent = 0.0f;
      }
    }
  }
}

} // namespace ECS
} // namespace Star

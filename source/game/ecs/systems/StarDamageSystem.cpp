#include "StarDamageSystem.hpp"

namespace Star {
namespace ECS {

DamageSystem::DamageSystem() = default;

void DamageSystem::update(float dt) {
  // Collect all damage sources
  for (auto [sourceEntity, sourceTransform, damageSource] : 
       m_world->view<TransformComponent, DamageSourceComponent>()) {
    
    // Skip if no active damage sources
    if (damageSource.damageSources.empty()) {
      continue;
    }
    
    // Get source bounds if available
    BoundsComponent* sourceBounds = m_world->getComponent<BoundsComponent>(sourceEntity);
    
    // Check against all damage receivers
    for (auto [targetEntity, targetTransform, targetReceiver] : 
         m_world->view<TransformComponent, DamageReceiverComponent>()) {
      
      // Don't damage self
      if (sourceEntity == targetEntity) {
        continue;
      }
      
      // Check if can damage (team checks, etc.)
      if (!canDamage(sourceEntity, damageSource, targetEntity, targetReceiver)) {
        continue;
      }
      
      // Check collision if bounds available
      if (sourceBounds) {
        if (!checkDamageCollision(sourceTransform, *sourceBounds, targetTransform, targetReceiver)) {
          continue;
        }
      }
      
      // Apply each damage source
      for (auto const& source : damageSource.damageSources) {
        float damage = calculateDamage(source, targetReceiver);
        
        // Create damage notification
        DamageNotification notification;
        notification.sourceEntityId = static_cast<EntityId>(sourceEntity);
        notification.targetEntityId = static_cast<EntityId>(targetEntity);
        notification.damageDealt = damage;
        notification.healthLost = damage;
        notification.hitType = HitType::Hit;
        notification.damageSourceKind = source.damageType;
        notification.targetMaterialKind = "flesh"; // Default
        notification.position = targetTransform.position;
        
        targetReceiver.takeDamage(std::move(notification));
      }
    }
  }
  
  // Process pending damage on all receivers
  for (auto [entity, health, receiver] : 
       m_world->view<HealthComponent, DamageReceiverComponent>()) {
    
    List<DamageNotification> pendingDamage = receiver.pullDamage();
    
    for (auto const& notification : pendingDamage) {
      // Skip if invulnerable
      if (health.invulnerable) {
        continue;
      }
      
      applyDamage(entity, health, notification.damageDealt);
    }
  }
}

bool DamageSystem::canDamage(Entity source, DamageSourceComponent const& sourceComp,
                             Entity target, DamageReceiverComponent const& targetComp) const {
  // Check team damage rules
  auto const& sourceTeam = sourceComp.team;
  auto const& targetTeam = targetComp.team;
  
  // Same team entities typically don't damage each other (friendly fire check)
  // Friendly entities on the same team should not damage each other
  if (sourceTeam.type == targetTeam.type && sourceTeam.team == targetTeam.team) {
    // Allow self-damage for special cases, but prevent friendly fire on same team
    // Only block if both are on the same team AND the type is Friendly
    if (sourceTeam.type == EntityDamageTeam::Type::Friendly) {
      return false;
    }
  }
  
  return true;
}

float DamageSystem::calculateDamage(DamageSource const& source, DamageReceiverComponent const& receiver) const {
  // Basic damage calculation - can be extended with armor, resistances, etc.
  return source.damage;
}

void DamageSystem::applyDamage(Entity entity, HealthComponent& health, float damage) {
  if (health.dead) {
    return;
  }
  
  health.currentHealth -= damage;
  
  if (health.currentHealth <= 0.0f) {
    health.currentHealth = 0.0f;
    health.dead = true;
  }
}

bool DamageSystem::checkDamageCollision(TransformComponent const& sourceTransform, BoundsComponent const& sourceBounds,
                                        TransformComponent const& targetTransform, DamageReceiverComponent const& targetReceiver) const {
  // Get source world bounds
  RectF sourceWorldBounds = sourceBounds.worldBounds(sourceTransform.position);
  
  // Check against target hit poly if available
  if (targetReceiver.hitPoly) {
    PolyF worldHitPoly = targetReceiver.hitPoly->translated(targetTransform.position);
    return sourceWorldBounds.intersects(worldHitPoly.boundBox());
  }
  
  // Fallback to simple position-based check
  return sourceWorldBounds.contains(targetTransform.position);
}

} // namespace ECS
} // namespace Star

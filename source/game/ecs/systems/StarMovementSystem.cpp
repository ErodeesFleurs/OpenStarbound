#include "StarMovementSystem.hpp"

namespace Star {
namespace ECS {

MovementSystem::MovementSystem() = default;

void MovementSystem::update(float dt) {
  // Process all entities with transform, velocity, and physics components
  for (auto [entity, transform, velocity, physics] : 
       m_world->view<TransformComponent, VelocityComponent, PhysicsBodyComponent>()) {
    
    // Apply gravity if enabled
    if (physics.gravityEnabled) {
      applyGravity(velocity, physics, dt);
    }
    
    // Check if entity has collision component for friction calculations
    CollisionComponent* collision = m_world->getComponent<CollisionComponent>(entity);
    if (collision && physics.frictionEnabled) {
      applyFriction(velocity, physics, *collision, dt);
    }
    
    // Apply speed limits
    applySpeedLimits(velocity, physics);
    
    // Update position based on velocity
    updatePosition(transform, velocity, dt);
    
    // Apply acceleration to velocity for next frame
    velocity.velocity += velocity.acceleration * dt;
  }
  
  // Process entities with only transform and velocity (no physics, simple movement)
  for (auto [entity, transform, velocity] : m_world->view<TransformComponent, VelocityComponent>()) {
    // Skip if already processed with physics
    if (m_world->hasComponent<PhysicsBodyComponent>(entity)) {
      continue;
    }
    
    updatePosition(transform, velocity, dt);
    velocity.velocity += velocity.acceleration * dt;
  }
}

void MovementSystem::applyGravity(VelocityComponent& velocity, PhysicsBodyComponent const& physics, float dt) {
  velocity.velocity[1] -= m_gravity * physics.gravityMultiplier * dt;
}

void MovementSystem::applyFriction(VelocityComponent& velocity, PhysicsBodyComponent const& physics,
                                   CollisionComponent const& collision, float dt) {
  float friction = 0.0f;
  
  if (collision.onGround) {
    friction = physics.groundFriction;
  } else if (collision.inLiquid) {
    friction = physics.liquidFriction;
  } else {
    friction = physics.airFriction;
  }
  
  if (friction > 0.0f) {
    // Apply exponential decay friction
    float frictionMultiplier = std::exp(-friction * dt);
    velocity.velocity *= frictionMultiplier;
  }
}

void MovementSystem::updatePosition(TransformComponent& transform, VelocityComponent const& velocity, float dt) {
  transform.position += velocity.velocity * dt;
}

void MovementSystem::applySpeedLimits(VelocityComponent& velocity, PhysicsBodyComponent const& physics) {
  float speed = vmag(velocity.velocity);
  float maxSpeed = physics.maxSpeed > 0 ? physics.maxSpeed : m_maxSpeed;
  if (speed > maxSpeed) {
    velocity.velocity = vnorm(velocity.velocity) * maxSpeed;
  }
}

} // namespace ECS
} // namespace Star

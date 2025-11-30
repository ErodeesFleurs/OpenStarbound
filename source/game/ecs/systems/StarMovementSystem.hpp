#pragma once

#include "ecs/StarEcs.hpp"
#include "StarGameComponents.hpp"

namespace Star {
namespace ECS {

// Movement system - processes physics and movement for all entities
// Priority: 100 (runs early to update positions before other systems)
class MovementSystem : public System {
public:
  MovementSystem();
  
  void update(float dt) override;
  int priority() const override { return 100; }
  
  // Configuration
  void setGravity(float gravity) { m_gravity = gravity; }
  float gravity() const { return m_gravity; }

private:
  void applyGravity(VelocityComponent& velocity, PhysicsBodyComponent const& physics, float dt);
  void applyFriction(VelocityComponent& velocity, PhysicsBodyComponent const& physics, 
                     CollisionComponent const& collision, float dt);
  void updatePosition(TransformComponent& transform, VelocityComponent const& velocity, float dt);
  void applySpeedLimits(VelocityComponent& velocity, PhysicsBodyComponent const& physics);
  
  float m_gravity = 20.0f; // Default gravity
  float m_maxSpeed = 100.0f; // Maximum velocity magnitude
};

} // namespace ECS
} // namespace Star

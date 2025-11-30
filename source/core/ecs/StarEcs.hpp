#pragma once

// Star ECS Framework
// Entity-Component-System architecture for OpenStarbound
//
// This framework provides:
// - Entity management with generation-based IDs
// - Type-safe component storage using sparse sets
// - System scheduling with priority ordering
// - Views for efficient iteration over entity archetypes
//
// Basic usage:
//
//   // Create a world
//   ECS::World world;
//
//   // Define components
//   struct Position { float x, y; };
//   struct Velocity { float dx, dy; };
//
//   // Create an entity
//   ECS::Entity entity = world.createEntity();
//
//   // Add components
//   world.addComponent<Position>(entity, 0.0f, 0.0f);
//   world.addComponent<Velocity>(entity, 1.0f, 0.0f);
//
//   // Define a system
//   class MovementSystem : public ECS::System {
//   public:
//     void update(float dt) override {
//       for (auto [entity, pos, vel] : m_world->view<Position, Velocity>()) {
//         pos.x += vel.dx * dt;
//         pos.y += vel.dy * dt;
//       }
//     }
//   };
//
//   // Add system to world
//   world.addSystem<MovementSystem>();
//
//   // Update all systems
//   world.update(deltaTime);

#include "StarEcsTypes.hpp"
#include "StarEcsComponent.hpp"
#include "StarEcsSystem.hpp"
#include "StarEcsView.hpp"
#include "StarEcsWorld.hpp"

namespace Star {
namespace ECS {

// Convenience aliases
using EntityId = Entity;

// Check if a type is a valid component (must be movable)
template<typename T>
constexpr bool is_component_v = std::is_move_constructible_v<T> && std::is_move_assignable_v<T>;

} // namespace ECS
} // namespace Star

#pragma once

// Star ECS Systems
// Include this header to get access to all game systems

#include "StarMovementSystem.hpp"
#include "StarDamageSystem.hpp"
#include "StarStatusEffectSystem.hpp"
#include "StarRenderSystem.hpp"

namespace Star {
namespace ECS {

// System priority order (lower number = runs earlier):
// 100 - MovementSystem     - Physics and movement
//  80 - DamageSystem       - Damage processing
//  70 - StatusEffectSystem - Status effects and resource regen
//  60 - AISystem           - AI behavior (to be implemented)
//  50 - ScriptSystem       - Lua scripts (to be implemented)
//  10 - RenderSystem       - Prepare render data
//   5 - NetworkSyncSystem  - Network synchronization (to be implemented)

// Helper function to register all core systems with a World
inline void registerCoreSystems(World& world) {
  world.addSystem<MovementSystem>();
  world.addSystem<DamageSystem>();
  world.addSystem<StatusEffectSystem>();
  world.addSystem<RenderSystem>();
}

} // namespace ECS
} // namespace Star

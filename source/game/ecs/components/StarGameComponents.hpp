#pragma once

// Game-specific ECS components for OpenStarbound
// This header includes all component definitions for convenience.
// Individual component headers can also be included separately.

// Include all component categories
#include "StarTransformComponents.hpp"
#include "StarPhysicsComponents.hpp"
#include "StarCombatComponents.hpp"
#include "StarVisualComponents.hpp"
#include "StarAIComponents.hpp"
#include "StarPlayerComponents.hpp"
#include "StarNetworkComponents.hpp"

#include "StarString.hpp"
#include "StarGameTypes.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Identity Components
//=============================================================================

// Entity name and description
struct NameComponent {
  String name = "";
  String description = "";
  Maybe<String> statusText = {};
  bool displayNametag = true;
  Vec3B nametagColor = {255, 255, 255};
  Vec2F nametagOrigin = {};
};

// Unique identifier for special entities
struct UniqueIdComponent {
  Maybe<String> uniqueId = {};
};

// Persistence settings
struct PersistenceComponent {
  bool persistent = false;
  bool keepAlive = false;
};

// Entity type tag
struct EntityTypeComponent {
  EntityType type = EntityType::Object;
  ClientEntityMode clientMode = ClientEntityMode::ClientSlaveOnly;
  bool masterOnly = false;
  bool ephemeral = false;
};

//=============================================================================
// Interaction Components
//=============================================================================

// Interactive entity flag
struct InteractiveComponent {
  bool interactive = true;
  float interactRadius = 2.0f;
  String interactAction = "";
  Json interactData = {};
};

//=============================================================================
// Tags (Empty components used as markers)
//=============================================================================

// Mark entity as a player
struct PlayerTag {};

// Mark entity as a monster
struct MonsterTag {};

// Mark entity as an NPC
struct NpcTag {};

// Mark entity as a projectile
struct ProjectileTag {};

// Mark entity as an object (tile entity)
struct ObjectTag {};

// Mark entity as a vehicle
struct VehicleTag {};

// Mark entity as a plant
struct PlantTag {};

// Mark entity as a plant drop
struct PlantDropTag {};

// Mark entity as an item drop
struct ItemDropTag {};

// Mark entity as a stagehand
struct StagehandTag {};

// Mark entity as aggressive
struct AggressiveTag {};

// Mark entity as scripted
struct ScriptedTag {};

// Mark entity as dead (pending removal)
struct DeadTag {};

// Mark entity as newly created (needs initialization)
struct NewEntityTag {};

} // namespace ECS
} // namespace Star

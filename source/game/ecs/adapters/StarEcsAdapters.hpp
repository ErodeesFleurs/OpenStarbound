#pragma once

// Star ECS Entity Adapters
// Include this header to get access to all entity adapters

#include "StarEntityAdapter.hpp"
#include "StarItemDropAdapter.hpp"
#include "StarPlantDropAdapter.hpp"
#include "StarProjectileAdapter.hpp"

namespace Star {
namespace ECS {

// Adapter registration and factory
// This allows creating the appropriate adapter based on entity type

class AdapterFactory {
public:
  // Create adapter for existing ECS entity based on its type
  static shared_ptr<EntityAdapter> createAdapter(World* ecsWorld, Entity ecsEntity) {
    // Check entity type component to determine adapter type
    if (ecsWorld->hasComponent<ItemDropTag>(ecsEntity)) {
      return make_shared<ItemDropAdapter>(ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<PlantDropTag>(ecsEntity)) {
      return make_shared<PlantDropAdapter>(ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<ProjectileTag>(ecsEntity)) {
      return make_shared<ProjectileAdapter>(ecsWorld, ecsEntity);
    }
    
    // Add more adapter types here as they are implemented:
    // if (ecsWorld->hasComponent<MonsterTag>(ecsEntity)) {
    //   return make_shared<MonsterAdapter>(ecsWorld, ecsEntity);
    // }
    // if (ecsWorld->hasComponent<PlayerTag>(ecsEntity)) {
    //   return make_shared<PlayerAdapter>(ecsWorld, ecsEntity);
    // }
    
    // Fallback to base adapter
    return make_shared<EntityAdapter>(ecsWorld, ecsEntity);
  }
};

// Migration helper functions

// Check if an entity type can be migrated to ECS
inline bool canMigrateToEcs(EntityType type) {
  switch (type) {
    case EntityType::ItemDrop:
    case EntityType::PlantDrop:
    case EntityType::Projectile:
      return true;
    // Add more types as adapters are implemented
    default:
      return false;
  }
}

// Get migration priority (lower = migrate first)
inline int migrationPriority(EntityType type) {
  switch (type) {
    case EntityType::ItemDrop: return 1;
    case EntityType::PlantDrop: return 2;
    case EntityType::Projectile: return 3;
    case EntityType::Plant: return 4;
    case EntityType::Stagehand: return 5;
    case EntityType::Object: return 6;
    case EntityType::Vehicle: return 7;
    case EntityType::Monster: return 8;
    case EntityType::Npc: return 9;
    case EntityType::Player: return 10;
    default: return 100;
  }
}

} // namespace ECS
} // namespace Star

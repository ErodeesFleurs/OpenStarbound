#pragma once

// Star ECS Entity Adapters
// Include this header to get access to all entity adapters

#include "StarEntityAdapter.hpp"
#include "StarItemDropAdapter.hpp"
#include "StarPlantDropAdapter.hpp"
#include "StarProjectileAdapter.hpp"
#include "StarPlantAdapter.hpp"
#include "StarStagehandAdapter.hpp"
#include "StarObjectAdapter.hpp"
#include "StarVehicleAdapter.hpp"
#include "StarMonsterAdapter.hpp"
#include "StarNpcAdapter.hpp"
#include "StarPlayerAdapter.hpp"

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
    
    if (ecsWorld->hasComponent<PlantTag>(ecsEntity)) {
      return make_shared<PlantAdapter>(ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<StagehandTag>(ecsEntity)) {
      return make_shared<StagehandAdapter>(ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<ObjectTag>(ecsEntity)) {
      return make_shared<ObjectAdapter>(ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<VehicleTag>(ecsEntity)) {
      return make_shared<VehicleAdapter>(ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<MonsterTag>(ecsEntity)) {
      return make_shared<MonsterAdapter>(*ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<NpcTag>(ecsEntity)) {
      return make_shared<NpcAdapter>(*ecsWorld, ecsEntity);
    }
    
    if (ecsWorld->hasComponent<PlayerTag>(ecsEntity)) {
      return make_shared<PlayerAdapter>(*ecsWorld, ecsEntity);
    }
    
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
    case EntityType::Plant:
    case EntityType::Stagehand:
    case EntityType::Object:
    case EntityType::Vehicle:
    case EntityType::Monster:
    case EntityType::Npc:
    case EntityType::Player:
      return true;
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

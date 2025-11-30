#pragma once

#include "StarEcsWorldIntegration.hpp"
#include "adapters/StarEcsAdapters.hpp"
#include "StarEntityFactory.hpp"

namespace Star {
namespace ECS {

// Migration configuration
struct MigrationConfig {
  // Enable/disable ECS for specific entity types
  bool useEcsForItemDrop = true;
  bool useEcsForPlantDrop = true;
  bool useEcsForProjectile = true;
  bool useEcsForPlant = true;
  bool useEcsForStagehand = true;
  bool useEcsForObject = true;
  bool useEcsForVehicle = true;
  bool useEcsForMonster = true;
  bool useEcsForNpc = true;
  bool useEcsForPlayer = true;
  
  // Global toggle to disable all ECS (fallback to legacy)
  bool ecsEnabled = true;
  
  // Performance options
  bool useBatchUpdates = true;
  size_t batchSize = 100;
  
  // Debug options
  bool debugMode = false;
  bool logMigrations = false;
  
  static MigrationConfig fromJson(Json const& json);
  Json toJson() const;
};

// ECS-aware entity factory wrapper
// This class wraps the existing EntityFactory and optionally creates ECS entities
class EcsEntityFactory {
public:
  EcsEntityFactory(EntityFactoryConstPtr legacyFactory, MigrationConfig config = {});
  ~EcsEntityFactory();
  
  // Set the ECS world integration (must be called before creating ECS entities)
  void setWorldIntegration(WorldIntegration* integration);
  
  // Get/set migration config
  MigrationConfig const& config() const;
  void setConfig(MigrationConfig const& config);
  
  // Check if a type should use ECS
  bool shouldUseEcs(EntityType type) const;
  
  // Network load - creates entity from network data
  EntityPtr netLoadEntity(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules = {}) const;
  
  // Disk load - creates entity from saved data
  EntityPtr diskLoadEntity(EntityType type, Json const& diskStore) const;
  
  // Network store - serializes entity for network
  ByteArray netStoreEntity(EntityPtr const& entity, NetCompatibilityRules rules = {}) const;
  
  // Disk store - serializes entity for saving
  Json diskStoreEntity(EntityPtr const& entity) const;
  
  // Get underlying legacy factory
  EntityFactoryConstPtr legacyFactory() const;
  
  // Statistics
  struct Stats {
    size_t legacyEntitiesCreated = 0;
    size_t ecsEntitiesCreated = 0;
    size_t migrationFailures = 0;
    HashMap<EntityType, size_t> entitiesByType;
  };
  
  Stats const& stats() const;
  void resetStats();
  
private:
  // Create ECS entity from network data
  EntityPtr createEcsEntityFromNet(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules) const;
  
  // Create ECS entity from disk data
  EntityPtr createEcsEntityFromDisk(EntityType type, Json const& diskStore) const;
  
  EntityFactoryConstPtr m_legacyFactory;
  MigrationConfig m_config;
  WorldIntegration* m_integration = nullptr;
  mutable Stats m_stats;
};

using EcsEntityFactoryPtr = shared_ptr<EcsEntityFactory>;
using EcsEntityFactoryConstPtr = shared_ptr<EcsEntityFactory const>;

// Convenience functions for gradual migration

// Check if ECS migration is enabled globally
bool isEcsMigrationEnabled();

// Enable/disable ECS migration globally
void setEcsMigrationEnabled(bool enabled);

// Get the global migration config
MigrationConfig& globalMigrationConfig();

} // namespace ECS
} // namespace Star

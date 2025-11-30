#include "StarEcsMigrationBridge.hpp"
#include "StarRoot.hpp"
#include "StarLogging.hpp"

namespace Star {
namespace ECS {

// Global migration state
static bool s_ecsMigrationEnabled = true;
static MigrationConfig s_globalConfig;

bool isEcsMigrationEnabled() {
  return s_ecsMigrationEnabled;
}

void setEcsMigrationEnabled(bool enabled) {
  s_ecsMigrationEnabled = enabled;
  if (s_globalConfig.logMigrations) {
    Logger::info("ECS migration {}", enabled ? "enabled" : "disabled");
  }
}

MigrationConfig& globalMigrationConfig() {
  return s_globalConfig;
}

// MigrationConfig implementation

MigrationConfig MigrationConfig::fromJson(Json const& json) {
  MigrationConfig config;
  
  if (json.isType(Json::Type::Object)) {
    config.useEcsForItemDrop = json.getBool("useEcsForItemDrop", true);
    config.useEcsForPlantDrop = json.getBool("useEcsForPlantDrop", true);
    config.useEcsForProjectile = json.getBool("useEcsForProjectile", true);
    config.useEcsForPlant = json.getBool("useEcsForPlant", true);
    config.useEcsForStagehand = json.getBool("useEcsForStagehand", true);
    config.useEcsForObject = json.getBool("useEcsForObject", true);
    config.useEcsForVehicle = json.getBool("useEcsForVehicle", true);
    config.useEcsForMonster = json.getBool("useEcsForMonster", true);
    config.useEcsForNpc = json.getBool("useEcsForNpc", true);
    config.useEcsForPlayer = json.getBool("useEcsForPlayer", true);
    config.ecsEnabled = json.getBool("ecsEnabled", true);
    config.useBatchUpdates = json.getBool("useBatchUpdates", true);
    config.batchSize = json.getUInt("batchSize", 100);
    config.debugMode = json.getBool("debugMode", false);
    config.logMigrations = json.getBool("logMigrations", false);
  }
  
  return config;
}

Json MigrationConfig::toJson() const {
  return JsonObject{
    {"useEcsForItemDrop", useEcsForItemDrop},
    {"useEcsForPlantDrop", useEcsForPlantDrop},
    {"useEcsForProjectile", useEcsForProjectile},
    {"useEcsForPlant", useEcsForPlant},
    {"useEcsForStagehand", useEcsForStagehand},
    {"useEcsForObject", useEcsForObject},
    {"useEcsForVehicle", useEcsForVehicle},
    {"useEcsForMonster", useEcsForMonster},
    {"useEcsForNpc", useEcsForNpc},
    {"useEcsForPlayer", useEcsForPlayer},
    {"ecsEnabled", ecsEnabled},
    {"useBatchUpdates", useBatchUpdates},
    {"batchSize", batchSize},
    {"debugMode", debugMode},
    {"logMigrations", logMigrations}
  };
}

// EcsEntityFactory implementation

EcsEntityFactory::EcsEntityFactory(EntityFactoryConstPtr legacyFactory, MigrationConfig config)
  : m_legacyFactory(std::move(legacyFactory))
  , m_config(std::move(config)) {
}

EcsEntityFactory::~EcsEntityFactory() = default;

void EcsEntityFactory::setWorldIntegration(WorldIntegration* integration) {
  m_integration = integration;
}

MigrationConfig const& EcsEntityFactory::config() const {
  return m_config;
}

void EcsEntityFactory::setConfig(MigrationConfig const& config) {
  m_config = config;
}

bool EcsEntityFactory::shouldUseEcs(EntityType type) const {
  if (!m_config.ecsEnabled || !isEcsMigrationEnabled())
    return false;
  
  if (!m_integration)
    return false;
  
  switch (type) {
    case EntityType::ItemDrop:
      return m_config.useEcsForItemDrop;
    case EntityType::PlantDrop:
      return m_config.useEcsForPlantDrop;
    case EntityType::Projectile:
      return m_config.useEcsForProjectile;
    case EntityType::Plant:
      return m_config.useEcsForPlant;
    case EntityType::Stagehand:
      return m_config.useEcsForStagehand;
    case EntityType::Object:
      return m_config.useEcsForObject;
    case EntityType::Vehicle:
      return m_config.useEcsForVehicle;
    case EntityType::Monster:
      return m_config.useEcsForMonster;
    case EntityType::Npc:
      return m_config.useEcsForNpc;
    case EntityType::Player:
      return m_config.useEcsForPlayer;
    default:
      return false;
  }
}

EntityPtr EcsEntityFactory::netLoadEntity(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules) const {
  if (shouldUseEcs(type)) {
    try {
      auto entity = createEcsEntityFromNet(type, netStore, rules);
      if (entity) {
        m_stats.ecsEntitiesCreated++;
        m_stats.entitiesByType[type]++;
        if (m_config.logMigrations) {
          Logger::info("ECS: Created {} entity from network", EntityTypeNames.getRight(type));
        }
        return entity;
      }
    } catch (std::exception const& e) {
      m_stats.migrationFailures++;
      Logger::warn("ECS: Failed to create {} entity from network: {}, falling back to legacy",
        EntityTypeNames.getRight(type), e.what());
    }
  }
  
  // Fallback to legacy factory
  m_stats.legacyEntitiesCreated++;
  return m_legacyFactory->netLoadEntity(type, netStore, rules);
}

EntityPtr EcsEntityFactory::diskLoadEntity(EntityType type, Json const& diskStore) const {
  if (shouldUseEcs(type)) {
    try {
      auto entity = createEcsEntityFromDisk(type, diskStore);
      if (entity) {
        m_stats.ecsEntitiesCreated++;
        m_stats.entitiesByType[type]++;
        if (m_config.logMigrations) {
          Logger::info("ECS: Created {} entity from disk", EntityTypeNames.getRight(type));
        }
        return entity;
      }
    } catch (std::exception const& e) {
      m_stats.migrationFailures++;
      Logger::warn("ECS: Failed to create {} entity from disk: {}, falling back to legacy",
        EntityTypeNames.getRight(type), e.what());
    }
  }
  
  // Fallback to legacy factory
  m_stats.legacyEntitiesCreated++;
  return m_legacyFactory->diskLoadEntity(type, diskStore);
}

ByteArray EcsEntityFactory::netStoreEntity(EntityPtr const& entity, NetCompatibilityRules rules) const {
  // For now, delegate to legacy factory
  // ECS adapters implement the same interface, so this should work
  return m_legacyFactory->netStoreEntity(entity, rules);
}

Json EcsEntityFactory::diskStoreEntity(EntityPtr const& entity) const {
  // For now, delegate to legacy factory
  // ECS adapters implement the same interface, so this should work
  return m_legacyFactory->diskStoreEntity(entity);
}

EntityFactoryConstPtr EcsEntityFactory::legacyFactory() const {
  return m_legacyFactory;
}

EcsEntityFactory::Stats const& EcsEntityFactory::stats() const {
  return m_stats;
}

void EcsEntityFactory::resetStats() {
  m_stats = Stats{};
}

EntityPtr EcsEntityFactory::createEcsEntityFromNet(EntityType type, ByteArray const& netStore, NetCompatibilityRules rules) const {
  if (!m_integration)
    return nullptr;
  
  World& ecsWorld = m_integration->ecsWorld();
  Entity ecsEntity = ecsWorld.createEntity();
  
  switch (type) {
    case EntityType::ItemDrop: {
      auto adapter = make_shared<ItemDropAdapter>(&ecsWorld, ecsEntity);
      // ItemDrop has a constructor from netStore
      // The adapter wraps the ECS entity
      return adapter;
    }
    
    case EntityType::PlantDrop: {
      auto adapter = make_shared<PlantDropAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Projectile: {
      auto adapter = make_shared<ProjectileAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Plant: {
      auto adapter = make_shared<PlantAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Stagehand: {
      auto adapter = make_shared<StagehandAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Object: {
      auto adapter = make_shared<ObjectAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Vehicle: {
      auto adapter = make_shared<VehicleAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Monster: {
      auto adapter = make_shared<MonsterAdapter>(ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Npc: {
      auto adapter = make_shared<NpcAdapter>(ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Player: {
      auto adapter = make_shared<PlayerAdapter>(ecsWorld, ecsEntity);
      return adapter;
    }
    
    default:
      return nullptr;
  }
}

EntityPtr EcsEntityFactory::createEcsEntityFromDisk(EntityType type, Json const& diskStore) const {
  if (!m_integration)
    return nullptr;
  
  World& ecsWorld = m_integration->ecsWorld();
  Entity ecsEntity = ecsWorld.createEntity();
  
  switch (type) {
    case EntityType::ItemDrop: {
      auto adapter = make_shared<ItemDropAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::PlantDrop: {
      auto adapter = make_shared<PlantDropAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Projectile: {
      auto adapter = make_shared<ProjectileAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Plant: {
      auto adapter = make_shared<PlantAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Stagehand: {
      auto adapter = make_shared<StagehandAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Object: {
      auto adapter = make_shared<ObjectAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Vehicle: {
      auto adapter = make_shared<VehicleAdapter>(&ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Monster: {
      auto adapter = make_shared<MonsterAdapter>(ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Npc: {
      auto adapter = make_shared<NpcAdapter>(ecsWorld, ecsEntity);
      return adapter;
    }
    
    case EntityType::Player: {
      auto adapter = make_shared<PlayerAdapter>(ecsWorld, ecsEntity);
      return adapter;
    }
    
    default:
      return nullptr;
  }
}

} // namespace ECS
} // namespace Star

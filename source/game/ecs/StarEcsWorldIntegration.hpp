#pragma once

#include "StarEcs.hpp"
#include "adapters/StarEcsAdapters.hpp"
#include "components/StarGameComponents.hpp"

namespace Star {

// Forward declarations
class World;
class WorldServer;
class WorldClient;
class Entity;
class EntityMap;

namespace ECS {

// Performance metrics for ECS systems
struct PerformanceMetrics {
  float frameTime = 0.0f;
  float movementSystemTime = 0.0f;
  float damageSystemTime = 0.0f;
  float statusSystemTime = 0.0f;
  float renderSystemTime = 0.0f;
  size_t totalEntities = 0;
  size_t activeEntities = 0;
  size_t componentsCount = 0;
  
  Json toJson() const {
    return JsonObject{
      {"frameTime", frameTime},
      {"movementSystemTime", movementSystemTime},
      {"damageSystemTime", damageSystemTime},
      {"statusSystemTime", statusSystemTime},
      {"renderSystemTime", renderSystemTime},
      {"totalEntities", totalEntities},
      {"activeEntities", activeEntities},
      {"componentsCount", componentsCount}
    };
  }
};

// ECS World Integration - bridges existing World with ECS
class WorldIntegration {
public:
  WorldIntegration();
  ~WorldIntegration();
  
  // Initialize with existing world
  void init(World* legacyWorld);
  
  // Get the ECS world
  World& ecsWorld();
  World const& ecsWorld() const;
  
  // Entity migration utilities
  
  // Check if an entity type can be migrated to ECS
  static bool canMigrate(EntityType type);
  
  // Migrate a legacy entity to ECS
  // Returns the ECS entity ID or NullEntity if migration failed
  Entity migrateEntity(EntityPtr const& legacyEntity);
  
  // Create a new ECS entity of the given type
  Entity createEntity(EntityType type, Json const& config);
  
  // Update all ECS systems
  void update(float dt);
  
  // Get performance metrics
  PerformanceMetrics const& metrics() const;
  
  // Entity queries
  
  // Find all entities within a region
  List<Entity> entitiesInRegion(RectF const& region) const;
  
  // Find all entities of a specific type
  List<Entity> entitiesOfType(EntityType type) const;
  
  // Find closest entity matching a predicate
  template<typename Predicate>
  Maybe<Entity> closestEntity(Vec2F const& position, float maxDistance, Predicate pred) const;
  
  // Component queries
  
  // Get transform for an entity
  Maybe<Vec2F> entityPosition(Entity entity) const;
  
  // Get velocity for an entity
  Maybe<Vec2F> entityVelocity(Entity entity) const;
  
  // Get bounding box for an entity
  Maybe<RectF> entityBounds(Entity entity) const;
  
  // Damage system helpers
  
  // Apply damage to an entity
  void applyDamage(Entity target, DamageSource const& damage);
  
  // Check if entity is alive
  bool isEntityAlive(Entity entity) const;
  
  // Network synchronization
  
  // Serialize entity for network transmission
  ByteArray serializeEntity(Entity entity) const;
  
  // Deserialize entity from network data
  Entity deserializeEntity(ByteArray const& data);
  
  // Mark entity as needing network sync
  void markDirty(Entity entity);
  
  // Debug utilities
  
  // Enable/disable debug drawing
  void setDebugMode(bool enabled);
  bool debugMode() const;
  
  // Get debug info for an entity
  String debugInfo(Entity entity) const;
  
private:
  struct Impl;
  unique_ptr<Impl> m_impl;
};

// Entity type tag helpers
template<EntityType Type>
struct EntityTypeTag {};

// Specializations for each entity type
using ItemDropTag = EntityTypeTag<EntityType::ItemDrop>;
using PlantDropTag = EntityTypeTag<EntityType::PlantDrop>;
using ProjectileTag = EntityTypeTag<EntityType::Projectile>;
using PlantTag = EntityTypeTag<EntityType::Plant>;
using StagehandTag = EntityTypeTag<EntityType::Stagehand>;
using ObjectTag = EntityTypeTag<EntityType::Object>;
using VehicleTag = EntityTypeTag<EntityType::Vehicle>;
using MonsterTag = EntityTypeTag<EntityType::Monster>;
using NpcTag = EntityTypeTag<EntityType::Npc>;
using PlayerTag = EntityTypeTag<EntityType::Player>;

// Migration state tracking
enum class MigrationState {
  NotMigrated,    // Using legacy entity system
  InProgress,     // Migration in progress
  Migrated,       // Fully migrated to ECS
  Failed          // Migration failed, rolled back
};

// Migration result
struct MigrationResult {
  MigrationState state;
  Entity ecsEntity;
  String errorMessage;
  
  bool success() const { return state == MigrationState::Migrated; }
};

// Batch migration utilities
class BatchMigration {
public:
  BatchMigration(WorldIntegration& integration);
  
  // Add entities to migration batch
  void addEntity(EntityPtr const& entity);
  
  // Execute migration
  List<MigrationResult> execute();
  
  // Get progress (0.0 - 1.0)
  float progress() const;
  
  // Cancel migration
  void cancel();
  
private:
  WorldIntegration& m_integration;
  List<EntityPtr> m_entities;
  size_t m_processed = 0;
  bool m_cancelled = false;
};

// Component pool optimization utilities
class ComponentPoolOptimizer {
public:
  ComponentPoolOptimizer(World& world);
  
  // Defragment component storage
  void defragment();
  
  // Pre-allocate component storage for expected entity count
  template<typename Component>
  void reserve(size_t count);
  
  // Get memory usage statistics
  struct MemoryStats {
    size_t totalAllocated;
    size_t totalUsed;
    size_t fragmentationPercent;
    HashMap<String, size_t> componentMemory;
  };
  
  MemoryStats memoryStats() const;
  
private:
  World& m_world;
};

// System scheduling optimization
class SystemScheduler {
public:
  SystemScheduler();
  
  // Add a system with dependencies
  void addSystem(SystemPtr system, List<String> dependencies = {});
  
  // Remove a system
  void removeSystem(String const& name);
  
  // Enable/disable a system
  void setEnabled(String const& name, bool enabled);
  
  // Update all enabled systems
  void update(World& world, float dt);
  
  // Get system execution order
  List<String> executionOrder() const;
  
  // Profile systems and get timing info
  HashMap<String, float> systemTimings() const;
  
private:
  struct SystemInfo {
    SystemPtr system;
    List<String> dependencies;
    bool enabled = true;
    float lastExecutionTime = 0.0f;
  };
  
  HashMap<String, SystemInfo> m_systems;
  List<String> m_executionOrder;
  bool m_orderDirty = true;
  
  void rebuildOrder();
};

// Entity archetype for fast entity creation
class EntityArchetype {
public:
  EntityArchetype(String name);
  
  // Define components for this archetype
  template<typename Component>
  EntityArchetype& withComponent(Component defaultValue = {});
  
  // Create an entity from this archetype
  Entity create(World& world) const;
  
  // Create multiple entities
  List<Entity> createBatch(World& world, size_t count) const;
  
  String const& name() const { return m_name; }
  
private:
  String m_name;
  List<function<void(World&, Entity)>> m_componentInitializers;
};

// Archetype registry
class ArchetypeRegistry {
public:
  static ArchetypeRegistry& instance();
  
  // Register an archetype
  void registerArchetype(EntityArchetype archetype);
  
  // Get an archetype by name
  Maybe<EntityArchetype const&> getArchetype(String const& name) const;
  
  // Create entity from archetype
  Entity createFromArchetype(World& world, String const& archetypeName) const;
  
  // List all registered archetypes
  List<String> archetypeNames() const;
  
private:
  ArchetypeRegistry();
  HashMap<String, EntityArchetype> m_archetypes;
};

// Event system for ECS
template<typename EventType>
class EventEmitter {
public:
  using Callback = function<void(EventType const&)>;
  
  void subscribe(Callback callback) {
    m_callbacks.append(std::move(callback));
  }
  
  void emit(EventType const& event) {
    for (auto& callback : m_callbacks) {
      callback(event);
    }
  }
  
  void clear() {
    m_callbacks.clear();
  }
  
private:
  List<Callback> m_callbacks;
};

// Common event types
struct EntityCreatedEvent {
  Entity entity;
  EntityType type;
};

struct EntityDestroyedEvent {
  Entity entity;
  EntityType type;
};

struct EntityDamagedEvent {
  Entity target;
  Entity source;
  float damage;
  String damageType;
};

struct EntityMovedEvent {
  Entity entity;
  Vec2F oldPosition;
  Vec2F newPosition;
};

// World event bus
class WorldEventBus {
public:
  EventEmitter<EntityCreatedEvent> entityCreated;
  EventEmitter<EntityDestroyedEvent> entityDestroyed;
  EventEmitter<EntityDamagedEvent> entityDamaged;
  EventEmitter<EntityMovedEvent> entityMoved;
  
  void clear() {
    entityCreated.clear();
    entityDestroyed.clear();
    entityDamaged.clear();
    entityMoved.clear();
  }
};

// Template implementations

template<typename Predicate>
Maybe<Entity> WorldIntegration::closestEntity(Vec2F const& position, float maxDistance, Predicate pred) const {
  Maybe<Entity> closest;
  float closestDistance = maxDistance;
  
  for (auto entity : ecsWorld().livingEntities()) {
    if (auto pos = entityPosition(entity)) {
      float dist = vmag(*pos - position);
      if (dist < closestDistance && pred(entity)) {
        closest = entity;
        closestDistance = dist;
      }
    }
  }
  
  return closest;
}

template<typename Component>
void ComponentPoolOptimizer::reserve(size_t count) {
  // Reserve space in component storage
  m_world.componentStorage<Component>().reserve(count);
}

template<typename Component>
EntityArchetype& EntityArchetype::withComponent(Component defaultValue) {
  m_componentInitializers.append([defaultValue](World& world, Entity entity) {
    world.addComponent<Component>(entity, defaultValue);
  });
  return *this;
}

} // namespace ECS
} // namespace Star

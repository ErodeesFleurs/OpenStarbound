#include "StarEcsWorldIntegration.hpp"
#include "adapters/StarEntityAdapter.hpp"
#include "adapters/StarItemDropAdapter.hpp"
#include "adapters/StarPlantDropAdapter.hpp"
#include "adapters/StarProjectileAdapter.hpp"
#include "adapters/StarPlantAdapter.hpp"
#include "adapters/StarStagehandAdapter.hpp"
#include "adapters/StarObjectAdapter.hpp"
#include "adapters/StarVehicleAdapter.hpp"
#include "adapters/StarMonsterAdapter.hpp"
#include "adapters/StarNpcAdapter.hpp"
#include "adapters/StarPlayerAdapter.hpp"
#include "systems/StarEcsSystems.hpp"
#include "StarEntity.hpp"
#include "StarWorld.hpp"

#include <chrono>

namespace Star {
namespace ECS {

// WorldIntegration implementation
struct WorldIntegration::Impl {
  ECS::World ecsWorld;
  Star::World* legacyWorld = nullptr;
  PerformanceMetrics metrics;
  SystemScheduler scheduler;
  WorldEventBus eventBus;
  bool debugMode = false;
  
  // Entity mapping: legacy EntityId -> ECS Entity
  HashMap<EntityId, Entity> entityMapping;
  // Reverse mapping: ECS Entity -> legacy EntityId
  HashMap<Entity, EntityId> reverseMapping;
};

WorldIntegration::WorldIntegration()
  : m_impl(make_unique<Impl>()) {
  // Register default systems
  m_impl->scheduler.addSystem(make_shared<MovementSystem>(), {});
  m_impl->scheduler.addSystem(make_shared<DamageSystem>(), {"MovementSystem"});
  m_impl->scheduler.addSystem(make_shared<StatusEffectSystem>(), {"DamageSystem"});
  m_impl->scheduler.addSystem(make_shared<RenderSystem>(), {"StatusEffectSystem"});
}

WorldIntegration::~WorldIntegration() = default;

void WorldIntegration::init(Star::World* legacyWorld) {
  m_impl->legacyWorld = legacyWorld;
}

ECS::World& WorldIntegration::ecsWorld() {
  return m_impl->ecsWorld;
}

ECS::World const& WorldIntegration::ecsWorld() const {
  return m_impl->ecsWorld;
}

bool WorldIntegration::canMigrate(EntityType type) {
  // All entity types are now supported for migration
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

Entity WorldIntegration::migrateEntity(EntityPtr const& legacyEntity) {
  if (!legacyEntity)
    return NullEntity;
  
  EntityType type = legacyEntity->entityType();
  
  if (!canMigrate(type))
    return NullEntity;
  
  // Check if already migrated
  if (m_impl->entityMapping.contains(legacyEntity->entityId()))
    return m_impl->entityMapping.get(legacyEntity->entityId());
  
  // Create ECS entity
  Entity entity = m_impl->ecsWorld.createEntity();
  
  // Store mapping
  m_impl->entityMapping[legacyEntity->entityId()] = entity;
  m_impl->reverseMapping[entity] = legacyEntity->entityId();
  
  // Initialize based on type
  Vec2F position = legacyEntity->position();
  
  // Add transform component
  m_impl->ecsWorld.addComponent<PositionComponent>(entity, {position});
  m_impl->ecsWorld.addComponent<BoundsComponent>(entity, {legacyEntity->metaBoundBox()});
  
  // Add network component
  NetworkStateComponent netState;
  netState.entityId = legacyEntity->entityId();
  netState.isDirty = true;
  m_impl->ecsWorld.addComponent<NetworkStateComponent>(entity, netState);
  
  // Emit event
  m_impl->eventBus.entityCreated.emit({entity, type});
  
  return entity;
}

Entity WorldIntegration::createEntity(EntityType type, Json const& config) {
  Entity entity = m_impl->ecsWorld.createEntity();
  
  // Add basic components
  m_impl->ecsWorld.addComponent<PositionComponent>(entity);
  
  // Add type-specific components based on type
  switch (type) {
    case EntityType::ItemDrop:
      m_impl->ecsWorld.addComponent<ItemDropTag>(entity);
      break;
    case EntityType::Projectile:
      m_impl->ecsWorld.addComponent<ProjectileTag>(entity);
      break;
    case EntityType::Monster:
      m_impl->ecsWorld.addComponent<MonsterTag>(entity);
      break;
    case EntityType::Npc:
      m_impl->ecsWorld.addComponent<NpcTag>(entity);
      break;
    case EntityType::Player:
      m_impl->ecsWorld.addComponent<PlayerTag>(entity);
      break;
    default:
      break;
  }
  
  // Emit event
  m_impl->eventBus.entityCreated.emit({entity, type});
  
  return entity;
}

void WorldIntegration::update(float dt) {
  auto startTime = std::chrono::high_resolution_clock::now();
  
  // Update all systems through scheduler
  m_impl->scheduler.update(m_impl->ecsWorld, dt);
  
  // Update metrics
  auto endTime = std::chrono::high_resolution_clock::now();
  m_impl->metrics.frameTime = std::chrono::duration<float>(endTime - startTime).count() * 1000.0f;
  m_impl->metrics.totalEntities = m_impl->ecsWorld.livingEntities().size();
  m_impl->metrics.activeEntities = m_impl->metrics.totalEntities; // All living entities are active
  
  // Get system timings
  auto timings = m_impl->scheduler.systemTimings();
  m_impl->metrics.movementSystemTime = timings.value("MovementSystem", 0.0f);
  m_impl->metrics.damageSystemTime = timings.value("DamageSystem", 0.0f);
  m_impl->metrics.statusSystemTime = timings.value("StatusEffectSystem", 0.0f);
  m_impl->metrics.renderSystemTime = timings.value("RenderSystem", 0.0f);
}

PerformanceMetrics const& WorldIntegration::metrics() const {
  return m_impl->metrics;
}

List<Entity> WorldIntegration::entitiesInRegion(RectF const& region) const {
  List<Entity> result;
  
  for (auto entity : m_impl->ecsWorld.livingEntities()) {
    if (auto pos = entityPosition(entity)) {
      if (region.contains(*pos))
        result.append(entity);
    }
  }
  
  return result;
}

List<Entity> WorldIntegration::entitiesOfType(EntityType type) const {
  List<Entity> result;
  
  // Query based on type tag components
  // This is a simplified implementation
  for (auto entity : m_impl->ecsWorld.livingEntities()) {
    result.append(entity);
  }
  
  return result;
}

Maybe<Vec2F> WorldIntegration::entityPosition(Entity entity) const {
  if (auto* pos = m_impl->ecsWorld.getComponent<PositionComponent>(entity))
    return pos->position;
  return {};
}

Maybe<Vec2F> WorldIntegration::entityVelocity(Entity entity) const {
  if (auto* vel = m_impl->ecsWorld.getComponent<VelocityComponent>(entity))
    return vel->velocity;
  return {};
}

Maybe<RectF> WorldIntegration::entityBounds(Entity entity) const {
  if (auto* bounds = m_impl->ecsWorld.getComponent<BoundsComponent>(entity))
    return bounds->boundingBox;
  return {};
}

void WorldIntegration::applyDamage(Entity target, DamageSource const& damage) {
  auto* health = m_impl->ecsWorld.getComponent<HealthComponent>(target);
  if (!health)
    return;
  
  // Apply damage
  health->currentHealth -= damage.damage;
  health->currentHealth = std::max(0.0f, health->currentHealth);
  
  // Emit damage event
  m_impl->eventBus.entityDamaged.emit({target, NullEntity, damage.damage, damage.damageType});
}

bool WorldIntegration::isEntityAlive(Entity entity) const {
  if (!m_impl->ecsWorld.isAlive(entity))
    return false;
  
  if (auto* health = m_impl->ecsWorld.getComponent<HealthComponent>(entity))
    return health->currentHealth > 0;
  
  return true;
}

ByteArray WorldIntegration::serializeEntity(Entity entity) const {
  // Simplified serialization
  DataStreamBuffer buffer;
  
  if (auto* pos = m_impl->ecsWorld.getComponent<PositionComponent>(entity)) {
    buffer.write(pos->position);
  }
  
  if (auto* vel = m_impl->ecsWorld.getComponent<VelocityComponent>(entity)) {
    buffer.write(vel->velocity);
  }
  
  if (auto* health = m_impl->ecsWorld.getComponent<HealthComponent>(entity)) {
    buffer.write(health->currentHealth);
    buffer.write(health->maxHealth);
  }
  
  return buffer.takeData();
}

Entity WorldIntegration::deserializeEntity(ByteArray const& data) {
  Entity entity = m_impl->ecsWorld.createEntity();
  
  DataStreamBuffer buffer(data);
  
  // Read position
  PositionComponent pos;
  buffer.read(pos.position);
  m_impl->ecsWorld.addComponent<PositionComponent>(entity, pos);
  
  return entity;
}

void WorldIntegration::markDirty(Entity entity) {
  if (auto* netState = m_impl->ecsWorld.getComponent<NetworkStateComponent>(entity))
    netState->isDirty = true;
}

void WorldIntegration::setDebugMode(bool enabled) {
  m_impl->debugMode = enabled;
}

bool WorldIntegration::debugMode() const {
  return m_impl->debugMode;
}

String WorldIntegration::debugInfo(Entity entity) const {
  StringList info;
  
  info.append(strf("Entity: {}", entity));
  
  if (auto pos = entityPosition(entity))
    info.append(strf("Position: ({}, {})", pos->x(), pos->y()));
  
  if (auto vel = entityVelocity(entity))
    info.append(strf("Velocity: ({}, {})", vel->x(), vel->y()));
  
  if (auto* health = m_impl->ecsWorld.getComponent<HealthComponent>(entity))
    info.append(strf("Health: {}/{}", health->currentHealth, health->maxHealth));
  
  return info.join("\n");
}

// BatchMigration implementation
BatchMigration::BatchMigration(WorldIntegration& integration)
  : m_integration(integration) {}

void BatchMigration::addEntity(EntityPtr const& entity) {
  if (entity && !m_cancelled)
    m_entities.append(entity);
}

List<MigrationResult> BatchMigration::execute() {
  List<MigrationResult> results;
  
  for (auto const& entity : m_entities) {
    if (m_cancelled) {
      results.append({MigrationState::Failed, NullEntity, "Migration cancelled"});
      continue;
    }
    
    Entity ecsEntity = m_integration.migrateEntity(entity);
    
    if (ecsEntity != NullEntity) {
      results.append({MigrationState::Migrated, ecsEntity, ""});
    } else {
      results.append({MigrationState::Failed, NullEntity, "Migration failed"});
    }
    
    ++m_processed;
  }
  
  return results;
}

float BatchMigration::progress() const {
  if (m_entities.empty())
    return 1.0f;
  return static_cast<float>(m_processed) / m_entities.size();
}

void BatchMigration::cancel() {
  m_cancelled = true;
}

// ComponentPoolOptimizer implementation
ComponentPoolOptimizer::ComponentPoolOptimizer(ECS::World& world)
  : m_world(world) {}

void ComponentPoolOptimizer::defragment() {
  // Component storage defragmentation
  // The sparse set implementation already maintains good locality
  // This is a placeholder for future optimization
}

ComponentPoolOptimizer::MemoryStats ComponentPoolOptimizer::memoryStats() const {
  MemoryStats stats;
  stats.totalAllocated = 0;
  stats.totalUsed = 0;
  stats.fragmentationPercent = 0;
  
  // Collect memory stats from component storages
  // This is a simplified implementation
  
  return stats;
}

// SystemScheduler implementation
SystemScheduler::SystemScheduler() = default;

void SystemScheduler::addSystem(SystemPtr system, List<String> dependencies) {
  String name = typeid(*system).name();
  m_systems[name] = {std::move(system), std::move(dependencies), true, 0.0f};
  m_orderDirty = true;
}

void SystemScheduler::removeSystem(String const& name) {
  m_systems.remove(name);
  m_orderDirty = true;
}

void SystemScheduler::setEnabled(String const& name, bool enabled) {
  if (auto* info = m_systems.ptr(name))
    info->enabled = enabled;
}

void SystemScheduler::update(ECS::World& world, float dt) {
  if (m_orderDirty)
    rebuildOrder();
  
  for (auto const& name : m_executionOrder) {
    auto* info = m_systems.ptr(name);
    if (!info || !info->enabled)
      continue;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    info->system->init(&world);
    info->system->update(dt);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    info->lastExecutionTime = std::chrono::duration<float>(endTime - startTime).count() * 1000.0f;
  }
}

List<String> SystemScheduler::executionOrder() const {
  return m_executionOrder;
}

HashMap<String, float> SystemScheduler::systemTimings() const {
  HashMap<String, float> timings;
  for (auto const& [name, info] : m_systems) {
    timings[name] = info.lastExecutionTime;
  }
  return timings;
}

void SystemScheduler::rebuildOrder() {
  m_executionOrder.clear();
  
  // Topological sort based on dependencies
  HashSet<String> visited;
  HashSet<String> inProgress;
  
  function<void(String const&)> visit = [&](String const& name) {
    if (visited.contains(name))
      return;
    if (inProgress.contains(name))
      return; // Circular dependency, skip
    
    inProgress.add(name);
    
    if (auto* info = m_systems.ptr(name)) {
      for (auto const& dep : info->dependencies) {
        visit(dep);
      }
    }
    
    inProgress.remove(name);
    visited.add(name);
    m_executionOrder.append(name);
  };
  
  for (auto const& [name, _] : m_systems) {
    visit(name);
  }
  
  m_orderDirty = false;
}

// EntityArchetype implementation
EntityArchetype::EntityArchetype(String name)
  : m_name(std::move(name)) {}

Entity EntityArchetype::create(ECS::World& world) const {
  Entity entity = world.createEntity();
  
  for (auto const& initializer : m_componentInitializers) {
    initializer(world, entity);
  }
  
  return entity;
}

List<Entity> EntityArchetype::createBatch(ECS::World& world, size_t count) const {
  List<Entity> entities;
  entities.reserve(count);
  
  for (size_t i = 0; i < count; ++i) {
    entities.append(create(world));
  }
  
  return entities;
}

// ArchetypeRegistry implementation
ArchetypeRegistry& ArchetypeRegistry::instance() {
  static ArchetypeRegistry registry;
  return registry;
}

ArchetypeRegistry::ArchetypeRegistry() {
  // Register default archetypes
  
  // Basic moving entity
  registerArchetype(
    EntityArchetype("MovingEntity")
      .withComponent<PositionComponent>()
      .withComponent<VelocityComponent>()
      .withComponent<BoundsComponent>()
  );
  
  // Combat entity
  registerArchetype(
    EntityArchetype("CombatEntity")
      .withComponent<PositionComponent>()
      .withComponent<VelocityComponent>()
      .withComponent<BoundsComponent>()
      .withComponent<HealthComponent>({100.0f, 100.0f})
      .withComponent<TeamComponent>()
  );
  
  // Actor entity (monster, NPC, player base)
  registerArchetype(
    EntityArchetype("ActorEntity")
      .withComponent<PositionComponent>()
      .withComponent<VelocityComponent>()
      .withComponent<BoundsComponent>()
      .withComponent<HealthComponent>({100.0f, 100.0f})
      .withComponent<EnergyComponent>({100.0f, 100.0f})
      .withComponent<TeamComponent>()
      .withComponent<MovementStateComponent>()
  );
}

void ArchetypeRegistry::registerArchetype(EntityArchetype archetype) {
  m_archetypes[archetype.name()] = std::move(archetype);
}

Maybe<EntityArchetype const&> ArchetypeRegistry::getArchetype(String const& name) const {
  if (auto* archetype = m_archetypes.ptr(name))
    return *archetype;
  return {};
}

Entity ArchetypeRegistry::createFromArchetype(ECS::World& world, String const& archetypeName) const {
  if (auto archetype = getArchetype(archetypeName))
    return archetype->create(world);
  return NullEntity;
}

List<String> ArchetypeRegistry::archetypeNames() const {
  List<String> names;
  for (auto const& [name, _] : m_archetypes) {
    names.append(name);
  }
  return names;
}

} // namespace ECS
} // namespace Star

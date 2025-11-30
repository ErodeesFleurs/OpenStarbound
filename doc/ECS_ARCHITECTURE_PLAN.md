# OpenStarbound ECS Architecture Refactoring Plan

## 中文概述 (Chinese Summary)

本文档详细描述了将OpenStarbound游戏从传统的面向对象架构重构为ECS（Entity-Component-System）架构的完整计划。

**ECS架构的核心概念：**
- **Entity（实体）**：一个唯一标识符（ID），不包含任何数据或行为
- **Component（组件）**：纯数据容器，不包含任何逻辑
- **System（系统）**：处理拥有特定组件组合的实体的逻辑

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Current Architecture Analysis](#current-architecture-analysis)
3. [Proposed ECS Architecture](#proposed-ecs-architecture)
4. [Component Design](#component-design)
5. [System Design](#system-design)
6. [Migration Strategy](#migration-strategy)
7. [Implementation Phases](#implementation-phases)
8. [Risk Assessment](#risk-assessment)
9. [Timeline Estimates](#timeline-estimates)

---

## Executive Summary

### Project Scope
- **Total Codebase Size**: 221,000+ lines of C++ code
- **Affected Files**: ~200+ source files in the game module
- **Estimated Effort**: 6-12 months for full migration (incremental approach recommended)

### Goals
1. Improve code modularity and maintainability
2. Enable better performance through data-oriented design
3. Simplify adding new entity types and behaviors
4. Reduce coupling between systems

### Non-Goals
1. Complete rewrite of the engine
2. Changing external interfaces (networking, Lua scripting)
3. Breaking mod compatibility

---

## Current Architecture Analysis

### Entity Hierarchy

The current codebase uses a traditional OOP inheritance hierarchy:

```
Entity (base class)
├── Player
│   └── inherits: ToolUserEntity, LoungingEntity, ChattyEntity,
│                 InspectableEntity, DamageBarEntity, PortraitEntity,
│                 NametagEntity, PhysicsEntity, EmoteEntity
├── Monster
│   └── inherits: DamageBarEntity, AggressiveEntity, ScriptedEntity,
│                 PhysicsEntity, NametagEntity, ChattyEntity,
│                 InteractiveEntity, ActorEntity
├── Npc
│   └── inherits: Similar to Monster + additional interfaces
├── Object
│   └── inherits: TileEntity, InteractiveEntity, ScriptedEntity, etc.
├── Vehicle
├── Projectile
├── ItemDrop
├── PlantDrop
├── Plant
└── Stagehand
```

### Current Components (as Controllers)

The game already has some component-like structures:
- `ActorMovementController` - Handles movement physics
- `StatusController` - Manages health, energy, status effects
- `TechController` - Handles tech/abilities
- `ToolUser` - Manages tool/weapon usage
- `ArmorWearer` - Manages equipment

### Current Systems

Implicit systems scattered throughout:
- Damage system (DamageManager)
- Physics/collision system
- Rendering system
- AI/Behavior system
- Inventory system
- Network synchronization system

### Problems with Current Architecture

1. **Diamond inheritance problem**: Multiple inheritance creates complexity
2. **Tight coupling**: Entity types are tightly coupled to their behaviors
3. **Code duplication**: Similar logic repeated across entity types
4. **Hard to extend**: Adding new behaviors requires modifying class hierarchies
5. **Memory layout**: Objects scattered in memory, poor cache locality

---

## Proposed ECS Architecture

### Core ECS Framework

```cpp
namespace Star::ECS {

// Entity is just an ID
using Entity = uint64_t;
constexpr Entity NullEntity = 0;

// Component base (optional, for reflection)
struct ComponentBase {
    virtual ~ComponentBase() = default;
    virtual StringView typeName() const = 0;
};

// Component storage interface
template<typename T>
class ComponentArray {
public:
    void insert(Entity entity, T component);
    void remove(Entity entity);
    T* get(Entity entity);
    T const* get(Entity entity) const;
    bool has(Entity entity) const;
    
    // Iteration support
    auto begin();
    auto end();
};

// World manages all entities and components
class World {
public:
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isAlive(Entity entity) const;
    
    template<typename T>
    T& addComponent(Entity entity, T component = {});
    
    template<typename T>
    void removeComponent(Entity entity);
    
    template<typename T>
    T* getComponent(Entity entity);
    
    template<typename T>
    bool hasComponent(Entity entity) const;
    
    // Query entities with specific components
    template<typename... Components>
    View<Components...> view();
};

// System base class
class System {
public:
    virtual ~System() = default;
    virtual void init(World* world) { m_world = world; }
    virtual void update(float dt) = 0;
    virtual int priority() const { return 0; }
    
protected:
    World* m_world = nullptr;
};

} // namespace Star::ECS
```

### Component Design Principles

1. **Data Only**: Components contain only data, no logic
2. **POD When Possible**: Prefer plain-old-data for better cache performance
3. **Single Responsibility**: Each component represents one aspect
4. **Composability**: Components can be combined freely

---

## Component Design

### Position & Transform Components

```cpp
// Basic transform data
struct TransformComponent {
    Vec2F position;
    float rotation = 0.0f;
    Vec2F scale = {1.0f, 1.0f};
};

// Velocity for moving entities
struct VelocityComponent {
    Vec2F velocity;
    Vec2F acceleration;
};

// Bounding box for spatial queries
struct BoundingBoxComponent {
    RectF boundingBox;
    RectF collisionArea;
};
```

### Physics Components

```cpp
struct PhysicsBodyComponent {
    float mass = 1.0f;
    float gravityMultiplier = 1.0f;
    float liquidBuoyancy = 0.0f;
    float airBuoyancy = 0.0f;
    float bounceFactor = 0.0f;
    bool collisionEnabled = true;
    bool gravityEnabled = true;
};

struct CollisionComponent {
    PolyF standingPoly;
    PolyF crouchingPoly;
    CollisionSet collisionSet;
    bool onGround = false;
    bool inLiquid = false;
};

struct MovementComponent {
    float walkSpeed = 8.0f;
    float runSpeed = 14.0f;
    float flySpeed = 15.0f;
    Direction facingDirection = Direction::Right;
    Direction movingDirection = Direction::Right;
    bool crouching = false;
    bool flying = false;
    bool jumping = false;
};
```

### Combat Components

```cpp
struct HealthComponent {
    float currentHealth = 100.0f;
    float maxHealth = 100.0f;
    bool invulnerable = false;
};

struct DamageSourceComponent {
    List<DamageSource> damageSources;
    EntityDamageTeam team;
};

struct DamageReceiverComponent {
    PolyF hitPoly;
    EntityDamageTeam team;
    List<DamageNotification> pendingNotifications;
};

struct StatusEffectsComponent {
    List<PersistentStatusEffect> persistentEffects;
    List<EphemeralStatusEffect> ephemeralEffects;
    StringMap<float> stats;
    StringMap<float> resources;
};
```

### Visual Components

```cpp
struct SpriteComponent {
    String imagePath;
    Directives directives;
    float animationTimer = 0.0f;
    int currentFrame = 0;
};

struct HumanoidComponent {
    HumanoidIdentity identity;
    HumanoidEmote emote;
    String dance;
    // Armor references
    Maybe<ArmorItem> headArmor;
    Maybe<ArmorItem> chestArmor;
    Maybe<ArmorItem> legsArmor;
    Maybe<ArmorItem> backArmor;
};

struct LightSourcesComponent {
    List<LightSource> lightSources;
};

struct ParticleEmitterComponent {
    List<Particle> pendingParticles;
};
```

### AI & Behavior Components

```cpp
struct AIComponent {
    String behaviorTree;
    Json behaviorConfig;
    BehaviorStatePtr currentState;
    bool aggressive = false;
};

struct PathfindingComponent {
    Maybe<Vec2F> targetPosition;
    Maybe<PlatformerAStar::Path> currentPath;
    size_t pathIndex = 0;
};

struct ScriptComponent {
    String scriptPath;
    LuaContextPtr context;
    StringMap<LuaValue> scriptData;
};
```

### Identity Components

```cpp
struct NameComponent {
    String name;
    String description;
    Maybe<String> statusText;
    bool displayNametag = true;
    Vec3B nametagColor = {255, 255, 255};
};

struct UniqueIdComponent {
    Maybe<String> uniqueId;
};

struct PersistenceComponent {
    bool persistent = false;
    bool keepAlive = false;
};
```

### Player-Specific Components

```cpp
struct PlayerComponent {
    Uuid uuid;
    PlayerMode mode;
    bool isAdmin = false;
};

struct InventoryComponent {
    PlayerInventoryPtr inventory;
    PlayerBlueprintsPtr blueprints;
};

struct QuestComponent {
    QuestManagerPtr questManager;
};

struct TechComponent {
    PlayerTechPtr techs;
    Maybe<StringList> overrideTech;
};
```

### Network Components

```cpp
struct NetworkSyncComponent {
    uint64_t netVersion = 0;
    bool isDirty = false;
    ByteArray lastNetState;
};

struct InterpolationComponent {
    float extrapolationHint = 0.0f;
    bool interpolationEnabled = false;
    float interpolationTime = 0.0f;
};
```

---

## System Design

### Core Systems

```cpp
// Movement system - processes physics and movement
class MovementSystem : public System {
public:
    void update(float dt) override {
        for (auto [entity, transform, velocity, physics, movement] : 
             m_world->view<TransformComponent, VelocityComponent, 
                          PhysicsBodyComponent, MovementComponent>()) {
            // Apply gravity
            if (physics.gravityEnabled) {
                velocity.velocity.y() -= m_gravity * physics.gravityMultiplier * dt;
            }
            
            // Apply movement controls
            applyMovementControls(entity, movement, velocity, dt);
            
            // Update position
            transform.position += velocity.velocity * dt;
        }
    }
    
    int priority() const override { return 100; }
};

// Collision system - handles collision detection and response
class CollisionSystem : public System {
public:
    void update(float dt) override {
        for (auto [entity, transform, velocity, collision] : 
             m_world->view<TransformComponent, VelocityComponent, CollisionComponent>()) {
            // Check tile collisions
            checkTileCollisions(entity, transform, velocity, collision);
            
            // Check entity collisions
            checkEntityCollisions(entity, transform, collision);
        }
    }
    
    int priority() const override { return 90; }
};

// Damage system - processes damage between entities
class DamageSystem : public System {
public:
    void update(float dt) override {
        // Process damage sources
        for (auto [sourceEntity, transform, damageSource] : 
             m_world->view<TransformComponent, DamageSourceComponent>()) {
            for (auto& source : damageSource.damageSources) {
                processSource(sourceEntity, transform, source);
            }
        }
        
        // Apply damage to receivers
        for (auto [entity, health, receiver] : 
             m_world->view<HealthComponent, DamageReceiverComponent>()) {
            for (auto& notification : receiver.pendingNotifications) {
                applyDamage(entity, health, notification);
            }
            receiver.pendingNotifications.clear();
        }
    }
    
    int priority() const override { return 80; }
};

// Status effect system
class StatusEffectSystem : public System {
public:
    void update(float dt) override {
        for (auto [entity, status] : 
             m_world->view<StatusEffectsComponent>()) {
            // Update effect durations
            updateEffectDurations(status, dt);
            
            // Apply stat modifications
            applyStatModifications(entity, status);
            
            // Update resources
            updateResources(entity, status, dt);
        }
    }
    
    int priority() const override { return 70; }
};

// Rendering system - prepares render data
class RenderSystem : public System {
public:
    void update(float dt) override {
        m_renderData.clear();
        
        for (auto [entity, transform, sprite] : 
             m_world->view<TransformComponent, SpriteComponent>()) {
            // Create drawable
            Drawable drawable = createDrawable(transform, sprite);
            m_renderData.append(drawable);
        }
        
        // Handle humanoids separately
        for (auto [entity, transform, humanoid] : 
             m_world->view<TransformComponent, HumanoidComponent>()) {
            List<Drawable> drawables = renderHumanoid(transform, humanoid);
            m_renderData.appendAll(drawables);
        }
    }
    
    List<Drawable> const& getRenderData() const { return m_renderData; }
    
    int priority() const override { return 10; }
    
private:
    List<Drawable> m_renderData;
};

// AI system - runs behavior trees
class AISystem : public System {
public:
    void update(float dt) override {
        for (auto [entity, transform, ai] : 
             m_world->view<TransformComponent, AIComponent>()) {
            if (ai.currentState) {
                ai.currentState->run(dt);
            }
        }
    }
    
    int priority() const override { return 60; }
};

// Script system - runs Lua scripts
class ScriptSystem : public System {
public:
    void update(float dt) override {
        for (auto [entity, script] : m_world->view<ScriptComponent>()) {
            if (script.context) {
                script.context->invoke("update", dt);
            }
        }
    }
    
    int priority() const override { return 50; }
};

// Network sync system
class NetworkSyncSystem : public System {
public:
    void update(float dt) override {
        for (auto [entity, netSync] : m_world->view<NetworkSyncComponent>()) {
            if (netSync.isDirty) {
                // Serialize entity state
                netSync.lastNetState = serializeEntity(entity);
                netSync.netVersion++;
                netSync.isDirty = false;
            }
        }
    }
    
    int priority() const override { return 5; }
};
```

---

## Migration Strategy

### Phase 1: Foundation (4-6 weeks)

**Goal**: Implement core ECS framework without modifying existing code

1. Create ECS namespace and core types
2. Implement Entity manager
3. Implement Component storage (sparse set recommended)
4. Implement System scheduler
5. Add comprehensive unit tests

**Files to create**:
- `source/core/ecs/StarEcsTypes.hpp`
- `source/core/ecs/StarEcsEntity.hpp`
- `source/core/ecs/StarEcsEntity.cpp`
- `source/core/ecs/StarEcsComponent.hpp`
- `source/core/ecs/StarEcsWorld.hpp`
- `source/core/ecs/StarEcsWorld.cpp`
- `source/core/ecs/StarEcsSystem.hpp`
- `source/core/ecs/StarEcsView.hpp`

### Phase 2: Component Definitions (2-3 weeks)

**Goal**: Define all components based on existing entity data

1. Analyze each entity type for data members
2. Create component definitions
3. Ensure components cover all existing functionality
4. Document component relationships

**Files to create**:
- `source/game/ecs/components/StarTransformComponents.hpp`
- `source/game/ecs/components/StarPhysicsComponents.hpp`
- `source/game/ecs/components/StarCombatComponents.hpp`
- `source/game/ecs/components/StarVisualComponents.hpp`
- `source/game/ecs/components/StarAIComponents.hpp`
- `source/game/ecs/components/StarPlayerComponents.hpp`
- `source/game/ecs/components/StarNetworkComponents.hpp`

### Phase 3: System Implementation (4-6 weeks)

**Goal**: Implement systems using existing logic

1. Extract logic from entity methods into systems
2. Ensure systems work with component data
3. Maintain backward compatibility through adapters

**Files to create**:
- `source/game/ecs/systems/StarMovementSystem.hpp/cpp`
- `source/game/ecs/systems/StarCollisionSystem.hpp/cpp`
- `source/game/ecs/systems/StarDamageSystem.hpp/cpp`
- `source/game/ecs/systems/StarStatusEffectSystem.hpp/cpp`
- `source/game/ecs/systems/StarRenderSystem.hpp/cpp`
- `source/game/ecs/systems/StarAISystem.hpp/cpp`
- `source/game/ecs/systems/StarScriptSystem.hpp/cpp`
- `source/game/ecs/systems/StarNetworkSyncSystem.hpp/cpp`

### Phase 4: Entity Adapters (3-4 weeks)

**Goal**: Create adapters to allow existing entities to work with ECS

1. Create adapter classes that wrap ECS entities
2. Implement Entity interface using ECS components
3. Allow gradual migration without breaking changes

```cpp
// Example adapter
class PlayerEcsAdapter : public Player {
public:
    PlayerEcsAdapter(ECS::World* world, ECS::Entity entity)
        : m_world(world), m_entity(entity) {}
    
    Vec2F position() const override {
        auto* transform = m_world->getComponent<TransformComponent>(m_entity);
        return transform ? transform->position : Vec2F();
    }
    
    // ... other interface methods
    
private:
    ECS::World* m_world;
    ECS::Entity m_entity;
};
```

### Phase 5: Incremental Migration (8-12 weeks)

**Goal**: Migrate entity types one at a time

**Migration Order** (simplest to most complex):
1. `ItemDrop` - Simple data, minimal behavior
2. `PlantDrop` - Similar to ItemDrop
3. `Projectile` - Well-defined lifecycle
4. `Plant` - Static entity
5. `Stagehand` - Script-driven
6. `Object` - Tile entity with interactions
7. `Vehicle` - More complex physics
8. `Monster` - AI + combat
9. `Npc` - AI + dialogue + interactions
10. `Player` - Most complex, migrate last

### Phase 6: Cleanup (2-4 weeks)

**Goal**: Remove legacy code and optimize

1. Remove old Entity class hierarchy
2. Remove adapter code
3. Optimize component storage
4. Profile and tune performance
5. Update documentation

---

## Implementation Phases

### Immediate Next Steps (This PR)

1. ✅ Create architecture documentation
2. ✅ Implement basic ECS framework
3. ✅ Add unit tests for core ECS functionality
4. ✅ Implement component definitions (Phase 2)
5. ✅ Implement core systems (Phase 3 - partial)
6. ✅ Implement entity adapters (Phase 4 - complete)
   - Base EntityAdapter class
   - ItemDropAdapter (first entity migration)
   - AdapterFactory for creating adapters
7. ✅ **Phase 5: Incremental Migration - COMPLETE**
   - ✅ PlantDropAdapter (second entity migration)
   - ✅ PlantDropTag and PlantDropDataComponent
   - ✅ ProjectileAdapter (third entity migration)
   - ✅ ProjectileDataComponent with full damage system
   - ✅ PlantAdapter - Trees, grass, bushes with wind animation
   - ✅ StagehandAdapter - Scripted world triggers
   - ✅ ObjectAdapter - Interactive tile entities with wiring support
   - ✅ VehicleAdapter - Mountable vehicles with physics
   - ✅ MonsterAdapter - AI + combat with scripting support
   - ✅ NpcAdapter - AI + dialogue + interactions + humanoid
   - ✅ PlayerAdapter - Full player implementation with all systems
8. ✅ **Phase 6: Cleanup and Integration - COMPLETE**
   - ✅ WorldIntegration - Bridge between legacy World and ECS
   - ✅ Performance metrics and profiling
   - ✅ BatchMigration utility for entity migration
   - ✅ ComponentPoolOptimizer for memory optimization
   - ✅ SystemScheduler with dependency resolution
   - ✅ EntityArchetype system for fast entity creation
   - ✅ ArchetypeRegistry with default archetypes
   - ✅ Event system (EntityCreated, EntityDestroyed, EntityDamaged, EntityMoved)
   - ✅ WorldEventBus for decoupled communication

### Short-term (1-2 months)

1. Complete Phase 1 (Foundation)
2. Start Phase 2 (Component Definitions)
3. Create integration tests

### Medium-term (3-4 months)

1. Complete Phase 2
2. Start Phase 3 (System Implementation)
3. Begin adapter development

### Long-term (6-12 months)

1. Complete all phases
2. Full migration to ECS
3. Performance optimization
4. Documentation updates

---

## Risk Assessment

### High Risk Areas

1. **Networking**: Entity serialization/deserialization must remain compatible
2. **Lua Scripting**: Script callbacks must work with new architecture
3. **Mod Compatibility**: Must maintain mod API compatibility
4. **Save Files**: Player/world saves must remain loadable

### Mitigation Strategies

1. **Comprehensive Testing**: Unit tests, integration tests, and manual testing
2. **Feature Flags**: Allow toggling between old/new systems
3. **Incremental Migration**: Never break existing functionality
4. **Backward Compatibility Layer**: Maintain old interfaces during transition

---

## Timeline Estimates

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1: Foundation | 4-6 weeks | None |
| Phase 2: Components | 2-3 weeks | Phase 1 |
| Phase 3: Systems | 4-6 weeks | Phase 2 |
| Phase 4: Adapters | 3-4 weeks | Phase 3 |
| Phase 5: Migration | 8-12 weeks | Phase 4 |
| Phase 6: Cleanup | 2-4 weeks | Phase 5 |
| **Total** | **23-35 weeks** | |

---

## Appendix: Code Examples

### Creating an Entity

```cpp
// Old way (current)
auto monster = make_shared<Monster>(monsterVariant, level);
world->addEntity(monster);

// New way (ECS)
ECS::Entity entity = ecsWorld.createEntity();
ecsWorld.addComponent<TransformComponent>(entity, {position, rotation});
ecsWorld.addComponent<VelocityComponent>(entity);
ecsWorld.addComponent<PhysicsBodyComponent>(entity, physicsParams);
ecsWorld.addComponent<HealthComponent>(entity, {100.0f, 100.0f});
ecsWorld.addComponent<AIComponent>(entity, {behaviorTree, config});
ecsWorld.addComponent<SpriteComponent>(entity, {imagePath, directives});
```

### Querying Entities

```cpp
// Find all entities with health below 50%
for (auto [entity, health] : ecsWorld.view<HealthComponent>()) {
    if (health.currentHealth < health.maxHealth * 0.5f) {
        // Apply healing effect
    }
}

// Find all aggressive monsters near player
for (auto [entity, transform, ai] : 
     ecsWorld.view<TransformComponent, AIComponent>()) {
    if (ai.aggressive && 
        vmag(transform.position - playerPos) < aggroRange) {
        // Target player
    }
}
```

### Sending Messages

```cpp
// Old way
entity->receiveMessage(senderId, "heal", {50});

// New way (via event system)
ecsWorld.emit<HealEvent>({targetEntity, 50, senderId});

// Or through script component
auto* script = ecsWorld.getComponent<ScriptComponent>(entity);
if (script && script->context) {
    script->context->invoke("receiveMessage", senderId, "heal", {50});
}
```

---

## Conclusion

This ECS refactoring plan provides a comprehensive roadmap for modernizing OpenStarbound's entity system. The incremental approach ensures minimal risk while delivering tangible improvements at each phase.

Key success factors:
1. Maintain backward compatibility throughout migration
2. Comprehensive testing at every phase
3. Clear documentation and code examples
4. Regular integration with main branch

The estimated timeline of 6-9 months is realistic for a complete migration, though useful improvements can be delivered much sooner through the phased approach.

---

## Implementation Progress

### ✅ Phase 1: Foundation (Complete)
- Core ECS framework (`source/core/ecs/`)
  - `StarEcsTypes.hpp` - Entity ID with generation counters
  - `StarEcsComponent.hpp` - Sparse set component storage
  - `StarEcsSystem.hpp` - System base class
  - `StarEcsView.hpp` - Multi-component entity queries
  - `StarEcsWorld.hpp/cpp` - World container
- Unit tests (`source/test/ecs_test.cpp`)

### ✅ Phase 2: Component Definitions (Complete)
- Game-specific components (`source/game/ecs/components/`)
  - `StarTransformComponents.hpp` - Position, velocity, bounds
  - `StarPhysicsComponents.hpp` - Physics body, collision, movement
  - `StarCombatComponents.hpp` - Health, energy, damage, protection
  - `StarVisualComponents.hpp` - Sprites, animations, lights
  - `StarAIComponents.hpp` - AI behavior, pathfinding, scripting
  - `StarPlayerComponents.hpp` - Player identity, input, tech
  - `StarNetworkComponents.hpp` - Network sync, interpolation

### ✅ Phase 3: System Implementation (Complete)
- Core game systems (`source/game/ecs/systems/`)
  - `StarMovementSystem` - Physics and movement processing
  - `StarDamageSystem` - Damage calculation and application
  - `StarStatusEffectSystem` - Status effects and regeneration
  - `StarRenderSystem` - Sprite and light collection

### ✅ Phase 4: Entity Adapters (Complete)
- Entity adapters (`source/game/ecs/adapters/`)
  - `StarEntityAdapter` - Base adapter class
  - `StarItemDropAdapter` - ItemDrop entity
  - `StarPlantDropAdapter` - PlantDrop entity
  - `StarProjectileAdapter` - Projectile entity
  - `StarPlantAdapter` - Plant entity
  - `StarStagehandAdapter` - Stagehand entity
  - `StarObjectAdapter` - Object entity
  - `StarVehicleAdapter` - Vehicle entity
  - `StarMonsterAdapter` - Monster entity
  - `StarNpcAdapter` - Npc entity
  - `StarPlayerAdapter` - Player entity

### ✅ Phase 5: Incremental Migration (Complete)
All 10 entity types have ECS adapters:
- ✅ ItemDrop
- ✅ PlantDrop
- ✅ Projectile
- ✅ Plant
- ✅ Stagehand
- ✅ Object
- ✅ Vehicle
- ✅ Monster
- ✅ Npc
- ✅ Player

### ✅ Phase 6: Cleanup and Integration (Complete)
- World integration utilities (`source/game/ecs/`)
  - `StarEcsWorldIntegration.hpp/cpp` - Bridge between Star::World and ECS::World
  - `StarEcsMigrationBridge.hpp/cpp` - Migration configuration and entity factory wrapper
- Integration features:
  - `EcsEntityFactory` - Factory wrapper that creates ECS or legacy entities
  - `MigrationConfig` - Per-entity-type ECS toggle
  - `BatchMigration` - Bulk entity migration utility
  - `ComponentPoolOptimizer` - Memory optimization
  - `SystemScheduler` - Dependency-based system execution
  - `EntityArchetype` - Fast entity creation templates
  - Event system (EntityCreated, EntityDestroyed, EntityDamaged, EntityMoved)

---

## Migration Usage

### Enable ECS for Specific Entity Types

```cpp
#include "ecs/StarEcsMigrationBridge.hpp"

// Get global config
auto& config = Star::ECS::globalMigrationConfig();

// Enable ECS for simple entities only (recommended initial setup)
config.useEcsForItemDrop = true;
config.useEcsForPlantDrop = true;
config.useEcsForProjectile = true;
config.ecsEnabled = true;

// Or enable for all entity types
config.useEcsForMonster = true;
config.useEcsForNpc = true;
config.useEcsForPlayer = true;

// Enable migration logging
config.logMigrations = true;
```

### Create ECS-Aware Entity Factory

```cpp
// In WorldServer or WorldClient initialization
auto legacyFactory = Root::singleton().entityFactory();
auto ecsFactory = make_shared<Star::ECS::EcsEntityFactory>(legacyFactory);

// Set up world integration
Star::ECS::WorldIntegration worldIntegration;
worldIntegration.init(this);
ecsFactory->setWorldIntegration(&worldIntegration);

// Use factory to create entities (automatically uses ECS or legacy based on config)
EntityPtr entity = ecsFactory->netLoadEntity(EntityType::Monster, netStore);
```

### Migration Statistics

```cpp
// Get statistics
auto& stats = ecsFactory->stats();
Logger::info("ECS entities: {}, Legacy entities: {}, Failures: {}",
  stats.ecsEntitiesCreated, stats.legacyEntitiesCreated, stats.migrationFailures);
```

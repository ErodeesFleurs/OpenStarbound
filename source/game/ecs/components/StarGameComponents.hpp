#pragma once

// Game-specific ECS components for OpenStarbound
// These components are designed to replace the data members
// currently spread across the entity class hierarchy.

#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarGameTypes.hpp"
#include "StarDamage.hpp"
#include "StarLightSource.hpp"
#include "StarCollisionBlock.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Transform Components
//=============================================================================

// Basic position and rotation
struct TransformComponent {
  Vec2F position = {};
  float rotation = 0.0f;
  Vec2F scale = {1.0f, 1.0f};
  
  // Convenience methods
  void setPosition(Vec2F const& pos) { position = pos; }
  void move(Vec2F const& delta) { position += delta; }
};

// Velocity for moving entities
struct VelocityComponent {
  Vec2F velocity = {};
  Vec2F acceleration = {};
  
  void setVelocity(Vec2F const& vel) { velocity = vel; }
  void addVelocity(Vec2F const& delta) { velocity += delta; }
};

// Bounding box for spatial queries
struct BoundsComponent {
  RectF metaBoundBox = {};
  RectF collisionArea = {};
  
  RectF worldBounds(Vec2F const& position) const {
    return metaBoundBox.translated(position);
  }
};

//=============================================================================
// Physics Components
//=============================================================================

// Physical properties for movement
struct PhysicsBodyComponent {
  float mass = 1.0f;
  float gravityMultiplier = 1.0f;
  float liquidBuoyancy = 0.0f;
  float airBuoyancy = 0.0f;
  float bounceFactor = 0.0f;
  float airFriction = 0.0f;
  float liquidFriction = 0.0f;
  float groundFriction = 0.0f;
  bool collisionEnabled = true;
  bool gravityEnabled = true;
  bool frictionEnabled = true;
};

// Collision shape and state
struct CollisionComponent {
  PolyF standingPoly = {};
  PolyF crouchingPoly = {};
  CollisionSet collisionSet = DefaultCollisionSet;
  bool onGround = false;
  bool inLiquid = false;
  float liquidPercentage = 0.0f;
  bool collidingWithPlatform = false;
};

// Movement state
struct MovementStateComponent {
  float walkSpeed = 8.0f;
  float runSpeed = 14.0f;
  float flySpeed = 15.0f;
  Direction facingDirection = Direction::Right;
  Direction movingDirection = Direction::Right;
  bool walking = false;
  bool running = false;
  bool crouching = false;
  bool flying = false;
  bool jumping = false;
  bool falling = false;
  bool groundMovement = false;
  bool liquidMovement = false;
};

//=============================================================================
// Combat Components
//=============================================================================

// Health and death state
struct HealthComponent {
  float currentHealth = 100.0f;
  float maxHealth = 100.0f;
  bool invulnerable = false;
  bool dead = false;
  
  float healthPercentage() const {
    return maxHealth > 0 ? currentHealth / maxHealth : 0.0f;
  }
  
  bool isAlive() const {
    return !dead && currentHealth > 0;
  }
};

// Energy resource
struct EnergyComponent {
  float currentEnergy = 100.0f;
  float maxEnergy = 100.0f;
  bool locked = false;
  float regenRate = 10.0f;
  float regenBlockPercent = 0.0f;
  
  bool consume(float amount) {
    if (locked || currentEnergy < amount) return false;
    currentEnergy -= amount;
    return true;
  }
};

// Damage sources this entity produces
struct DamageSourceComponent {
  List<DamageSource> damageSources = {};
  EntityDamageTeam team = {};
  bool damageOnTouch = false;
};

// Damage receiving capability
struct DamageReceiverComponent {
  Maybe<PolyF> hitPoly = {};
  EntityDamageTeam team = {};
  List<DamageNotification> pendingDamage = {};
  
  void takeDamage(DamageNotification notification) {
    pendingDamage.append(std::move(notification));
  }
  
  List<DamageNotification> pullDamage() {
    List<DamageNotification> result;
    std::swap(result, pendingDamage);
    return result;
  }
};

// Damage bar display
struct DamageBarComponent {
  DamageBarType type = DamageBarType::Default;
  bool displayHealthBar = true;
};

//=============================================================================
// Visual Components
//=============================================================================

// Light sources
struct LightSourceComponent {
  List<LightSource> sources = {};
  
  void addLight(LightSource source) {
    sources.append(std::move(source));
  }
  
  void clearLights() {
    sources.clear();
  }
};

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
};

//=============================================================================
// Network Components
//=============================================================================

// Network synchronization state
struct NetworkSyncComponent {
  uint64_t netVersion = 0;
  bool isDirty = false;
  bool interpolationEnabled = false;
  float interpolationTime = 0.0f;
  float extrapolationHint = 0.0f;
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

// Mark entity as aggressive
struct AggressiveTag {};

// Mark entity as scripted
struct ScriptedTag {};

} // namespace ECS
} // namespace Star

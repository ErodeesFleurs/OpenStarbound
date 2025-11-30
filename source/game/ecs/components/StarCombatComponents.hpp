#pragma once

// Combat-related ECS components for OpenStarbound
// These components handle health, damage, and combat state

#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarMaybe.hpp"
#include "StarList.hpp"
#include "StarDamage.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Combat Components
//=============================================================================

// Health and death state
struct HealthComponent {
  float currentHealth = 100.0f;
  float maxHealth = 100.0f;
  bool invulnerable = false;
  bool dead = false;
  float invulnerabilityTimer = 0.0f;
  float deathTimer = 0.0f;
  
  float healthPercentage() const {
    return maxHealth > 0 ? currentHealth / maxHealth : 0.0f;
  }
  
  bool isAlive() const {
    return !dead && currentHealth > 0;
  }
  
  void heal(float amount) {
    if (dead) return;
    currentHealth = std::min(currentHealth + amount, maxHealth);
  }
  
  void damage(float amount) {
    if (dead || invulnerable || invulnerabilityTimer > 0) return;
    currentHealth -= amount;
    if (currentHealth <= 0) {
      currentHealth = 0;
      dead = true;
    }
  }
  
  void setMaxHealth(float newMax, bool keepPercentage = true) {
    if (keepPercentage) {
      float percentage = healthPercentage();
      maxHealth = newMax;
      currentHealth = percentage * maxHealth;
    } else {
      maxHealth = newMax;
      currentHealth = std::min(currentHealth, maxHealth);
    }
  }
};

// Energy resource
struct EnergyComponent {
  float currentEnergy = 100.0f;
  float maxEnergy = 100.0f;
  bool locked = false;
  float regenRate = 10.0f;
  float regenBlockPercent = 0.0f;
  float regenBlockTimer = 0.0f;
  
  float energyPercentage() const {
    return maxEnergy > 0 ? currentEnergy / maxEnergy : 0.0f;
  }
  
  bool isFull() const {
    return currentEnergy >= maxEnergy;
  }
  
  bool consume(float amount) {
    if (locked || currentEnergy < amount) return false;
    currentEnergy -= amount;
    return true;
  }
  
  void restore(float amount) {
    currentEnergy = std::min(currentEnergy + amount, maxEnergy);
  }
  
  void blockRegen(float duration) {
    regenBlockTimer = std::max(regenBlockTimer, duration);
    regenBlockPercent = 1.0f;
  }
};

// Damage sources this entity produces
struct DamageSourceComponent {
  List<DamageSource> damageSources = {};
  EntityDamageTeam team = {};
  bool damageOnTouch = false;
  float knockbackMultiplier = 1.0f;
  
  void clearSources() {
    damageSources.clear();
  }
  
  void addSource(DamageSource source) {
    damageSources.append(std::move(source));
  }
};

// Damage receiving capability
struct DamageReceiverComponent {
  Maybe<PolyF> hitPoly = {};
  EntityDamageTeam team = {};
  List<DamageNotification> pendingDamage = {};
  float knockbackResistance = 0.0f;
  
  void takeDamage(DamageNotification notification) {
    pendingDamage.append(std::move(notification));
  }
  
  List<DamageNotification> pullDamage() {
    List<DamageNotification> result;
    std::swap(result, pendingDamage);
    return result;
  }
  
  bool hasPendingDamage() const {
    return !pendingDamage.empty();
  }
};

// Damage bar display
struct DamageBarComponent {
  DamageBarType type = DamageBarType::Default;
  bool displayHealthBar = true;
  float displayTime = 2.0f;
  float displayTimer = 0.0f;
  
  void triggerDisplay() {
    displayTimer = displayTime;
  }
  
  bool shouldDisplay() const {
    return displayHealthBar && displayTimer > 0;
  }
};

// Protection/armor stats
struct ProtectionComponent {
  float protection = 0.0f;
  StringMap<float> resistances = {};
  
  float effectiveDamage(float baseDamage, String const& damageType = "") const {
    float multiplier = 1.0f - (protection / 100.0f);
    multiplier = std::max(multiplier, 0.0f);
    
    if (!damageType.empty()) {
      auto it = resistances.find(damageType);
      if (it != resistances.end()) {
        multiplier *= (1.0f - it->second);
      }
    }
    
    return baseDamage * multiplier;
  }
};

// Aggro/threat tracking
struct AggroComponent {
  Entity currentTarget = 0; // NullEntity
  float aggroRange = 20.0f;
  float deaggroRange = 30.0f;
  float aggroTimer = 0.0f;
  bool aggressive = false;
  
  void setTarget(Entity target) {
    currentTarget = target;
    aggroTimer = 0.0f;
  }
  
  void clearTarget() {
    currentTarget = 0;
    aggroTimer = 0.0f;
  }
  
  bool hasTarget() const {
    return currentTarget != 0;
  }
};

} // namespace ECS
} // namespace Star

#pragma once

// AI/Behavior ECS components for OpenStarbound
// These components handle AI behavior, pathfinding, and scripting

#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarJson.hpp"
#include "StarMaybe.hpp"
#include "StarList.hpp"
#include "StarPlatformerAStarTypes.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// AI Components
//=============================================================================

// AI behavior configuration
struct AIComponent {
  String behaviorTree = "";
  Json behaviorConfig = {};
  String currentBehavior = "";
  float behaviorTimer = 0.0f;
  bool aggressive = false;
  float alertLevel = 0.0f;
  
  void setBehavior(String const& behavior) {
    currentBehavior = behavior;
    behaviorTimer = 0.0f;
  }
};

// Blackboard for AI decision making
struct AIBlackboardComponent {
  JsonObject data = {};
  
  void set(String const& key, Json value) {
    data[key] = std::move(value);
  }
  
  Json get(String const& key, Json defaultValue = {}) const {
    auto it = data.find(key);
    return it != data.end() ? it->second : defaultValue;
  }
  
  bool has(String const& key) const {
    return data.contains(key);
  }
  
  void remove(String const& key) {
    data.erase(key);
  }
  
  void clear() {
    data.clear();
  }
};

// Pathfinding component
struct PathfindingComponent {
  Maybe<Vec2F> targetPosition = {};
  Maybe<PlatformerAStar::Path> currentPath = {};
  size_t pathIndex = 0;
  float pathUpdateTimer = 0.0f;
  float pathUpdateInterval = 0.5f;
  bool pathBlocked = false;
  float stuckTimer = 0.0f;
  float stuckThreshold = 2.0f;
  
  bool hasPath() const {
    return currentPath.isValid() && pathIndex < currentPath->size();
  }
  
  void clearPath() {
    currentPath = {};
    pathIndex = 0;
    pathBlocked = false;
  }
  
  PlatformerAStar::Node const* currentNode() const {
    if (!hasPath()) return nullptr;
    return &(*currentPath)[pathIndex];
  }
  
  void advancePath() {
    if (hasPath()) {
      pathIndex++;
    }
  }
};

// Script component for Lua-scripted entities
struct ScriptComponent {
  String scriptPath = "";
  StringList scripts = {};
  JsonObject scriptStorage = {};
  bool scriptInitialized = false;
  float scriptUpdateTimer = 0.0f;
  
  void setStorage(String const& key, Json value) {
    scriptStorage[key] = std::move(value);
  }
  
  Json getStorage(String const& key, Json defaultValue = {}) const {
    auto it = scriptStorage.find(key);
    return it != scriptStorage.end() ? it->second : defaultValue;
  }
};

// Target tracking for AI
struct TargetTrackingComponent {
  Entity targetEntity = 0; // NullEntity
  Vec2F lastKnownPosition = {};
  float trackingTimer = 0.0f;
  float lostTargetTimer = 0.0f;
  float maxTrackingTime = 10.0f;
  bool hasLineOfSight = false;
  
  void setTarget(Entity target, Vec2F position) {
    targetEntity = target;
    lastKnownPosition = position;
    trackingTimer = 0.0f;
    lostTargetTimer = 0.0f;
  }
  
  void updatePosition(Vec2F position) {
    lastKnownPosition = position;
    lostTargetTimer = 0.0f;
  }
  
  void loseTarget() {
    hasLineOfSight = false;
  }
  
  void clearTarget() {
    targetEntity = 0;
    trackingTimer = 0.0f;
  }
  
  bool hasTarget() const {
    return targetEntity != 0;
  }
  
  bool isTargetLost() const {
    return lostTargetTimer > maxTrackingTime;
  }
};

// Wandering/patrol behavior
struct WanderComponent {
  Vec2F homePosition = {};
  float wanderRadius = 10.0f;
  float wanderTimer = 0.0f;
  float wanderInterval = 5.0f;
  Maybe<Vec2F> wanderTarget = {};
  bool returnHome = false;
  
  void setHome(Vec2F position) {
    homePosition = position;
  }
  
  bool isHome(Vec2F currentPosition, float tolerance = 1.0f) const {
    return vmag(currentPosition - homePosition) <= tolerance;
  }
};

// Skill/ability AI
struct SkillUsageComponent {
  String activeSkill = "";
  float skillCooldown = 0.0f;
  StringMap<float> skillCooldowns = {};
  float skillTimer = 0.0f;
  
  bool canUseSkill(String const& skill) const {
    auto it = skillCooldowns.find(skill);
    return it == skillCooldowns.end() || it->second <= 0.0f;
  }
  
  void useSkill(String const& skill, float cooldown) {
    activeSkill = skill;
    skillCooldowns[skill] = cooldown;
    skillTimer = 0.0f;
  }
  
  void updateCooldowns(float dt) {
    for (auto& [skill, cooldown] : skillCooldowns) {
      if (cooldown > 0.0f) {
        cooldown -= dt;
      }
    }
  }
};

} // namespace ECS
} // namespace Star

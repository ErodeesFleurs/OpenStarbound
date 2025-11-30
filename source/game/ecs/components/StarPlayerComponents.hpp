#pragma once

// Player-specific ECS components for OpenStarbound
// These components handle player-only functionality

#include "StarVector.hpp"
#include "StarString.hpp"
#include "StarUuid.hpp"
#include "StarJson.hpp"
#include "StarPlayerTypes.hpp"

namespace Star {
namespace ECS {

//=============================================================================
// Player Components
//=============================================================================

// Core player identity
struct PlayerComponent {
  Uuid uuid;
  PlayerMode mode = PlayerMode::Casual;
  bool isAdmin = false;
  bool isLocal = false;
  ConnectionId connection = 0;
  
  bool isSurvival() const {
    return mode == PlayerMode::Survival;
  }
  
  bool isHardcore() const {
    return mode == PlayerMode::Hardcore;
  }
};

// Player appearance/identity
struct PlayerIdentityComponent {
  String name = "";
  String species = "human";
  String gender = "";
  Json appearance = {};
  Color favoriteColor = Color::White;
  String description = "";
  
  void setAppearance(Json const& app) {
    appearance = app;
  }
};

// Player input state
struct PlayerInputComponent {
  Vec2F aimPosition = {};
  Vec2F moveVector = {};
  bool primaryFire = false;
  bool altFire = false;
  bool interact = false;
  bool shift = false;
  bool special1 = false;
  bool special2 = false;
  bool special3 = false;
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool jump = false;
  
  void clearInputs() {
    primaryFire = false;
    altFire = false;
    interact = false;
    shift = false;
    special1 = false;
    special2 = false;
    special3 = false;
    up = false;
    down = false;
    left = false;
    right = false;
    jump = false;
  }
};

// Player tech/abilities
struct PlayerTechComponent {
  StringList equippedTechs = {};
  StringList availableTechs = {};
  Maybe<StringList> overrideTechs = {};
  
  bool hasTech(String const& tech) const {
    return availableTechs.contains(tech);
  }
  
  bool techEquipped(String const& tech) const {
    return equippedTechs.contains(tech);
  }
};

// Player quests
struct PlayerQuestComponent {
  StringList activeQuests = {};
  StringList completedQuests = {};
  StringList failedQuests = {};
  Maybe<String> trackedQuest = {};
  
  bool hasQuest(String const& quest) const {
    return activeQuests.contains(quest);
  }
  
  bool questCompleted(String const& quest) const {
    return completedQuests.contains(quest);
  }
};

// Player inventory reference (actual inventory is complex, this is a marker)
struct PlayerInventoryRefComponent {
  bool inventoryDirty = false;
  size_t currencyCount = 0;
  
  void markDirty() {
    inventoryDirty = true;
  }
};

// Player blueprints/recipes
struct PlayerBlueprintsComponent {
  StringSet knownBlueprints = {};
  StringSet newBlueprints = {};
  
  bool knowsBlueprint(String const& blueprint) const {
    return knownBlueprints.contains(blueprint);
  }
  
  void learnBlueprint(String const& blueprint) {
    if (!knownBlueprints.contains(blueprint)) {
      knownBlueprints.add(blueprint);
      newBlueprints.add(blueprint);
    }
  }
};

// Player statistics/log
struct PlayerStatsComponent {
  JsonObject stats = {};
  float playTime = 0.0f;
  int deaths = 0;
  int kills = 0;
  
  void incrementStat(String const& stat, float amount = 1.0f) {
    if (stats.contains(stat)) {
      stats[stat] = stats[stat].toFloat() + amount;
    } else {
      stats[stat] = amount;
    }
  }
  
  float getStat(String const& stat) const {
    auto it = stats.find(stat);
    return it != stats.end() ? it->second.toFloat() : 0.0f;
  }
};

// Player teleportation state
struct PlayerWarpComponent {
  bool teleporting = false;
  bool teleportingOut = false;
  String teleportAnimation = "default";
  float teleportTimer = 0.0f;
  Maybe<Json> pendingWarp = {};
  
  void startTeleport(bool out, String const& anim = "default") {
    teleporting = true;
    teleportingOut = out;
    teleportAnimation = anim;
    teleportTimer = 0.0f;
  }
  
  void finishTeleport() {
    teleporting = false;
    teleportingOut = false;
    teleportTimer = 0.0f;
  }
};

// Player camera control
struct PlayerCameraComponent {
  Maybe<Entity> focusEntity = {};
  Vec2F cameraOffset = {};
  float zoomLevel = 1.0f;
  bool cameraLocked = false;
};

} // namespace ECS
} // namespace Star

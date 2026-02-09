#pragma once

#include "StarConfig.hpp"
#include "StarEntitySplash.hpp"
#include "StarException.hpp"
#include "StarHumanoid.hpp"
#include "StarItemDescriptor.hpp"

namespace Star {

class Player;
class Rebuilder;

using PlayerException = ExceptionDerived<"PlayerException">;

// The player has a large number of shared config states, so this is a shared
// config object to hold them.
struct PlayerConfig {
  PlayerConfig(JsonObject const& cfg);

  HumanoidIdentity defaultIdentity;
  Humanoid::HumanoidTiming humanoidTiming;

  List<ItemDescriptor> defaultItems;
  List<ItemDescriptor> defaultBlueprints;

  RectF metaBoundBox;

  Json movementParameters;
  Json zeroGMovementParameters;
  Json statusControllerSettings;

  float footstepTiming;
  Vec2F footstepSensor;

  Vec2F underwaterSensor;
  float underwaterMinWaterLevel;

  String effectsAnimator;

  float teleportInTime;
  float teleportOutTime;

  float deployInTime;
  float deployOutTime;

  String bodyMaterialKind;

  EntitySplashConfig splashConfig;

  Json companionsConfig;

  Json deploymentConfig;

  StringMap<String> genericScriptContexts;
};

class PlayerFactory {
public:
  PlayerFactory();

  [[nodiscard]] auto create() const -> Ptr<Player>;
  [[nodiscard]] auto diskLoadPlayer(Json const& diskStore) const -> Ptr<Player>;
  [[nodiscard]] auto netLoadPlayer(ByteArray const& netStore, NetCompatibilityRules rules = {}) const -> Ptr<Player>;

private:
  Ptr<PlayerConfig> m_config;

  Ptr<Rebuilder> m_rebuilder;
};

}// namespace Star

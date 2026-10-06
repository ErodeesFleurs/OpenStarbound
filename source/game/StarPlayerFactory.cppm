module;

#include "StarItemDescriptor.hpp"
#include "StarHumanoid.hpp"
#include "StarJson.hpp"
#include "StarParticle.hpp"

import star.entity_splash;

namespace Star {

STAR_CLASS(Rebuilder);
STAR_CLASS(Player);
STAR_STRUCT(PlayerConfig);
STAR_CLASS(PlayerFactory);

struct PlayerExceptionTag {
  static constexpr char const* name() { return "PlayerException"; }
};
using PlayerException = StarError<PlayerExceptionTag, StarException>;

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

  PlayerPtr create() const;
  PlayerPtr diskLoadPlayer(Json const& diskStore) const;
  PlayerPtr netLoadPlayer(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

private:
  PlayerConfigPtr m_config;

  RebuilderPtr m_rebuilder;
};

}

export module star.player_factory;

export namespace Star {
  using ::Star::Rebuilder;
  using ::Star::RebuilderPtr;
  using ::Star::RebuilderConstPtr;
  using ::Star::RebuilderWeakPtr;
  using ::Star::RebuilderConstWeakPtr;
  using ::Star::RebuilderUPtr;
  using ::Star::RebuilderConstUPtr;
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::PlayerConfig;
  using ::Star::PlayerConfigPtr;
  using ::Star::PlayerConfigConstPtr;
  using ::Star::PlayerConfigWeakPtr;
  using ::Star::PlayerConfigConstWeakPtr;
  using ::Star::PlayerConfigUPtr;
  using ::Star::PlayerConfigConstUPtr;
  using ::Star::PlayerFactory;
  using ::Star::PlayerFactoryPtr;
  using ::Star::PlayerFactoryConstPtr;
  using ::Star::PlayerFactoryWeakPtr;
  using ::Star::PlayerFactoryConstWeakPtr;
  using ::Star::PlayerFactoryUPtr;
  using ::Star::PlayerFactoryConstUPtr;
  using ::Star::PlayerExceptionTag;
  using ::Star::PlayerException;
}

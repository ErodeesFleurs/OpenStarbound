module;
import std;
import star.format;
import star.json;
import star.net_compatibility;
import star.byte_array;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.string;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.vector;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.maybe;
import star.color;
import star.rect;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;


import star.item_descriptor;
import star.drawable;
import star.animated_part_set;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.animation;
import star.particle;

import star.entity_splash;

namespace Star {

class Rebuilder;
using RebuilderPtr = std::shared_ptr<Rebuilder>;
using RebuilderConstPtr = std::shared_ptr<const Rebuilder>;
using RebuilderWeakPtr = std::weak_ptr<Rebuilder>;
using RebuilderConstWeakPtr = std::weak_ptr<const Rebuilder>;
using RebuilderUPtr = std::unique_ptr<Rebuilder>;
using RebuilderConstUPtr = std::unique_ptr<const Rebuilder>;
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<const Player>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<const Player>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<const Player>;
struct PlayerConfig;
using PlayerConfigPtr = std::shared_ptr<PlayerConfig>;
using PlayerConfigConstPtr = std::shared_ptr<const PlayerConfig>;
using PlayerConfigWeakPtr = std::weak_ptr<PlayerConfig>;
using PlayerConfigConstWeakPtr = std::weak_ptr<const PlayerConfig>;
using PlayerConfigUPtr = std::unique_ptr<PlayerConfig>;
using PlayerConfigConstUPtr = std::unique_ptr<const PlayerConfig>;
class PlayerFactory;
using PlayerFactoryPtr = std::shared_ptr<PlayerFactory>;
using PlayerFactoryConstPtr = std::shared_ptr<const PlayerFactory>;
using PlayerFactoryWeakPtr = std::weak_ptr<PlayerFactory>;
using PlayerFactoryConstWeakPtr = std::weak_ptr<const PlayerFactory>;
using PlayerFactoryUPtr = std::unique_ptr<PlayerFactory>;
using PlayerFactoryConstUPtr = std::unique_ptr<const PlayerFactory>;

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

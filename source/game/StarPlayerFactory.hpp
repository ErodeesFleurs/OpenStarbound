#pragma once

#include "StarAssets.hpp"
#include "StarItemDescriptor.hpp"
#include "StarHumanoid.hpp"
#include "StarEntitySplash.hpp"

namespace Star {

class Rebuilder;
using RebuilderPtr = SharedPtr<Rebuilder>;
class Player;
using PlayerPtr = SharedPtr<Player>;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class QuestTemplateDatabase;
using QuestTemplateDatabaseConstPtr = SharedPtr<QuestTemplateDatabase const>;
class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;
struct PlayerConfig;
using PlayerConfigPtr = SharedPtr<PlayerConfig>;

struct PlayerExceptionTag { static constexpr char const* typeName = "PlayerException"; };
using PlayerException = TypedException<StarException, PlayerExceptionTag>;

// The player has a large number of shared config states, so this is a shared
// config object to hold them.
struct PlayerConfig {
  PlayerConfig(JsonObject const& cfg, IAssetsConstPtr assets = {});

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
  PlayerFactory(AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase);

  PlayerPtr create() const;
  PlayerPtr diskLoadPlayer(Json const& diskStore) const;
  PlayerPtr netLoadPlayer(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

private:
  AssetsConstPtr m_assets;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  QuestTemplateDatabaseConstPtr m_questTemplateDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;
  PlayerConfigPtr m_config;

  RebuilderPtr m_rebuilder;
};

}

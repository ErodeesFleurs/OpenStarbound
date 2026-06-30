#pragma once

#include "StarAssets.hpp"
#include "StarEntitySplash.hpp"
#include "StarHumanoid.hpp"
#include "StarItemDescriptor.hpp"
#include "StarLuaRoot.hpp"

#include "StarConfiguration.hpp"

namespace Star {

class Rebuilder;
using RebuilderPtr = SharedPtr<Rebuilder>;
class Player;
using PlayerPtr = SharedPtr<Player>;
class MaterialDatabase;
using MaterialDatabaseConstPtr = SharedPtr<MaterialDatabase const>;
class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class QuestTemplateDatabase;
using QuestTemplateDatabaseConstPtr = SharedPtr<QuestTemplateDatabase const>;
class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;
class CodexDatabase;
using CodexDatabaseConstPtr = SharedPtr<CodexDatabase const>;
class DanceDatabase;
using DanceDatabaseConstPtr = SharedPtr<DanceDatabase const>;
class EmoteProcessor;
using EmoteProcessorConstPtr = SharedPtr<EmoteProcessor const>;
class RadioMessageDatabase;
using RadioMessageDatabaseConstPtr = SharedPtr<RadioMessageDatabase const>;
class AiDatabase;
using AiDatabaseConstPtr = SharedPtr<AiDatabase const>;
class CollectionDatabase;
using CollectionDatabaseConstPtr = SharedPtr<CollectionDatabase const>;
class SpeciesDatabase;
using SpeciesDatabaseConstPtr = SharedPtr<SpeciesDatabase const>;
class EntityFactory;
using EntityFactoryConstPtr = SharedPtr<EntityFactory const>;
class LiquidsDatabase;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;
class TechDatabase;
using TechDatabaseConstPtr = SharedPtr<TechDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;
struct PlayerConfig;
using PlayerConfigPtr = SharedPtr<PlayerConfig>;

struct PlayerExceptionTag {
  static constexpr char const* typeName = "PlayerException";
};
using PlayerException = TypedException<StarException, PlayerExceptionTag>;

// The player has a large number of shared config states, so this is a shared
// config object to hold them.
struct PlayerConfig {
  PlayerConfig(JsonObject const& cfg, AssetsConstPtr assets = {});

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
  PlayerFactory(AssetsConstPtr assets, ConfigurationPtr configuration, MaterialDatabaseConstPtr materialDatabase, ItemDatabaseConstPtr itemDatabase, ObjectDatabaseConstPtr objectDatabase, QuestTemplateDatabaseConstPtr questTemplateDatabase, VersioningDatabaseConstPtr versioningDatabase, CodexDatabaseConstPtr codexDatabase, DanceDatabaseConstPtr danceDatabase, EmoteProcessorConstPtr emoteProcessor, RadioMessageDatabaseConstPtr radioMessageDatabase, AiDatabaseConstPtr aiDatabase, CollectionDatabaseConstPtr collectionDatabase, SpeciesDatabaseConstPtr speciesDatabase, function<EntityFactoryConstPtr()> entityFactory, LiquidsDatabaseConstPtr liquidsDatabase, TechDatabaseConstPtr techDatabase, StatusEffectDatabaseConstPtr statusEffectDatabase, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, LuaRootServices luaRootServices);

  [[nodiscard]] PlayerPtr create() const;
  [[nodiscard]] PlayerPtr diskLoadPlayer(Json const& diskStore) const;
  [[nodiscard]] PlayerPtr netLoadPlayer(ByteArray const& netStore, NetCompatibilityRules rules = {}) const;

private:
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  MaterialDatabaseConstPtr m_materialDatabase;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  QuestTemplateDatabaseConstPtr m_questTemplateDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;
  CodexDatabaseConstPtr m_codexDatabase;
  DanceDatabaseConstPtr m_danceDatabase;
  EmoteProcessorConstPtr m_emoteProcessor;
  RadioMessageDatabaseConstPtr m_radioMessageDatabase;
  AiDatabaseConstPtr m_aiDatabase;
  CollectionDatabaseConstPtr m_collectionDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  function<EntityFactoryConstPtr()> m_entityFactory;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  TechDatabaseConstPtr m_techDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  PlayerConfigPtr m_config;

  RebuilderPtr m_rebuilder;
};

}// namespace Star

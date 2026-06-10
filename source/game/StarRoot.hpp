#pragma once

#include "StarRootBase.hpp"
#include "StarJson.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarConfiguration.hpp"

namespace Star {

class ItemDatabase;
using ItemDatabasePtr = SharedPtr<ItemDatabase>;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;

class MaterialDatabase;
using MaterialDatabasePtr = SharedPtr<MaterialDatabase>;
using MaterialDatabaseConstPtr = SharedPtr<MaterialDatabase const>;

class ObjectDatabase;
using ObjectDatabasePtr = SharedPtr<ObjectDatabase>;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;

class PlantDatabase;
using PlantDatabasePtr = SharedPtr<PlantDatabase>;
using PlantDatabaseConstPtr = SharedPtr<PlantDatabase const>;

class ProjectileDatabase;
using ProjectileDatabasePtr = SharedPtr<ProjectileDatabase>;
using ProjectileDatabaseConstPtr = SharedPtr<ProjectileDatabase const>;

class MonsterDatabase;
using MonsterDatabasePtr = SharedPtr<MonsterDatabase>;
using MonsterDatabaseConstPtr = SharedPtr<MonsterDatabase const>;

class NpcDatabase;
using NpcDatabasePtr = SharedPtr<NpcDatabase>;
using NpcDatabaseConstPtr = SharedPtr<NpcDatabase const>;

class StagehandDatabase;
using StagehandDatabasePtr = SharedPtr<StagehandDatabase>;
using StagehandDatabaseConstPtr = SharedPtr<StagehandDatabase const>;

class VehicleDatabase;
using VehicleDatabasePtr = SharedPtr<VehicleDatabase>;
using VehicleDatabaseConstPtr = SharedPtr<VehicleDatabase const>;

class PlayerFactory;
using PlayerFactoryPtr = SharedPtr<PlayerFactory>;
using PlayerFactoryConstPtr = SharedPtr<PlayerFactory const>;

class EntityFactory;
using EntityFactoryPtr = SharedPtr<EntityFactory>;
using EntityFactoryConstPtr = SharedPtr<EntityFactory const>;

class TerrainDatabase;
using TerrainDatabasePtr = SharedPtr<TerrainDatabase>;
using TerrainDatabaseConstPtr = SharedPtr<TerrainDatabase const>;

class BiomeDatabase;
using BiomeDatabasePtr = SharedPtr<BiomeDatabase>;
using BiomeDatabaseConstPtr = SharedPtr<BiomeDatabase const>;

class LiquidsDatabase;
using LiquidsDatabasePtr = SharedPtr<LiquidsDatabase>;
using LiquidsDatabaseConstPtr = SharedPtr<LiquidsDatabase const>;

class StatusEffectDatabase;
using StatusEffectDatabasePtr = SharedPtr<StatusEffectDatabase>;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;

class DamageDatabase;
using DamageDatabasePtr = SharedPtr<DamageDatabase>;
using DamageDatabaseConstPtr = SharedPtr<DamageDatabase const>;

class ParticleDatabase;
using ParticleDatabasePtr = SharedPtr<ParticleDatabase>;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;

class EffectSourceDatabase;
using EffectSourceDatabasePtr = SharedPtr<EffectSourceDatabase>;
using EffectSourceDatabaseConstPtr = SharedPtr<EffectSourceDatabase const>;

class FunctionDatabase;
using FunctionDatabasePtr = SharedPtr<FunctionDatabase>;
using FunctionDatabaseConstPtr = SharedPtr<FunctionDatabase const>;

class TreasureDatabase;
using TreasureDatabasePtr = SharedPtr<TreasureDatabase>;
using TreasureDatabaseConstPtr = SharedPtr<TreasureDatabase const>;

class DungeonDefinitions;
using DungeonDefinitionsPtr = SharedPtr<DungeonDefinitions>;
using DungeonDefinitionsConstPtr = SharedPtr<DungeonDefinitions const>;

class TilesetDatabase;
using TilesetDatabasePtr = SharedPtr<TilesetDatabase>;
using TilesetDatabaseConstPtr = SharedPtr<TilesetDatabase const>;

class StatisticsDatabase;
using StatisticsDatabasePtr = SharedPtr<StatisticsDatabase>;
using StatisticsDatabaseConstPtr = SharedPtr<StatisticsDatabase const>;

class EmoteProcessor;
using EmoteProcessorPtr = SharedPtr<EmoteProcessor>;
using EmoteProcessorConstPtr = SharedPtr<EmoteProcessor const>;

class SpeciesDatabase;
using SpeciesDatabasePtr = SharedPtr<SpeciesDatabase>;
using SpeciesDatabaseConstPtr = SharedPtr<SpeciesDatabase const>;

class ImageMetadataDatabase;
using ImageMetadataDatabasePtr = SharedPtr<ImageMetadataDatabase>;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

class VersioningDatabase;
using VersioningDatabasePtr = SharedPtr<VersioningDatabase>;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;

class QuestTemplateDatabase;
using QuestTemplateDatabasePtr = SharedPtr<QuestTemplateDatabase>;
using QuestTemplateDatabaseConstPtr = SharedPtr<QuestTemplateDatabase const>;

class AiDatabase;
using AiDatabasePtr = SharedPtr<AiDatabase>;
using AiDatabaseConstPtr = SharedPtr<AiDatabase const>;

class TechDatabase;
using TechDatabasePtr = SharedPtr<TechDatabase>;
using TechDatabaseConstPtr = SharedPtr<TechDatabase const>;

class CodexDatabase;
using CodexDatabasePtr = SharedPtr<CodexDatabase>;
using CodexDatabaseConstPtr = SharedPtr<CodexDatabase const>;

class BehaviorDatabase;
using BehaviorDatabasePtr = SharedPtr<BehaviorDatabase>;
using BehaviorDatabaseConstPtr = SharedPtr<BehaviorDatabase const>;

class TenantDatabase;
using TenantDatabasePtr = SharedPtr<TenantDatabase>;
using TenantDatabaseConstPtr = SharedPtr<TenantDatabase const>;

class PatternedNameGenerator;
using PatternedNameGeneratorPtr = SharedPtr<PatternedNameGenerator>;
using PatternedNameGeneratorConstPtr = SharedPtr<PatternedNameGenerator const>;

class DanceDatabase;
using DanceDatabasePtr = SharedPtr<DanceDatabase>;
using DanceDatabaseConstPtr = SharedPtr<DanceDatabase const>;

class SpawnTypeDatabase;
using SpawnTypeDatabasePtr = SharedPtr<SpawnTypeDatabase>;
using SpawnTypeDatabaseConstPtr = SharedPtr<SpawnTypeDatabase const>;

class RadioMessageDatabase;
using RadioMessageDatabasePtr = SharedPtr<RadioMessageDatabase>;
using RadioMessageDatabaseConstPtr = SharedPtr<RadioMessageDatabase const>;

class CollectionDatabase;
using CollectionDatabasePtr = SharedPtr<CollectionDatabase>;
using CollectionDatabaseConstPtr = SharedPtr<CollectionDatabase const>;

class Root;

// Singleton Root object for starbound providing access to the unique
// Configuration class, as well as the assets, root factories, and databases.
// Root, and all members of Root, should be thread safe.  Root initialization
// should be completed before any code dependent on Root is started in any
// thread, and all Root dependent code in any thread should be finished before
// letting Root destruct.
class Root final : public RootBase {
public:
  struct Settings {
    Assets::Settings assetsSettings;

    // Asset sources are scanned for in the given directories, in order.
    StringList assetDirectories;

    // Just raw asset source paths.
    StringList assetSources;

    Json defaultConfiguration;

    // Top-level storage directory under which all game data is saved
    String storageDirectory;

    // Directory to store logs - if not set, uses storage directory and keeps old logs in seperate folder
    Maybe<String> logDirectory;

    // Name of the log file that should be written, if any, relative to the
    // log directory
    Maybe<String> logFile;

    // Number of rotated log file backups
    unsigned logFileBackups;

    // The minimum log level to write to any log sink
    LogLevel logLevel;

    // If true, doesn't write any logging to stdout, only to the log file if
    // given.
    bool quiet;

    // If true, loads UGC from platform services if available. True by default.
    bool includeUGC;

    // If given, will write changed configuration to the given file within the
    // storage directory.
    Maybe<String> runtimeConfigFile;
  };

  // Get pointer to the singleton root instance, if it exists.  Otherwise,
  // returns nullptr.
  static Root* singletonPtr();

  // Gets reference to root singleton, throws RootException if root is not
  // initialized.
  static Root& singleton();

  // Initializes the starbound root object and does the initial load.  All of
  // the Root members will be just in time loaded as they are accessed, unless
  // fullyLoad is called beforehand.
  Root(Settings settings);

  Root(Root const&) = delete;
  Root& operator=(Root const&) = delete;

  ~Root();

  // Clears existing Root members, allowing them to be loaded fresh from disk.
  void reload();

  // Reloads with the given mod sources applied on top of the base mod source
  // specified in the settings.  Mods in the base mod source will override mods
  // in the given mod sources
  void loadMods(StringList modDirectories, bool _reload = true);

  // Ensures all Root members are loaded without waiting for them to be auto
  // loaded.
  void fullyLoad();

  // Add a listener that will be called on Root reload.  Automatically managed,
  // if the listener is destroyed then it will automatically be removed from
  // the internal listener list.
  void registerReloadListener(ListenerWeakPtr reloadListener);

  void hotReload();

  // Translates the given path to be relative to the configured storage
  // location.
  String toStoragePath(String const& path) const;

  // All of the Root member accessors are safe to call at any time after Root
  // initialization, if they are not loaded they will load before returning.

  AssetsConstPtr assets() override;
  ConfigurationPtr configuration() override;

  ObjectDatabaseConstPtr objectDatabase();
  PlantDatabaseConstPtr plantDatabase();
  ProjectileDatabaseConstPtr projectileDatabase();
  MonsterDatabaseConstPtr monsterDatabase();
  NpcDatabaseConstPtr npcDatabase();
  StagehandDatabaseConstPtr stagehandDatabase();
  VehicleDatabaseConstPtr vehicleDatabase();
  PlayerFactoryConstPtr playerFactory();

  EntityFactoryConstPtr entityFactory();

  PatternedNameGeneratorConstPtr nameGenerator();

  ItemDatabaseConstPtr itemDatabase();
  MaterialDatabaseConstPtr materialDatabase();
  TerrainDatabaseConstPtr terrainDatabase();
  BiomeDatabaseConstPtr biomeDatabase();
  LiquidsDatabaseConstPtr liquidsDatabase();
  StatusEffectDatabaseConstPtr statusEffectDatabase();
  DamageDatabaseConstPtr damageDatabase();
  ParticleDatabaseConstPtr particleDatabase();
  EffectSourceDatabaseConstPtr effectSourceDatabase();
  FunctionDatabaseConstPtr functionDatabase();
  TreasureDatabaseConstPtr treasureDatabase();
  DungeonDefinitionsConstPtr dungeonDefinitions();
  TilesetDatabaseConstPtr tilesetDatabase();
  StatisticsDatabaseConstPtr statisticsDatabase();
  EmoteProcessorConstPtr emoteProcessor();
  SpeciesDatabaseConstPtr speciesDatabase();
  ImageMetadataDatabaseConstPtr imageMetadataDatabase();
  VersioningDatabaseConstPtr versioningDatabase();
  QuestTemplateDatabaseConstPtr questTemplateDatabase();
  AiDatabaseConstPtr aiDatabase();
  TechDatabaseConstPtr techDatabase();
  CodexDatabaseConstPtr codexDatabase();
  BehaviorDatabaseConstPtr behaviorDatabase();
  TenantDatabaseConstPtr tenantDatabase();
  DanceDatabaseConstPtr danceDatabase();
  SpawnTypeDatabaseConstPtr spawnTypeDatabase();
  RadioMessageDatabaseConstPtr radioMessageDatabase();
  CollectionDatabaseConstPtr collectionDatabase();

  Settings& settings();

private:
  static StringList scanForAssetSources(StringList const& directories, StringList const& manual = {});
  template <typename T, typename... Params>
  static shared_ptr<T> loadMember(shared_ptr<T>& ptr, Mutex& mutex, char const* name, Params&&... params);
  template <typename T>
  static shared_ptr<T> loadMemberFunction(shared_ptr<T>& ptr, Mutex& mutex, char const* name, function<shared_ptr<T>()> loadFunction);

  // m_configurationMutex must be held when calling
  void writeConfig();

  Settings m_settings;

  Mutex m_modsMutex;
  StringList m_modDirectories;

  ListenerGroup m_reloadListeners;

  Json m_lastRuntimeConfig;
  Maybe<String> m_runtimeConfigFile;

  ThreadFunction<void> m_maintenanceThread;
  Mutex m_maintenanceStopMutex;
  ConditionVariable m_maintenanceStopCondition;
  bool m_stopMaintenanceThread;

  AssetsPtr m_assets;
  Mutex m_assetsMutex;

  ConfigurationPtr m_configuration;
  Mutex m_configurationMutex;

  ObjectDatabasePtr m_objectDatabase;
  Mutex m_objectDatabaseMutex;

  PlantDatabasePtr m_plantDatabase;
  Mutex m_plantDatabaseMutex;

  ProjectileDatabasePtr m_projectileDatabase;
  Mutex m_projectileDatabaseMutex;

  MonsterDatabasePtr m_monsterDatabase;
  Mutex m_monsterDatabaseMutex;

  NpcDatabasePtr m_npcDatabase;
  Mutex m_npcDatabaseMutex;

  StagehandDatabasePtr m_stagehandDatabase;
  Mutex m_stagehandDatabaseMutex;

  VehicleDatabasePtr m_vehicleDatabase;
  Mutex m_vehicleDatabaseMutex;

  PlayerFactoryPtr m_playerFactory;
  Mutex m_playerFactoryMutex;

  EntityFactoryPtr m_entityFactory;
  Mutex m_entityFactoryMutex;

  PatternedNameGeneratorPtr m_nameGenerator;
  Mutex m_nameGeneratorMutex;

  ItemDatabasePtr m_itemDatabase;
  Mutex m_itemDatabaseMutex;

  MaterialDatabasePtr m_materialDatabase;
  Mutex m_materialDatabaseMutex;

  TerrainDatabasePtr m_terrainDatabase;
  Mutex m_terrainDatabaseMutex;

  BiomeDatabasePtr m_biomeDatabase;
  Mutex m_biomeDatabaseMutex;

  LiquidsDatabasePtr m_liquidsDatabase;
  Mutex m_liquidsDatabaseMutex;

  StatusEffectDatabasePtr m_statusEffectDatabase;
  Mutex m_statusEffectDatabaseMutex;

  DamageDatabasePtr m_damageDatabase;
  Mutex m_damageDatabaseMutex;

  ParticleDatabasePtr m_particleDatabase;
  Mutex m_particleDatabaseMutex;

  EffectSourceDatabasePtr m_effectSourceDatabase;
  Mutex m_effectSourceDatabaseMutex;

  FunctionDatabasePtr m_functionDatabase;
  Mutex m_functionDatabaseMutex;

  TreasureDatabasePtr m_treasureDatabase;
  Mutex m_treasureDatabaseMutex;

  DungeonDefinitionsPtr m_dungeonDefinitions;
  Mutex m_dungeonDefinitionsMutex;

  TilesetDatabasePtr m_tilesetDatabase;
  Mutex m_tilesetDatabaseMutex;

  StatisticsDatabasePtr m_statisticsDatabase;
  Mutex m_statisticsDatabaseMutex;

  EmoteProcessorPtr m_emoteProcessor;
  Mutex m_emoteProcessorMutex;

  SpeciesDatabasePtr m_speciesDatabase;
  Mutex m_speciesDatabaseMutex;

  ImageMetadataDatabasePtr m_imageMetadataDatabase;
  Mutex m_imageMetadataDatabaseMutex;

  VersioningDatabasePtr m_versioningDatabase;
  Mutex m_versioningDatabaseMutex;

  QuestTemplateDatabasePtr m_questTemplateDatabase;
  Mutex m_questTemplateDatabaseMutex;

  AiDatabasePtr m_aiDatabase;
  Mutex m_aiDatabaseMutex;

  TechDatabasePtr m_techDatabase;
  Mutex m_techDatabaseMutex;

  CodexDatabasePtr m_codexDatabase;
  Mutex m_codexDatabaseMutex;

  BehaviorDatabasePtr m_behaviorDatabase;
  Mutex m_behaviorDatabaseMutex;

  TenantDatabasePtr m_tenantDatabase;
  Mutex m_tenantDatabaseMutex;

  DanceDatabasePtr m_danceDatabase;
  Mutex m_danceDatabaseMutex;

  SpawnTypeDatabasePtr m_spawnTypeDatabase;
  Mutex m_spawnTypeDatabaseMutex;

  RadioMessageDatabasePtr m_radioMessageDatabase;
  Mutex m_radioMessageDatabaseMutex;

  CollectionDatabasePtr m_collectionDatabase;
  Mutex m_collectionDatabaseMutex;
};

}

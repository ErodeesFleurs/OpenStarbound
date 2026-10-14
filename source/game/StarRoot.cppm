module;
import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.maybe;
import star.version;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;

namespace Star {

class ItemDatabase;
using ItemDatabasePtr = std::shared_ptr<ItemDatabase>;
using ItemDatabaseConstPtr = std::shared_ptr<const ItemDatabase>;
using ItemDatabaseWeakPtr = std::weak_ptr<ItemDatabase>;
using ItemDatabaseConstWeakPtr = std::weak_ptr<const ItemDatabase>;
using ItemDatabaseUPtr = std::unique_ptr<ItemDatabase>;
using ItemDatabaseConstUPtr = std::unique_ptr<const ItemDatabase>;
class MaterialDatabase;
using MaterialDatabasePtr = std::shared_ptr<MaterialDatabase>;
using MaterialDatabaseConstPtr = std::shared_ptr<const MaterialDatabase>;
using MaterialDatabaseWeakPtr = std::weak_ptr<MaterialDatabase>;
using MaterialDatabaseConstWeakPtr = std::weak_ptr<const MaterialDatabase>;
using MaterialDatabaseUPtr = std::unique_ptr<MaterialDatabase>;
using MaterialDatabaseConstUPtr = std::unique_ptr<const MaterialDatabase>;
class ObjectDatabase;
using ObjectDatabasePtr = std::shared_ptr<ObjectDatabase>;
using ObjectDatabaseConstPtr = std::shared_ptr<const ObjectDatabase>;
using ObjectDatabaseWeakPtr = std::weak_ptr<ObjectDatabase>;
using ObjectDatabaseConstWeakPtr = std::weak_ptr<const ObjectDatabase>;
using ObjectDatabaseUPtr = std::unique_ptr<ObjectDatabase>;
using ObjectDatabaseConstUPtr = std::unique_ptr<const ObjectDatabase>;
class PlantDatabase;
using PlantDatabasePtr = std::shared_ptr<PlantDatabase>;
using PlantDatabaseConstPtr = std::shared_ptr<const PlantDatabase>;
using PlantDatabaseWeakPtr = std::weak_ptr<PlantDatabase>;
using PlantDatabaseConstWeakPtr = std::weak_ptr<const PlantDatabase>;
using PlantDatabaseUPtr = std::unique_ptr<PlantDatabase>;
using PlantDatabaseConstUPtr = std::unique_ptr<const PlantDatabase>;
class ProjectileDatabase;
using ProjectileDatabasePtr = std::shared_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstPtr = std::shared_ptr<const ProjectileDatabase>;
using ProjectileDatabaseWeakPtr = std::weak_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstWeakPtr = std::weak_ptr<const ProjectileDatabase>;
using ProjectileDatabaseUPtr = std::unique_ptr<ProjectileDatabase>;
using ProjectileDatabaseConstUPtr = std::unique_ptr<const ProjectileDatabase>;
class MonsterDatabase;
using MonsterDatabasePtr = std::shared_ptr<MonsterDatabase>;
using MonsterDatabaseConstPtr = std::shared_ptr<const MonsterDatabase>;
using MonsterDatabaseWeakPtr = std::weak_ptr<MonsterDatabase>;
using MonsterDatabaseConstWeakPtr = std::weak_ptr<const MonsterDatabase>;
using MonsterDatabaseUPtr = std::unique_ptr<MonsterDatabase>;
using MonsterDatabaseConstUPtr = std::unique_ptr<const MonsterDatabase>;
class NpcDatabase;
using NpcDatabasePtr = std::shared_ptr<NpcDatabase>;
using NpcDatabaseConstPtr = std::shared_ptr<const NpcDatabase>;
using NpcDatabaseWeakPtr = std::weak_ptr<NpcDatabase>;
using NpcDatabaseConstWeakPtr = std::weak_ptr<const NpcDatabase>;
using NpcDatabaseUPtr = std::unique_ptr<NpcDatabase>;
using NpcDatabaseConstUPtr = std::unique_ptr<const NpcDatabase>;
class StagehandDatabase;
using StagehandDatabasePtr = std::shared_ptr<StagehandDatabase>;
using StagehandDatabaseConstPtr = std::shared_ptr<const StagehandDatabase>;
using StagehandDatabaseWeakPtr = std::weak_ptr<StagehandDatabase>;
using StagehandDatabaseConstWeakPtr = std::weak_ptr<const StagehandDatabase>;
using StagehandDatabaseUPtr = std::unique_ptr<StagehandDatabase>;
using StagehandDatabaseConstUPtr = std::unique_ptr<const StagehandDatabase>;
class VehicleDatabase;
using VehicleDatabasePtr = std::shared_ptr<VehicleDatabase>;
using VehicleDatabaseConstPtr = std::shared_ptr<const VehicleDatabase>;
using VehicleDatabaseWeakPtr = std::weak_ptr<VehicleDatabase>;
using VehicleDatabaseConstWeakPtr = std::weak_ptr<const VehicleDatabase>;
using VehicleDatabaseUPtr = std::unique_ptr<VehicleDatabase>;
using VehicleDatabaseConstUPtr = std::unique_ptr<const VehicleDatabase>;
class PlayerFactory;
using PlayerFactoryPtr = std::shared_ptr<PlayerFactory>;
using PlayerFactoryConstPtr = std::shared_ptr<const PlayerFactory>;
using PlayerFactoryWeakPtr = std::weak_ptr<PlayerFactory>;
using PlayerFactoryConstWeakPtr = std::weak_ptr<const PlayerFactory>;
using PlayerFactoryUPtr = std::unique_ptr<PlayerFactory>;
using PlayerFactoryConstUPtr = std::unique_ptr<const PlayerFactory>;
class EntityFactory;
using EntityFactoryPtr = std::shared_ptr<EntityFactory>;
using EntityFactoryConstPtr = std::shared_ptr<const EntityFactory>;
using EntityFactoryWeakPtr = std::weak_ptr<EntityFactory>;
using EntityFactoryConstWeakPtr = std::weak_ptr<const EntityFactory>;
using EntityFactoryUPtr = std::unique_ptr<EntityFactory>;
using EntityFactoryConstUPtr = std::unique_ptr<const EntityFactory>;
class TerrainDatabase;
using TerrainDatabasePtr = std::shared_ptr<TerrainDatabase>;
using TerrainDatabaseConstPtr = std::shared_ptr<const TerrainDatabase>;
using TerrainDatabaseWeakPtr = std::weak_ptr<TerrainDatabase>;
using TerrainDatabaseConstWeakPtr = std::weak_ptr<const TerrainDatabase>;
using TerrainDatabaseUPtr = std::unique_ptr<TerrainDatabase>;
using TerrainDatabaseConstUPtr = std::unique_ptr<const TerrainDatabase>;
class BiomeDatabase;
using BiomeDatabasePtr = std::shared_ptr<BiomeDatabase>;
using BiomeDatabaseConstPtr = std::shared_ptr<const BiomeDatabase>;
using BiomeDatabaseWeakPtr = std::weak_ptr<BiomeDatabase>;
using BiomeDatabaseConstWeakPtr = std::weak_ptr<const BiomeDatabase>;
using BiomeDatabaseUPtr = std::unique_ptr<BiomeDatabase>;
using BiomeDatabaseConstUPtr = std::unique_ptr<const BiomeDatabase>;
class LiquidsDatabase;
using LiquidsDatabasePtr = std::shared_ptr<LiquidsDatabase>;
using LiquidsDatabaseConstPtr = std::shared_ptr<const LiquidsDatabase>;
using LiquidsDatabaseWeakPtr = std::weak_ptr<LiquidsDatabase>;
using LiquidsDatabaseConstWeakPtr = std::weak_ptr<const LiquidsDatabase>;
using LiquidsDatabaseUPtr = std::unique_ptr<LiquidsDatabase>;
using LiquidsDatabaseConstUPtr = std::unique_ptr<const LiquidsDatabase>;
class StatusEffectDatabase;
using StatusEffectDatabasePtr = std::shared_ptr<StatusEffectDatabase>;
using StatusEffectDatabaseConstPtr = std::shared_ptr<const StatusEffectDatabase>;
using StatusEffectDatabaseWeakPtr = std::weak_ptr<StatusEffectDatabase>;
using StatusEffectDatabaseConstWeakPtr = std::weak_ptr<const StatusEffectDatabase>;
using StatusEffectDatabaseUPtr = std::unique_ptr<StatusEffectDatabase>;
using StatusEffectDatabaseConstUPtr = std::unique_ptr<const StatusEffectDatabase>;
class DamageDatabase;
using DamageDatabasePtr = std::shared_ptr<DamageDatabase>;
using DamageDatabaseConstPtr = std::shared_ptr<const DamageDatabase>;
using DamageDatabaseWeakPtr = std::weak_ptr<DamageDatabase>;
using DamageDatabaseConstWeakPtr = std::weak_ptr<const DamageDatabase>;
using DamageDatabaseUPtr = std::unique_ptr<DamageDatabase>;
using DamageDatabaseConstUPtr = std::unique_ptr<const DamageDatabase>;
class ParticleDatabase;
using ParticleDatabasePtr = std::shared_ptr<ParticleDatabase>;
using ParticleDatabaseConstPtr = std::shared_ptr<const ParticleDatabase>;
using ParticleDatabaseWeakPtr = std::weak_ptr<ParticleDatabase>;
using ParticleDatabaseConstWeakPtr = std::weak_ptr<const ParticleDatabase>;
using ParticleDatabaseUPtr = std::unique_ptr<ParticleDatabase>;
using ParticleDatabaseConstUPtr = std::unique_ptr<const ParticleDatabase>;
class EffectSourceDatabase;
using EffectSourceDatabasePtr = std::shared_ptr<EffectSourceDatabase>;
using EffectSourceDatabaseConstPtr = std::shared_ptr<const EffectSourceDatabase>;
using EffectSourceDatabaseWeakPtr = std::weak_ptr<EffectSourceDatabase>;
using EffectSourceDatabaseConstWeakPtr = std::weak_ptr<const EffectSourceDatabase>;
using EffectSourceDatabaseUPtr = std::unique_ptr<EffectSourceDatabase>;
using EffectSourceDatabaseConstUPtr = std::unique_ptr<const EffectSourceDatabase>;
class FunctionDatabase;
using FunctionDatabasePtr = std::shared_ptr<FunctionDatabase>;
using FunctionDatabaseConstPtr = std::shared_ptr<const FunctionDatabase>;
using FunctionDatabaseWeakPtr = std::weak_ptr<FunctionDatabase>;
using FunctionDatabaseConstWeakPtr = std::weak_ptr<const FunctionDatabase>;
using FunctionDatabaseUPtr = std::unique_ptr<FunctionDatabase>;
using FunctionDatabaseConstUPtr = std::unique_ptr<const FunctionDatabase>;
class TreasureDatabase;
using TreasureDatabasePtr = std::shared_ptr<TreasureDatabase>;
using TreasureDatabaseConstPtr = std::shared_ptr<const TreasureDatabase>;
using TreasureDatabaseWeakPtr = std::weak_ptr<TreasureDatabase>;
using TreasureDatabaseConstWeakPtr = std::weak_ptr<const TreasureDatabase>;
using TreasureDatabaseUPtr = std::unique_ptr<TreasureDatabase>;
using TreasureDatabaseConstUPtr = std::unique_ptr<const TreasureDatabase>;
class DungeonDefinitions;
using DungeonDefinitionsPtr = std::shared_ptr<DungeonDefinitions>;
using DungeonDefinitionsConstPtr = std::shared_ptr<const DungeonDefinitions>;
using DungeonDefinitionsWeakPtr = std::weak_ptr<DungeonDefinitions>;
using DungeonDefinitionsConstWeakPtr = std::weak_ptr<const DungeonDefinitions>;
using DungeonDefinitionsUPtr = std::unique_ptr<DungeonDefinitions>;
using DungeonDefinitionsConstUPtr = std::unique_ptr<const DungeonDefinitions>;
class TilesetDatabase;
using TilesetDatabasePtr = std::shared_ptr<TilesetDatabase>;
using TilesetDatabaseConstPtr = std::shared_ptr<const TilesetDatabase>;
using TilesetDatabaseWeakPtr = std::weak_ptr<TilesetDatabase>;
using TilesetDatabaseConstWeakPtr = std::weak_ptr<const TilesetDatabase>;
using TilesetDatabaseUPtr = std::unique_ptr<TilesetDatabase>;
using TilesetDatabaseConstUPtr = std::unique_ptr<const TilesetDatabase>;
class StatisticsDatabase;
using StatisticsDatabasePtr = std::shared_ptr<StatisticsDatabase>;
using StatisticsDatabaseConstPtr = std::shared_ptr<const StatisticsDatabase>;
using StatisticsDatabaseWeakPtr = std::weak_ptr<StatisticsDatabase>;
using StatisticsDatabaseConstWeakPtr = std::weak_ptr<const StatisticsDatabase>;
using StatisticsDatabaseUPtr = std::unique_ptr<StatisticsDatabase>;
using StatisticsDatabaseConstUPtr = std::unique_ptr<const StatisticsDatabase>;
class EmoteProcessor;
using EmoteProcessorPtr = std::shared_ptr<EmoteProcessor>;
using EmoteProcessorConstPtr = std::shared_ptr<const EmoteProcessor>;
using EmoteProcessorWeakPtr = std::weak_ptr<EmoteProcessor>;
using EmoteProcessorConstWeakPtr = std::weak_ptr<const EmoteProcessor>;
using EmoteProcessorUPtr = std::unique_ptr<EmoteProcessor>;
using EmoteProcessorConstUPtr = std::unique_ptr<const EmoteProcessor>;
class SpeciesDatabase;
using SpeciesDatabasePtr = std::shared_ptr<SpeciesDatabase>;
using SpeciesDatabaseConstPtr = std::shared_ptr<const SpeciesDatabase>;
using SpeciesDatabaseWeakPtr = std::weak_ptr<SpeciesDatabase>;
using SpeciesDatabaseConstWeakPtr = std::weak_ptr<const SpeciesDatabase>;
using SpeciesDatabaseUPtr = std::unique_ptr<SpeciesDatabase>;
using SpeciesDatabaseConstUPtr = std::unique_ptr<const SpeciesDatabase>;
class ImageMetadataDatabase;
using ImageMetadataDatabasePtr = std::shared_ptr<ImageMetadataDatabase>;
using ImageMetadataDatabaseConstPtr = std::shared_ptr<const ImageMetadataDatabase>;
using ImageMetadataDatabaseWeakPtr = std::weak_ptr<ImageMetadataDatabase>;
using ImageMetadataDatabaseConstWeakPtr = std::weak_ptr<const ImageMetadataDatabase>;
using ImageMetadataDatabaseUPtr = std::unique_ptr<ImageMetadataDatabase>;
using ImageMetadataDatabaseConstUPtr = std::unique_ptr<const ImageMetadataDatabase>;
class VersioningDatabase;
using VersioningDatabasePtr = std::shared_ptr<VersioningDatabase>;
using VersioningDatabaseConstPtr = std::shared_ptr<const VersioningDatabase>;
using VersioningDatabaseWeakPtr = std::weak_ptr<VersioningDatabase>;
using VersioningDatabaseConstWeakPtr = std::weak_ptr<const VersioningDatabase>;
using VersioningDatabaseUPtr = std::unique_ptr<VersioningDatabase>;
using VersioningDatabaseConstUPtr = std::unique_ptr<const VersioningDatabase>;
class QuestTemplateDatabase;
using QuestTemplateDatabasePtr = std::shared_ptr<QuestTemplateDatabase>;
using QuestTemplateDatabaseConstPtr = std::shared_ptr<const QuestTemplateDatabase>;
using QuestTemplateDatabaseWeakPtr = std::weak_ptr<QuestTemplateDatabase>;
using QuestTemplateDatabaseConstWeakPtr = std::weak_ptr<const QuestTemplateDatabase>;
using QuestTemplateDatabaseUPtr = std::unique_ptr<QuestTemplateDatabase>;
using QuestTemplateDatabaseConstUPtr = std::unique_ptr<const QuestTemplateDatabase>;
class AiDatabase;
using AiDatabasePtr = std::shared_ptr<AiDatabase>;
using AiDatabaseConstPtr = std::shared_ptr<const AiDatabase>;
using AiDatabaseWeakPtr = std::weak_ptr<AiDatabase>;
using AiDatabaseConstWeakPtr = std::weak_ptr<const AiDatabase>;
using AiDatabaseUPtr = std::unique_ptr<AiDatabase>;
using AiDatabaseConstUPtr = std::unique_ptr<const AiDatabase>;
class TechDatabase;
using TechDatabasePtr = std::shared_ptr<TechDatabase>;
using TechDatabaseConstPtr = std::shared_ptr<const TechDatabase>;
using TechDatabaseWeakPtr = std::weak_ptr<TechDatabase>;
using TechDatabaseConstWeakPtr = std::weak_ptr<const TechDatabase>;
using TechDatabaseUPtr = std::unique_ptr<TechDatabase>;
using TechDatabaseConstUPtr = std::unique_ptr<const TechDatabase>;
class CodexDatabase;
using CodexDatabasePtr = std::shared_ptr<CodexDatabase>;
using CodexDatabaseConstPtr = std::shared_ptr<const CodexDatabase>;
using CodexDatabaseWeakPtr = std::weak_ptr<CodexDatabase>;
using CodexDatabaseConstWeakPtr = std::weak_ptr<const CodexDatabase>;
using CodexDatabaseUPtr = std::unique_ptr<CodexDatabase>;
using CodexDatabaseConstUPtr = std::unique_ptr<const CodexDatabase>;
class BehaviorDatabase;
using BehaviorDatabasePtr = std::shared_ptr<BehaviorDatabase>;
using BehaviorDatabaseConstPtr = std::shared_ptr<const BehaviorDatabase>;
using BehaviorDatabaseWeakPtr = std::weak_ptr<BehaviorDatabase>;
using BehaviorDatabaseConstWeakPtr = std::weak_ptr<const BehaviorDatabase>;
using BehaviorDatabaseUPtr = std::unique_ptr<BehaviorDatabase>;
using BehaviorDatabaseConstUPtr = std::unique_ptr<const BehaviorDatabase>;
class TenantDatabase;
using TenantDatabasePtr = std::shared_ptr<TenantDatabase>;
using TenantDatabaseConstPtr = std::shared_ptr<const TenantDatabase>;
using TenantDatabaseWeakPtr = std::weak_ptr<TenantDatabase>;
using TenantDatabaseConstWeakPtr = std::weak_ptr<const TenantDatabase>;
using TenantDatabaseUPtr = std::unique_ptr<TenantDatabase>;
using TenantDatabaseConstUPtr = std::unique_ptr<const TenantDatabase>;
class PatternedNameGenerator;
using PatternedNameGeneratorPtr = std::shared_ptr<PatternedNameGenerator>;
using PatternedNameGeneratorConstPtr = std::shared_ptr<const PatternedNameGenerator>;
using PatternedNameGeneratorWeakPtr = std::weak_ptr<PatternedNameGenerator>;
using PatternedNameGeneratorConstWeakPtr = std::weak_ptr<const PatternedNameGenerator>;
using PatternedNameGeneratorUPtr = std::unique_ptr<PatternedNameGenerator>;
using PatternedNameGeneratorConstUPtr = std::unique_ptr<const PatternedNameGenerator>;
class DanceDatabase;
using DanceDatabasePtr = std::shared_ptr<DanceDatabase>;
using DanceDatabaseConstPtr = std::shared_ptr<const DanceDatabase>;
using DanceDatabaseWeakPtr = std::weak_ptr<DanceDatabase>;
using DanceDatabaseConstWeakPtr = std::weak_ptr<const DanceDatabase>;
using DanceDatabaseUPtr = std::unique_ptr<DanceDatabase>;
using DanceDatabaseConstUPtr = std::unique_ptr<const DanceDatabase>;
class SpawnTypeDatabase;
using SpawnTypeDatabasePtr = std::shared_ptr<SpawnTypeDatabase>;
using SpawnTypeDatabaseConstPtr = std::shared_ptr<const SpawnTypeDatabase>;
using SpawnTypeDatabaseWeakPtr = std::weak_ptr<SpawnTypeDatabase>;
using SpawnTypeDatabaseConstWeakPtr = std::weak_ptr<const SpawnTypeDatabase>;
using SpawnTypeDatabaseUPtr = std::unique_ptr<SpawnTypeDatabase>;
using SpawnTypeDatabaseConstUPtr = std::unique_ptr<const SpawnTypeDatabase>;
class RadioMessageDatabase;
using RadioMessageDatabasePtr = std::shared_ptr<RadioMessageDatabase>;
using RadioMessageDatabaseConstPtr = std::shared_ptr<const RadioMessageDatabase>;
using RadioMessageDatabaseWeakPtr = std::weak_ptr<RadioMessageDatabase>;
using RadioMessageDatabaseConstWeakPtr = std::weak_ptr<const RadioMessageDatabase>;
using RadioMessageDatabaseUPtr = std::unique_ptr<RadioMessageDatabase>;
using RadioMessageDatabaseConstUPtr = std::unique_ptr<const RadioMessageDatabase>;
class CollectionDatabase;
using CollectionDatabasePtr = std::shared_ptr<CollectionDatabase>;
using CollectionDatabaseConstPtr = std::shared_ptr<const CollectionDatabase>;
using CollectionDatabaseWeakPtr = std::weak_ptr<CollectionDatabase>;
using CollectionDatabaseConstWeakPtr = std::weak_ptr<const CollectionDatabase>;
using CollectionDatabaseUPtr = std::unique_ptr<CollectionDatabase>;
using CollectionDatabaseConstUPtr = std::unique_ptr<const CollectionDatabase>;

class Root;
using RootPtr = std::shared_ptr<Root>;
using RootConstPtr = std::shared_ptr<const Root>;
using RootWeakPtr = std::weak_ptr<Root>;
using RootConstWeakPtr = std::weak_ptr<const Root>;
using RootUPtr = std::unique_ptr<Root>;
using RootConstUPtr = std::unique_ptr<const Root>;

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
  static std::shared_ptr<T> loadMember(std::shared_ptr<T>& ptr, Mutex& mutex, char const* name, Params&&... params);
  template <typename T>
  static std::shared_ptr<T> loadMemberFunction(std::shared_ptr<T>& ptr, Mutex& mutex, char const* name, std::function<std::shared_ptr<T>()> loadFunction);

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

export module star.root;

export namespace Star {
  using ::Star::ItemDatabase;
  using ::Star::ItemDatabasePtr;
  using ::Star::ItemDatabaseConstPtr;
  using ::Star::ItemDatabaseWeakPtr;
  using ::Star::ItemDatabaseConstWeakPtr;
  using ::Star::ItemDatabaseUPtr;
  using ::Star::ItemDatabaseConstUPtr;
  using ::Star::MaterialDatabase;
  using ::Star::MaterialDatabasePtr;
  using ::Star::MaterialDatabaseConstPtr;
  using ::Star::MaterialDatabaseWeakPtr;
  using ::Star::MaterialDatabaseConstWeakPtr;
  using ::Star::MaterialDatabaseUPtr;
  using ::Star::MaterialDatabaseConstUPtr;
  using ::Star::ObjectDatabase;
  using ::Star::ObjectDatabasePtr;
  using ::Star::ObjectDatabaseConstPtr;
  using ::Star::ObjectDatabaseWeakPtr;
  using ::Star::ObjectDatabaseConstWeakPtr;
  using ::Star::ObjectDatabaseUPtr;
  using ::Star::ObjectDatabaseConstUPtr;
  using ::Star::PlantDatabase;
  using ::Star::PlantDatabasePtr;
  using ::Star::PlantDatabaseConstPtr;
  using ::Star::PlantDatabaseWeakPtr;
  using ::Star::PlantDatabaseConstWeakPtr;
  using ::Star::PlantDatabaseUPtr;
  using ::Star::PlantDatabaseConstUPtr;
  using ::Star::ProjectileDatabase;
  using ::Star::ProjectileDatabasePtr;
  using ::Star::ProjectileDatabaseConstPtr;
  using ::Star::ProjectileDatabaseWeakPtr;
  using ::Star::ProjectileDatabaseConstWeakPtr;
  using ::Star::ProjectileDatabaseUPtr;
  using ::Star::ProjectileDatabaseConstUPtr;
  using ::Star::MonsterDatabase;
  using ::Star::MonsterDatabasePtr;
  using ::Star::MonsterDatabaseConstPtr;
  using ::Star::MonsterDatabaseWeakPtr;
  using ::Star::MonsterDatabaseConstWeakPtr;
  using ::Star::MonsterDatabaseUPtr;
  using ::Star::MonsterDatabaseConstUPtr;
  using ::Star::NpcDatabase;
  using ::Star::NpcDatabasePtr;
  using ::Star::NpcDatabaseConstPtr;
  using ::Star::NpcDatabaseWeakPtr;
  using ::Star::NpcDatabaseConstWeakPtr;
  using ::Star::NpcDatabaseUPtr;
  using ::Star::NpcDatabaseConstUPtr;
  using ::Star::StagehandDatabase;
  using ::Star::StagehandDatabasePtr;
  using ::Star::StagehandDatabaseConstPtr;
  using ::Star::StagehandDatabaseWeakPtr;
  using ::Star::StagehandDatabaseConstWeakPtr;
  using ::Star::StagehandDatabaseUPtr;
  using ::Star::StagehandDatabaseConstUPtr;
  using ::Star::VehicleDatabase;
  using ::Star::VehicleDatabasePtr;
  using ::Star::VehicleDatabaseConstPtr;
  using ::Star::VehicleDatabaseWeakPtr;
  using ::Star::VehicleDatabaseConstWeakPtr;
  using ::Star::VehicleDatabaseUPtr;
  using ::Star::VehicleDatabaseConstUPtr;
  using ::Star::PlayerFactory;
  using ::Star::PlayerFactoryPtr;
  using ::Star::PlayerFactoryConstPtr;
  using ::Star::PlayerFactoryWeakPtr;
  using ::Star::PlayerFactoryConstWeakPtr;
  using ::Star::PlayerFactoryUPtr;
  using ::Star::PlayerFactoryConstUPtr;
  using ::Star::EntityFactory;
  using ::Star::EntityFactoryPtr;
  using ::Star::EntityFactoryConstPtr;
  using ::Star::EntityFactoryWeakPtr;
  using ::Star::EntityFactoryConstWeakPtr;
  using ::Star::EntityFactoryUPtr;
  using ::Star::EntityFactoryConstUPtr;
  using ::Star::TerrainDatabase;
  using ::Star::TerrainDatabasePtr;
  using ::Star::TerrainDatabaseConstPtr;
  using ::Star::TerrainDatabaseWeakPtr;
  using ::Star::TerrainDatabaseConstWeakPtr;
  using ::Star::TerrainDatabaseUPtr;
  using ::Star::TerrainDatabaseConstUPtr;
  using ::Star::BiomeDatabase;
  using ::Star::BiomeDatabasePtr;
  using ::Star::BiomeDatabaseConstPtr;
  using ::Star::BiomeDatabaseWeakPtr;
  using ::Star::BiomeDatabaseConstWeakPtr;
  using ::Star::BiomeDatabaseUPtr;
  using ::Star::BiomeDatabaseConstUPtr;
  using ::Star::LiquidsDatabase;
  using ::Star::LiquidsDatabasePtr;
  using ::Star::LiquidsDatabaseConstPtr;
  using ::Star::LiquidsDatabaseWeakPtr;
  using ::Star::LiquidsDatabaseConstWeakPtr;
  using ::Star::LiquidsDatabaseUPtr;
  using ::Star::LiquidsDatabaseConstUPtr;
  using ::Star::StatusEffectDatabase;
  using ::Star::StatusEffectDatabasePtr;
  using ::Star::StatusEffectDatabaseConstPtr;
  using ::Star::StatusEffectDatabaseWeakPtr;
  using ::Star::StatusEffectDatabaseConstWeakPtr;
  using ::Star::StatusEffectDatabaseUPtr;
  using ::Star::StatusEffectDatabaseConstUPtr;
  using ::Star::DamageDatabase;
  using ::Star::DamageDatabasePtr;
  using ::Star::DamageDatabaseConstPtr;
  using ::Star::DamageDatabaseWeakPtr;
  using ::Star::DamageDatabaseConstWeakPtr;
  using ::Star::DamageDatabaseUPtr;
  using ::Star::DamageDatabaseConstUPtr;
  using ::Star::ParticleDatabase;
  using ::Star::ParticleDatabasePtr;
  using ::Star::ParticleDatabaseConstPtr;
  using ::Star::ParticleDatabaseWeakPtr;
  using ::Star::ParticleDatabaseConstWeakPtr;
  using ::Star::ParticleDatabaseUPtr;
  using ::Star::ParticleDatabaseConstUPtr;
  using ::Star::EffectSourceDatabase;
  using ::Star::EffectSourceDatabasePtr;
  using ::Star::EffectSourceDatabaseConstPtr;
  using ::Star::EffectSourceDatabaseWeakPtr;
  using ::Star::EffectSourceDatabaseConstWeakPtr;
  using ::Star::EffectSourceDatabaseUPtr;
  using ::Star::EffectSourceDatabaseConstUPtr;
  using ::Star::FunctionDatabase;
  using ::Star::FunctionDatabasePtr;
  using ::Star::FunctionDatabaseConstPtr;
  using ::Star::FunctionDatabaseWeakPtr;
  using ::Star::FunctionDatabaseConstWeakPtr;
  using ::Star::FunctionDatabaseUPtr;
  using ::Star::FunctionDatabaseConstUPtr;
  using ::Star::TreasureDatabase;
  using ::Star::TreasureDatabasePtr;
  using ::Star::TreasureDatabaseConstPtr;
  using ::Star::TreasureDatabaseWeakPtr;
  using ::Star::TreasureDatabaseConstWeakPtr;
  using ::Star::TreasureDatabaseUPtr;
  using ::Star::TreasureDatabaseConstUPtr;
  using ::Star::DungeonDefinitions;
  using ::Star::DungeonDefinitionsPtr;
  using ::Star::DungeonDefinitionsConstPtr;
  using ::Star::DungeonDefinitionsWeakPtr;
  using ::Star::DungeonDefinitionsConstWeakPtr;
  using ::Star::DungeonDefinitionsUPtr;
  using ::Star::DungeonDefinitionsConstUPtr;
  using ::Star::TilesetDatabase;
  using ::Star::TilesetDatabasePtr;
  using ::Star::TilesetDatabaseConstPtr;
  using ::Star::TilesetDatabaseWeakPtr;
  using ::Star::TilesetDatabaseConstWeakPtr;
  using ::Star::TilesetDatabaseUPtr;
  using ::Star::TilesetDatabaseConstUPtr;
  using ::Star::StatisticsDatabase;
  using ::Star::StatisticsDatabasePtr;
  using ::Star::StatisticsDatabaseConstPtr;
  using ::Star::StatisticsDatabaseWeakPtr;
  using ::Star::StatisticsDatabaseConstWeakPtr;
  using ::Star::StatisticsDatabaseUPtr;
  using ::Star::StatisticsDatabaseConstUPtr;
  using ::Star::EmoteProcessor;
  using ::Star::EmoteProcessorPtr;
  using ::Star::EmoteProcessorConstPtr;
  using ::Star::EmoteProcessorWeakPtr;
  using ::Star::EmoteProcessorConstWeakPtr;
  using ::Star::EmoteProcessorUPtr;
  using ::Star::EmoteProcessorConstUPtr;
  using ::Star::SpeciesDatabase;
  using ::Star::SpeciesDatabasePtr;
  using ::Star::SpeciesDatabaseConstPtr;
  using ::Star::SpeciesDatabaseWeakPtr;
  using ::Star::SpeciesDatabaseConstWeakPtr;
  using ::Star::SpeciesDatabaseUPtr;
  using ::Star::SpeciesDatabaseConstUPtr;
  using ::Star::ImageMetadataDatabase;
  using ::Star::ImageMetadataDatabasePtr;
  using ::Star::ImageMetadataDatabaseConstPtr;
  using ::Star::ImageMetadataDatabaseWeakPtr;
  using ::Star::ImageMetadataDatabaseConstWeakPtr;
  using ::Star::ImageMetadataDatabaseUPtr;
  using ::Star::ImageMetadataDatabaseConstUPtr;
  using ::Star::VersioningDatabase;
  using ::Star::VersioningDatabasePtr;
  using ::Star::VersioningDatabaseConstPtr;
  using ::Star::VersioningDatabaseWeakPtr;
  using ::Star::VersioningDatabaseConstWeakPtr;
  using ::Star::VersioningDatabaseUPtr;
  using ::Star::VersioningDatabaseConstUPtr;
  using ::Star::QuestTemplateDatabase;
  using ::Star::QuestTemplateDatabasePtr;
  using ::Star::QuestTemplateDatabaseConstPtr;
  using ::Star::QuestTemplateDatabaseWeakPtr;
  using ::Star::QuestTemplateDatabaseConstWeakPtr;
  using ::Star::QuestTemplateDatabaseUPtr;
  using ::Star::QuestTemplateDatabaseConstUPtr;
  using ::Star::AiDatabase;
  using ::Star::AiDatabasePtr;
  using ::Star::AiDatabaseConstPtr;
  using ::Star::AiDatabaseWeakPtr;
  using ::Star::AiDatabaseConstWeakPtr;
  using ::Star::AiDatabaseUPtr;
  using ::Star::AiDatabaseConstUPtr;
  using ::Star::TechDatabase;
  using ::Star::TechDatabasePtr;
  using ::Star::TechDatabaseConstPtr;
  using ::Star::TechDatabaseWeakPtr;
  using ::Star::TechDatabaseConstWeakPtr;
  using ::Star::TechDatabaseUPtr;
  using ::Star::TechDatabaseConstUPtr;
  using ::Star::CodexDatabase;
  using ::Star::CodexDatabasePtr;
  using ::Star::CodexDatabaseConstPtr;
  using ::Star::CodexDatabaseWeakPtr;
  using ::Star::CodexDatabaseConstWeakPtr;
  using ::Star::CodexDatabaseUPtr;
  using ::Star::CodexDatabaseConstUPtr;
  using ::Star::BehaviorDatabase;
  using ::Star::BehaviorDatabasePtr;
  using ::Star::BehaviorDatabaseConstPtr;
  using ::Star::BehaviorDatabaseWeakPtr;
  using ::Star::BehaviorDatabaseConstWeakPtr;
  using ::Star::BehaviorDatabaseUPtr;
  using ::Star::BehaviorDatabaseConstUPtr;
  using ::Star::TenantDatabase;
  using ::Star::TenantDatabasePtr;
  using ::Star::TenantDatabaseConstPtr;
  using ::Star::TenantDatabaseWeakPtr;
  using ::Star::TenantDatabaseConstWeakPtr;
  using ::Star::TenantDatabaseUPtr;
  using ::Star::TenantDatabaseConstUPtr;
  using ::Star::PatternedNameGenerator;
  using ::Star::PatternedNameGeneratorPtr;
  using ::Star::PatternedNameGeneratorConstPtr;
  using ::Star::PatternedNameGeneratorWeakPtr;
  using ::Star::PatternedNameGeneratorConstWeakPtr;
  using ::Star::PatternedNameGeneratorUPtr;
  using ::Star::PatternedNameGeneratorConstUPtr;
  using ::Star::DanceDatabase;
  using ::Star::DanceDatabasePtr;
  using ::Star::DanceDatabaseConstPtr;
  using ::Star::DanceDatabaseWeakPtr;
  using ::Star::DanceDatabaseConstWeakPtr;
  using ::Star::DanceDatabaseUPtr;
  using ::Star::DanceDatabaseConstUPtr;
  using ::Star::SpawnTypeDatabase;
  using ::Star::SpawnTypeDatabasePtr;
  using ::Star::SpawnTypeDatabaseConstPtr;
  using ::Star::SpawnTypeDatabaseWeakPtr;
  using ::Star::SpawnTypeDatabaseConstWeakPtr;
  using ::Star::SpawnTypeDatabaseUPtr;
  using ::Star::SpawnTypeDatabaseConstUPtr;
  using ::Star::RadioMessageDatabase;
  using ::Star::RadioMessageDatabasePtr;
  using ::Star::RadioMessageDatabaseConstPtr;
  using ::Star::RadioMessageDatabaseWeakPtr;
  using ::Star::RadioMessageDatabaseConstWeakPtr;
  using ::Star::RadioMessageDatabaseUPtr;
  using ::Star::RadioMessageDatabaseConstUPtr;
  using ::Star::CollectionDatabase;
  using ::Star::CollectionDatabasePtr;
  using ::Star::CollectionDatabaseConstPtr;
  using ::Star::CollectionDatabaseWeakPtr;
  using ::Star::CollectionDatabaseConstWeakPtr;
  using ::Star::CollectionDatabaseUPtr;
  using ::Star::CollectionDatabaseConstUPtr;
  using ::Star::Root;
  using ::Star::RootPtr;
  using ::Star::RootConstPtr;
  using ::Star::RootWeakPtr;
  using ::Star::RootConstWeakPtr;
  using ::Star::RootUPtr;
  using ::Star::RootConstUPtr;
}

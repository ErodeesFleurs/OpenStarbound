module;
#include "StarJson.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;

namespace Star {

STAR_CLASS(ItemDatabase);
STAR_CLASS(MaterialDatabase);
STAR_CLASS(ObjectDatabase);
STAR_CLASS(PlantDatabase);
STAR_CLASS(ProjectileDatabase);
STAR_CLASS(MonsterDatabase);
STAR_CLASS(NpcDatabase);
STAR_CLASS(StagehandDatabase);
STAR_CLASS(VehicleDatabase);
STAR_CLASS(PlayerFactory);
STAR_CLASS(EntityFactory);
STAR_CLASS(TerrainDatabase);
STAR_CLASS(BiomeDatabase);
STAR_CLASS(LiquidsDatabase);
STAR_CLASS(StatusEffectDatabase);
STAR_CLASS(DamageDatabase);
STAR_CLASS(ParticleDatabase);
STAR_CLASS(EffectSourceDatabase);
STAR_CLASS(FunctionDatabase);
STAR_CLASS(TreasureDatabase);
STAR_CLASS(DungeonDefinitions);
STAR_CLASS(TilesetDatabase);
STAR_CLASS(StatisticsDatabase);
STAR_CLASS(EmoteProcessor);
STAR_CLASS(SpeciesDatabase);
STAR_CLASS(ImageMetadataDatabase);
STAR_CLASS(VersioningDatabase);
STAR_CLASS(QuestTemplateDatabase);
STAR_CLASS(AiDatabase);
STAR_CLASS(TechDatabase);
STAR_CLASS(CodexDatabase);
STAR_CLASS(BehaviorDatabase);
STAR_CLASS(TenantDatabase);
STAR_CLASS(PatternedNameGenerator);
STAR_CLASS(DanceDatabase);
STAR_CLASS(SpawnTypeDatabase);
STAR_CLASS(RadioMessageDatabase);
STAR_CLASS(CollectionDatabase);

STAR_CLASS(Root);

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

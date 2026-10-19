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
class MaterialDatabase;
class ObjectDatabase;
class PlantDatabase;
class ProjectileDatabase;
class MonsterDatabase;
class NpcDatabase;
class StagehandDatabase;
class VehicleDatabase;
class PlayerFactory;
class EntityFactory;
class TerrainDatabase;
class BiomeDatabase;
class LiquidsDatabase;
class StatusEffectDatabase;
class DamageDatabase;
class ParticleDatabase;
class EffectSourceDatabase;
class FunctionDatabase;
class TreasureDatabase;
class DungeonDefinitions;
class TilesetDatabase;
class StatisticsDatabase;
class EmoteProcessor;
class SpeciesDatabase;
class ImageMetadataDatabase;
class VersioningDatabase;
class QuestTemplateDatabase;
class AiDatabase;
class TechDatabase;
class CodexDatabase;
class BehaviorDatabase;
class TenantDatabase;
class PatternedNameGenerator;
class DanceDatabase;
class SpawnTypeDatabase;
class RadioMessageDatabase;
class CollectionDatabase;

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

  // Callbacks run synchronously on the reload/hotReload caller, not on the
  // maintenance thread. Retain the subscription; owners of raw-this callbacks
  // must disconnect before tearing down any state used by the callback.
  [[nodiscard]] ListenerSubscription registerReloadListener(std::weak_ptr<Listener> reloadListener);

  void hotReload();

  // Translates the given path to be relative to the configured storage
  // location.
  String toStoragePath(String const& path) const;

  // All of the Root member accessors are safe to call at any time after Root
  // initialization, if they are not loaded they will load before returning.

  std::shared_ptr<Assets const> assets() override;
  std::shared_ptr<Configuration> configuration() override;

  std::shared_ptr<ObjectDatabase const> objectDatabase();
  std::shared_ptr<PlantDatabase const> plantDatabase();
  std::shared_ptr<ProjectileDatabase const> projectileDatabase();
  std::shared_ptr<MonsterDatabase const> monsterDatabase();
  std::shared_ptr<NpcDatabase const> npcDatabase();
  std::shared_ptr<StagehandDatabase const> stagehandDatabase();
  std::shared_ptr<VehicleDatabase const> vehicleDatabase();
  std::shared_ptr<PlayerFactory const> playerFactory();

  std::shared_ptr<EntityFactory const> entityFactory();

  std::shared_ptr<PatternedNameGenerator const> nameGenerator();

  std::shared_ptr<ItemDatabase const> itemDatabase();
  std::shared_ptr<MaterialDatabase const> materialDatabase();
  std::shared_ptr<TerrainDatabase const> terrainDatabase();
  std::shared_ptr<BiomeDatabase const> biomeDatabase();
  std::shared_ptr<LiquidsDatabase const> liquidsDatabase();
  std::shared_ptr<StatusEffectDatabase const> statusEffectDatabase();
  std::shared_ptr<DamageDatabase const> damageDatabase();
  std::shared_ptr<ParticleDatabase const> particleDatabase();
  std::shared_ptr<EffectSourceDatabase const> effectSourceDatabase();
  std::shared_ptr<FunctionDatabase const> functionDatabase();
  std::shared_ptr<TreasureDatabase const> treasureDatabase();
  std::shared_ptr<DungeonDefinitions const> dungeonDefinitions();
  std::shared_ptr<TilesetDatabase const> tilesetDatabase();
  std::shared_ptr<StatisticsDatabase const> statisticsDatabase();
  std::shared_ptr<EmoteProcessor const> emoteProcessor();
  std::shared_ptr<SpeciesDatabase const> speciesDatabase();
  std::shared_ptr<ImageMetadataDatabase const> imageMetadataDatabase();
  std::shared_ptr<VersioningDatabase const> versioningDatabase();
  std::shared_ptr<QuestTemplateDatabase const> questTemplateDatabase();
  std::shared_ptr<AiDatabase const> aiDatabase();
  std::shared_ptr<TechDatabase const> techDatabase();
  std::shared_ptr<CodexDatabase const> codexDatabase();
  std::shared_ptr<BehaviorDatabase const> behaviorDatabase();
  std::shared_ptr<TenantDatabase const> tenantDatabase();
  std::shared_ptr<DanceDatabase const> danceDatabase();
  std::shared_ptr<SpawnTypeDatabase const> spawnTypeDatabase();
  std::shared_ptr<RadioMessageDatabase const> radioMessageDatabase();
  std::shared_ptr<CollectionDatabase const> collectionDatabase();

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

  std::shared_ptr<Assets> m_assets;
  Mutex m_assetsMutex;

  std::shared_ptr<Configuration> m_configuration;
  Mutex m_configurationMutex;

  std::shared_ptr<ObjectDatabase> m_objectDatabase;
  Mutex m_objectDatabaseMutex;

  std::shared_ptr<PlantDatabase> m_plantDatabase;
  Mutex m_plantDatabaseMutex;

  std::shared_ptr<ProjectileDatabase> m_projectileDatabase;
  Mutex m_projectileDatabaseMutex;

  std::shared_ptr<MonsterDatabase> m_monsterDatabase;
  Mutex m_monsterDatabaseMutex;

  std::shared_ptr<NpcDatabase> m_npcDatabase;
  Mutex m_npcDatabaseMutex;

  std::shared_ptr<StagehandDatabase> m_stagehandDatabase;
  Mutex m_stagehandDatabaseMutex;

  std::shared_ptr<VehicleDatabase> m_vehicleDatabase;
  Mutex m_vehicleDatabaseMutex;

  std::shared_ptr<PlayerFactory> m_playerFactory;
  Mutex m_playerFactoryMutex;

  std::shared_ptr<EntityFactory> m_entityFactory;
  Mutex m_entityFactoryMutex;

  std::shared_ptr<PatternedNameGenerator> m_nameGenerator;
  Mutex m_nameGeneratorMutex;

  std::shared_ptr<ItemDatabase> m_itemDatabase;
  Mutex m_itemDatabaseMutex;

  std::shared_ptr<MaterialDatabase> m_materialDatabase;
  Mutex m_materialDatabaseMutex;

  std::shared_ptr<TerrainDatabase> m_terrainDatabase;
  Mutex m_terrainDatabaseMutex;

  std::shared_ptr<BiomeDatabase> m_biomeDatabase;
  Mutex m_biomeDatabaseMutex;

  std::shared_ptr<LiquidsDatabase> m_liquidsDatabase;
  Mutex m_liquidsDatabaseMutex;

  std::shared_ptr<StatusEffectDatabase> m_statusEffectDatabase;
  Mutex m_statusEffectDatabaseMutex;

  std::shared_ptr<DamageDatabase> m_damageDatabase;
  Mutex m_damageDatabaseMutex;

  std::shared_ptr<ParticleDatabase> m_particleDatabase;
  Mutex m_particleDatabaseMutex;

  std::shared_ptr<EffectSourceDatabase> m_effectSourceDatabase;
  Mutex m_effectSourceDatabaseMutex;

  std::shared_ptr<FunctionDatabase> m_functionDatabase;
  Mutex m_functionDatabaseMutex;

  std::shared_ptr<TreasureDatabase> m_treasureDatabase;
  Mutex m_treasureDatabaseMutex;

  std::shared_ptr<DungeonDefinitions> m_dungeonDefinitions;
  Mutex m_dungeonDefinitionsMutex;

  std::shared_ptr<TilesetDatabase> m_tilesetDatabase;
  Mutex m_tilesetDatabaseMutex;

  std::shared_ptr<StatisticsDatabase> m_statisticsDatabase;
  Mutex m_statisticsDatabaseMutex;

  std::shared_ptr<EmoteProcessor> m_emoteProcessor;
  Mutex m_emoteProcessorMutex;

  std::shared_ptr<SpeciesDatabase> m_speciesDatabase;
  Mutex m_speciesDatabaseMutex;

  std::shared_ptr<ImageMetadataDatabase> m_imageMetadataDatabase;
  Mutex m_imageMetadataDatabaseMutex;

  std::shared_ptr<VersioningDatabase> m_versioningDatabase;
  Mutex m_versioningDatabaseMutex;

  std::shared_ptr<QuestTemplateDatabase> m_questTemplateDatabase;
  Mutex m_questTemplateDatabaseMutex;

  std::shared_ptr<AiDatabase> m_aiDatabase;
  Mutex m_aiDatabaseMutex;

  std::shared_ptr<TechDatabase> m_techDatabase;
  Mutex m_techDatabaseMutex;

  std::shared_ptr<CodexDatabase> m_codexDatabase;
  Mutex m_codexDatabaseMutex;

  std::shared_ptr<BehaviorDatabase> m_behaviorDatabase;
  Mutex m_behaviorDatabaseMutex;

  std::shared_ptr<TenantDatabase> m_tenantDatabase;
  Mutex m_tenantDatabaseMutex;

  std::shared_ptr<DanceDatabase> m_danceDatabase;
  Mutex m_danceDatabaseMutex;

  std::shared_ptr<SpawnTypeDatabase> m_spawnTypeDatabase;
  Mutex m_spawnTypeDatabaseMutex;

  std::shared_ptr<RadioMessageDatabase> m_radioMessageDatabase;
  Mutex m_radioMessageDatabaseMutex;

  std::shared_ptr<CollectionDatabase> m_collectionDatabase;
  Mutex m_collectionDatabaseMutex;
};

}

export module star.root;

export namespace Star {
  using ::Star::ItemDatabase;
  using ::Star::MaterialDatabase;
  using ::Star::ObjectDatabase;
  using ::Star::PlantDatabase;
  using ::Star::ProjectileDatabase;
  using ::Star::MonsterDatabase;
  using ::Star::NpcDatabase;
  using ::Star::StagehandDatabase;
  using ::Star::VehicleDatabase;
  using ::Star::PlayerFactory;
  using ::Star::EntityFactory;
  using ::Star::TerrainDatabase;
  using ::Star::BiomeDatabase;
  using ::Star::LiquidsDatabase;
  using ::Star::StatusEffectDatabase;
  using ::Star::DamageDatabase;
  using ::Star::ParticleDatabase;
  using ::Star::EffectSourceDatabase;
  using ::Star::FunctionDatabase;
  using ::Star::TreasureDatabase;
  using ::Star::DungeonDefinitions;
  using ::Star::TilesetDatabase;
  using ::Star::StatisticsDatabase;
  using ::Star::EmoteProcessor;
  using ::Star::SpeciesDatabase;
  using ::Star::ImageMetadataDatabase;
  using ::Star::VersioningDatabase;
  using ::Star::QuestTemplateDatabase;
  using ::Star::AiDatabase;
  using ::Star::TechDatabase;
  using ::Star::CodexDatabase;
  using ::Star::BehaviorDatabase;
  using ::Star::TenantDatabase;
  using ::Star::PatternedNameGenerator;
  using ::Star::DanceDatabase;
  using ::Star::SpawnTypeDatabase;
  using ::Star::RadioMessageDatabase;
  using ::Star::CollectionDatabase;
  using ::Star::Root;
}

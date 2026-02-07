#pragma once

#include "StarConfig.hpp"
#include "StarConfiguration.hpp"
#include "StarJson.hpp"
#include "StarListener.hpp"
#include "StarLogging.hpp"
#include "StarRootBase.hpp"

import std;

namespace Star {

class ObjectDatabase;
class PlantDatabase;
class ProjectileDatabase;
class MonsterDatabase;
class NpcDatabase;
class StagehandDatabase;
class VehicleDatabase;
class PlayerFactory;
class EntityFactory;
class PatternedNameGenerator;
class ItemDatabase;
class MaterialDatabase;
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
    std::optional<String> logDirectory;

    // Name of the log file that should be written, if any, relative to the
    // log directory
    std::optional<String> logFile;

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
    std::optional<String> runtimeConfigFile;
  };

  // Get pointer to the singleton root instance, if it exists.  Otherwise,
  // returns nullptr.
  static auto singletonPtr() -> Root*;

  // Gets reference to root singleton, throws RootException if root is not
  // initialized.
  static auto singleton() -> Root&;

  // Initializes the starbound root object and does the initial load.  All of
  // the Root members will be just in time loaded as they are accessed, unless
  // fullyLoad is called beforehand.
  Root(Settings settings);

  Root(Root const&) = delete;
  auto operator=(Root const&) -> Root& = delete;

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
  void registerReloadListener(WeakPtr<Listener> reloadListener);

  void hotReload();

  // Translates the given path to be relative to the configured storage
  // location.
  [[nodiscard]] auto toStoragePath(String const& path) const -> String;

  // All of the Root member accessors are safe to call at any time after Root
  // initialization, if they are not loaded they will load before returning.

  auto assets() -> ConstPtr<Assets> override;
  auto configuration() -> Ptr<Configuration> override;

  auto objectDatabase() -> ConstPtr<ObjectDatabase>;
  auto plantDatabase() -> ConstPtr<PlantDatabase>;
  auto projectileDatabase() -> ConstPtr<ProjectileDatabase>;
  auto monsterDatabase() -> ConstPtr<MonsterDatabase>;
  auto npcDatabase() -> ConstPtr<NpcDatabase>;
  auto stagehandDatabase() -> ConstPtr<StagehandDatabase>;
  auto vehicleDatabase() -> ConstPtr<VehicleDatabase>;
  auto playerFactory() -> ConstPtr<PlayerFactory>;

  auto entityFactory() -> ConstPtr<EntityFactory>;

  auto nameGenerator() -> ConstPtr<PatternedNameGenerator>;

  auto itemDatabase() -> ConstPtr<ItemDatabase>;
  auto materialDatabase() -> ConstPtr<MaterialDatabase>;
  auto terrainDatabase() -> ConstPtr<TerrainDatabase>;
  auto biomeDatabase() -> ConstPtr<BiomeDatabase>;
  auto liquidsDatabase() -> ConstPtr<LiquidsDatabase>;
  auto statusEffectDatabase() -> ConstPtr<StatusEffectDatabase>;
  auto damageDatabase() -> ConstPtr<DamageDatabase>;
  auto particleDatabase() -> ConstPtr<ParticleDatabase>;
  auto effectSourceDatabase() -> ConstPtr<EffectSourceDatabase>;
  auto functionDatabase() -> ConstPtr<FunctionDatabase>;
  auto treasureDatabase() -> ConstPtr<TreasureDatabase>;
  auto dungeonDefinitions() -> ConstPtr<DungeonDefinitions>;
  auto tilesetDatabase() -> ConstPtr<TilesetDatabase>;
  auto statisticsDatabase() -> ConstPtr<StatisticsDatabase>;
  auto emoteProcessor() -> ConstPtr<EmoteProcessor>;
  auto speciesDatabase() -> ConstPtr<SpeciesDatabase>;
  auto imageMetadataDatabase() -> ConstPtr<ImageMetadataDatabase>;
  auto versioningDatabase() -> ConstPtr<VersioningDatabase>;
  auto questTemplateDatabase() -> ConstPtr<QuestTemplateDatabase>;
  auto aiDatabase() -> ConstPtr<AiDatabase>;
  auto techDatabase() -> ConstPtr<TechDatabase>;
  auto codexDatabase() -> ConstPtr<CodexDatabase>;
  auto behaviorDatabase() -> ConstPtr<BehaviorDatabase>;
  auto tenantDatabase() -> ConstPtr<TenantDatabase>;
  auto danceDatabase() -> ConstPtr<DanceDatabase>;
  auto spawnTypeDatabase() -> ConstPtr<SpawnTypeDatabase>;
  auto radioMessageDatabase() -> ConstPtr<RadioMessageDatabase>;
  auto collectionDatabase() -> ConstPtr<CollectionDatabase>;

  auto settings() -> Settings&;

private:
  static auto scanForAssetSources(StringList const& directories, StringList const& manual = {}) -> StringList;
  template <typename T, typename... Params>
  static auto loadMember(std::shared_ptr<T>& ptr, Mutex& mutex, char const* name, Params&&... params) -> std::shared_ptr<T>;
  template <typename T>
  static auto loadMemberFunction(std::shared_ptr<T>& ptr, Mutex& mutex, char const* name, std::function<std::shared_ptr<T>()> loadFunction) -> std::shared_ptr<T>;

  // m_configurationMutex must be held when calling
  void writeConfig();

  Settings m_settings;

  Mutex m_modsMutex;
  StringList m_modDirectories;

  ListenerGroup m_reloadListeners;

  Json m_lastRuntimeConfig;
  std::optional<String> m_runtimeConfigFile;

  ThreadFunction<void> m_maintenanceThread;
  Mutex m_maintenanceStopMutex;
  ConditionVariable m_maintenanceStopCondition;
  bool m_stopMaintenanceThread;

  Ptr<Assets> m_assets;
  Mutex m_assetsMutex;

  Ptr<Configuration> m_configuration;
  Mutex m_configurationMutex;

  Ptr<ObjectDatabase> m_objectDatabase;
  Mutex m_objectDatabaseMutex;

  Ptr<PlantDatabase> m_plantDatabase;
  Mutex m_plantDatabaseMutex;

  Ptr<ProjectileDatabase> m_projectileDatabase;
  Mutex m_projectileDatabaseMutex;

  Ptr<MonsterDatabase> m_monsterDatabase;
  Mutex m_monsterDatabaseMutex;

  Ptr<NpcDatabase> m_npcDatabase;
  Mutex m_npcDatabaseMutex;

  Ptr<StagehandDatabase> m_stagehandDatabase;
  Mutex m_stagehandDatabaseMutex;

  Ptr<VehicleDatabase> m_vehicleDatabase;
  Mutex m_vehicleDatabaseMutex;

  Ptr<PlayerFactory> m_playerFactory;
  Mutex m_playerFactoryMutex;

  Ptr<EntityFactory> m_entityFactory;
  Mutex m_entityFactoryMutex;

  Ptr<PatternedNameGenerator> m_nameGenerator;
  Mutex m_nameGeneratorMutex;

  Ptr<ItemDatabase> m_itemDatabase;
  Mutex m_itemDatabaseMutex;

  Ptr<MaterialDatabase> m_materialDatabase;
  Mutex m_materialDatabaseMutex;

  Ptr<TerrainDatabase> m_terrainDatabase;
  Mutex m_terrainDatabaseMutex;

  Ptr<BiomeDatabase> m_biomeDatabase;
  Mutex m_biomeDatabaseMutex;

  Ptr<LiquidsDatabase> m_liquidsDatabase;
  Mutex m_liquidsDatabaseMutex;

  Ptr<StatusEffectDatabase> m_statusEffectDatabase;
  Mutex m_statusEffectDatabaseMutex;

  Ptr<DamageDatabase> m_damageDatabase;
  Mutex m_damageDatabaseMutex;

  Ptr<ParticleDatabase> m_particleDatabase;
  Mutex m_particleDatabaseMutex;

  Ptr<EffectSourceDatabase> m_effectSourceDatabase;
  Mutex m_effectSourceDatabaseMutex;

  Ptr<FunctionDatabase> m_functionDatabase;
  Mutex m_functionDatabaseMutex;

  Ptr<TreasureDatabase> m_treasureDatabase;
  Mutex m_treasureDatabaseMutex;

  Ptr<DungeonDefinitions> m_dungeonDefinitions;
  Mutex m_dungeonDefinitionsMutex;

  Ptr<TilesetDatabase> m_tilesetDatabase;
  Mutex m_tilesetDatabaseMutex;

  Ptr<StatisticsDatabase> m_statisticsDatabase;
  Mutex m_statisticsDatabaseMutex;

  Ptr<EmoteProcessor> m_emoteProcessor;
  Mutex m_emoteProcessorMutex;

  Ptr<SpeciesDatabase> m_speciesDatabase;
  Mutex m_speciesDatabaseMutex;

  Ptr<ImageMetadataDatabase> m_imageMetadataDatabase;
  Mutex m_imageMetadataDatabaseMutex;

  Ptr<VersioningDatabase> m_versioningDatabase;
  Mutex m_versioningDatabaseMutex;

  Ptr<QuestTemplateDatabase> m_questTemplateDatabase;
  Mutex m_questTemplateDatabaseMutex;

  Ptr<AiDatabase> m_aiDatabase;
  Mutex m_aiDatabaseMutex;

  Ptr<TechDatabase> m_techDatabase;
  Mutex m_techDatabaseMutex;

  Ptr<CodexDatabase> m_codexDatabase;
  Mutex m_codexDatabaseMutex;

  Ptr<BehaviorDatabase> m_behaviorDatabase;
  Mutex m_behaviorDatabaseMutex;

  Ptr<TenantDatabase> m_tenantDatabase;
  Mutex m_tenantDatabaseMutex;

  Ptr<DanceDatabase> m_danceDatabase;
  Mutex m_danceDatabaseMutex;

  Ptr<SpawnTypeDatabase> m_spawnTypeDatabase;
  Mutex m_spawnTypeDatabaseMutex;

  Ptr<RadioMessageDatabase> m_radioMessageDatabase;
  Mutex m_radioMessageDatabaseMutex;

  Ptr<CollectionDatabase> m_collectionDatabase;
  Mutex m_collectionDatabaseMutex;
};

}// namespace Star

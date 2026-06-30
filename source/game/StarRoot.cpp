#include "StarRoot.hpp"
#include "StarAiDatabase.hpp"
#include "StarAssets.hpp"
#include "StarBehaviorDatabase.hpp"
#include "StarBiomeDatabase.hpp"
#include "StarCodexDatabase.hpp"
#include "StarCollectionDatabase.hpp"
#include "StarConfiguration.hpp"
#include "StarDamageDatabase.hpp"
#include "StarDanceDatabase.hpp"
#include "StarDirectoryAssetSource.hpp"
#include "StarDungeonGenerator.hpp"
#include "StarEffectSourceDatabase.hpp"
#include "StarEmoteProcessor.hpp"
#include "StarEncode.hpp"
#include "StarEntityFactory.hpp"
#include "StarFile.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarItemDatabase.hpp"
#include "StarItemDrop.hpp"
#include "StarIterator.hpp"
#include "StarJsonBuilder.hpp"
#include "StarJsonExtra.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLogging.hpp"
#include "StarLuaRoot.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarMonster.hpp"
#include "StarNameGenerator.hpp"
#include "StarNpc.hpp"
#include "StarObject.hpp"
#include "StarObjectDatabase.hpp"
#include "StarPackedAssetSource.hpp"
#include "StarParticleDatabase.hpp"
#include "StarPlant.hpp"
#include "StarPlantDrop.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerFactory.hpp"
#include "StarProjectile.hpp"
#include "StarProjectileDatabase.hpp"
#include "StarQuestTemplateDatabase.hpp"
#include "StarRadioMessageDatabase.hpp"
#include "StarSpawnTypeDatabase.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarStagehandDatabase.hpp"
#include "StarStatisticsDatabase.hpp"
#include "StarStatusEffectDatabase.hpp"
#include "StarStoredFunctions.hpp"
#include "StarTechDatabase.hpp"
#include "StarTenantDatabase.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarTilesetDatabase.hpp"
#include "StarTreasure.hpp"
#include "StarVehicleDatabase.hpp"
#include "StarWorkerPool.hpp"

namespace Star {

namespace {
unsigned const RootMaintenanceSleep = 5000;
unsigned const RootLoadThreads = 4;
}// namespace

atomic<Root*> Root::s_activeRoot{nullptr};

Root::Root(Settings settings) {
  Root* oldRoot = nullptr;
  if (!s_activeRoot.compare_exchange_strong(oldRoot, this))
    throw RootException("Root has been constructed twice");

  m_settings = std::move(settings);
  if (m_settings.runtimeConfigFile)
    m_runtimeConfigFile = toStoragePath(*m_settings.runtimeConfigFile);

  if (!File::isDirectory(m_settings.storageDirectory))
    File::makeDirectory(m_settings.storageDirectory);

  if (m_settings.logFile) {
    String logFile = File::relativeTo(m_settings.logDirectory.value(m_settings.storageDirectory), *m_settings.logFile);
    String oldLogDirectory = m_settings.logDirectory.value(File::relativeTo(m_settings.storageDirectory, "logs"));
    if (!File::isDirectory(oldLogDirectory))
      File::makeDirectory(oldLogDirectory);

    File::backupFileInSequence(logFile, File::relativeTo(oldLogDirectory, *m_settings.logFile), m_settings.logFileBackups);
    Logger::addSink(make_shared<FileLogSink>(logFile, m_settings.logLevel, true));
  }
  Logger::stdoutSink()->setLevel(m_settings.logLevel);

  if (m_settings.quiet)
    Logger::removeStdoutSink();

  Logger::info("Root: Preparing...");

  m_stopMaintenanceThread = false;
  m_maintenanceThread = Thread::invoke("Root::maintenanceMain", [this]() {
    MutexLocker locker(m_maintenanceStopMutex);
    while (!m_stopMaintenanceThread) {
      m_reloadListeners.clearExpiredListeners();

      ObjectDatabasePtr objectDb;
      ItemDatabasePtr itemDb;
      MonsterDatabasePtr monsterDb;
      AssetsPtr assets;
      TenantDatabasePtr tenantDb;
      ImageMetadataDatabasePtr imgMetaDb;
      {
        RecursiveMutexLocker loadLocker(m_loadMutex);
        objectDb = m_objectDatabase;
        itemDb = m_itemDatabase;
        monsterDb = m_monsterDatabase;
        assets = m_assets;
        tenantDb = m_tenantDatabase;
        imgMetaDb = m_imageMetadataDatabase;
      }

      if (objectDb)
        objectDb->cleanup();
      if (itemDb)
        itemDb->cleanup();
      if (monsterDb)
        monsterDb->cleanup();
      if (assets)
        assets->cleanup();
      if (tenantDb)
        tenantDb->cleanup();
      if (imgMetaDb)
        imgMetaDb->cleanup();

      Random::addEntropy();

      {
        RecursiveMutexLocker loadLocker(m_loadMutex);
        writeConfig();
      }

      m_maintenanceStopCondition.wait(m_maintenanceStopMutex, RootMaintenanceSleep);
    }
  });

  Logger::info("Root: Done preparing Root.");
}

Root::~Root() {
  Logger::info("Root: Shutting down Root");

  {
    MutexLocker locker(m_maintenanceStopMutex);
    m_stopMaintenanceThread = true;
    m_maintenanceStopCondition.signal();
  }
  m_maintenanceThread.finish();

  m_reloadListeners.clearAllListeners();

  writeConfig();

  s_activeRoot.store(nullptr);
}

void Root::reload() {
  Logger::info("Root: Reloading from disk");

  {
    RecursiveMutexLocker locker(m_loadMutex);

    writeConfig();

    m_entityFactory.reset();
    m_speciesDatabase.reset();
    m_itemDatabase.reset();
    m_objectDatabase.reset();
    m_playerFactory.reset();
    m_stagehandDatabase.reset();
    m_vehicleDatabase.reset();
    m_npcDatabase.reset();
    m_monsterDatabase.reset();
    m_plantDatabase.reset();
    m_projectileDatabase.reset();
    m_biomeDatabase.reset();
    m_dungeonDefinitions.reset();
    m_tilesetDatabase.reset();
    m_statisticsDatabase.reset();
    m_liquidsDatabase.reset();
    m_materialDatabase.reset();
    m_damageDatabase.reset();
    m_effectSourceDatabase.reset();
    m_statusEffectDatabase.reset();
    m_treasureDatabase.reset();
    m_codexDatabase.reset();
    m_behaviorDatabase.reset();
    m_techDatabase.reset();
    m_aiDatabase.reset();
    m_questTemplateDatabase.reset();
    m_emoteProcessor.reset();
    m_terrainDatabase.reset();
    m_particleDatabase.reset();
    m_versioningDatabase.reset();
    m_functionDatabase.reset();
    m_imageMetadataDatabase.reset();
    m_tenantDatabase.reset();
    m_nameGenerator.reset();
    m_danceDatabase.reset();
    m_spawnTypeDatabase.reset();
    m_radioMessageDatabase.reset();
    m_collectionDatabase.reset();
    m_assets.reset();
    m_configuration.reset();
  }

  m_reloadListeners.trigger();
}

void Root::loadMods(StringList modDirectories, bool _reload) {
  // Need to clear mod directories because there was an update for UGC, which have been added already as it assumes an update isn't needed.
  if (_reload)
    m_modDirectories.clear();

  MutexLocker locker(m_modsMutex);
  m_modDirectories = std::move(modDirectories);

  if (_reload)
    reload();
}

void Root::fullyLoad() {
  auto workerPool = WorkerPool("Root::fullyLoad", RootLoadThreads);
  List<WorkerPoolHandle> loaders;

  loaders.reserve(40);

  loaders.append(workerPool.addWork(swallow([this]() { return assets(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return configuration(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return codexDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return behaviorDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return techDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return aiDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return questTemplateDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return emoteProcessor(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return terrainDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return particleDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return versioningDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return functionDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return imageMetadataDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return tenantDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return nameGenerator(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return danceDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return spawnTypeDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return radioMessageDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return collectionDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return statisticsDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return speciesDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return projectileDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return stagehandDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return damageDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return effectSourceDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return statusEffectDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return treasureDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return materialDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return objectDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return npcDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return plantDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return itemDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return monsterDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return vehicleDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return playerFactory(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return entityFactory(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return biomeDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return liquidsDatabase(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return dungeonDefinitions(); })));
  loaders.append(workerPool.addWork(swallow([this]() { return tilesetDatabase(); })));

  auto startSeconds = Time::monotonicTime();
  for (auto& loader : loaders)
    loader.finish();
  Logger::info("Root: Loaded everything in {} seconds", Time::monotonicTime() - startSeconds);

  {
    RecursiveMutexLocker locker(m_loadMutex);
    if (m_assets)
      m_assets->clearCache();
  }
}

void Root::registerReloadListener(ListenerWeakPtr reloadListener) {
  m_reloadListeners.addListener(std::move(reloadListener));
}

void Root::hotReload() {
  assets()->hotReload();
  m_reloadListeners.trigger();
}

String Root::toStoragePath(String const& path) const {
  return File::relativeTo(m_settings.storageDirectory, File::convertDirSeparators(path));
}

AssetsConstPtr Root::assets() {
  return loadMemberFunction<Assets>(m_assets, m_loadMutex, "Assets", [this]() {
    StringList assetDirectories = m_settings.assetDirectories;
    assetDirectories.appendAll(m_modDirectories);
    StringList assetSources = scanForAssetSources(assetDirectories, m_settings.assetSources);

    auto assets = make_shared<Assets>(m_settings.assetsSettings, assetSources);
    Logger::info("Assets digest is {}", hexEncode(assets->digest()));
    return assets;
  });
}

ConfigurationPtr Root::configuration() {
  return loadMemberFunction<Configuration>(m_configuration, m_loadMutex, "Configuration", [this]() {
    Json currentConfig;

    if (m_runtimeConfigFile) {
      if (!File::isFile(*m_runtimeConfigFile)) {
        Logger::info("Root: no runtime config file, creating new default runtime config");
        currentConfig = m_settings.defaultConfiguration;
      } else {
        try {
          Json jConfig = Json::parseJson(File::readFileString(*m_runtimeConfigFile));
          if (!jConfig.isType(Json::Type::Object))
            throw ConfigurationException("User config is not of JSON type Object");

          if (jConfig.get("configurationVersion", {}) != m_settings.defaultConfiguration.get("configurationVersion", {}))
            throw ConfigurationException("User config version does not match default config version");

          auto config = jConfig.toObject();
          for (auto const& [configKey, defaultValue] : *m_settings.defaultConfiguration.objectPtr()) {
            if (!config.contains(configKey))
              config.insert(configKey, defaultValue);
          }

          currentConfig = config;
        } catch (std::exception const& e) {
          Logger::warn("Root: Failed to load user configuration file {}, resetting user config: {}", *m_runtimeConfigFile, outputException(e, false));
          currentConfig = m_settings.defaultConfiguration;
          File::rename(*m_runtimeConfigFile, *m_runtimeConfigFile + ".old");
        }
      }
    } else {
      currentConfig = m_settings.defaultConfiguration;
    }

    return make_shared<Configuration>(m_settings.defaultConfiguration, currentConfig);
  });
}

ObjectDatabaseConstPtr Root::objectDatabase() {
  return loadMember(m_objectDatabase, m_loadMutex, "ObjectDatabase", assets(), materialDatabase(), imageMetadataDatabase(), particleDatabase(), [this]() {
    return itemDatabase();
  }, luaRootServices());
}

PlantDatabaseConstPtr Root::plantDatabase() {
  return loadMember(m_plantDatabase, m_loadMutex, "PlantDatabase", assets(), imageMetadataDatabase());
}

ProjectileDatabaseConstPtr Root::projectileDatabase() {
  return loadMember(m_projectileDatabase, m_loadMutex, "ProjectileDatabase", assets());
}

MonsterDatabaseConstPtr Root::monsterDatabase() {
  return loadMember(m_monsterDatabase, m_loadMutex, "MonsterDatabase", assets(), liquidsDatabase(), statusEffectDatabase(), particleDatabase(), imageMetadataDatabase(), luaRootServices());
}

NpcDatabaseConstPtr Root::npcDatabase() {
  return loadMember(m_npcDatabase, m_loadMutex, "NpcDatabase", assets(), itemDatabase(), objectDatabase(), speciesDatabase(), nameGenerator(), functionDatabase(), danceDatabase(), emoteProcessor(), versioningDatabase(), liquidsDatabase(), statusEffectDatabase(), particleDatabase(), imageMetadataDatabase(), luaRootServices());
}

StagehandDatabaseConstPtr Root::stagehandDatabase() {
  return loadMember(m_stagehandDatabase, m_loadMutex, "StagehandDatabase", assets());
}

VehicleDatabaseConstPtr Root::vehicleDatabase() {
  return loadMember(m_vehicleDatabase, m_loadMutex, "VehicleDatabase", assets(), particleDatabase(), imageMetadataDatabase(), luaRootServices());
}

PlayerFactoryConstPtr Root::playerFactory() {
  return loadMemberFunction<PlayerFactory>(m_playerFactory, m_loadMutex, "PlayerFactory", [this]() {
    return make_shared<PlayerFactory>(assets(), configuration(), materialDatabase(), itemDatabase(), objectDatabase(), questTemplateDatabase(), versioningDatabase(), codexDatabase(), danceDatabase(), emoteProcessor(), radioMessageDatabase(), aiDatabase(), collectionDatabase(), speciesDatabase(), [this]() { return entityFactory(); }, liquidsDatabase(), techDatabase(), statusEffectDatabase(), particleDatabase(), imageMetadataDatabase(), luaRootServices());
  });
}

EntityFactoryConstPtr Root::entityFactory() {
  return loadMemberFunction<EntityFactory>(m_entityFactory, m_loadMutex, "EntityFactory", [this]() {
    return make_shared<EntityFactory>(assets(), playerFactory(), monsterDatabase(),
                                      objectDatabase(), projectileDatabase(), npcDatabase(), vehicleDatabase(),
                                      versioningDatabase(), itemDatabase(), imageMetadataDatabase());
  });
}

PatternedNameGeneratorConstPtr Root::nameGenerator() {
  return loadMember(m_nameGenerator, m_loadMutex, "NameGenerator", assets());
}

ItemDatabaseConstPtr Root::itemDatabase() {
  return loadMember(m_itemDatabase, m_loadMutex, "ItemDatabase", assets(), [this]() { return objectDatabase(); }, liquidsDatabase(), functionDatabase(), codexDatabase(), materialDatabase(), versioningDatabase(), particleDatabase(), imageMetadataDatabase(), luaRootServices());
}

MaterialDatabaseConstPtr Root::materialDatabase() {
  return loadMember(m_materialDatabase, m_loadMutex, "MaterialDatabase", assets(), particleDatabase(), imageMetadataDatabase());
}

TerrainDatabaseConstPtr Root::terrainDatabase() {
  return loadMember(m_terrainDatabase, m_loadMutex, "TerrainDatabase", assets());
}

BiomeDatabaseConstPtr Root::biomeDatabase() {
  return loadMember(m_biomeDatabase, m_loadMutex, "BiomeDatabase", assets(), materialDatabase(), functionDatabase(), imageMetadataDatabase(), plantDatabase());
}

LiquidsDatabaseConstPtr Root::liquidsDatabase() {
  return loadMember(m_liquidsDatabase, m_loadMutex, "LiquidsDatabase", assets(), materialDatabase());
}

StatusEffectDatabaseConstPtr Root::statusEffectDatabase() {
  return loadMember(m_statusEffectDatabase, m_loadMutex, "StatusEffectDatabase", assets());
}

DamageDatabaseConstPtr Root::damageDatabase() {
  return loadMember(m_damageDatabase, m_loadMutex, "DamageDatabase", assets());
}

ParticleDatabaseConstPtr Root::particleDatabase() {
  return loadMember(m_particleDatabase, m_loadMutex, "ParticleDatabase", assets(), imageMetadataDatabase());
}

EffectSourceDatabaseConstPtr Root::effectSourceDatabase() {
  return loadMember(m_effectSourceDatabase, m_loadMutex, "EffectSourceDatabase", assets());
}

FunctionDatabaseConstPtr Root::functionDatabase() {
  return loadMember(m_functionDatabase, m_loadMutex, "FunctionDatabase", assets());
}

TreasureDatabaseConstPtr Root::treasureDatabase() {
  return loadMember(m_treasureDatabase, m_loadMutex, "TreasureDatabase", assets(), itemDatabase(), objectDatabase());
}

DungeonDefinitionsConstPtr Root::dungeonDefinitions() {
  return loadMember(m_dungeonDefinitions, m_loadMutex, "DungeonDefinitions", assets(), tilesetDatabase());
}

TilesetDatabaseConstPtr Root::tilesetDatabase() {
  return loadMember(m_tilesetDatabase, m_loadMutex, "TilesetDatabase", assets());
}

StatisticsDatabaseConstPtr Root::statisticsDatabase() {
  return loadMember(m_statisticsDatabase, m_loadMutex, "StatisticsDatabase", assets());
}

EmoteProcessorConstPtr Root::emoteProcessor() {
  return loadMember(m_emoteProcessor, m_loadMutex, "EmoteProcessor", assets());
}

SpeciesDatabaseConstPtr Root::speciesDatabase() {
  return loadMember(m_speciesDatabase, m_loadMutex, "SpeciesDatabase", assets(), nameGenerator(), luaRootServices());
}

ImageMetadataDatabaseConstPtr Root::imageMetadataDatabase() {
  return loadMember(m_imageMetadataDatabase, m_loadMutex, "ImageMetadataDatabase", assets());
}

VersioningDatabaseConstPtr Root::versioningDatabase() {
  return loadMember(m_versioningDatabase, m_loadMutex, "VersioningDatabase", assets(), liquidsDatabase(), biomeDatabase(), [this](String const& path) { return toStoragePath(path); }, luaRootServices());
}

QuestTemplateDatabaseConstPtr Root::questTemplateDatabase() {
  return loadMember(m_questTemplateDatabase, m_loadMutex, "QuestTemplateDatabase", assets());
}

AiDatabaseConstPtr Root::aiDatabase() {
  return loadMember(m_aiDatabase, m_loadMutex, "AiDatabase", assets(), imageMetadataDatabase());
}

TechDatabaseConstPtr Root::techDatabase() {
  return loadMember(m_techDatabase, m_loadMutex, "TechDatabase", assets());
}

CodexDatabaseConstPtr Root::codexDatabase() {
  return loadMember(m_codexDatabase, m_loadMutex, "CodexDatabase", assets());
}

BehaviorDatabaseConstPtr Root::behaviorDatabase() {
  return loadMember(m_behaviorDatabase, m_loadMutex, "BehaviorDatabase", assets());
}

TenantDatabaseConstPtr Root::tenantDatabase() {
  return loadMember(m_tenantDatabase, m_loadMutex, "TenantDatabase", assets());
}

DanceDatabaseConstPtr Root::danceDatabase() {
  return loadMember(m_danceDatabase, m_loadMutex, "DanceDatabase", assets());
}

SpawnTypeDatabaseConstPtr Root::spawnTypeDatabase() {
  return loadMember(m_spawnTypeDatabase, m_loadMutex, "SpawnTypeDatabase", assets());
}

RadioMessageDatabaseConstPtr Root::radioMessageDatabase() {
  return loadMember(m_radioMessageDatabase, m_loadMutex, "RadioMessageDatabase", assets());
}

CollectionDatabaseConstPtr Root::collectionDatabase() {
  return loadMember(m_collectionDatabase, m_loadMutex, "CollectionDatabase", assets(), monsterDatabase(), itemDatabase());
}

Root::Settings& Root::settings() {
  return m_settings;
}

StringList Root::scanForAssetSources(StringList const& directories, StringList const& manual) {
  struct AssetSource {
    String path;
    Maybe<String> name;
    Maybe<String> version;
    float priority;
    StringList requires_;
    StringList includes;
  };
  List<shared_ptr<AssetSource>> assetSources;
  StringMap<shared_ptr<AssetSource>> namedSources;

  auto processEntry = [&](String const& sourcePath, bool isDirectory) -> bool {
    AssetSourcePtr source;
    auto name = File::baseName(sourcePath);
    if (name.beginsWith(".") || name.beginsWith("_"))
      Logger::info("Root: Skipping hidden '{}' in asset directory", name);
    else if (isDirectory)
      source = make_shared<DirectoryAssetSource>(sourcePath);
    else if (sourcePath.endsWith(".pak"))
      source = make_shared<PackedAssetSource>(sourcePath);
    else
      Logger::warn("Root: Unrecognized file in asset directory '{}', skipping", name);

    if (!source)
      return false;

    auto metadata = source->metadata();

    auto assetSource = make_shared<AssetSource>();
    assetSource->path = sourcePath;
    assetSource->name = metadata.maybe("name").apply(mem_fn(&Json::toString));
    assetSource->version = metadata.maybe("version").apply(mem_fn(&Json::printString));
    assetSource->priority = metadata.value("priority", 0.0f).toFloat();
    assetSource->requires_ = jsonToStringList(metadata.value("requires", JsonArray{}));
    assetSource->includes = jsonToStringList(metadata.value("includes", JsonArray{}));

    if (assetSource->name.value() == "opensb_base" && assetSource->version.value() != OpenStarVersionString) {
      throw AssetSourceException(strf("\n\nOpenStarbound assets version mismatch!\nOpenStarbound v{}, but opensb.pak is v{}\n",
                                      OpenStarVersionString, assetSource->version.value()),
                                 false);
    }

    if (assetSource->name) {
      if (auto oldAssetSource = namedSources.value(*assetSource->name)) {
        if (oldAssetSource->priority <= assetSource->priority) {
          Logger::warn("Root: Overriding duplicate asset source '{}' named '{}' with higher or equal priority source '{}",
                       oldAssetSource->path, *assetSource->name, assetSource->path);
          *oldAssetSource = *assetSource;
        } else {
          Logger::warn("Root: Skipping duplicate asset source '{}' named '{}', previous source '{}' has higher priority",
                       assetSource->path, *assetSource->name, oldAssetSource->priority);
        }
      } else {
        namedSources[*assetSource->name] = assetSource;
        assetSources.append(std::move(assetSource));
      }
    } else {
      assetSources.append(std::move(assetSource));
    }

    return true;
  };

  // Scan for assets in each given directory, the first-level ordering of asset
  // sources comes from the scanning order here, and then alphabetically by the
  // file / directory name

  for (auto const& directory : directories) {
    if (!File::isDirectory(directory)) {
      Logger::info("Root: Skipping asset directory '{}', directory not found", directory);
      continue;
    }

    Logger::info("Root: Scanning for asset sources in directory '{}'", directory);
    for (auto const& [assetName, isDirectory] : File::dirList(directory, true).sorted())
      processEntry(File::relativeTo(directory, assetName), isDirectory);
  }

  // Take in any manual asset source paths

  for (auto& path : manual)
    processEntry(path, File::isDirectory(path));

  // Then, order asset sources so that lower priority assets come before higher
  // priority ones

  assetSources.sort([](auto const& a, auto const& b) {
    return a->priority == b->priority ? a->name.value(a->path) < b->name.value(b->path) : a->priority < b->priority;
  });

  // Finally, sort asset sources so that sources that have dependencies come
  // after their dependencies.

  HashSet<shared_ptr<AssetSource>> workingSet;
  OrderedHashSet<shared_ptr<AssetSource>> dependencySortedSources;

  function<void(shared_ptr<AssetSource>)> dependencySortVisit;
  dependencySortVisit = [&](shared_ptr<AssetSource> source) {
    if (workingSet.contains(source))
      throw AssetSourceException("Asset dependencies form a cycle");

    if (dependencySortedSources.contains(source))
      return;

    workingSet.add(source);

    for (auto const& includeName : source->includes) {
      if (auto include = namedSources.ptr(includeName))
        dependencySortVisit(*include);
    }

    for (auto const& requirementName : source->requires_) {
      if (auto requirement = namedSources.ptr(requirementName))
        dependencySortVisit(*requirement);
      else
        throw AssetSourceException(strf("Asset source '{}' is missing dependency '{}'{}", source->name ? *source->name : "<unnamed>", requirementName,
                                        requirementName != "base" ? "" : "\n\nThe base Starbound asset package could not be found, please copy it from another Starbound install!\n"
                                                                         "(Locate 'packed.pak' in vanilla Starbound's assets folder, then copy it to OpenStarbound's assets folder.)\n"),
                                   false);
    }

    workingSet.remove(source);

    dependencySortedSources.add(std::move(source));
  };

  for (auto source : assetSources)
    dependencySortVisit(std::move(source));

  StringList sourcePaths;
  for (auto const& source : dependencySortedSources) {
    auto path = File::convertDirSeparators(source->path);
    if (source->name)
      Logger::info("Root: Detected asset source named '{}'{} at '{}'", *source->name, source->version ? strf(" version '{}'", *source->version) : "", path);
    else
      Logger::info("Root: Detected unnamed asset source at '{}'", path);
    sourcePaths.append(path);
  }

  return sourcePaths;
}

void Root::writeConfig() {
  if (m_configuration) {
    auto currentConfig = m_configuration->currentConfiguration();
    if (m_lastRuntimeConfig != currentConfig) {
      if (m_runtimeConfigFile) {
        Logger::info("Root: Writing runtime configuration to '{}'", *m_runtimeConfigFile);
        File::overwriteFileWithRename(m_configuration->printConfiguration(), *m_runtimeConfigFile);
      }
      m_lastRuntimeConfig = currentConfig;
    }
  }
}

LuaRootServices Root::luaRootServices() {
  return LuaRootServices{
    observer_ptr<Root>(this),
    assets(),
    configuration(),
    [this](ListenerWeakPtr reloadListener) { registerReloadListener(std::move(reloadListener)); },
    toStoragePath("lua")};
}

template <typename T, typename... Params>
shared_ptr<T> Root::loadMember(shared_ptr<T>& ptr, RecursiveMutex& mutex, char const* name, Params&&... params) {
  return loadMemberFunction<T>(ptr, mutex, name, [&]() {
    return make_shared<T>(std::forward<Params>(params)...);
  });
}

template <typename T>
shared_ptr<T> Root::loadMemberFunction(shared_ptr<T>& ptr, RecursiveMutex& mutex, char const* name, function<shared_ptr<T>()> loadFunction) {
  RecursiveMutexLocker locker(mutex);
  if (!ptr) {
    auto startSeconds = Time::monotonicTime();
    ptr = loadFunction();
    Logger::info("Root: Loaded {} in {} seconds", name, Time::monotonicTime() - startSeconds);
  }
  return ptr;
}

}// namespace Star

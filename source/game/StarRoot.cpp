import star.json;
import star.json_extra;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.variant;
import star.strong_typedef;
import star.rpc_promise;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.file;
import star.string;
import star.byte_array;
import star.encode;
import std;
import star.format;
import star.version;
import star.parametric_function;
import star.multi_array_interpolator;
import star.multi_table;
import star.weighted_pool;
import star.logging;
import star.io_device;
import star.json_builder;
import star.set;
import star.color;
import star.ref_ptr;
import star.listener;
import star.lua;
import star.audio;
import star.map;
import star.periodic_function;
import star.periodic;
import star.ordered_set;
import star.either;
import star.image;

import star.lua_root;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.drawable;
import star.item;
import star.item_descriptor;
import star.item_recipe;
import star.collision_block;
import star.assets;
import star.root_base;
import star.root;
import star.configuration;
import star.scripted_entity;
import star.status_effect_entity;
import star.animation;
import star.particle;
import star.lua_components;
import star.effect_emitter;
import star.projectile;
import star.mixer;
import star.entity_rendering_types;
import star.entity_rendering;
import star.animated_part_set;
import star.networked_animator;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.aggressive_entity;
import star.chat_action;
import star.chatty_entity;
import star.lua_animation_component;
import star.lua_actor_movement_component;
import star.actor_entity;
import star.monster_database;
import star.monster;
import star.humanoid;
import star.emote_entity;
import star.lounging_entities;
import star.tool_user_entity;
import star.npc_database;
import star.npc;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.object;
import star.plant;
import star.mobile_entity;
import star.uuid;
import star.inventory_types;
import star.ai_types;
import star.player_types;
import star.player;
import star.item_drop;
import star.dungeon_generator;
import star.asset_source;
import star.directory_asset_source;
import star.worker_pool;
import star.vehicle;
import star.player_factory;
import star.entity_factory;
import star.stored_functions;
import star.treasure;
import star.name_generator;

import star.terrain_database;
import star.liquids_database;
import star.vehicle_database;
import star.tileset_database;
import star.species_database;
import star.projectile_database;
import star.ai_database;
import star.behavior_database;
import star.packed_asset_source;
import star.dance_database;
import star.tech_database;
import star.codex_database;
import star.collection_database;
import star.stagehand_database;
import star.emote_processor;
import star.damage_database;
import star.effect_source_database;
import star.particle_database;
import star.radio_message_database;
import star.status_effect_database;
import star.quest_template_database;
import star.spawn_type_database;
import star.tenant_database;
import star.statistics_database;
import star.versioning_database;
import star.item_database;
import star.material_database;
import star.biome_database;
import star.image_metadata_database;
import star.object_database;
import star.plant_database;


import star.plant_drop;

import star.algorithm;
namespace Star {

namespace {
  unsigned const RootMaintenanceSleep = 5000;
  unsigned const RootLoadThreads = 4;
}

Root::Root(Settings settings) : m_identity(m_random), m_workers("Root CPU") {
  m_settings = std::move(settings);
  if (m_settings.runtimeConfigFile)
    m_runtimeConfigFile = toStoragePath(*m_settings.runtimeConfigFile);

  if (!File::isDirectory(m_settings.storageDirectory))
    File::makeDirectory(m_settings.storageDirectory);

  if (m_settings.logFile) {
    String logFile = File::relativeTo(m_settings.logDirectory.value_or(m_settings.storageDirectory), *m_settings.logFile);
    String oldLogDirectory = m_settings.logDirectory.value_or(File::relativeTo(m_settings.storageDirectory, "logs"));
    if (!File::isDirectory(oldLogDirectory))
      File::makeDirectory(oldLogDirectory);

    File::backupFileInSequence(logFile, File::relativeTo(oldLogDirectory, *m_settings.logFile), m_settings.logFileBackups);
    m_logger.addSink(std::make_shared<FileLogSink>(logFile, m_settings.logLevel, true));
  }
  m_logger.stdoutSink()->setLevel(m_settings.logLevel);

  if (m_settings.quiet)
    m_logger.removeStdoutSink();

  m_logger.info("Root: Preparing...");

  m_stopMaintenanceThread = false;
  m_maintenanceThread = Thread::invoke("Root::maintenanceMain", [this]() {
      MutexLocker locker(m_maintenanceStopMutex);
      while (!m_stopMaintenanceThread) {
        m_reloadListeners.clearExpiredListeners();

        {
          MutexLocker locker(m_objectDatabaseMutex);
          if (std::shared_ptr<ObjectDatabase> objectDb = m_objectDatabase) {
            locker.unlock();
            objectDb->cleanup();
          }
        }
        {
          MutexLocker locker(m_itemDatabaseMutex);
          if (std::shared_ptr<ItemDatabase> itemDb = m_itemDatabase) {
            locker.unlock();
            itemDb->cleanup();
          }
        }
        {
          MutexLocker locker(m_monsterDatabaseMutex);
          if (std::shared_ptr<MonsterDatabase> monsterDb = m_monsterDatabase) {
            locker.unlock();
            monsterDb->cleanup();
          }
        }
        {
          MutexLocker locker(m_assetsMutex);
          if (std::shared_ptr<Assets> assets = m_assets) {
            locker.unlock();
            assets->cleanup();
          }
        }
        {
          MutexLocker locker(m_tenantDatabaseMutex);
          if (std::shared_ptr<TenantDatabase> tenantDb = m_tenantDatabase) {
            locker.unlock();
            tenantDb->cleanup();
          }
        }
        {
          MutexLocker locker(m_imageMetadataDatabaseMutex);
          if (std::shared_ptr<ImageMetadataDatabase> imgMetaDb = m_imageMetadataDatabase) {
            locker.unlock();
            imgMetaDb->cleanup();
          }
        }

        m_random.addEntropy();

        {
          MutexLocker locker(m_configurationMutex);
          writeConfig();
        }

        m_maintenanceStopCondition.wait(m_maintenanceStopMutex, RootMaintenanceSleep);
      }
    });

  m_logger.info("Root: Done preparing Root.");
}

Root::~Root() {
  m_logger.info("Root: Shutting down Root");

  {
    MutexLocker locker(m_maintenanceStopMutex);
    m_stopMaintenanceThread = true;
    m_maintenanceStopCondition.signal();
  }
  m_maintenanceThread.finish();
  m_workers.finish();

  m_reloadListeners.clearAllListeners();

  writeConfig();

}

void Root::reload() {
  m_logger.info("Root: Reloading from disk");

  {
    // We need to lock all the mutexes to reset everything to cause it to be
    // reloaded, but whenever we lock individual members we should always do it
    // in the same order (well, the same order*ing* not necessarily the same
    // order) to avoid deadlocks.  This means that we need to enumerate the
    // finicky, implicit dependency order that we have due to each member's
    // constructor referencing root recursively.  We could avoid doing this
    // explicitly with C++11's std::lock (if c++11 threading primitives were
    // finally reliable on all targets), or some other equivalent deadlock
    // avoidance algorithm.

    // Entity factory depends on all the entity databases and the versioning
    // database.
    MutexLocker entityFactoryLock(m_entityFactoryMutex);

    // Species database depends on the item database.
    MutexLocker speciesDatabaseLock(m_speciesDatabaseMutex);

    // Item database depends on object database and codex database
    MutexLocker itemDatabaseLock(m_itemDatabaseMutex);

    // These databases depend on various things below, but not the item database
    MutexLocker objectDatabaseLock(m_objectDatabaseMutex);
    MutexLocker playerFactoryLock(m_playerFactoryMutex);
    MutexLocker npcDatabaseLock(m_npcDatabaseMutex);
    MutexLocker stagehandDatabaseLock(m_stagehandDatabaseMutex);
    MutexLocker vehicleDatabaseLock(m_vehicleDatabaseMutex);
    MutexLocker monsterDatabaseLock(m_monsterDatabaseMutex);
    MutexLocker plantDatabaseLock(m_plantDatabaseMutex);
    MutexLocker projectileDatabaseLock(m_projectileDatabaseMutex);

    // Biome database depends on liquids, materials, and stored function
    // databases.
    MutexLocker biomeDatabaseLock(m_biomeDatabaseMutex);

    // Dungeon definitions database depends on the material and liquids database
    MutexLocker dungeonDefinitionsLock(m_dungeonDefinitionsMutex);
    MutexLocker tilesetDatabaseLock(m_tilesetDatabaseMutex);

    MutexLocker statisticsDatabaseLock(m_statisticsDatabaseMutex);

    // Liquids database depends on the materials database
    MutexLocker liquidsDatabaseLock(m_liquidsDatabaseMutex);

    // Material database depends on particle database
    MutexLocker materialDatabaseLock(m_materialDatabaseMutex);

    // Databases that depend on functions database.
    MutexLocker damageDatabaseLock(m_damageDatabaseMutex);
    MutexLocker effectSourceDatabaseLock(m_effectSourceDatabaseMutex);
    MutexLocker statusEffectDatabaseLock(m_statusEffectDatabaseMutex);
    MutexLocker treasureDatabaseLock(m_treasureDatabaseMutex);

    // Databases that don't depend on anything other than assets
    MutexLocker codexDatabaseLock(m_codexDatabaseMutex);
    MutexLocker behaviorDatabaseMutex(m_behaviorDatabaseMutex);
    MutexLocker techDatabaseLock(m_techDatabaseMutex);
    MutexLocker aiDatabaseLock(m_aiDatabaseMutex);
    MutexLocker questTemplateDatabaseLock(m_questTemplateDatabaseMutex);
    MutexLocker emoteProcessorLock(m_emoteProcessorMutex);
    MutexLocker terrainDatabaseLock(m_terrainDatabaseMutex);
    MutexLocker particleDatabaseLock(m_particleDatabaseMutex);
    MutexLocker versioningDatabaseLock(m_versioningDatabaseMutex);
    MutexLocker functionDatabaseLock(m_functionDatabaseMutex);
    MutexLocker imageMetadataDatabaseLock(m_imageMetadataDatabaseMutex);
    MutexLocker tenantDatabaseLock(m_tenantDatabaseMutex);
    MutexLocker nameGeneratorLock(m_nameGeneratorMutex);
    MutexLocker danceDatabaseLock(m_danceDatabaseMutex);
    MutexLocker spawnTypeDatabaseLock(m_spawnTypeDatabaseMutex);
    MutexLocker radioMessageDatabaseLock(m_radioMessageDatabaseMutex);
    MutexLocker collectionDatabaseLock(m_collectionDatabaseMutex);

    // Configuration and Assets are at the very bottom of the hierarchy.
    MutexLocker configurationLock(m_configurationMutex);
    MutexLocker assetsLock(m_assetsMutex);

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

  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::assets, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::configuration, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::codexDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::behaviorDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::techDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::aiDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::questTemplateDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::emoteProcessor, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::terrainDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::particleDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::versioningDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::functionDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::imageMetadataDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::tenantDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::nameGenerator, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::danceDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::spawnTypeDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::radioMessageDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::collectionDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::statisticsDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::speciesDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::projectileDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::stagehandDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::damageDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::effectSourceDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::statusEffectDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::treasureDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::materialDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::objectDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::npcDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::plantDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::itemDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::monsterDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::vehicleDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::playerFactory, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::entityFactory, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::biomeDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::liquidsDatabase, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::dungeonDefinitions, this))));
  loaders.append(workerPool.addWork(swallow(std::bind_front(&Root::tilesetDatabase, this))));

  auto startSeconds = Time::monotonicTime();
  for (auto& loader : loaders)
    loader.finish();
  m_logger.info("Root: Loaded everything in {} seconds", Time::monotonicTime() - startSeconds);

  {
    MutexLocker locker(m_assetsMutex);
    if (m_assets)
      m_assets->clearCache();
  }
}

ListenerSubscription Root::registerReloadListener(std::weak_ptr<Listener> reloadListener) {
  return m_reloadListeners.addListener(std::move(reloadListener));
}

void Root::hotReload() {
  assets()->hotReload();
  m_reloadListeners.trigger();
}

String Root::toStoragePath(String const& path) const {
  return File::relativeTo(m_settings.storageDirectory, File::convertDirSeparators(path));
}

std::shared_ptr<Assets const> Root::assets() {
  return loadMemberFunction<Assets>(m_assets, m_assetsMutex, "Assets", [this]() {
      StringList assetDirectories = m_settings.assetDirectories;
      assetDirectories.appendAll(m_modDirectories);
      StringList assetSources = scanForAssetSources(assetDirectories, m_settings.assetSources);

      auto assets = std::make_shared<Assets>(m_logger, m_random, m_logMap, m_settings.assetsSettings, assetSources);
      m_logger.info("Assets digest is {}", hexEncode(assets->digest()));
      return assets;
    });
}

std::shared_ptr<Configuration> Root::configuration() {
  return loadMemberFunction<Configuration>(m_configuration, m_configurationMutex, "Configuration", [this]() {
      Json currentConfig;

      if (m_runtimeConfigFile) {
        if (!File::isFile(*m_runtimeConfigFile)) {
          m_logger.info("Root: no runtime config file, creating new default runtime config");
          currentConfig = m_settings.defaultConfiguration;
        } else {
          try {
            Json jConfig = Json::parseJson(File::readFileString(*m_runtimeConfigFile));
            if (!jConfig.isType(Json::Type::Object))
              throw ConfigurationException("User config is not of JSON type Object");

            if (jConfig.get("configurationVersion", {}) != m_settings.defaultConfiguration.get("configurationVersion", {}))
              throw ConfigurationException("User config version does not match default config version");

            auto config = jConfig.toObject();
            for (auto& entry : *m_settings.defaultConfiguration.objectPtr()) {
              if (!config.contains(entry.first))
                config.insert(entry.first, entry.second);
            }

            currentConfig = config;
          } catch (std::exception const&) {
            ExceptionDiagnostic const e(std::current_exception());
            m_logger.warn("Root: Failed to load user configuration file {}, resetting user config: {}", *m_runtimeConfigFile, outputException(e, false));
            currentConfig = m_settings.defaultConfiguration;
            File::rename(*m_runtimeConfigFile, *m_runtimeConfigFile + ".old");
          }
        }
      } else {
        currentConfig = m_settings.defaultConfiguration;
      }

      return std::make_shared<Configuration>(m_settings.defaultConfiguration, currentConfig);
    });
}

std::shared_ptr<ObjectDatabase const> Root::objectDatabase() {
  return loadMember(m_objectDatabase, m_objectDatabaseMutex, "ObjectDatabase", *this);
}

std::shared_ptr<PlantDatabase const> Root::plantDatabase() {
  return loadMember(m_plantDatabase, m_plantDatabaseMutex, "PlantDatabase", *this);
}

std::shared_ptr<ProjectileDatabase const> Root::projectileDatabase() {
  return loadMember(m_projectileDatabase, m_projectileDatabaseMutex, "ProjectileDatabase", *this);
}

std::shared_ptr<MonsterDatabase const> Root::monsterDatabase() {
  return loadMember(m_monsterDatabase, m_monsterDatabaseMutex, "MonsterDatabase", *this);
}

std::shared_ptr<NpcDatabase const> Root::npcDatabase() {
  return loadMember(m_npcDatabase, m_npcDatabaseMutex, "NpcDatabase", *this);
}

std::shared_ptr<StagehandDatabase const> Root::stagehandDatabase() {
  return loadMember(m_stagehandDatabase, m_stagehandDatabaseMutex, "StagehandDatabase", *this);
}

std::shared_ptr<VehicleDatabase const> Root::vehicleDatabase() {
  return loadMember(m_vehicleDatabase, m_vehicleDatabaseMutex, "VehicleDatabase", *this);
}

std::shared_ptr<PlayerFactory const> Root::playerFactory() {
  return loadMember(m_playerFactory, m_playerFactoryMutex, "PlayerFactory", *this);
}

std::shared_ptr<EntityFactory const> Root::entityFactory() {
  return loadMember(m_entityFactory, m_entityFactoryMutex, "EntityFactory", *this);
}

std::shared_ptr<PatternedNameGenerator const> Root::nameGenerator() {
  return loadMember(m_nameGenerator, m_nameGeneratorMutex, "NameGenerator", *this);
}

std::shared_ptr<ItemDatabase const> Root::itemDatabase() {
  return loadMember(m_itemDatabase, m_itemDatabaseMutex, "ItemDatabase", *this);
}

std::shared_ptr<MaterialDatabase const> Root::materialDatabase() {
  return loadMemberFunction<MaterialDatabase>(m_materialDatabase, m_materialDatabaseMutex, "MaterialDatabase", [this] {
    return std::make_shared<MaterialDatabase>(assets(), particleDatabase(), *imageMetadataDatabase(), logger(), random());
  });
}

std::shared_ptr<TerrainDatabase const> Root::terrainDatabase() {
  return loadMemberFunction<TerrainDatabase>(m_terrainDatabase, m_terrainDatabaseMutex, "TerrainDatabase", [this] {
    return std::make_shared<TerrainDatabase>(assets(), random());
  });
}

std::shared_ptr<BiomeDatabase const> Root::biomeDatabase() {
  return loadMember(m_biomeDatabase, m_biomeDatabaseMutex, "BiomeDatabase", *this);
}

std::shared_ptr<LiquidsDatabase const> Root::liquidsDatabase() {
  return loadMemberFunction<LiquidsDatabase>(m_liquidsDatabase, m_liquidsDatabaseMutex, "LiquidsDatabase", [this] {
    return std::make_shared<LiquidsDatabase>(assets(), materialDatabase());
  });
}

std::shared_ptr<StatusEffectDatabase const> Root::statusEffectDatabase() {
  return loadMemberFunction<StatusEffectDatabase>(m_statusEffectDatabase, m_statusEffectDatabaseMutex, "StatusEffectDatabase", [this] {
    return std::make_shared<StatusEffectDatabase>(assets());
  });
}

std::shared_ptr<DamageDatabase const> Root::damageDatabase() {
  return loadMemberFunction<DamageDatabase>(m_damageDatabase, m_damageDatabaseMutex, "DamageDatabase", [this] {
    return std::make_shared<DamageDatabase>(assets());
  });
}

std::shared_ptr<ParticleDatabase const> Root::particleDatabase() {
  return loadMember(m_particleDatabase, m_particleDatabaseMutex, "ParticleDatabase", *this);
}

std::shared_ptr<EffectSourceDatabase const> Root::effectSourceDatabase() {
  return loadMember(m_effectSourceDatabase, m_effectSourceDatabaseMutex, "EffectSourceDatabase", *this);
}

std::shared_ptr<FunctionDatabase const> Root::functionDatabase() {
  return loadMemberFunction<FunctionDatabase>(m_functionDatabase, m_functionDatabaseMutex, "FunctionDatabase", [this] {
    return std::make_shared<FunctionDatabase>(assets());
  });
}

std::shared_ptr<TreasureDatabase const> Root::treasureDatabase() {
  return loadMember(m_treasureDatabase, m_treasureDatabaseMutex, "TreasureDatabase", *this);
}

std::shared_ptr<DungeonDefinitions const> Root::dungeonDefinitions() {
  return loadMember(m_dungeonDefinitions, m_dungeonDefinitionsMutex, "DungeonDefinitions", *this);
}

std::shared_ptr<TilesetDatabase const> Root::tilesetDatabase() {
  return loadMember(m_tilesetDatabase, m_tilesetDatabaseMutex, "TilesetDatabase", *this);
}

std::shared_ptr<StatisticsDatabase const> Root::statisticsDatabase() {
  return loadMember(m_statisticsDatabase, m_statisticsDatabaseMutex, "StatisticsDatabase", *this);
}

std::shared_ptr<EmoteProcessor const> Root::emoteProcessor() {
  return loadMemberFunction<EmoteProcessor>(m_emoteProcessor, m_emoteProcessorMutex, "EmoteProcessor", [this] {
    return std::make_shared<EmoteProcessor>(assets());
  });
}

std::shared_ptr<SpeciesDatabase const> Root::speciesDatabase() {
  return loadMember(m_speciesDatabase, m_speciesDatabaseMutex, "SpeciesDatabase", *this);
}

std::shared_ptr<ImageMetadataDatabase const> Root::imageMetadataDatabase() {
  return loadMember(m_imageMetadataDatabase, m_imageMetadataDatabaseMutex, "ImageMetadataDatabase", *this);
}

std::shared_ptr<VersioningDatabase const> Root::versioningDatabase() {
  return loadMember(m_versioningDatabase, m_versioningDatabaseMutex, "VersioningDatabase", *this);
}

std::shared_ptr<QuestTemplateDatabase const> Root::questTemplateDatabase() {
  return loadMemberFunction<QuestTemplateDatabase>(m_questTemplateDatabase, m_questTemplateDatabaseMutex, "QuestTemplateDatabase", [this] {
    return std::make_shared<QuestTemplateDatabase>(assets(), logger());
  });
}

std::shared_ptr<AiDatabase const> Root::aiDatabase() {
  return loadMember(m_aiDatabase, m_aiDatabaseMutex, "AiDatabase", *this);
}

std::shared_ptr<TechDatabase const> Root::techDatabase() {
  return loadMemberFunction<TechDatabase>(m_techDatabase, m_techDatabaseMutex, "TechDatabase", [this] {
    return std::make_shared<TechDatabase>(assets());
  });
}

std::shared_ptr<CodexDatabase const> Root::codexDatabase() {
  return loadMemberFunction<CodexDatabase>(m_codexDatabase, m_codexDatabaseMutex, "CodexDatabase", [this] {
    return std::make_shared<CodexDatabase>(assets());
  });
}

std::shared_ptr<BehaviorDatabase const> Root::behaviorDatabase() {
  return loadMemberFunction<BehaviorDatabase>(m_behaviorDatabase, m_behaviorDatabaseMutex, "BehaviorDatabase", [this] {
    return std::make_shared<BehaviorDatabase>(assets());
  });
}

std::shared_ptr<TenantDatabase const> Root::tenantDatabase() {
  return loadMember(m_tenantDatabase, m_tenantDatabaseMutex, "TenantDatabase", *this);
}

std::shared_ptr<DanceDatabase const> Root::danceDatabase() {
  return loadMemberFunction<DanceDatabase>(m_danceDatabase, m_danceDatabaseMutex, "DanceDatabase", [this] {
    return std::make_shared<DanceDatabase>(assets(), logger());
  });
}

std::shared_ptr<SpawnTypeDatabase const> Root::spawnTypeDatabase() {
  return loadMemberFunction<SpawnTypeDatabase>(m_spawnTypeDatabase, m_spawnTypeDatabaseMutex, "SpawnTypeDatabase", [this] {
    return std::make_shared<SpawnTypeDatabase>(assets());
  });
}

std::shared_ptr<RadioMessageDatabase const> Root::radioMessageDatabase() {
  return loadMember(m_radioMessageDatabase, m_radioMessageDatabaseMutex, "RadioMessageDatabase", *this);
}

std::shared_ptr<CollectionDatabase const> Root::collectionDatabase() {
  return loadMember(m_collectionDatabase, m_collectionDatabaseMutex, "CollectionDatabase", *this);
}

Root::Settings& Root::settings() {
  return m_settings;
}

StringList Root::scanForAssetSources(StringList const& directories, StringList const& manual) {
  struct AssetSource {
    String path;
    std::optional<String> name;
    std::optional<String> version;
    float priority;
    StringList requires_;
    StringList includes;
  };
  List<std::shared_ptr<AssetSource>> assetSources;
  StringMap<std::shared_ptr<AssetSource>> namedSources;

  auto processEntry = [&](String const& sourcePath, bool isDirectory) -> bool {
    std::shared_ptr<Star::AssetSource> source;
    auto name = File::baseName(sourcePath);
    if (name.beginsWith(".") || name.beginsWith("_"))
      m_logger.info("Root: Skipping hidden '{}' in asset directory", name);
    else if (isDirectory)
      source = std::make_shared<DirectoryAssetSource>(m_logger, sourcePath);
    else if (sourcePath.endsWith(".pak"))
      source = std::make_shared<PackedAssetSource>(sourcePath);
    else
      m_logger.warn("Root: Unrecognized file in asset directory '{}', skipping", name);

    if (!source)
      return false;

    auto metadata = source->metadata();

    auto assetSource = std::make_shared<AssetSource>();
    assetSource->path = sourcePath;
    assetSource->name = metadata.maybe("name").transform([](Json const& value) { return value.toString(); });
    assetSource->version = metadata.maybe("version").transform(std::mem_fn(&Json::printString));
    assetSource->priority = metadata.value("priority", 0.0f).toFloat();
    assetSource->requires_ = jsonToStringList(metadata.value("requires", JsonArray{}));
    assetSource->includes = jsonToStringList(metadata.value("includes", JsonArray{}));

    if (assetSource->name.value_or(String{}) == "opensb_base" && assetSource->version.value_or(String{}) != OpenStarVersionString) {
      throw AssetSourceException(strf("\n\nOpenStarbound assets version mismatch!\nOpenStarbound v{}, but opensb.pak is v{}\n",
        OpenStarVersionString, assetSource->version.value_or(String{})), false);
    }

    if (assetSource->name) {
      if (auto oldAssetSource = namedSources.value(*assetSource->name)) {
        if (oldAssetSource->priority <= assetSource->priority) {
          m_logger.warn("Root: Overriding duplicate asset source '{}' named '{}' with higher or equal priority source '{}",
              oldAssetSource->path, *assetSource->name, assetSource->path);
          *oldAssetSource = *assetSource;
        } else {
          m_logger.warn("Root: Skipping duplicate asset source '{}' named '{}', previous source '{}' has higher priority",
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
      m_logger.info("Root: Skipping asset directory '{}', directory not found", directory);
      continue;
    }

    m_logger.info("Root: Scanning for asset sources in directory '{}'", directory);
    for (auto& entry : File::dirList(directory, true).sorted())
      processEntry(File::relativeTo(directory, entry.first), entry.second);
  }

  // Take in any manual asset source paths

  for (auto& path : manual)
    processEntry(path, File::isDirectory(path));

  // Then, order asset sources so that lower priority assets come before higher
  // priority ones

  assetSources.sort([](auto const& a, auto const& b) {
      return 
        a->priority == b->priority ? 
          a->name.value_or(a->path) < b->name.value_or(b->path) : 
          a->priority < b->priority;
    });

  // Finally, sort asset sources so that sources that have dependencies come
  // after their dependencies.

  HashSet<std::shared_ptr<AssetSource>> workingSet;
  OrderedHashSet<std::shared_ptr<AssetSource>> dependencySortedSources;

  std::function<void(std::shared_ptr<AssetSource>)> dependencySortVisit;
  dependencySortVisit = [&](std::shared_ptr<AssetSource> source) {
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
          requirementName != "base" ? "" :
            "\n\nThe base Starbound asset package could not be found, please copy it from another Starbound install!\n"
            "(Locate 'packed.pak' in vanilla Starbound's assets folder, then copy it to OpenStarbound's assets folder.)\n"), false);
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
      m_logger.info("Root: Detected asset source named '{}'{} at '{}'", *source->name, source->version ? strf(" version '{}'", *source->version) : "", path);
    else
      m_logger.info("Root: Detected unnamed asset source at '{}'", path);
    sourcePaths.append(path);
  }

  return sourcePaths;
}

void Root::writeConfig() {
  if (m_configuration) {
    auto currentConfig = m_configuration->currentConfiguration();
    if (m_lastRuntimeConfig != currentConfig) {
      if (m_runtimeConfigFile) {
        m_logger.info("Root: Writing runtime configuration to '{}'", *m_runtimeConfigFile);
        File::overwriteFileWithRename(m_configuration->printConfiguration(), *m_runtimeConfigFile);
      }
      m_lastRuntimeConfig = currentConfig;
    }
  }
}

WorkerPool& Root::workers() {
  std::call_once(m_workersStarted, [this] {
    m_workers.start(std::max(1u, std::thread::hardware_concurrency()));
  });
  return m_workers;
}

bool& Root::humanoidHeadRotation() {
  std::call_once(m_headRotationInitialized, [this] {
    m_humanoidHeadRotation = configuration()->get("humanoidHeadRotation").optBool().value_or(true);
  });
  return m_humanoidHeadRotation;
}

template <typename T, typename... Params>
std::shared_ptr<T> Root::loadMember(std::shared_ptr<T>& ptr, Mutex& mutex, char const* name, Params&&... params) {
  return loadMemberFunction<T>(ptr, mutex, name, [&]() {
      return std::make_shared<T>(std::forward<Params>(params)...);
    });
}

template <typename T>
std::shared_ptr<T> Root::loadMemberFunction(std::shared_ptr<T>& ptr, Mutex& mutex, char const* name, std::function<std::shared_ptr<T>()> loadFunction) {
  MutexLocker locker(mutex);
  if (!ptr) {
    auto startSeconds = Time::monotonicTime();
    ptr = loadFunction();
    m_logger.info("Root: Loaded {} in {} seconds", name, Time::monotonicTime() - startSeconds);
  }
  return ptr;
}

}

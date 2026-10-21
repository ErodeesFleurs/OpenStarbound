import star.memory;
import std;
import star.xx_hash;
import star.json;
import star.ordered_map;
import star.bi_map;
import star.data_stream;
import star.thread;
import star.byte_array;
import star.id_map;
import star.set;
import star.btree;
import star.lru_cache;
import star.data_stream_devices;
import star.btree_database;
import star.casting;
import star.ordered_set;
import star.list;
import star.game_types;
import star.rpc_promise;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.strong_typedef;
import star.worker_pool;

import star.sector_array_2d;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.file;
import star.logging;
import star.time;
import star.format;
import star.version;
import star.a_star;
import star.audio;
import star.map;
import star.string;
import star.either;
import star.periodic_function;
import star.io_device;
import star.ref_ptr;
import star.listener;
import star.text;

import star.lua_root;
import star.uuid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.interaction_types;
import star.item_descriptor;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.world_storage;
import star.configuration;
import star.drawable;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.tile_modification;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;

import star.versioning_database;
import star.player_storage;
import star.player_factory;
import star.entity_factory;

namespace Star {

PlayerStorage::PlayerStorage(Root& root, String const& storageDir) : m_root(root) {
  m_storageDirectory = storageDir;
  m_backupDirectory = File::relativeTo(m_storageDirectory, "backup");
  if (!File::isDirectory(m_storageDirectory)) {
    m_root.logger().info("Creating player storage directory");
    File::makeDirectory(m_storageDirectory);
    return;
  }

  auto configuration = m_root.configuration();
  if (configuration->get("clearPlayerFiles").toBool()) {
    m_root.logger().info("Clearing all player files");
    for (auto file : File::dirList(m_storageDirectory)) {
      if (!file.second)
        File::remove(File::relativeTo(m_storageDirectory, file.first));
    }
  } else {
    auto versioningDatabase = m_root.versioningDatabase();
    auto entityFactory = m_root.entityFactory();

    for (auto file : File::dirList(m_storageDirectory)) {
      if (file.second)
        continue;

      String filename = File::relativeTo(m_storageDirectory, file.first);
      if (filename.endsWith(".player")) {
        try {
          auto json = VersionedJson::readFile(filename);
          Uuid uuid(json.content.getString("uuid"));
          if (m_playerFileNames.insert(uuid, file.first.rsplit('.', 1).at(0))) {
            auto& playerCacheData = m_savedPlayersCache[uuid];
            playerCacheData = entityFactory->loadVersionedJson(json, EntityType::Player);
          } else {
            m_root.logger().warn("Duplicate player? Skipping player file {} because it has the same UUID as {}.player ({})", file.first, m_playerFileNames.getRight(uuid), uuid.hex());
          }
        } catch (std::exception const&) {
          ExceptionDiagnostic const e(std::current_exception());
          m_root.logger().error("Error loading player file, ignoring! {} : {}", filename, outputException(e, false));
        }
      }
    }

    // Remove all the player entries that are missing player data or fail to
    // load.
    for (auto it = m_savedPlayersCache.begin(); it != m_savedPlayersCache.end();) {
      auto& entry = *it;
      if (entry.second.isNull()) {
        it = m_savedPlayersCache.erase(it);
      } else {
        try {
          auto entityFactory = m_root.entityFactory();
          auto player = as<Player>(entityFactory->diskLoadEntity(EntityType::Player, entry.second));
          if (player->uuid() != entry.first)
            throw PlayerException(strf("Uuid mismatch in loaded player with filename uuid '{}'", entry.first.hex()));
        } catch (StarException const& e) {
          auto& fileName = uuidFileName(entry.first);
          String uuidHex = entry.first.hex();
          if (uuidHex == fileName)
            m_root.logger().error("Failed to validate player with uuid {} : {}", uuidHex, outputException(e, true));
          else
            m_root.logger().error("Failed to validate player with uuid {} ({}.player) : {}", uuidHex, fileName, outputException(e, true));
          it = m_savedPlayersCache.erase(it);
          continue;
        }
        ++it;
      }
    }
  }

  try {
    String filename = File::relativeTo(m_storageDirectory, "metadata");
    m_metadata = Json::parseJson(File::readFileString(filename)).toObject();

    if (auto order = m_metadata.value("order")) {
      for (auto const& jUuid : order.iterateArray()) {
        auto entry = m_savedPlayersCache.find(Uuid(jUuid.toString()));
        if (entry != m_savedPlayersCache.end())
          m_savedPlayersCache.toBack(entry);
      }
    }
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    m_root.logger().warn("Error loading player storage metadata file, resetting: {}", outputException(e, false));
  }
}

PlayerStorage::~PlayerStorage() {
  writeMetadata();
}

std::size_t PlayerStorage::playerCount() const {
  RecursiveMutexLocker locker(m_mutex);
  return m_savedPlayersCache.size();
}

std::optional<Uuid> PlayerStorage::playerUuidAt(std::size_t index) {
  RecursiveMutexLocker locker(m_mutex);
  if (index < m_savedPlayersCache.size())
    return m_savedPlayersCache.keyAt(index);
  else
    return {};
}

std::optional<Uuid> PlayerStorage::playerUuidByName(String const& name, std::optional<Uuid> except) {
  String cleanMatch = Text::stripEscapeCodes(name).toLower();
  std::optional<Uuid> uuid;

  RecursiveMutexLocker locker(m_mutex);

  std::size_t longest = std::numeric_limits<std::size_t>::max();
  for (auto& cache : m_savedPlayersCache) {
    if (except && *except == cache.first)
      continue;
    else if (auto name = cache.second.optQueryString("identity.name")) {
      auto cleanName = Text::stripEscapeCodes(*name).toLower();
      auto len = cleanName.size();
      if (len < longest && cleanName.utf8().rfind(cleanMatch.utf8()) == 0) {
        longest = len;
        uuid = cache.first;
      }
    }
  }

  return uuid;
}

List<Uuid> PlayerStorage::playerUuidListByName(String const& name, std::optional<Uuid> except) {
  String cleanMatch = Text::stripEscapeCodes(name).toLower();
  List<Uuid> list = {};

  RecursiveMutexLocker locker(m_mutex);

  for (auto& cache : m_savedPlayersCache) {
    if (except && *except == cache.first)
      continue;
    else if (auto name = cache.second.optQueryString("identity.name")) {
      auto cleanName = Text::stripEscapeCodes(*name).toLower();
      if (cleanMatch == "" || cleanName.utf8().rfind(cleanMatch.utf8()) != NPos) {
        list.append(cache.first);
      }
    }
  }

  return list;
}


Json PlayerStorage::savePlayer(std::shared_ptr<Player> const& player) {
  auto entityFactory = m_root.entityFactory();
  auto versioningDatabase = m_root.versioningDatabase();

  RecursiveMutexLocker locker(m_mutex);

  auto uuid = player->uuid();

  auto& playerCacheData = m_savedPlayersCache[uuid];
  if (!m_playerFileNames.hasLeftValue(uuid))
    m_playerFileNames.insert(uuid, uuid.hex());
  auto newPlayerData = player->diskStore();
  if (playerCacheData != newPlayerData) {
    playerCacheData = newPlayerData;
    VersionedJson versionedJson = entityFactory->storeVersionedJson(EntityType::Player, playerCacheData);
    auto fileName = strf("{}.player", uuidFileName(uuid));
    VersionedJson::writeFile(versionedJson, File::relativeTo(m_storageDirectory, fileName));
    m_root.logger().debug("Saved player {} to {}", Text::stripEscapeCodes(player->name()), fileName);
  }
  return newPlayerData;
}

std::optional<Json> PlayerStorage::maybeGetPlayerData(Uuid const& uuid) {
  RecursiveMutexLocker locker(m_mutex);
  if (auto cache = m_savedPlayersCache.ptr(uuid))
    return *cache;
  else
    return {};
}

Json PlayerStorage::getPlayerData(Uuid const& uuid) {
  auto data = maybeGetPlayerData(uuid);
  if (!data)
    throw PlayerException(strf("No such stored player with uuid '{}'", uuid.hex()));
  else
    return *data;
}

std::shared_ptr<Player> PlayerStorage::loadPlayer(Uuid const& uuid) {
  auto playerCacheData = getPlayerData(uuid);
  auto entityFactory = m_root.entityFactory();
  try {
    auto player = convert<Player>(entityFactory->diskLoadEntity(EntityType::Player, playerCacheData));
    if (player->uuid() != uuid)
      throw PlayerException(strf("Uuid mismatch in loaded player with filename uuid '{}'", uuid.hex()));
    return player;
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    m_root.logger().error("Error loading player file, ignoring! {}", outputException(e, false));
    RecursiveMutexLocker locker(m_mutex);
    m_savedPlayersCache.remove(uuid);
    return {};
  }
}

void PlayerStorage::deletePlayer(Uuid const& uuid) {
  RecursiveMutexLocker locker(m_mutex);
  if (!m_savedPlayersCache.contains(uuid))
    throw PlayerException(strf("No such stored player with uuid '{}'", uuid.hex()));

  m_savedPlayersCache.remove(uuid);

  auto uuidHex = uuid.hex();
  auto storagePrefix = File::relativeTo(m_storageDirectory, uuidHex);
  auto backupPrefix = File::relativeTo(m_backupDirectory, uuidHex);

  auto removeIfExists = [](String const& prefix, String const& suffix) {
    if (File::exists(prefix + suffix)) {
      File::remove(prefix + suffix);
    }
  };

  removeIfExists(storagePrefix, ".player");
  removeIfExists(storagePrefix, ".shipworld");

  auto configuration = m_root.configuration();
  unsigned playerBackupFileCount = configuration->get("playerBackupFileCount").toUInt();

  for (unsigned i = 1; i <= playerBackupFileCount; ++i) {
    removeIfExists(backupPrefix, strf(".player.bak{}", i));
    removeIfExists(backupPrefix, strf(".shipworld.bak{}", i));
  }
}

WorldChunks PlayerStorage::loadShipData(Uuid const& uuid) {
  RecursiveMutexLocker locker(m_mutex);
  if (!m_savedPlayersCache.contains(uuid))
    throw PlayerException(strf("No such stored player with uuid '{}'", uuid.hex()));

  String filename = File::relativeTo(m_storageDirectory, strf("{}.shipworld", uuidFileName(uuid)));
  try {
    if (File::exists(filename))
      return WorldStorage::getWorldChunksFromFile(m_root.logger(), filename);
  } catch (StarException const& e) {
    m_root.logger().error("Failed to load shipworld file, removing {} : {}", filename, outputException(e, false));
    File::remove(filename);
  }

  return {};
}

void PlayerStorage::applyShipUpdates(Uuid const& uuid, WorldChunks const& updates) {
  RecursiveMutexLocker locker(m_mutex);
  if (!m_savedPlayersCache.contains(uuid))
    throw PlayerException(strf("No such stored player with uuid '{}'", uuid.hex()));

  if (updates.empty())
    return;
  String filePath = File::relativeTo(m_storageDirectory, strf("{}.shipworld", uuidFileName(uuid)));
  WorldStorage::applyWorldChunksUpdateToFile(m_root.logger(), filePath, updates);
}

void PlayerStorage::moveToFront(Uuid const& uuid) {
  m_savedPlayersCache.toFront(uuid);
  writeMetadata();
}

void PlayerStorage::backupCycle(Uuid const& uuid) {
  RecursiveMutexLocker locker(m_mutex);

  auto configuration = m_root.configuration();
  unsigned playerBackupFileCount = configuration->get("playerBackupFileCount").toUInt();
  auto& fileName = uuidFileName(uuid);

  auto path = [&](String const& dir, String const& extension) {
    return File::relativeTo(dir, strf("{}.{}", fileName, extension));
  };

  if (!File::isDirectory(m_backupDirectory))
    File::makeDirectory(m_backupDirectory);

  File::backupFileInSequence(path(m_storageDirectory, "player"), path(m_backupDirectory, "player"), playerBackupFileCount, ".bak");
  File::backupFileInSequence(path(m_storageDirectory, "shipworld"), path(m_backupDirectory, "shipworld"), playerBackupFileCount, ".bak");
  File::backupFileInSequence(path(m_storageDirectory, "metadata"), path(m_backupDirectory, "metadata"), playerBackupFileCount, ".bak");
}

void PlayerStorage::setMetadata(String key, Json value) {
  auto& val = m_metadata[std::move(key)];
  if (val != value) {
    val = std::move(value);
    writeMetadata();
  }
}

Json PlayerStorage::getMetadata(String const& key) {
  return m_metadata.value(key);
}

String const& PlayerStorage::uuidFileName(Uuid const& uuid) {
  if (auto fileName = m_playerFileNames.rightPtr(uuid))
    return *fileName;
  else {
    m_playerFileNames.insert(uuid, uuid.hex());
    return *m_playerFileNames.rightPtr(uuid);
  }
}

void PlayerStorage::writeMetadata() {
  JsonArray order;
  for (auto const& p : m_savedPlayersCache)
    order.append(p.first.hex());

  m_metadata["order"] = std::move(order);

  String filename = File::relativeTo(m_storageDirectory, "metadata");
  File::overwriteFileWithRename(Json(m_metadata).printJson(0), filename);
}

}

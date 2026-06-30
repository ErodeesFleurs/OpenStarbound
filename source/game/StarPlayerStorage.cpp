#include "StarPlayerStorage.hpp"
#include "StarFile.hpp"
#include "StarLogging.hpp"
#include "StarIterator.hpp"
#include "StarTime.hpp"
#include "StarConfiguration.hpp"
#include "StarPlayer.hpp"
#include "StarAssets.hpp"
#include "StarEntityFactory.hpp"
#include "StarText.hpp"
#include "StarAlgorithm.hpp"

namespace Star {

PlayerStorage::PlayerStorage(String const& storageDir, ConfigurationPtr configuration, EntityFactoryConstPtr entityFactory)
  : m_configuration(requireServiceValueAs<PlayerException>(std::move(configuration), "PlayerStorage", "configuration")),
    m_entityFactory(requireServiceValueAs<PlayerException>(std::move(entityFactory), "PlayerStorage", "entity factory")) {
  m_storageDirectory = storageDir;
  m_backupDirectory = File::relativeTo(m_storageDirectory, "backup");
  if (!File::isDirectory(m_storageDirectory)) {
    Logger::info("Creating player storage directory");
    File::makeDirectory(m_storageDirectory);
    return;
  }

  if (m_configuration->get("clearPlayerFiles").toBool()) {
    Logger::info("Clearing all player files");
    for (auto const& [fileName, isDirectory] : File::dirList(m_storageDirectory)) {
      if (!isDirectory)
        File::remove(File::relativeTo(m_storageDirectory, fileName));
    }
  } else {
    for (auto const& [fileName, isDirectory] : File::dirList(m_storageDirectory)) {
      if (isDirectory)
        continue;

      String filename = File::relativeTo(m_storageDirectory, fileName);
      if (filename.endsWith(".player")) {
        try {
          auto json = VersionedJson::readFile(filename);
          Uuid uuid(json.content.getString("uuid"));
          if (m_playerFileNames.insert(uuid, fileName.rsplit('.', 1).at(0))) {
            auto& playerCacheData = m_savedPlayersCache[uuid];
            playerCacheData = m_entityFactory->loadVersionedJson(json, EntityType::Player);
          } else {
            Logger::warn("Duplicate player? Skipping player file {} because it has the same UUID as {}.player ({})", fileName, m_playerFileNames.getRight(uuid), uuid.hex());
          }
        } catch (std::exception const& e) {
          Logger::error("Error loading player file, ignoring! {} : {}", filename, outputException(e, false));
        }
      }
    }

    // Remove all the player entries that are missing player data or fail to
    // load.
    auto it = makeSMutableMapIterator(m_savedPlayersCache);
    while (it.hasNext()) {
      auto& entry = it.next();
      auto const& [playerUuid, playerData] = entry;
      if (playerData.isNull()) {
        it.remove();
      } else {
        try {
          auto player = as<Player>(m_entityFactory->diskLoadEntity(EntityType::Player, playerData));
          if (player->uuid() != playerUuid)
            throw PlayerException(strf("Uuid mismatch in loaded player with filename uuid '{}'", playerUuid.hex()));
        } catch (StarException const& e) {
          auto& fileName = uuidFileName(playerUuid);
          String uuidHex = playerUuid.hex();
          if (uuidHex == fileName)
            Logger::error("Failed to validate player with uuid {} : {}", uuidHex, outputException(e, true));
          else
            Logger::error("Failed to validate player with uuid {} ({}.player) : {}", uuidHex, fileName, outputException(e, true));
          it.remove();
        }
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
  } catch (std::exception const& e) {
    Logger::warn("Error loading player storage metadata file, resetting: {}", outputException(e, false));
  }
}

PlayerStorage::~PlayerStorage() {
  writeMetadata();
}

size_t PlayerStorage::playerCount() const {
  RecursiveMutexLocker locker(m_mutex);
  return m_savedPlayersCache.size();
}

Maybe<Uuid> PlayerStorage::playerUuidAt(size_t index) {
  RecursiveMutexLocker locker(m_mutex);
  if (index < m_savedPlayersCache.size())
    return m_savedPlayersCache.keyAt(index);
  else
    return {};
}

Maybe<Uuid> PlayerStorage::playerUuidByName(String const& name, Maybe<Uuid> except) {
  String cleanMatch = Text::stripEscapeCodes(name).toLower();
  Maybe<Uuid> uuid;

  RecursiveMutexLocker locker(m_mutex);

  size_t longest = std::numeric_limits<size_t>::max();
  for (auto const& [playerUuid, playerData] : m_savedPlayersCache) {
    if (except && *except == playerUuid)
      continue;
    else if (auto playerName = playerData.optQueryString("identity.name")) {
      auto cleanName = Text::stripEscapeCodes(*playerName).toLower();
      auto len = cleanName.size();
      if (len < longest && cleanName.utf8().rfind(cleanMatch.utf8()) == 0) {
        longest = len;
        uuid = playerUuid;
      }
    }
  }

  return uuid;
}

List<Uuid> PlayerStorage::playerUuidListByName(String const& name, Maybe<Uuid> except) {
  String cleanMatch = Text::stripEscapeCodes(name).toLower();
  List<Uuid> list = {};

  RecursiveMutexLocker locker(m_mutex);

  for (auto const& [playerUuid, playerData] : m_savedPlayersCache) {
    if (except && *except == playerUuid)
      continue;
    else if (auto playerName = playerData.optQueryString("identity.name")) {
      auto cleanName = Text::stripEscapeCodes(*playerName).toLower();
      if (cleanMatch == "" || cleanName.utf8().rfind(cleanMatch.utf8()) != NPos) {
        list.append(playerUuid);
      }
    }
  }

  return list;
}


Json PlayerStorage::savePlayer(PlayerPtr const& player) {
  RecursiveMutexLocker locker(m_mutex);

  auto uuid = player->uuid();

  auto& playerCacheData = m_savedPlayersCache[uuid];
  if (!m_playerFileNames.hasLeftValue(uuid))
    m_playerFileNames.insert(uuid, uuid.hex());
  auto newPlayerData = player->diskStore();
  if (playerCacheData != newPlayerData) {
    playerCacheData = newPlayerData;
    VersionedJson versionedJson = m_entityFactory->storeVersionedJson(EntityType::Player, playerCacheData);
    auto fileName = strf("{}.player", uuidFileName(uuid));
    VersionedJson::writeFile(versionedJson, File::relativeTo(m_storageDirectory, fileName));
    Logger::debug("Saved player {} to {}", Text::stripEscapeCodes(player->name()), fileName);
  }
  return newPlayerData;
}

Maybe<Json> PlayerStorage::maybeGetPlayerData(Uuid const& uuid) {
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

PlayerPtr PlayerStorage::loadPlayer(Uuid const& uuid) {
  auto playerCacheData = getPlayerData(uuid);
  try {
    auto player = convert<Player>(m_entityFactory->diskLoadEntity(EntityType::Player, playerCacheData));
    if (player->uuid() != uuid)
      throw PlayerException(strf("Uuid mismatch in loaded player with filename uuid '{}'", uuid.hex()));
    return player;
  } catch (std::exception const& e) {
    Logger::error("Error loading player file, ignoring! {}", outputException(e, false));
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

  unsigned playerBackupFileCount = m_configuration->get("playerBackupFileCount").toUInt();

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
      return WorldStorage::getWorldChunksFromFile(filename);
  } catch (StarException const& e) {
    Logger::error("Failed to load shipworld file, removing {} : {}", filename, outputException(e, false));
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
  WorldStorage::applyWorldChunksUpdateToFile(filePath, updates);
}

void PlayerStorage::moveToFront(Uuid const& uuid) {
  m_savedPlayersCache.toFront(uuid);
  writeMetadata();
}

void PlayerStorage::backupCycle(Uuid const& uuid) {
  RecursiveMutexLocker locker(m_mutex);

  unsigned playerBackupFileCount = m_configuration->get("playerBackupFileCount").toUInt();
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
  for (auto const& [uuid, _] : m_savedPlayersCache)
    order.append(uuid.hex());

  m_metadata["order"] = std::move(order);

  String filename = File::relativeTo(m_storageDirectory, "metadata");
  File::overwriteFileWithRename(Json(m_metadata).printJson(0), filename);
}

}

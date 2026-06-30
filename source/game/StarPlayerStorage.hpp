#pragma once

#include "StarOrderedMap.hpp"
#include "StarUuid.hpp"
#include "StarPlayerFactory.hpp"
#include "StarThread.hpp"
#include "StarWorldStorage.hpp"
#include "StarStatistics.hpp"

#include "StarConfiguration.hpp"
namespace Star {

class EntityFactory;
using EntityFactoryConstPtr = SharedPtr<EntityFactory const>;

class PlayerStorage {
public:
  PlayerStorage(String const& storageDir, ConfigurationPtr configuration, EntityFactoryConstPtr entityFactory);
  ~PlayerStorage();

  [[nodiscard]] size_t playerCount() const;
  // Returns nothing if index is out of bounds.
  [[nodiscard]] Maybe<Uuid> playerUuidAt(size_t index);
  // Returns nothing if name doesn't match a player.
  [[nodiscard]] Maybe<Uuid> playerUuidByName(String const& name, Maybe<Uuid> except = {});
  // Returns nothing if name doesn't match a player.
  [[nodiscard]] List<Uuid> playerUuidListByName(String const& name, Maybe<Uuid> except = {});

  // Also returns the diskStore Json if needed.
  [[nodiscard]] Json savePlayer(PlayerPtr const& player);

  [[nodiscard]] Maybe<Json> maybeGetPlayerData(Uuid const& uuid);
  [[nodiscard]] Json getPlayerData(Uuid const& uuid);
  [[nodiscard]] PlayerPtr loadPlayer(Uuid const& uuid);
  void deletePlayer(Uuid const& uuid);

  [[nodiscard]] WorldChunks loadShipData(Uuid const& uuid);
  void applyShipUpdates(Uuid const& uuid, WorldChunks const& updates);

  // Move the given player to the top of the player ordering.
  void moveToFront(Uuid const& uuid);

  // Copy all the player relevant files for this uuid into .bak1 .bak2 etc
  // files for however many backups are configured
  void backupCycle(Uuid const& uuid);

  // Get / Set PlayerStorage global metadata
  void setMetadata(String key, Json value);
  [[nodiscard]] Json getMetadata(String const& key);

private:
  [[nodiscard]] String const& uuidFileName(Uuid const& uuid);
  void writeMetadata();

  mutable RecursiveMutex m_mutex;
  String m_storageDirectory;
  String m_backupDirectory;
  ConfigurationPtr m_configuration;
  EntityFactoryConstPtr m_entityFactory;
  OrderedHashMap<Uuid, Json> m_savedPlayersCache;
  BiMap<Uuid, String> m_playerFileNames;
  JsonObject m_metadata;
};

}

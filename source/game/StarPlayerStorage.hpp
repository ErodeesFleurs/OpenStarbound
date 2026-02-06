#pragma once

#include <optional>

#include "StarOrderedMap.hpp"
#include "StarUuid.hpp"
#include "StarPlayerFactory.hpp"
#include "StarThread.hpp"
#include "StarWorldStorage.hpp"

namespace Star {

class PlayerStorage {
public:
  PlayerStorage(String const& storageDir);
  ~PlayerStorage();

  size_t playerCount() const;
  // Returns nothing if index is out of bounds.
  std::optional<Uuid> playerUuidAt(size_t index);
  // Returns nothing if name doesn't match a player.
  std::optional<Uuid> playerUuidByName(String const& name, std::optional<Uuid> except = {});
  // Returns nothing if name doesn't match a player.
  List<Uuid> playerUuidListByName(String const& name, std::optional<Uuid> except = {});

  // Also returns the diskStore Json if needed.
  Json savePlayer(PlayerPtr const& player);

  std::optional<Json> maybeGetPlayerData(Uuid const& uuid);
  Json getPlayerData(Uuid const& uuid);
  PlayerPtr loadPlayer(Uuid const& uuid);
  void deletePlayer(Uuid const& uuid);

  WorldChunks loadShipData(Uuid const& uuid);
  void applyShipUpdates(Uuid const& uuid, WorldChunks const& updates);

  // Move the given player to the top of the player ordering.
  void moveToFront(Uuid const& uuid);

  // Copy all the player relevant files for this uuid into .bak1 .bak2 etc
  // files for however many backups are configured
  void backupCycle(Uuid const& uuid);

  // Get / Set PlayerStorage global metadata
  void setMetadata(String key, Json value);
  Json getMetadata(String const& key);

private:
  String const& uuidFileName(Uuid const& uuid);
  void writeMetadata();

  mutable RecursiveMutex m_mutex;
  String m_storageDirectory;
  String m_backupDirectory;
  OrderedHashMap<Uuid, Json> m_savedPlayersCache;
  BiMap<Uuid, String> m_playerFileNames;
  JsonObject m_metadata;
};

}

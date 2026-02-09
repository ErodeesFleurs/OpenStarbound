#pragma once

#include "StarOrderedMap.hpp"
#include "StarPlayerFactory.hpp"
#include "StarThread.hpp"
#include "StarUuid.hpp"
#include "StarWorldStorage.hpp"

import std;

namespace Star {

class PlayerStorage {
public:
  PlayerStorage(String const& storageDir);
  ~PlayerStorage();

  auto playerCount() const -> size_t;
  // Returns nothing if index is out of bounds.
  auto playerUuidAt(size_t index) -> std::optional<Uuid>;
  // Returns nothing if name doesn't match a player.
  auto playerUuidByName(String const& name, std::optional<Uuid> except = {}) -> std::optional<Uuid>;
  // Returns nothing if name doesn't match a player.
  auto playerUuidListByName(String const& name, std::optional<Uuid> except = {}) -> List<Uuid>;

  // Also returns the diskStore Json if needed.
  auto savePlayer(Ptr<Player> const& player) -> Json;

  auto maybeGetPlayerData(Uuid const& uuid) -> std::optional<Json>;
  auto getPlayerData(Uuid const& uuid) -> Json;
  auto loadPlayer(Uuid const& uuid) -> Ptr<Player>;
  void deletePlayer(Uuid const& uuid);

  auto loadShipData(Uuid const& uuid) -> WorldChunks;
  void applyShipUpdates(Uuid const& uuid, WorldChunks const& updates);

  // Move the given player to the top of the player ordering.
  void moveToFront(Uuid const& uuid);

  // Copy all the player relevant files for this uuid into .bak1 .bak2 etc
  // files for however many backups are configured
  void backupCycle(Uuid const& uuid);

  // Get / Set PlayerStorage global metadata
  void setMetadata(String key, Json value);
  auto getMetadata(String const& key) -> Json;

private:
  auto uuidFileName(Uuid const& uuid) -> String const&;
  void writeMetadata();

  mutable RecursiveMutex m_mutex;
  String m_storageDirectory;
  String m_backupDirectory;
  OrderedHashMap<Uuid, Json> m_savedPlayersCache;
  BiMap<Uuid, String> m_playerFileNames;
  JsonObject m_metadata;
};

}// namespace Star

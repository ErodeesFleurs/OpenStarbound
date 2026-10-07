module;
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarIdMap.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarCasting.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVersion.hpp"
#include "StarRpcPromise.hpp"
#include "StarPerlin.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include <functional>
#include "StarRect.hpp"
#include "StarSectorArray2D.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarVariant.hpp"
#include "StarSet.hpp"
#include "StarThread.hpp"


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
import star.worker_pool;
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
#include "StarLuaRoot.hpp"
import star.versioning_database;
import star.world_storage;

namespace Star {

STAR_CLASS(Player);

class PlayerStorage {
public:
  PlayerStorage(String const& storageDir);
  ~PlayerStorage();

  size_t playerCount() const;
  // Returns nothing if index is out of bounds.
  Maybe<Uuid> playerUuidAt(size_t index);
  // Returns nothing if name doesn't match a player.
  Maybe<Uuid> playerUuidByName(String const& name, Maybe<Uuid> except = {});
  // Returns nothing if name doesn't match a player.
  List<Uuid> playerUuidListByName(String const& name, Maybe<Uuid> except = {});

  // Also returns the diskStore Json if needed.
  Json savePlayer(PlayerPtr const& player);

  Maybe<Json> maybeGetPlayerData(Uuid const& uuid);
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

export module star.player_storage;

export namespace Star {
using ::Star::PlayerStorage;
}

module;
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarNetElementSystem.hpp"
#include "StarJsonRpc.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarIdMap.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarCasting.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarVersion.hpp"
#include "StarRpcPromise.hpp"
#include "StarPerlin.hpp"
#include "StarWeightedPool.hpp"
#include <functional>
#include "StarRect.hpp"
#include "StarSectorArray2D.hpp"
#include "StarThread.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarVariant.hpp"
#include "StarSet.hpp"
#include "StarBiMap.hpp"
#include "StarEither.hpp"


import star.damage_types;
import star.celestial_coordinate;
import star.uuid;
import star.celestial_coordinate;
import star.warping;
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
import star.player_types;

namespace Star {

STAR_CLASS(CelestialLog);
STAR_CLASS(ClientContext);

class ClientContext {
public:
  ClientContext(Uuid serverUuid, Uuid playerUuid);

  Uuid serverUuid() const;
  // The player Uuid can differ from the mainPlayer's Uuid
  //  if the player has swapped character - use this for ship saving.
  Uuid playerUuid() const;

  // The coordinate for the world which the player's ship is currently
  // orbiting.
  CelestialCoordinate shipCoordinate() const;

  Maybe<pair<WarpAction, WarpMode>> orbitWarpAction() const;

  // The current world id of the player
  WorldId playerWorldId() const;

  bool isAdmin() const;
  EntityDamageTeam team() const;

  JsonRpcInterfacePtr rpcInterface() const;

  WorldChunks newShipUpdates();
  ShipUpgrades shipUpgrades() const;
  
  StringMap<WorldChunks> newCustomWorldUpdates();

  void readUpdate(ByteArray data, NetCompatibilityRules rules);
  ByteArray writeUpdate(NetCompatibilityRules rules);

  void setConnectionId(ConnectionId connectionId);
  ConnectionId connectionId() const;

  void setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules);
  NetCompatibilityRules netCompatibilityRules() const;

private:
  Uuid m_serverUuid;
  Uuid m_playerUuid;
  ConnectionId m_connectionId = 0;
  NetCompatibilityRules m_netCompatibilityRules;

  JsonRpcPtr m_rpc;

  NetElementTopGroup m_netGroup;
  NetElementData<Maybe<pair<WarpAction, WarpMode>>> m_orbitWarpActionNetState;
  NetElementData<WorldId> m_playerWorldIdNetState;
  NetElementBool m_isAdminNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementData<ShipUpgrades> m_shipUpgrades;
  NetElementData<CelestialCoordinate> m_shipCoordinate;
  WorldChunks m_newShipUpdates;
  StringMap<WorldChunks> m_newCustomWorldUpdates;
};

}

export module star.client_context;

export namespace Star {
using ::Star::CelestialLog;
using ::Star::CelestialLogPtr;
using ::Star::CelestialLogConstPtr;
using ::Star::CelestialLogWeakPtr;
using ::Star::CelestialLogConstWeakPtr;
using ::Star::CelestialLogUPtr;
using ::Star::CelestialLogConstUPtr;
using ::Star::ClientContext;
using ::Star::ClientContextPtr;
using ::Star::ClientContextConstPtr;
using ::Star::ClientContextWeakPtr;
using ::Star::ClientContextConstWeakPtr;
using ::Star::ClientContextUPtr;
using ::Star::ClientContextConstUPtr;
}

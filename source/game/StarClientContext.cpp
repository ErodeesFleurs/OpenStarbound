#include "StarJsonExtra.hpp"
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
#include "StarSet.hpp"
#include "StarBTree.hpp"
#include "StarOrderedMap.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
#include "StarThread.hpp"
import star.btree_database;
#include "StarCasting.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarRpcPromise.hpp"
#include "StarBiMap.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;
import star.weighted_pool;
#include <functional>
#include "StarRect.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarThread.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarVariant.hpp"
#include "StarSet.hpp"
#include "StarBiMap.hpp"
#include "StarEither.hpp"
#include "StarDataStreamExtra.hpp"

#include "StarLuaRoot.hpp"
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
import star.versioning_database;
import star.world_storage;
import star.player_types;

import star.client_context;

namespace Star {

DataStream& operator>>(DataStream& ds, ShipUpgrades& upgrades) {
  ds.read(upgrades.shipLevel);
  ds.read(upgrades.maxFuel);
  ds.read(upgrades.crewSize);
  ds.read(upgrades.fuelEfficiency);
  ds.read(upgrades.shipSpeed);
  ds.read(upgrades.capabilities);
  return ds;
}

DataStream& operator<<(DataStream& ds, ShipUpgrades const& upgrades) {
  ds.write(upgrades.shipLevel);
  ds.write(upgrades.maxFuel);
  ds.write(upgrades.crewSize);
  ds.write(upgrades.fuelEfficiency);
  ds.write(upgrades.shipSpeed);
  ds.write(upgrades.capabilities);
  return ds;
}

ClientContext::ClientContext(Uuid serverUuid, Uuid playerUuid) {
  m_serverUuid = std::move(serverUuid);
  m_playerUuid = std::move(playerUuid);
  m_rpc = std::make_shared<JsonRpc>();

  m_netGroup.addNetElement(&m_orbitWarpActionNetState);
  m_netGroup.addNetElement(&m_playerWorldIdNetState);
  m_netGroup.addNetElement(&m_isAdminNetState);
  m_netGroup.addNetElement(&m_teamNetState);
  m_netGroup.addNetElement(&m_shipUpgrades);
  m_netGroup.addNetElement(&m_shipCoordinate);
}

Uuid ClientContext::serverUuid() const {
  return m_serverUuid;
}

Uuid ClientContext::playerUuid() const {
  return m_playerUuid;
}

CelestialCoordinate ClientContext::shipCoordinate() const {
  return m_shipCoordinate.get();
}

Maybe<pair<WarpAction, WarpMode>> ClientContext::orbitWarpAction() const {
  return m_orbitWarpActionNetState.get();
}

WorldId ClientContext::playerWorldId() const {
  return m_playerWorldIdNetState.get();
}

bool ClientContext::isAdmin() const {
  return m_isAdminNetState.get();
}

EntityDamageTeam ClientContext::team() const {
  return m_teamNetState.get();
}

JsonRpcInterfacePtr ClientContext::rpcInterface() const {
  return m_rpc;
}

WorldChunks ClientContext::newShipUpdates() {
  return take(m_newShipUpdates);
}

StringMap<WorldChunks> ClientContext::newCustomWorldUpdates() {
  return take(m_newCustomWorldUpdates);
}

ShipUpgrades ClientContext::shipUpgrades() const {
  return m_shipUpgrades.get();
}

void ClientContext::readUpdate(ByteArray data, NetCompatibilityRules rules) {
  if (data.empty())
    return;

  DataStreamBuffer ds(std::move(data));
  ds.setStreamCompatibilityVersion(rules);

  m_rpc->receive(ds.read<ByteArray>());

  auto shipUpdates = ds.read<ByteArray>();
  if (!shipUpdates.empty())
    m_newShipUpdates.merge(DataStreamBuffer::deserialize<WorldChunks>(std::move(shipUpdates)), true);

  if (rules.version() >= 15) {
    auto customWorldUpdates = ds.read<StringMap<ByteArray>>();
    for (auto& p : customWorldUpdates) {
      m_newCustomWorldUpdates[p.first].merge(DataStreamBuffer::deserialize<WorldChunks>(std::move(p.second)), true);
    }
  }
  
  m_netGroup.readNetState(ds.read<ByteArray>(), 0.0f, rules);
}

ByteArray ClientContext::writeUpdate(NetCompatibilityRules) {
  return m_rpc->send();
}

void ClientContext::setConnectionId(ConnectionId connectionId) {
  m_connectionId = connectionId;
}

ConnectionId ClientContext::connectionId() const {
  return m_connectionId;
}

void ClientContext::setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules) {
  m_netCompatibilityRules = netCompatibilityRules;
}

NetCompatibilityRules ClientContext::netCompatibilityRules() const {
  return m_netCompatibilityRules;
}

}

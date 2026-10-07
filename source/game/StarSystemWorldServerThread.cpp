#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarStrongTypedef.hpp"
#include "StarEither.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarThread.hpp"
#include "StarIdMap.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarCasting.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSet.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarBTree.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
#include "StarOrderedSet.hpp"
#include "StarRpcPromise.hpp"
#include "StarSet.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.uuid;
import star.warping;
import star.sky_parameters;
import star.system_world;
import star.system_world_server;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
import star.celestial_types;
import star.chat_types;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.world_geometry;
import star.wiring;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
#include "StarLuaRoot.hpp"
import star.world_storage;
import star.player_types;
import star.damage_manager;
import star.net_packets;
import star.system_world_server_thread;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.versioning_database;
import star.tick_rate_monitor;

namespace Star {

SystemWorldServerThread::SystemWorldServerThread(Vec3I const& location, SystemWorldServerPtr systemWorld, String storageFile)
  : Thread(strf("SystemWorldServer: {}", location))
  , m_systemLocation(location)
  , m_systemWorld(std::move(systemWorld))
  , m_storageFile(storageFile)
{
}

SystemWorldServerThread::~SystemWorldServerThread() {
  m_stop = true;
  join();
}

Vec3I SystemWorldServerThread::location() const {
  return m_systemLocation;
}

List<ConnectionId> SystemWorldServerThread::clients() {
  return m_clients.values();
}

void SystemWorldServerThread::addClient(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation const& location) {
  WriteLocker locker(m_mutex);
  m_clients.add(clientId);
  m_outgoingPacketQueue.set(clientId, List<PacketPtr>());

  m_systemWorld->addClientShip(clientId, uuid, shipSpeed, location);

  m_clientShipLocations.set(clientId, {m_systemWorld->clientShipLocation(clientId), m_systemWorld->clientSkyParameters(clientId)});
  if (auto warpAction = m_systemWorld->clientWarpAction(clientId))
    m_clientWarpActions.set(clientId, *warpAction);
}

void SystemWorldServerThread::removeClient(ConnectionId clientId) {
  WriteLocker locker(m_mutex);
  m_systemWorld->removeClientShip(clientId);
  m_clients.remove(clientId);
  m_clientShipDestinations.remove(clientId);
  m_clientShipLocations.remove(clientId);
  m_outgoingPacketQueue.remove(clientId);
}

void SystemWorldServerThread::setPause(shared_ptr<const atomic<bool>> pause) {
  m_pause = std::move(pause);
}

void SystemWorldServerThread::run() {
  TickRateApproacher tickApproacher(1.0 / SystemWorldTimestep, 0.5);

  while (!m_stop) {
    LogMap::set(strf("system_{}_update_rate", m_systemLocation), strf("{:4.2f}Hz", tickApproacher.rate()));

    update();

    m_periodicStorage -= 1.0 / tickApproacher.rate();
    if (m_triggerStorage || m_periodicStorage <= 0.0) {
      m_triggerStorage = false;
      m_periodicStorage = 300.0; // store every 5 minutes
      store();
    }

    tickApproacher.tick();

    double spareTime = tickApproacher.spareTime();
    uint64_t millis = floor(spareTime * 1000);
    if (spareTime > 0)
      sleepPrecise(millis);
  }

  store();
}

void SystemWorldServerThread::stop() {
  m_stop = true;
}

void SystemWorldServerThread::update() {
  WriteLocker queueLocker(m_queueMutex);
  WriteLocker locker(m_mutex);

  for (auto p : take(m_incomingPacketQueue))
    m_systemWorld->handleIncomingPacket(p.first, p.second);

  for (auto p : take(m_clientShipActions))
    p.second(m_systemWorld->clientShip(p.first).get());

  if (!m_pause || *m_pause == false)
    m_systemWorld->update(SystemWorldTimestep * GlobalTimescale);
  m_triggerStorage = m_systemWorld->triggeredStorage();

  // important to set destinations before getting locations
  // setting a destination nullifies the current location
  for (auto p : take(m_clientShipDestinations))
    m_systemWorld->setClientDestination(p.first, p.second);

  m_activeInstanceWorlds = m_systemWorld->activeInstanceWorlds();

  for (auto clientId : m_clients) {
    m_outgoingPacketQueue[clientId].appendAll(m_systemWorld->pullOutgoingPackets(clientId));
    auto shipSystemLocation = m_systemWorld->clientShipLocation(clientId);
    auto& shipLocation = m_clientShipLocations[clientId];
    if (shipLocation.first != shipSystemLocation) {
      shipLocation.first = shipSystemLocation;
      shipLocation.second = m_systemWorld->clientSkyParameters(clientId);
    }
    if (auto warpAction = m_systemWorld->clientWarpAction(clientId))
      m_clientWarpActions.set(clientId, *warpAction);
    else if (m_clientWarpActions.contains(clientId))
      m_clientWarpActions.remove(clientId);
  }
  queueLocker.unlock();

  if (m_updateAction)
    m_updateAction(this);
}

void SystemWorldServerThread::setClientDestination(ConnectionId clientId, SystemLocation const& destination) {
  WriteLocker locker(m_queueMutex);
  m_clientShipDestinations.set(clientId, destination);
}

void SystemWorldServerThread::executeClientShipAction(ConnectionId clientId, ClientShipAction action) {
  WriteLocker locker(m_queueMutex);
  m_clientShipActions.append({clientId, std::move(action)});
}

SystemLocation SystemWorldServerThread::clientShipLocation(ConnectionId clientId) {
  ReadLocker locker(m_queueMutex);
  // while a ship destination is pending the ship is assumed to be flying
  if (m_clientShipDestinations.contains(clientId))
    return {};
  return m_clientShipLocations.get(clientId).first;
}

Maybe<pair<WarpAction, WarpMode>> SystemWorldServerThread::clientWarpAction(ConnectionId clientId) {
  ReadLocker locker(m_queueMutex);
  if (m_clientShipDestinations.contains(clientId))
    return {};
  return m_clientWarpActions.maybe(clientId);
}

SkyParameters SystemWorldServerThread::clientSkyParameters(ConnectionId clientId) {
  ReadLocker locker(m_queueMutex);
  return m_clientShipLocations.get(clientId).second;
}

List<InstanceWorldId> SystemWorldServerThread::activeInstanceWorlds() const {
  return m_activeInstanceWorlds;
}

void SystemWorldServerThread::setUpdateAction(function<void(SystemWorldServerThread*)> updateAction) {
  m_updateAction = updateAction;
}

void SystemWorldServerThread::pushIncomingPacket(ConnectionId clientId, PacketPtr packet) {
  WriteLocker locker(m_queueMutex);
  m_incomingPacketQueue.append({std::move(clientId), std::move(packet)});
}

List<PacketPtr> SystemWorldServerThread::pullOutgoingPackets(ConnectionId clientId) {
  WriteLocker locker(m_queueMutex);
  return take(m_outgoingPacketQueue[clientId]);
}

void SystemWorldServerThread::store() {
  ReadLocker locker(m_mutex);
  Json store = m_systemWorld->diskStore();
  locker.unlock();

  Logger::debug("Trigger disk storage for system world {}:{}:{}", m_systemLocation.x(), m_systemLocation.y(), m_systemLocation.z());
  auto versioningDatabase = Root::singleton().versioningDatabase();
  auto versionedStore = versioningDatabase->makeCurrentVersionedJson("System", store);
  VersionedJson::writeFile(versionedStore, m_storageFile);
}

}

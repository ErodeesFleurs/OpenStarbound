#include "StarSystemWorldServerThread.hpp"

#include "StarConfig.hpp"
#include "StarLogging.hpp"
#include "StarNetPackets.hpp"
#include "StarRoot.hpp"
#include "StarTickRateMonitor.hpp"

import std;

namespace Star {

SystemWorldServerThread::SystemWorldServerThread(Vec3I const& location, Ptr<SystemWorldServer> systemWorld, String storageFile)
    : Thread(strf("SystemWorldServer: {}", location)), m_systemLocation(location), m_systemWorld(std::move(systemWorld)), m_storageFile(std::move(storageFile)) {
}

SystemWorldServerThread::~SystemWorldServerThread() {
  m_stop = true;
  join();
}

auto SystemWorldServerThread::location() const -> Vec3I {
  return m_systemLocation;
}

auto SystemWorldServerThread::clients() -> List<ConnectionId> {
  return m_clients.values();
}

void SystemWorldServerThread::addClient(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation const& location) {
  WriteLocker locker(m_mutex);
  m_clients.add(clientId);
  m_outgoingPacketQueue.set(clientId, List<Ptr<Packet>>());

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

void SystemWorldServerThread::setPause(std::shared_ptr<const std::atomic<bool>> pause) {
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
      m_periodicStorage = 300.0;// store every 5 minutes
      store();
    }

    tickApproacher.tick();

    double spareTime = tickApproacher.spareTime();
    std::uint64_t millis = std::floor(spareTime * 1000);
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

auto SystemWorldServerThread::clientShipLocation(ConnectionId clientId) -> SystemLocation {
  ReadLocker locker(m_queueMutex);
  // while a ship destination is pending the ship is assumed to be flying
  if (m_clientShipDestinations.contains(clientId))
    return {};
  return m_clientShipLocations.get(clientId).first;
}

auto SystemWorldServerThread::clientWarpAction(ConnectionId clientId) -> std::optional<std::pair<WarpAction, WarpMode>> {
  ReadLocker locker(m_queueMutex);
  if (m_clientShipDestinations.contains(clientId))
    return {};
  return m_clientWarpActions.maybe(clientId);
}

auto SystemWorldServerThread::clientSkyParameters(ConnectionId clientId) -> SkyParameters {
  ReadLocker locker(m_queueMutex);
  return m_clientShipLocations.get(clientId).second;
}

auto SystemWorldServerThread::activeInstanceWorlds() const -> List<InstanceWorldId> {
  return m_activeInstanceWorlds;
}

void SystemWorldServerThread::setUpdateAction(std::function<void(SystemWorldServerThread*)> updateAction) {
  m_updateAction = updateAction;
}

void SystemWorldServerThread::pushIncomingPacket(ConnectionId clientId, Ptr<Packet> packet) {
  WriteLocker locker(m_queueMutex);
  m_incomingPacketQueue.append({std::move(clientId), std::move(packet)});
}

auto SystemWorldServerThread::pullOutgoingPackets(ConnectionId clientId) -> List<Ptr<Packet>> {
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

}// namespace Star

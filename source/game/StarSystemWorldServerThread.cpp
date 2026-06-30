#include "StarSystemWorldServerThread.hpp"
#include "StarAlgorithm.hpp"
#include "StarTickRateMonitor.hpp"
#include "StarNetPackets.hpp"

namespace Star {

SystemWorldServerThread::SystemWorldServerThread(Vec3I const& location, SystemWorldServerPtr systemWorld, String storageFile, VersioningDatabaseConstPtr versioningDatabase)
  : Thread(strf("SystemWorldServer: {}", location))
  , m_systemLocation(location)
  , m_systemWorld(requireServiceValueAs<StarException>(std::move(systemWorld), "SystemWorldServerThread", "system world"))
  , m_storageFile(storageFile)
  , m_versioningDatabase(requireServiceValueAs<StarException>(std::move(versioningDatabase), "SystemWorldServerThread", "versioning database"))
{
}

SystemWorldServerThread::~SystemWorldServerThread() {
  m_stop = true;
  (void)join();
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

  m_clientShipLocations.set(clientId, ClientShipState{m_systemWorld->clientShipLocation(clientId), m_systemWorld->clientSkyParameters(clientId)});
  if (auto warpAction = m_systemWorld->clientWarpAction(clientId))
    m_clientWarpActions.set(clientId, ClientWarpAction{warpAction->first, warpAction->second});
}

void SystemWorldServerThread::removeClient(ConnectionId clientId) {
  WriteLocker locker(m_mutex);
  m_systemWorld->removeClientShip(clientId);
  m_clients.remove(clientId);
  m_clientShipDestinations.remove(clientId);
  m_clientShipLocations.remove(clientId);
  m_outgoingPacketQueue.remove(clientId);
}

void SystemWorldServerThread::setPause(SharedPtr<atomic<bool> const> pause) {
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

  for (auto queuedPacket : take(m_incomingPacketQueue))
    m_systemWorld->handleIncomingPacket(queuedPacket.clientId, queuedPacket.packet);

  for (auto queuedAction : take(m_clientShipActions))
    queuedAction.action(m_systemWorld->clientShip(queuedAction.clientId).get());

  if (!m_pause || *m_pause == false)
    m_systemWorld->update(SystemWorldTimestep * GlobalTimescale);
  m_triggerStorage = m_systemWorld->triggeredStorage();

  // important to set destinations before getting locations
  // setting a destination nullifies the current location
  for (auto [clientId, destination] : take(m_clientShipDestinations))
    m_systemWorld->setClientDestination(clientId, destination);

  m_activeInstanceWorlds = m_systemWorld->activeInstanceWorlds();

  for (auto clientId : m_clients) {
    m_outgoingPacketQueue[clientId].appendAll(m_systemWorld->pullOutgoingPackets(clientId));
    auto shipSystemLocation = m_systemWorld->clientShipLocation(clientId);
    auto& shipLocation = m_clientShipLocations[clientId];
    if (shipLocation.location != shipSystemLocation) {
      shipLocation.location = shipSystemLocation;
      shipLocation.skyParameters = m_systemWorld->clientSkyParameters(clientId);
    }
    if (auto warpAction = m_systemWorld->clientWarpAction(clientId))
      m_clientWarpActions.set(clientId, ClientWarpAction{warpAction->first, warpAction->second});
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
  m_clientShipActions.append(QueuedClientShipAction{clientId, std::move(action)});
}

SystemLocation SystemWorldServerThread::clientShipLocation(ConnectionId clientId) {
  ReadLocker locker(m_queueMutex);
  // while a ship destination is pending the ship is assumed to be flying
  if (m_clientShipDestinations.contains(clientId))
    return {};
  return m_clientShipLocations.get(clientId).location;
}

Maybe<pair<WarpAction, WarpMode>> SystemWorldServerThread::clientWarpAction(ConnectionId clientId) {
  ReadLocker locker(m_queueMutex);
  if (m_clientShipDestinations.contains(clientId))
    return {};
  return m_clientWarpActions.maybe(clientId).apply([](ClientWarpAction const& warpAction) {
      return pair<WarpAction, WarpMode>(warpAction.action, warpAction.mode);
    });
}

SkyParameters SystemWorldServerThread::clientSkyParameters(ConnectionId clientId) {
  ReadLocker locker(m_queueMutex);
  return m_clientShipLocations.get(clientId).skyParameters;
}

List<InstanceWorldId> SystemWorldServerThread::activeInstanceWorlds() const {
  return m_activeInstanceWorlds;
}

void SystemWorldServerThread::setUpdateAction(function<void(SystemWorldServerThread*)> updateAction) {
  m_updateAction = updateAction;
}

void SystemWorldServerThread::pushIncomingPacket(ConnectionId clientId, PacketPtr packet) {
  WriteLocker locker(m_queueMutex);
  m_incomingPacketQueue.append(QueuedIncomingPacket{clientId, std::move(packet)});
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
  auto versioningDatabase = m_versioningDatabase;
  auto versionedStore = versioningDatabase->makeCurrentVersionedJson("System", store);
  VersionedJson::writeFile(versionedStore, m_storageFile);
}

}

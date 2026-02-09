#pragma once

#include "StarConfig.hpp"
#include "StarNetPackets.hpp"
#include "StarSystemWorldServer.hpp"
#include "StarThread.hpp"

import std;

namespace Star {

using ClientShipAction = std::function<void(SystemClientShip*)>;

class SystemWorldServerThread : public Thread {
public:
  SystemWorldServerThread(Vec3I const& location, Ptr<SystemWorldServer> systemWorld, String storageFile);
  ~SystemWorldServerThread() override;

  auto location() const -> Vec3I;

  auto clients() -> List<ConnectionId>;
  void addClient(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation const& location);
  void removeClient(ConnectionId clientId);

  void setPause(std::shared_ptr<const std::atomic<bool>> pause);
  void run() override;
  void stop();

  void update();

  void setClientDestination(ConnectionId clientId, SystemLocation const& location);
  void executeClientShipAction(ConnectionId clientId, ClientShipAction action);

  auto clientShipLocation(ConnectionId clientId) -> SystemLocation;
  auto clientWarpAction(ConnectionId clientId) -> std::optional<std::pair<WarpAction, WarpMode>>;
  auto clientSkyParameters(ConnectionId clientId) -> SkyParameters;

  auto activeInstanceWorlds() const -> List<InstanceWorldId>;

  // callback to be run after update in the server thread
  void setUpdateAction(std::function<void(SystemWorldServerThread*)> updateAction);
  void pushIncomingPacket(ConnectionId clientId, Ptr<Packet> packet);
  auto pullOutgoingPackets(ConnectionId clientId) -> List<Ptr<Packet>>;

  void store();

private:
  Vec3I m_systemLocation;
  Ptr<SystemWorldServer> m_systemWorld;

  std::atomic<bool> m_stop{false};
  float m_periodicStorage{300.0f};
  bool m_triggerStorage{false};
  String m_storageFile;

  std::shared_ptr<const std::atomic<bool>> m_pause;
  std::function<void(SystemWorldServerThread*)> m_updateAction;

  ReadersWriterMutex m_mutex;
  ReadersWriterMutex m_queueMutex;

  HashSet<ConnectionId> m_clients;
  HashMap<ConnectionId, SystemLocation> m_clientShipDestinations;
  HashMap<ConnectionId, std::pair<SystemLocation, SkyParameters>> m_clientShipLocations;
  HashMap<ConnectionId, std::pair<WarpAction, WarpMode>> m_clientWarpActions;
  List<std::pair<ConnectionId, ClientShipAction>> m_clientShipActions;
  List<InstanceWorldId> m_activeInstanceWorlds;
  Map<ConnectionId, List<Ptr<Packet>>> m_outgoingPacketQueue;
  List<std::pair<ConnectionId, Ptr<Packet>>> m_incomingPacketQueue;
};

}// namespace Star

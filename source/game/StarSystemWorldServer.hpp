#pragma once

#include "StarConfig.hpp"
#include "StarGameTypes.hpp"
#include "StarRandom.hpp"
#include "StarSystemWorld.hpp"
#include "StarUuid.hpp"

import std;

namespace Star {

struct Packet;

class SystemWorldServer : public SystemWorld {
public:
  // create new system world server
  SystemWorldServer(Vec3I location, ConstPtr<Clock> universeClock, Ptr<CelestialDatabase> celestialDatabase);
  // load system world server from storage
  SystemWorldServer(Json const& diskStore, ConstPtr<Clock> universeClock, Ptr<CelestialDatabase> celestialDatabase);

  void setClientDestination(ConnectionId const& clientId, SystemLocation const& destination);

  // null while flying, system coordinate when in space or at a system object
  // planet coordinates while orbiting a planet
  [[nodiscard]] auto clientShip(ConnectionId clientId) const -> Ptr<SystemClientShip>;
  [[nodiscard]] auto clientShipLocation(ConnectionId clientId) const -> SystemLocation;
  [[nodiscard]] auto clientWarpAction(ConnectionId clientId) const -> std::optional<std::pair<WarpAction, WarpMode>>;
  [[nodiscard]] auto clientSkyParameters(ConnectionId clientId) const -> SkyParameters;

  [[nodiscard]] auto clients() const -> List<ConnectionId>;
  void addClientShip(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation location);
  void removeClientShip(ConnectionId clientId);
  [[nodiscard]] auto shipsAtLocation(SystemLocation const& location) const -> List<Ptr<SystemClientShip>>;
  [[nodiscard]] auto activeInstanceWorlds() const -> List<InstanceWorldId>;

  // removeObject queues up object for destruction, any ships at the location
  // are moved away
  void removeObject(Uuid objectUuid);
  auto addObject(Ptr<SystemObject> object, bool doRangeCheck = false) -> bool;

  void update(float dt);

  [[nodiscard]] auto objects() const -> List<Ptr<SystemObject>> override;
  [[nodiscard]] auto objectKeys() const -> List<Uuid> override;
  [[nodiscard]] auto getObject(Uuid const& uuid) const -> Ptr<SystemObject> override;

  auto pullShipFlights() -> List<ConnectionId>;

  void handleIncomingPacket(ConnectionId clientId, Ptr<Packet> packet);
  auto pullOutgoingPackets(ConnectionId clientId) -> List<Ptr<Packet>>;

  auto triggeredStorage() -> bool;
  auto diskStore() -> Json;

private:
  struct ClientNetVersions {
    HashMap<Uuid, std::uint64_t> ships;
    HashMap<Uuid, std::uint64_t> objects;
  };

  void queueUpdatePackets();
  void setClientShipDestination(ConnectionId clientId, SystemLocation const& destination);

  // random position for new ships entering the system
  void placeInitialObjects();
  void spawnObjects();
  auto randomObjectSpawnPosition(RandomSource& rand) const -> Vec2F;

  [[nodiscard]] auto locationSkyParameters(SystemLocation const& location) const -> SkyParameters;

  // setting this to true asynchronously triggers storage from the server thread
  bool m_triggerStorage;

  double m_lastSpawn;
  double m_objectSpawnTime;

  // objects to be destroyed as soon as there are no ships at the location
  List<Uuid> m_objectDestroyQueue;
  // ships to be destroyed after update packets have been queued
  List<Uuid> m_shipDestroyQueue;

  HashMap<ConnectionId, ClientNetVersions> m_clientNetVersions;
  HashMap<ConnectionId, Uuid> m_clientShips;
  HashMap<Uuid, Ptr<SystemObject>> m_objects;
  HashMap<Uuid, Ptr<SystemClientShip>> m_ships;
  // client ID and flight start position
  List<ConnectionId> m_shipFlights;

  HashMap<ConnectionId, List<Ptr<Packet>>> m_outgoingPackets;
};

}// namespace Star

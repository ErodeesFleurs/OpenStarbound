#pragma once

#include "StarLiquidsDatabase.hpp"
#include "StarSystemWorld.hpp"
#include "StarUuid.hpp"

namespace Star {

class SystemWorldServer;

struct Packet;
using PacketPtr = SharedPtr<Packet>;

class SystemWorldServer : public SystemWorld {
public:
  // create new system world server
  SystemWorldServer(AssetsConstPtr assets, LiquidsDatabaseConstPtr liquidsDatabase, Vec3I location, ClockConstPtr universeClock, CelestialDatabasePtr celestialDatabase, PatternedNameGeneratorConstPtr nameGenerator);
  // load system world server from storage
  SystemWorldServer(AssetsConstPtr assets, LiquidsDatabaseConstPtr liquidsDatabase, Json const& diskStore, ClockConstPtr universeClock, CelestialDatabasePtr celestialDatabase, PatternedNameGeneratorConstPtr nameGenerator);

  void setClientDestination(ConnectionId const& clientId, SystemLocation const& destination);

  // null while flying, system coordinate when in space or at a system object
  // planet coordinates while orbiting a planet
  [[nodiscard]] SystemClientShipPtr clientShip(ConnectionId clientId) const;
  [[nodiscard]] SystemLocation clientShipLocation(ConnectionId clientId) const;
  [[nodiscard]] Maybe<pair<WarpAction, WarpMode>> clientWarpAction(ConnectionId clientId) const;
  [[nodiscard]] SkyParameters clientSkyParameters(ConnectionId clientId) const;

  [[nodiscard]] List<ConnectionId> clients() const;
  void addClientShip(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation location);
  void removeClientShip(ConnectionId clientId);
  [[nodiscard]] List<SystemClientShipPtr> shipsAtLocation(SystemLocation const& location) const;
  [[nodiscard]] List<InstanceWorldId> activeInstanceWorlds() const;

  // removeObject queues up object for destruction, any ships at the location
  // are moved away
  void removeObject(Uuid objectUuid);
  [[nodiscard]] bool addObject(SystemObjectPtr object, bool doRangeCheck = false);

  void update(float dt);

  [[nodiscard]] List<SystemObjectPtr> objects() const override;
  [[nodiscard]] List<Uuid> objectKeys() const override;
  [[nodiscard]] SystemObjectPtr getObject(Uuid const& uuid) const override;

  [[nodiscard]] List<ConnectionId> pullShipFlights();

  void handleIncomingPacket(ConnectionId clientId, PacketPtr packet);
  [[nodiscard]] List<PacketPtr> pullOutgoingPackets(ConnectionId clientId);

  [[nodiscard]] bool triggeredStorage();
  [[nodiscard]] Json diskStore();

private:
  struct ClientNetVersions {
    HashMap<Uuid, uint64_t> ships;
    HashMap<Uuid, uint64_t> objects;
  };

  void queueUpdatePackets();
  void setClientShipDestination(ConnectionId clientId, SystemLocation const& destination);

  // random position for new ships entering the system
  void placeInitialObjects();
  void spawnObjects();
  [[nodiscard]] Vec2F randomObjectSpawnPosition(RandomSource& rand) const;

  [[nodiscard]] SkyParameters locationSkyParameters(SystemLocation const& location) const;

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
  HashMap<Uuid, SystemObjectPtr> m_objects;
  HashMap<Uuid, SystemClientShipPtr> m_ships;
  // client ID and flight start position
  List<ConnectionId> m_shipFlights;

  HashMap<ConnectionId, List<PacketPtr>> m_outgoingPackets;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
};

}// namespace Star

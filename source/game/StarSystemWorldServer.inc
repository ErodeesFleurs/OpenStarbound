#pragma once
import std;
import star.map;
import star.json;
import star.vector;
import star.variant;
import star.game_types;
import star.color;
import star.bi_map;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
#include "StarStrongTypedef.hpp"
import star.either;

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

namespace Star {

struct Packet;

class SystemWorldServer : public SystemWorld {
public:
  // create new system world server
  SystemWorldServer(Vec3I location, std::shared_ptr<Clock const> universeClock, std::shared_ptr<CelestialDatabase> celestialDatabase);
  // load system world server from storage
  SystemWorldServer(Json const& diskStore, std::shared_ptr<Clock const> universeClock, std::shared_ptr<CelestialDatabase> celestialDatabase);

  void setClientDestination(ConnectionId const& clientId, SystemLocation const& destination);

  // null while flying, system coordinate when in space or at a system object
  // planet coordinates while orbiting a planet
  std::shared_ptr<SystemClientShip> clientShip(ConnectionId clientId) const;
  SystemLocation clientShipLocation(ConnectionId clientId) const;
  Maybe<std::pair<WarpAction, WarpMode>> clientWarpAction(ConnectionId clientId) const;
  SkyParameters clientSkyParameters(ConnectionId clientId) const;

  List<ConnectionId> clients() const;
  void addClientShip(ConnectionId clientId, Uuid const& uuid, float shipSpeed, SystemLocation location);
  void removeClientShip(ConnectionId clientId);
  List<std::shared_ptr<SystemClientShip>> shipsAtLocation(SystemLocation const& location) const;
  List<InstanceWorldId> activeInstanceWorlds() const;

  // removeObject queues up object for destruction, any ships at the location
  // are moved away
  void removeObject(Uuid objectUuid);
  bool addObject(std::shared_ptr<SystemObject> object, bool doRangeCheck = false);

  void update(float dt);

  List<std::shared_ptr<SystemObject>> objects() const override;
  List<Uuid> objectKeys() const override;
  std::shared_ptr<SystemObject> getObject(Uuid const& uuid) const override;

  List<ConnectionId> pullShipFlights();

  void handleIncomingPacket(ConnectionId clientId, std::shared_ptr<Packet> packet);
  List<std::shared_ptr<Packet>> pullOutgoingPackets(ConnectionId clientId);

  bool triggeredStorage();
  Json diskStore();

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
  Vec2F randomObjectSpawnPosition(RandomSource& rand) const;

  SkyParameters locationSkyParameters(SystemLocation const& location) const;

  // setting this to true asynchronously triggers storage from the server thread
  bool m_triggerStorage{false};
  
  double m_lastSpawn;
  double m_objectSpawnTime;

  // objects to be destroyed as soon as there are no ships at the location
  List<Uuid> m_objectDestroyQueue;
  // ships to be destroyed after update packets have been queued
  List<Uuid> m_shipDestroyQueue;

  HashMap<ConnectionId, ClientNetVersions> m_clientNetVersions;
  HashMap<ConnectionId, Uuid> m_clientShips;
  HashMap<Uuid, std::shared_ptr<SystemObject>> m_objects;
  HashMap<Uuid, std::shared_ptr<SystemClientShip>> m_ships;
  // client ID and flight start position
  List<ConnectionId> m_shipFlights;

  HashMap<ConnectionId, List<std::shared_ptr<Packet>>> m_outgoingPackets;
};


}


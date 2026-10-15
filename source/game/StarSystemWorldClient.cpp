import star.xx_hash;
import star.json;
#include "StarDataStreamExtra.hpp"
import star.ordered_map;
import star.either;
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
import star.id_map;
import star.string;
import star.version;
#include "StarCasting.hpp"
import star.net_compatibility;
import std;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
#include "StarBTree.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
import star.ordered_set;
import star.rpc_promise;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;

#include "StarLuaRoot.hpp"
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.uuid;
import star.warping;
import star.sky_parameters;
import star.system_world;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
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
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.damage_manager;
import star.net_packets;
import star.system_world_client;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.player_universe_map;


import star.celestial_database;

import star.byte_array;
import star.algorithm;
namespace Star {

SystemWorldClient::SystemWorldClient(std::shared_ptr<Clock const> universeClock, std::shared_ptr<CelestialDatabase> celestialDatabase, std::shared_ptr<PlayerUniverseMap> universeMap)
  : SystemWorld(universeClock, celestialDatabase), m_universeMap(std::move(universeMap)) {}

CelestialCoordinate SystemWorldClient::currentSystem() const {
  return CelestialCoordinate(m_location);
}

Maybe<Vec2F> SystemWorldClient::shipPosition() const {
  if (m_ship)
    return m_ship->position();
  else
    return {};
}

SystemLocation SystemWorldClient::shipLocation() const {
  if (m_ship)
    return m_ship->systemLocation();
  else
    return {};
}

SystemLocation SystemWorldClient::shipDestination() const {
  if (m_ship)
    return m_ship->destination();
  else
    return {};
}

// ship is flying if the system world is uninitialized or the ship doesn't have a location
bool SystemWorldClient::flying() const {
  if (m_ship)
    return m_ship->systemLocation().empty();
  return true;
}

void SystemWorldClient::update(float dt) {
  if (!m_ship)
    return;

  m_ship->clientUpdate(dt);

  auto location = m_ship->systemLocation();
  if (auto uuid = location.maybe<Uuid>()) {
    if (auto object = getObject(*uuid)) {
      Maybe<CelestialOrbit> orbit;

      if (object->permanent())
        m_universeMap->addMappedObject(CelestialCoordinate(m_location), *uuid, object->name(), object->orbit(), object->parameters());
      else
        m_universeMap->addMappedObject(CelestialCoordinate(m_location), *uuid, object->name());
    }
  } else if (auto coordinate = location.maybe<CelestialCoordinate>()) {
    if (coordinate->isPlanetaryBody() || coordinate->isSatelliteBody())
      m_universeMap->addMappedCoordinate(coordinate->planet());
  } else if (auto orbit = location.maybe<CelestialOrbit>()) {
    if (orbit->target.isPlanetaryBody() || orbit->target.isSatelliteBody())
      m_universeMap->addMappedCoordinate(orbit->target.planet());
  }

  for (auto p : m_clientShips)
    p.second->clientUpdate(dt);
  for (auto p : m_objects)
    p.second->clientUpdate(dt);

  if (currentSystem().isNull()) {
    m_objects.clear();
    m_clientShips.clear();
    m_ship = {};
    m_location = Vec3I();
  } else if (auto celestialSlave = as<CelestialSlaveDatabase>(m_celestialDatabase))
    celestialSlave->signalSystem(currentSystem()); // keeps the celestial chunk for our current system alive
}

List<std::shared_ptr<SystemObject>> SystemWorldClient::objects() const {
  return m_objects.values();
}

List<Uuid> SystemWorldClient::objectKeys() const {
  return m_objects.keys();
}

std::shared_ptr<SystemObject> SystemWorldClient::getObject(Uuid const& uuid) const {
  return m_objects.maybe(uuid).value({});
}

List<std::shared_ptr<SystemClientShip>> SystemWorldClient::ships() const
{
  return m_clientShips.values();
}

std::shared_ptr<SystemClientShip> SystemWorldClient::getShip(Uuid const & uuid) const
{
  return m_clientShips.maybe(uuid).value({});
}

Uuid SystemWorldClient::spawnObject(String typeName, Maybe<Vec2F> position, Maybe<Uuid> const& uuid, JsonObject overrides) {
  Uuid objectUuid = uuid.value(Uuid());
  m_outgoingPackets.append(std::make_shared<SystemObjectSpawnPacket>(std::move(typeName), objectUuid, std::move(position), std::move(overrides)));
  return objectUuid;
}

bool SystemWorldClient::handleIncomingPacket(std::shared_ptr<Packet> packet) {
  if (auto updatePacket = as<SystemWorldUpdatePacket>(packet)) {
    auto location = m_ship->systemLocation();
    for (auto p : updatePacket->shipUpdates) {
      if (m_ship && p.first == m_ship->uuid())
        m_ship->readNetState(p.second, SystemWorldTimestep);
      else
        m_clientShips[p.first]->readNetState(p.second, SystemWorldTimestep);
    }
    for (auto p : updatePacket->objectUpdates) {
      auto object = getObject(p.first);
      object->readNetState(p.second, SystemWorldTimestep);
    }

  } else if (auto createPacket = as<SystemObjectCreatePacket>(packet)) {
    auto object = netLoadObject(createPacket->objectStore);
    m_objects.set(object->uuid(), object);

  } else if (auto destroyPacket = as<SystemObjectDestroyPacket>(packet)) {
    m_objects.remove(destroyPacket->objectUuid);
    m_universeMap->removeMappedObject(CelestialCoordinate(m_location), destroyPacket->objectUuid);

  } else if (auto shipCreatePacket = as<SystemShipCreatePacket>(packet)) {
    auto ship = netLoadShip(shipCreatePacket->shipStore);
    m_clientShips.set(ship->uuid(), ship);

  } else if (auto shipDestroyPacket = as<SystemShipDestroyPacket>(packet)) {
    m_clientShips.remove(shipDestroyPacket->shipUuid);

  } else if (auto startPacket = as<SystemWorldStartPacket>(packet)) {
    m_objects.clear();
    m_clientShips.clear();
    m_location = startPacket->location;
    for (auto netStore: startPacket->objectStores) {
      auto object = netLoadObject(netStore);
      m_objects.set(object->uuid(), object);
    }
    for (auto netStore : startPacket->shipStores) {
      auto ship = netLoadShip(netStore);
      m_clientShips.set(ship->uuid(), ship);
    }
    m_ship = std::make_shared<SystemClientShip>(this, startPacket->clientShip.first, startPacket->clientShip.second);

    m_universeMap->addMappedCoordinate(CelestialCoordinate(m_location));
    m_universeMap->filterMappedObjects(CelestialCoordinate(m_location), m_objects.keys());
  } else {
    // packet type not handled by system world client
    return false;
  }

  // packet was handled
  return true;
}

List<std::shared_ptr<Packet>> SystemWorldClient::pullOutgoingPackets() {
  return take(m_outgoingPackets);
}

std::shared_ptr<SystemObject> SystemWorldClient::netLoadObject(ByteArray netStore) {
  DataStreamBuffer ds(std::move(netStore));
  Uuid uuid = ds.read<Uuid>();

  String name = ds.read<String>();
  auto objectConfig = systemObjectConfig(name, uuid);

  Vec2F position = ds.read<Vec2F>();

  JsonObject parameters = ds.read<JsonObject>();
  return std::make_shared<SystemObject>(objectConfig, uuid, position, parameters);
}

std::shared_ptr<SystemClientShip> SystemWorldClient::netLoadShip(ByteArray netStore)
{
  DataStreamBuffer ds(std::move(netStore));
  Uuid uuid = ds.read<Uuid>();
  SystemLocation location = ds.read<SystemLocation>();
  return std::make_shared<SystemClientShip>(this, uuid, location);
}

}

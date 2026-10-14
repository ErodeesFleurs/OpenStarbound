module;
import star.xx_hash;
import star.json;
import star.byte_array;
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
import star.id_map;
import star.string;
import star.map;
import star.version;
import star.ordered_map;
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
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.damage_manager;
import star.net_packets;

namespace Star {

class CelestialDatabase;
using CelestialDatabasePtr = std::shared_ptr<CelestialDatabase>;
using CelestialDatabaseConstPtr = std::shared_ptr<const CelestialDatabase>;
using CelestialDatabaseWeakPtr = std::weak_ptr<CelestialDatabase>;
using CelestialDatabaseConstWeakPtr = std::weak_ptr<const CelestialDatabase>;
using CelestialDatabaseUPtr = std::unique_ptr<CelestialDatabase>;
using CelestialDatabaseConstUPtr = std::unique_ptr<const CelestialDatabase>;
class PlayerUniverseMap;
using PlayerUniverseMapPtr = std::shared_ptr<PlayerUniverseMap>;
using PlayerUniverseMapConstPtr = std::shared_ptr<const PlayerUniverseMap>;
using PlayerUniverseMapWeakPtr = std::weak_ptr<PlayerUniverseMap>;
using PlayerUniverseMapConstWeakPtr = std::weak_ptr<const PlayerUniverseMap>;
using PlayerUniverseMapUPtr = std::unique_ptr<PlayerUniverseMap>;
using PlayerUniverseMapConstUPtr = std::unique_ptr<const PlayerUniverseMap>;
class Celestial;
using CelestialPtr = std::shared_ptr<Celestial>;
using CelestialConstPtr = std::shared_ptr<const Celestial>;
using CelestialWeakPtr = std::weak_ptr<Celestial>;
using CelestialConstWeakPtr = std::weak_ptr<const Celestial>;
using CelestialUPtr = std::unique_ptr<Celestial>;
using CelestialConstUPtr = std::unique_ptr<const Celestial>;
class Clock;
using ClockPtr = std::shared_ptr<Clock>;
using ClockConstPtr = std::shared_ptr<const Clock>;
using ClockWeakPtr = std::weak_ptr<Clock>;
using ClockConstWeakPtr = std::weak_ptr<const Clock>;
using ClockUPtr = std::unique_ptr<Clock>;
using ClockConstUPtr = std::unique_ptr<const Clock>;
class ClientContext;
using ClientContextPtr = std::shared_ptr<ClientContext>;
using ClientContextConstPtr = std::shared_ptr<const ClientContext>;
using ClientContextWeakPtr = std::weak_ptr<ClientContext>;
using ClientContextConstWeakPtr = std::weak_ptr<const ClientContext>;
using ClientContextUPtr = std::unique_ptr<ClientContext>;
using ClientContextConstUPtr = std::unique_ptr<const ClientContext>;

class SystemWorldClient : public SystemWorld {
public:
  SystemWorldClient(ClockConstPtr universeClock, CelestialDatabasePtr celestialDatabase, PlayerUniverseMapPtr clientContext);

  CelestialCoordinate currentSystem() const;

  Maybe<Vec2F> shipPosition() const;
  SystemLocation shipLocation() const;
  SystemLocation shipDestination() const;
  bool flying() const;

  void update(float dt);

  List<SystemObjectPtr> objects() const override;
  List<Uuid> objectKeys() const override;
  SystemObjectPtr getObject(Uuid const& uuid) const override;

  List<SystemClientShipPtr> ships() const;
  SystemClientShipPtr getShip(Uuid const& uuid) const;

  Uuid spawnObject(String typeName, Maybe<Vec2F> position = {}, Maybe<Uuid> const& uuid = {}, JsonObject parameters = {});

  // returns whether the packet was handled
  bool handleIncomingPacket(PacketPtr packet);
  List<PacketPtr> pullOutgoingPackets();
private:
  SystemObjectPtr netLoadObject(ByteArray netStore);
  SystemClientShipPtr netLoadShip(ByteArray netStore);

  // m_ship can be a null pointer, indicating that the system is not initialized
  SystemClientShipPtr m_ship;
  HashMap<Uuid, SystemObjectPtr> m_objects;
  HashMap<Uuid, SystemClientShipPtr> m_clientShips;

  PlayerUniverseMapPtr m_universeMap;

  List<PacketPtr> m_outgoingPackets;
};


}

export module star.system_world_client;

export namespace Star {
  using ::Star::CelestialDatabase;
  using ::Star::CelestialDatabasePtr;
  using ::Star::CelestialDatabaseConstPtr;
  using ::Star::CelestialDatabaseWeakPtr;
  using ::Star::CelestialDatabaseConstWeakPtr;
  using ::Star::CelestialDatabaseUPtr;
  using ::Star::CelestialDatabaseConstUPtr;
  using ::Star::PlayerUniverseMap;
  using ::Star::PlayerUniverseMapPtr;
  using ::Star::PlayerUniverseMapConstPtr;
  using ::Star::PlayerUniverseMapWeakPtr;
  using ::Star::PlayerUniverseMapConstWeakPtr;
  using ::Star::PlayerUniverseMapUPtr;
  using ::Star::PlayerUniverseMapConstUPtr;
  using ::Star::Celestial;
  using ::Star::CelestialPtr;
  using ::Star::CelestialConstPtr;
  using ::Star::CelestialWeakPtr;
  using ::Star::CelestialConstWeakPtr;
  using ::Star::CelestialUPtr;
  using ::Star::CelestialConstUPtr;
  using ::Star::Clock;
  using ::Star::ClockPtr;
  using ::Star::ClockConstPtr;
  using ::Star::ClockWeakPtr;
  using ::Star::ClockConstWeakPtr;
  using ::Star::ClockUPtr;
  using ::Star::ClockConstUPtr;
  using ::Star::ClientContext;
  using ::Star::ClientContextPtr;
  using ::Star::ClientContextConstPtr;
  using ::Star::ClientContextWeakPtr;
  using ::Star::ClientContextConstWeakPtr;
  using ::Star::ClientContextUPtr;
  using ::Star::ClientContextConstUPtr;
  using ::Star::SystemWorldClient;
}

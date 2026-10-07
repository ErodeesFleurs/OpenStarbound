module;
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarStrongTypedef.hpp"
#include "StarEither.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarIdMap.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarXXHash.hpp"
#include "StarMathCommon.hpp"
#include "StarVersion.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarCasting.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarThread.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarRpcPromise.hpp"
#include "StarSet.hpp"

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
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.damage_manager;
import star.net_packets;

namespace Star {

STAR_CLASS(CelestialDatabase);
STAR_CLASS(PlayerUniverseMap);
STAR_CLASS(Celestial);
STAR_CLASS(Clock);
STAR_CLASS(ClientContext);

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

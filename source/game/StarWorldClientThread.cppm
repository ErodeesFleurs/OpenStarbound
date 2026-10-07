module;
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarGameTypes.hpp"
#include "StarImage.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVersion.hpp"
#include "StarColor.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarAssetPath.hpp"
#include "StarEither.hpp"
#include "StarMaybe.hpp"
#include "StarDirectives.hpp"
#include "StarWeightedPool.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarThread.hpp"
#include "StarRect.hpp"
#include "StarInterpolation.hpp"
#include "StarAudio.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.warping;
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
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.mixer;
import star.entity_rendering;
import star.world;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;

namespace Star {

STAR_CLASS(WorldClientThread);
STAR_CLASS(UniverseClient);

// Runs a WorldThreadedClient in a separate thread.
class WorldClientThread : public Thread {
public:
  struct Message {
    String message;
    JsonArray args;
    RpcPromiseKeeper<Json> promise;
  };

  typedef function<void(WorldClientThread*, WorldClient*)> WorldClientAction;

  WorldClientThread(ClientSubWorldId subWorldId, UniverseClient* universe);
  ~WorldClientThread();

  ClientSubWorldId subWorldId() const;

  void start();
  // Signals the WorldClientThread to stop and then joins it
  void stop();
  void setPause(shared_ptr<const atomic<bool>> pause);

  // An exception occurred from the actual WorldThreadedClient itself and the
  // WorldClientThread has stopped running.
  bool errorOccurred();
  bool shouldExpire();

  // Clients that have caused an error with incoming packets are removed from
  // the world and no further packets are handled from them.  They are still
  // added to this WorldClientThread, and must be removed and the final
  // outgoing packets should be sent to them.

  void pushIncomingPackets(List<PacketPtr> packets);
  List<PacketPtr> pullOutgoingPackets();

  // Executes the given action on the world in a thread safe context.  This
  // does *not* catch exceptions thrown by the action or set the error
  // flag.
  void executeAction(WorldClientAction action);

  // If a callback is set here, then this is called after every world update,
  // also in a thread safe context.
  void setUpdateAction(WorldClientAction updateAction);

  // 
  void passMessage(Message&& message);
  void clearMessages();

protected:
  virtual void run();

private:
  void update();
  void sync();

  mutable RecursiveMutex m_mutex;

  WorldClientPtr m_worldClient;
  ClientSubWorldId m_subWorldId;
  WorldClientAction m_updateAction;

  mutable RecursiveMutex m_queueMutex;
  List<PacketPtr> m_incomingPacketQueue;
  List<PacketPtr> m_outgoingPacketQueue;

  mutable RecursiveMutex m_messageMutex;
  List<Message> m_messages;

  atomic<bool> m_stop;
  shared_ptr<const atomic<bool>> m_pause;
  mutable atomic<bool> m_errorOccurred;
  mutable atomic<bool> m_shouldExpire;
};

}

export module star.world_client_thread;

export namespace Star {
  using ::Star::WorldClientThread;
  using ::Star::WorldClientThreadPtr;
  using ::Star::WorldClientThreadConstPtr;
  using ::Star::WorldClientThreadWeakPtr;
  using ::Star::WorldClientThreadConstWeakPtr;
  using ::Star::WorldClientThreadUPtr;
  using ::Star::WorldClientThreadConstUPtr;
  using ::Star::UniverseClient;
  using ::Star::UniverseClientPtr;
  using ::Star::UniverseClientConstPtr;
  using ::Star::UniverseClientWeakPtr;
  using ::Star::UniverseClientConstWeakPtr;
  using ::Star::UniverseClientUPtr;
  using ::Star::UniverseClientConstUPtr;
}

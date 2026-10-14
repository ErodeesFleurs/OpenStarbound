module;
import std;
import star.string;
import star.list;
import star.set;
import star.maybe;
import star.json;

import star.game_types;
import star.byte_array;
import star.map;
import star.net_compatibility;
#include "StarStrongTypedef.hpp"
import star.data_stream;
import star.uuid;
import star.vector;
import star.celestial_coordinate;
import star.warping;
import star.thread;
import star.rpc_promise;

namespace Star {

class WorldServer;
using WorldServerPtr = std::shared_ptr<WorldServer>;
using WorldServerConstPtr = std::shared_ptr<const WorldServer>;
using WorldServerWeakPtr = std::weak_ptr<WorldServer>;
using WorldServerConstWeakPtr = std::weak_ptr<const WorldServer>;
using WorldServerUPtr = std::unique_ptr<WorldServer>;
using WorldServerConstUPtr = std::unique_ptr<const WorldServer>;
struct Packet;
using PacketPtr = std::shared_ptr<Packet>;
using PacketConstPtr = std::shared_ptr<const Packet>;
using PacketWeakPtr = std::weak_ptr<Packet>;
using PacketConstWeakPtr = std::weak_ptr<const Packet>;
using PacketUPtr = std::unique_ptr<Packet>;
using PacketConstUPtr = std::unique_ptr<const Packet>;
enum class WorldServerFidelity;
typedef HashMap<ByteArray, Maybe<ByteArray>> WorldChunks;

class WorldServerThread;
using WorldServerThreadPtr = std::shared_ptr<WorldServerThread>;
using WorldServerThreadConstPtr = std::shared_ptr<const WorldServerThread>;
using WorldServerThreadWeakPtr = std::weak_ptr<WorldServerThread>;
using WorldServerThreadConstWeakPtr = std::weak_ptr<const WorldServerThread>;
using WorldServerThreadUPtr = std::unique_ptr<WorldServerThread>;
using WorldServerThreadConstUPtr = std::unique_ptr<const WorldServerThread>;

// Runs a WorldServer in a separate thread and guards exceptions that occur in
// it.  All methods are designed to not throw exceptions, but will instead log
// the error and trigger the WorldServerThread error state.
class WorldServerThread : public Thread {
public:
  struct Message {
    String message;
    JsonArray args;
    RpcPromiseKeeper<Json> promise;
  };

  typedef std::function<void(WorldServerThread*, WorldServer*)> WorldServerAction;

  WorldServerThread(WorldServerPtr server, WorldId worldId);
  ~WorldServerThread();

  WorldId worldId() const;

  void start();
  // Signals the WorldServerThread to stop and then joins it
  void stop();
  void setPause(std::shared_ptr<const std::atomic<bool>> pause);

  // An exception occurred from the actual WorldServer itself and the
  // WorldServerThread has stopped running.
  bool serverErrorOccurred();
  bool shouldExpire();

  bool spawnTargetValid(SpawnTarget const& spawnTarget);

  bool addClient(ConnectionId clientId, SpawnTarget const& spawnTarget, bool isLocal, bool isAdmin = false, NetCompatibilityRules netRules = {}, ClientSubWorldId subWorldId = MainClientWorldId);
  // Returns final outgoing packets
  List<PacketPtr> removeClient(ConnectionId clientId);

  List<ConnectionId> clients() const;
  bool hasClient(ConnectionId clientId) const;
  bool noClients() const;
  ClientSubWorldId clientSubWorld(ConnectionId clientId) const;

  // Clients that have caused an error with incoming packets are removed from
  // the world and no further packets are handled from them.  They are still
  // added to this WorldServerThread, and must be removed and the final
  // outgoing packets should be sent to them.
  List<ConnectionId> erroredClients() const;

  void pushIncomingPackets(ConnectionId clientId, List<PacketPtr> packets);
  List<PacketPtr> pullOutgoingPackets(ConnectionId clientId);

  Maybe<Vec2F> playerRevivePosition(ConnectionId clientId) const;

  // Worlds use this to notify the universe server that their celestial type should change
  Maybe<std::pair<String, String>> pullNewPlanetType();

  // Executes the given action on the world in a thread safe context.  This
  // does *not* catch exceptions thrown by the action or set the server error
  // flag.
  void executeAction(WorldServerAction action);

  // If a callback is set here, then this is called after every world update,
  // also in a thread safe context.
  void setUpdateAction(WorldServerAction updateAction);

  // 
  void passMessages(List<Message>&& messages);

  void unloadAll(bool force = false);

  // Syncs all active sectors to disk and reads the full content of the world
  // into memory, useful for the ship.
  WorldChunks readChunks();

protected:
  virtual void run();

private:
  void update(WorldServerFidelity fidelity);
  void sync();

  mutable RecursiveMutex m_mutex;

  HashSet<ConnectionId> m_clients;

  WorldServerPtr m_worldServer;
  WorldId m_worldId;
  WorldServerAction m_updateAction;

  mutable RecursiveMutex m_queueMutex;
  Map<ConnectionId, List<PacketPtr>> m_incomingPacketQueue;
  Map<ConnectionId, List<PacketPtr>> m_outgoingPacketQueue;

  mutable RecursiveMutex m_messageMutex;
  List<Message> m_messages;

  std::atomic<bool> m_stop;
  std::shared_ptr<const std::atomic<bool>> m_pause;
  mutable std::atomic<bool> m_errorOccurred;
  mutable std::atomic<bool> m_shouldExpire;
};

}

export module star.world_server_thread;

export namespace Star {
  using ::Star::WorldChunks;
  using ::Star::WorldServerThread;
  using ::Star::WorldServerThreadPtr;
  using ::Star::WorldServerThreadConstPtr;
  using ::Star::WorldServerThreadWeakPtr;
  using ::Star::WorldServerThreadConstWeakPtr;
  using ::Star::WorldServerThreadUPtr;
  using ::Star::WorldServerThreadConstUPtr;
}

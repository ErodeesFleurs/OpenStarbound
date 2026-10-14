module;
import star.json;
import star.xx_hash;
import star.io_device;
import star.string;
import star.either;
import star.thread;
import std;
import star.id_map;
import star.data_stream;
import star.list;
import star.bi_map;
import star.game_types;
import star.map;
import star.format;
import star.version;
import star.ordered_map;
import star.variant;
import star.color;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
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
import star.byte_array;

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;


namespace Star {
struct Packet;
using PacketPtr = std::shared_ptr<Packet>;
using PacketConstPtr = std::shared_ptr<const Packet>;
using PacketWeakPtr = std::weak_ptr<Packet>;
using PacketConstWeakPtr = std::weak_ptr<const Packet>;
using PacketUPtr = std::unique_ptr<Packet>;
using PacketConstUPtr = std::unique_ptr<const Packet>;
}

import star.net_packet_socket;

namespace Star {

class UniverseConnection;
using UniverseConnectionPtr = std::shared_ptr<UniverseConnection>;
using UniverseConnectionConstPtr = std::shared_ptr<const UniverseConnection>;
using UniverseConnectionWeakPtr = std::weak_ptr<UniverseConnection>;
using UniverseConnectionConstWeakPtr = std::weak_ptr<const UniverseConnection>;
using UniverseConnectionUPtr = std::unique_ptr<UniverseConnection>;
using UniverseConnectionConstUPtr = std::unique_ptr<const UniverseConnection>;
class UniverseConnectionServer;
using UniverseConnectionServerPtr = std::shared_ptr<UniverseConnectionServer>;
using UniverseConnectionServerConstPtr = std::shared_ptr<const UniverseConnectionServer>;
using UniverseConnectionServerWeakPtr = std::weak_ptr<UniverseConnectionServer>;
using UniverseConnectionServerConstWeakPtr = std::weak_ptr<const UniverseConnectionServer>;
using UniverseConnectionServerUPtr = std::unique_ptr<UniverseConnectionServer>;
using UniverseConnectionServerConstUPtr = std::unique_ptr<const UniverseConnectionServer>;

struct UniverseConnectionExceptionTag {
  static constexpr char const* name() { return "UniverseConnectionException"; }
};
using UniverseConnectionException = StarError<UniverseConnectionExceptionTag, StarException>;

// Symmetric NetPacket based connection between the UniverseServer and the
// UniverseClient.
class UniverseConnection {
public:
  explicit UniverseConnection(PacketSocketUPtr packetSocket);
  UniverseConnection(UniverseConnection&& rhs);
  ~UniverseConnection();

  UniverseConnection& operator=(UniverseConnection&& rhs);

  bool isOpen() const;
  void close();

  // Push packets onto the send queue.
  void push(List<PacketPtr> packets);
  void pushSingle(PacketPtr packet);

  // Pull packets from the receive queue.
  List<PacketPtr> pull();
  PacketPtr pullSingle();

  // Send all data that we can without blocking, returns true if any data was
  // sent.
  bool send();

  // Block, trying to send the entire send queue before the given timeout.
  // Returns true if the entire send queue was sent before the timeout, false
  // otherwise.
  bool sendAll(unsigned timeout);

  // Receive all the data that we can without blocking, returns true if any
  // data was received.
  bool receive();

  // Block, trying to read at least one packet into the receive queue before
  // the timeout.  Returns true once any packets are on the receive queue,
  // false if the timeout was reached with no packets receivable.
  bool receiveAny(unsigned timeout);

  // Returns a reference to the packet socket.
  PacketSocket& packetSocket();

  // Packet stats for the most recent one second window of activity incoming
  // and outgoing.  Will only return valid stats if the underlying PacketSocket
  // implements stat collection.
  Maybe<PacketStats> incomingStats() const;
  Maybe<PacketStats> outgoingStats() const;

private:
  friend class UniverseConnectionServer;

  UniverseConnection() = default;

  mutable Mutex m_mutex;
  PacketSocketUPtr m_packetSocket;
  List<PacketPtr> m_sendQueue;
  Deque<PacketPtr> m_receiveQueue;
};

// Manage a set of UniverseConnections cheaply and in an asynchronous way.
// Uses multiple background threads to handle remote sending and receiving.
class UniverseConnectionServer {
public:
  // The packet receive callback is called asynchronously on every packet group
  // received.  It will be called such that it is safe to recursively call any
  // method on the UniverseConnectionServer without deadlocking.  The receive
  // callback will not be called for any client until the previous callback for
  // that client is complete.
  typedef std::function<void(UniverseConnectionServer*, ConnectionId, List<PacketPtr>)> PacketReceiveCallback;

  UniverseConnectionServer(PacketReceiveCallback packetReceiver, std::size_t numWorkerThreads = 0);
  ~UniverseConnectionServer();

  bool hasConnection(ConnectionId clientId) const;
  List<ConnectionId> allConnections() const;
  bool connectionIsOpen(ConnectionId clientId) const;
  std::int64_t lastActivityTime(ConnectionId clientId) const;

  void addConnection(ConnectionId clientId, UniverseConnection connection);
  UniverseConnection removeConnection(ConnectionId clientId);
  List<UniverseConnection> removeAllConnections();

  void sendPackets(ConnectionId clientId, List<PacketPtr> packets);

  // Get total packets processed across all worker threads
  std::uint64_t totalPacketsProcessed() const;
  // Get number of worker threads
  std::size_t numWorkerThreads() const;

private:
  struct Connection {
    Mutex mutex;
    PacketSocketUPtr packetSocket;
    List<PacketPtr> sendQueue;
    Deque<PacketPtr> receiveQueue;
    std::int64_t lastActivityTime;
    std::size_t workerIndex;
  };

  struct WorkerStats {
    std::atomic<std::uint64_t> packetsProcessed{0};
    std::atomic<std::uint64_t> bytesReceived{0};
    std::atomic<std::uint64_t> bytesSent{0};
    std::atomic<std::uint64_t> connectionsHandled{0};

    WorkerStats() = default;
    WorkerStats(WorkerStats&& other) noexcept : packetsProcessed(other.packetsProcessed.load()), bytesReceived(other.bytesReceived.load()), bytesSent(other.bytesSent.load()), connectionsHandled(other.connectionsHandled.load()) {
                                                };
    WorkerStats(const WorkerStats&) = delete;
    WorkerStats& operator=(WorkerStats&& other) noexcept {
      if (this != &other) {
        packetsProcessed = other.packetsProcessed.load();
        bytesReceived = other.bytesReceived.load();
        bytesSent = other.bytesSent.load();
        connectionsHandled = other.connectionsHandled.load();
      }
      return *this;
    };
    WorkerStats& operator=(const WorkerStats&) = delete;
  };

  PacketReceiveCallback const m_packetReceiver;

  mutable RecursiveMutex m_connectionsMutex;
  HashMap<ConnectionId, std::shared_ptr<Connection>> m_connections;

  List<ThreadFunction<void>> m_processingThreads;
  List<WorkerStats> m_workerStats;
  std::atomic<bool> m_shutdown;
  std::size_t m_numWorkerThreads;
};

}// namespace Star

export module star.universe_connection;

export namespace Star {
  using ::Star::UniverseConnectionExceptionTag;
  using ::Star::UniverseConnectionException;
  using ::Star::UniverseConnection;
  using ::Star::UniverseConnectionPtr;
  using ::Star::UniverseConnectionConstPtr;
  using ::Star::UniverseConnectionWeakPtr;
  using ::Star::UniverseConnectionConstWeakPtr;
  using ::Star::UniverseConnectionUPtr;
  using ::Star::UniverseConnectionConstUPtr;
  using ::Star::UniverseConnectionServer;
  using ::Star::UniverseConnectionServerPtr;
  using ::Star::UniverseConnectionServerConstPtr;
  using ::Star::UniverseConnectionServerWeakPtr;
  using ::Star::UniverseConnectionServerConstWeakPtr;
  using ::Star::UniverseConnectionServerUPtr;
  using ::Star::UniverseConnectionServerConstUPtr;
}

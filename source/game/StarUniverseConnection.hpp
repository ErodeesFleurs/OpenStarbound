#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarNetPacketSocket.hpp"

import std;

namespace Star {

using UniverseConnectionException = ExceptionDerived<"UniverseConnectionException">;

// Symmetric NetPacket based connection between the UniverseServer and the
// UniverseClient.
class UniverseConnection {
public:
  explicit UniverseConnection(UPtr<PacketSocket> packetSocket);
  UniverseConnection(UniverseConnection&& rhs);
  ~UniverseConnection();

  auto operator=(UniverseConnection&& rhs) -> UniverseConnection&;

  auto isOpen() const -> bool;
  void close();

  // Push packets onto the send queue.
  void push(List<Ptr<Packet>> packets);
  void pushSingle(Ptr<Packet> packet);

  // Pull packets from the receive queue.
  auto pull() -> List<Ptr<Packet>>;
  auto pullSingle() -> Ptr<Packet>;

  // Send all data that we can without blocking, returns true if any data was
  // sent.
  auto send() -> bool;

  // Block, trying to send the entire send queue before the given timeout.
  // Returns true if the entire send queue was sent before the timeout, false
  // otherwise.
  auto sendAll(unsigned timeout) -> bool;

  // Receive all the data that we can without blocking, returns true if any
  // data was received.
  auto receive() -> bool;

  // Block, trying to read at least one packet into the receive queue before
  // the timeout.  Returns true once any packets are on the receive queue,
  // false if the timeout was reached with no packets receivable.
  auto receiveAny(unsigned timeout) -> bool;

  // Returns a reference to the packet socket.
  auto packetSocket() -> PacketSocket&;

  // Packet stats for the most recent one second window of activity incoming
  // and outgoing.  Will only return valid stats if the underlying PacketSocket
  // implements stat collection.
  auto incomingStats() const -> std::optional<PacketStats>;
  auto outgoingStats() const -> std::optional<PacketStats>;

private:
  friend class UniverseConnectionServer;

  UniverseConnection() = default;

  mutable Mutex m_mutex;
  UPtr<PacketSocket> m_packetSocket;
  List<Ptr<Packet>> m_sendQueue;
  Deque<Ptr<Packet>> m_receiveQueue;
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
  using PacketReceiveCallback = std::function<void(UniverseConnectionServer*, ConnectionId, List<Ptr<Packet>>)>;

  UniverseConnectionServer(PacketReceiveCallback packetReceiver, size_t numWorkerThreads = 0);
  ~UniverseConnectionServer();

  auto hasConnection(ConnectionId clientId) const -> bool;
  auto allConnections() const -> List<ConnectionId>;
  auto connectionIsOpen(ConnectionId clientId) const -> bool;
  auto lastActivityTime(ConnectionId clientId) const -> std::int64_t;

  void addConnection(ConnectionId clientId, UniverseConnection connection);
  auto removeConnection(ConnectionId clientId) -> UniverseConnection;
  auto removeAllConnections() -> List<UniverseConnection>;

  void sendPackets(ConnectionId clientId, List<Ptr<Packet>> packets);

  // Get total packets processed across all worker threads
  auto totalPacketsProcessed() const -> std::uint64_t;
  // Get number of worker threads
  auto numWorkerThreads() const -> size_t;

private:
  struct Connection {
    Mutex mutex;
    UPtr<PacketSocket> packetSocket;
    List<Ptr<Packet>> sendQueue;
    Deque<Ptr<Packet>> receiveQueue;
    std::int64_t lastActivityTime;
    size_t workerIndex;
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
    auto operator=(WorkerStats&& other) noexcept -> WorkerStats& {
      if (this != &other) {
        packetsProcessed = other.packetsProcessed.load();
        bytesReceived = other.bytesReceived.load();
        bytesSent = other.bytesSent.load();
        connectionsHandled = other.connectionsHandled.load();
      }
      return *this;
    };
    auto operator=(const WorkerStats&) -> WorkerStats& = delete;
  };

  PacketReceiveCallback const m_packetReceiver;

  mutable RecursiveMutex m_connectionsMutex;
  HashMap<ConnectionId, std::shared_ptr<Connection>> m_connections;

  List<ThreadFunction<void>> m_processingThreads;
  List<WorkerStats> m_workerStats;
  std::atomic<bool> m_shutdown;
  size_t m_numWorkerThreads;
};

}// namespace Star

#pragma once

#include "StarAtomicSharedPtr.hpp"
#include "StarNetCompatibility.hpp"
#include "StarNetPackets.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarTcp.hpp"
#include "StarZSTDCompression.hpp"

import std;

namespace Star {

struct PacketStats {
  HashMap<PacketType, float> packetBytesPerSecond;
  float bytesPerSecond;
  PacketType worstPacketType;
  size_t worstPacketSize;
};

// Collects PacketStats over a given window of time.
class PacketStatCollector {
public:
  PacketStatCollector(float calculationWindow = 1.0f);

  void mix(size_t size);
  void mix(PacketType type, size_t size, bool addToTotal = true);
  void mix(HashMap<PacketType, size_t> const& sizes, bool addToTotal = true);

  // Should always return packet statistics for the most recent completed
  // window of time
  [[nodiscard]] auto stats() const -> PacketStats;

private:
  void calculate();

  float m_calculationWindow;
  PacketStats m_stats;
  Map<PacketType, float> m_unmixed;
  size_t m_totalBytes;
  std::int64_t m_lastMixTime;
};

// Interface for bidirectional communication using NetPackets, based around a
// simple non-blocking polling interface.  Communication is assumed to be done
// via writeData() and readData(), and any delay in calling writeData or
// readData may translate directly into increased latency.
class PacketSocket {
public:
  virtual ~PacketSocket() = default;

  [[nodiscard]] virtual auto isOpen() const -> bool = 0;
  virtual void close() = 0;

  // Takes all packets from the given list and queues them for sending.
  virtual void sendPackets(List<Ptr<Packet>> packets) = 0;
  // Receives any packets from the incoming queue, if available
  virtual auto receivePackets() -> List<Ptr<Packet>> = 0;

  // Returns true if any sent packets on the queue are still not completely
  // written.
  [[nodiscard]] virtual auto sentPacketsPending() const -> bool = 0;

  // Write all data possible without blocking, returns true if any data was
  // actually written.
  virtual auto writeData() -> bool = 0;
  // Read all data available without blocking, returns true if any data was
  // actually received.
  virtual auto readData() -> bool = 0;

  // Should return incoming / outgoing packet stats, if they are tracked.
  // Default implementations return nothing.
  [[nodiscard]] virtual auto incomingStats() const -> std::optional<PacketStats>;
  [[nodiscard]] virtual auto outgoingStats() const -> std::optional<PacketStats>;

  virtual void setNetRules(NetCompatibilityRules netRules);
  [[nodiscard]] virtual auto netRules() const -> NetCompatibilityRules;

private:
  NetCompatibilityRules m_netRules;
};

class CompressedPacketSocket : public PacketSocket {
public:
  ~CompressedPacketSocket() override = default;

  virtual void setCompressionStreamEnabled(bool enabled);
  [[nodiscard]] virtual auto compressionStreamEnabled() const -> bool;

private:
  bool m_useCompressionStream = false;

protected:
  CompressionStream m_compressionStream;
  DecompressionStream m_decompressionStream;
};

// PacketSocket for local communication.
class LocalPacketSocket : public PacketSocket {
public:
  static auto openPair() -> std::pair<UPtr<LocalPacketSocket>, UPtr<LocalPacketSocket>>;

  auto isOpen() const -> bool override;
  void close() override;

  void sendPackets(List<Ptr<Packet>> packets) override;
  auto receivePackets() -> List<Ptr<Packet>> override;

  auto sentPacketsPending() const -> bool override;

  // write / read for local sockets is actually a no-op, sendPackets places
  // packets directly in the incoming queue of the paired local socket.
  auto writeData() -> bool override;
  auto readData() -> bool override;

private:
  struct Pipe {
    Mutex mutex;
    Deque<Ptr<Packet>> queue;
  };

  LocalPacketSocket(std::shared_ptr<Pipe> incomingPipe, std::weak_ptr<Pipe> outgoingPipe);

  AtomicSharedPtr<Pipe> m_incomingPipe;
  std::weak_ptr<Pipe> m_outgoingPipe;
};

// Wraps a TCP socket into a PacketSocket.
class TcpPacketSocket : public CompressedPacketSocket {
public:
  static auto open(Ptr<TcpSocket> socket) -> UPtr<TcpPacketSocket>;

  [[nodiscard]] auto isOpen() const -> bool override;
  void close() override;

  void sendPackets(List<Ptr<Packet>> packets) override;
  auto receivePackets() -> List<Ptr<Packet>> override;

  [[nodiscard]] auto sentPacketsPending() const -> bool override;

  auto writeData() -> bool override;
  auto readData() -> bool override;

  [[nodiscard]] auto incomingStats() const -> std::optional<PacketStats> override;
  [[nodiscard]] auto outgoingStats() const -> std::optional<PacketStats> override;

private:
  TcpPacketSocket(Ptr<TcpSocket> socket);

  Ptr<TcpSocket> m_socket;

  PacketStatCollector m_incomingStats;
  PacketStatCollector m_outgoingStats;
  ByteArray m_outputBuffer;
  ByteArray m_inputBuffer;
  ByteArray m_compressedOutputBuffer;
};

// Wraps a P2PSocket into a PacketSocket
class P2PPacketSocket : public CompressedPacketSocket {
public:
  static auto open(UPtr<P2PSocket> socket) -> UPtr<P2PPacketSocket>;

  [[nodiscard]] auto isOpen() const -> bool override;
  void close() override;

  void sendPackets(List<Ptr<Packet>> packets) override;
  auto receivePackets() -> List<Ptr<Packet>> override;

  [[nodiscard]] auto sentPacketsPending() const -> bool override;

  auto writeData() -> bool override;
  auto readData() -> bool override;

  [[nodiscard]] auto incomingStats() const -> std::optional<PacketStats> override;
  [[nodiscard]] auto outgoingStats() const -> std::optional<PacketStats> override;

private:
  P2PPacketSocket(Ptr<P2PSocket> socket);

  Ptr<P2PSocket> m_socket;

  PacketStatCollector m_incomingStats;
  PacketStatCollector m_outgoingStats;
  Deque<ByteArray> m_outputMessages;
  Deque<ByteArray> m_inputMessages;
};

}// namespace Star

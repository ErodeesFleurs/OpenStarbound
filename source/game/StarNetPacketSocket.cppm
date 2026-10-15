module;

import star.list;
import star.maybe;
import star.io_device;
import star.string;
import star.either;
import star.host_address;
import star.thread;
import star.socket;
import star.tcp;
import std;

import star.p2p_networking_service;
import star.byte_array;
import star.map;
#include "StarDataStreamDevices.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;

namespace Star {

struct Packet;
enum class PacketType : std::uint8_t;

struct PacketStats {
  HashMap<PacketType, float> packetBytesPerSecond;
  float bytesPerSecond;
  PacketType worstPacketType;
  std::size_t worstPacketSize;
};

// Collects PacketStats over a given window of time.
class PacketStatCollector {
public:
  PacketStatCollector();
  PacketStatCollector(float calculationWindow);

  void mix(std::size_t size);
  void mix(PacketType type, std::size_t size, bool addToTotal = true);
  void mix(HashMap<PacketType, std::size_t> const& sizes, bool addToTotal = true);

  // Should always return packet statistics for the most recent completed
  // window of time
  PacketStats stats() const;

private:
  void calculate();

  float m_calculationWindow;
  PacketStats m_stats;
  Map<PacketType, float> m_unmixed;
  std::size_t m_totalBytes;
  std::int64_t m_lastMixTime;
};

// Interface for bidirectional communication using NetPackets, based around a
// simple non-blocking polling interface.  Communication is assumed to be done
// via writeData() and readData(), and any delay in calling writeData or
// readData may translate directly into increased latency.
class PacketSocket {
public:
  virtual ~PacketSocket() = default;

  virtual bool isOpen() const = 0;
  virtual void close() = 0;

  // Takes all packets from the given list and queues them for sending.
  virtual void sendPackets(List<std::shared_ptr<Packet>> packets) = 0;
  // Receives any packets from the incoming queue, if available
  virtual List<std::shared_ptr<Packet>> receivePackets() = 0;

  // Returns true if any sent packets on the queue are still not completely
  // written.
  virtual bool sentPacketsPending() const = 0;

  // Write all data possible without blocking, returns true if any data was
  // actually written.
  virtual bool writeData() = 0;
  // Read all data available without blocking, returns true if any data was
  // actually received.
  virtual bool readData() = 0;

  // Should return incoming / outgoing packet stats, if they are tracked.
  // Default implementations return nothing.
  virtual Maybe<PacketStats> incomingStats() const;
  virtual Maybe<PacketStats> outgoingStats() const;

  virtual void setNetRules(NetCompatibilityRules netRules);
  virtual NetCompatibilityRules netRules() const;

private:
  NetCompatibilityRules m_netRules;
};

class CompressedPacketSocket : public PacketSocket {
public:
  virtual ~CompressedPacketSocket() = default;

  virtual void setCompressionStreamEnabled(bool enabled);
  virtual bool compressionStreamEnabled() const;
private:
  bool m_useCompressionStream = false;
protected:
  CompressionStream m_compressionStream;
  DecompressionStream m_decompressionStream;
};

// PacketSocket for local communication.
class LocalPacketSocket : public PacketSocket {
public:
  static std::pair<std::unique_ptr<LocalPacketSocket>, std::unique_ptr<LocalPacketSocket>> openPair();

  bool isOpen() const override;
  void close() override;

  void sendPackets(List<std::shared_ptr<Packet>> packets) override;
  List<std::shared_ptr<Packet>> receivePackets() override;

  bool sentPacketsPending() const override;

  // write / read for local sockets is actually a no-op, sendPackets places
  // packets directly in the incoming queue of the paired local socket.
  bool writeData() override;
  bool readData() override;

private:
  struct Pipe {
    Mutex mutex;
    Deque<std::shared_ptr<Packet>> queue;
  };

  LocalPacketSocket(std::shared_ptr<Pipe> incomingPipe, std::weak_ptr<Pipe> outgoingPipe);

  // Ubuntu 22.04's libstdc++ lacks std::atomic<shared_ptr<T>>.
  std::shared_ptr<Pipe> m_incomingPipe;
  std::weak_ptr<Pipe> m_outgoingPipe;
};

// Wraps a TCP socket into a PacketSocket.
class TcpPacketSocket : public CompressedPacketSocket {
public:
  static std::unique_ptr<TcpPacketSocket> open(std::shared_ptr<TcpSocket> socket);

  bool isOpen() const override;
  void close() override;

  void sendPackets(List<std::shared_ptr<Packet>> packets) override;
  List<std::shared_ptr<Packet>> receivePackets() override;

  bool sentPacketsPending() const override;

  bool writeData() override;
  bool readData() override;

  Maybe<PacketStats> incomingStats() const override;
  Maybe<PacketStats> outgoingStats() const override;
private:
  TcpPacketSocket(std::shared_ptr<TcpSocket> socket);

  std::shared_ptr<TcpSocket> m_socket;

  PacketStatCollector m_incomingStats;
  PacketStatCollector m_outgoingStats;
  ByteArray m_outputBuffer;
  ByteArray m_inputBuffer;
  ByteArray m_compressedOutputBuffer;
};

// Wraps a P2PSocket into a PacketSocket
class P2PPacketSocket : public CompressedPacketSocket {
public:
  static std::unique_ptr<P2PPacketSocket> open(std::unique_ptr<P2PSocket> socket);

  bool isOpen() const override;
  void close() override;

  void sendPackets(List<std::shared_ptr<Packet>> packets) override;
  List<std::shared_ptr<Packet>> receivePackets() override;

  bool sentPacketsPending() const override;

  bool writeData() override;
  bool readData() override;

  Maybe<PacketStats> incomingStats() const override;
  Maybe<PacketStats> outgoingStats() const override;

private:
  P2PPacketSocket(std::shared_ptr<P2PSocket> socket);

  std::shared_ptr<P2PSocket> m_socket;

  PacketStatCollector m_incomingStats;
  PacketStatCollector m_outgoingStats;
  Deque<ByteArray> m_outputMessages;
  Deque<ByteArray> m_inputMessages;
};

}

export module star.net_packet_socket;

export namespace Star {
  using ::Star::PacketStats;
  using ::Star::PacketStatCollector;
  using ::Star::CompressedPacketSocket;
  using ::Star::PacketSocket;
  using ::Star::LocalPacketSocket;
  using ::Star::TcpPacketSocket;
  using ::Star::P2PPacketSocket;
}

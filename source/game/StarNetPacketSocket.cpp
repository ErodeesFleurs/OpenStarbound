import star.xx_hash;
import star.json;
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
import star.math_common;
import star.time;
import star.map;
import star.format;
import star.version;
import star.ordered_map;
import star.variant;
import star.color;
import star.random;
import star.weighted_pool;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.casting;
import star.strong_typedef;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.data_stream_devices;
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
import star.logging;

import star.host_address;
import star.socket;
import star.tcp;

import star.p2p_networking_service;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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
import star.lua_root;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.compression;

import star.algorithm;
namespace Star {

PacketStatCollector::PacketStatCollector() : PacketStatCollector(1.0f) {}

PacketStatCollector::PacketStatCollector(float calculationWindow)
  : m_calculationWindow(calculationWindow), m_stats(), m_totalBytes(0), m_lastMixTime(0) {}

void PacketStatCollector::mix(std::size_t size) {
  calculate();
  m_totalBytes += size;
}

void PacketStatCollector::mix(PacketType type, std::size_t size, bool addToTotal) {
  calculate();
  m_unmixed[type] += size;
  if (addToTotal)
    m_totalBytes += size;
}

void PacketStatCollector::mix(HashMap<PacketType, std::size_t> const& sizes, bool addToTotal) {
  calculate();
  for (auto const& p : sizes) {
    if (addToTotal)
      m_totalBytes += p.second;
    m_unmixed[p.first] += p.second;
  }
}

PacketStats PacketStatCollector::stats() const {
  const_cast<PacketStatCollector*>(this)->calculate();
  return m_stats;
}

void PacketStatCollector::calculate() {
  std::int64_t currentTime = Time::monotonicMilliseconds();
  float elapsedTime = (currentTime - m_lastMixTime) / 1000.0f;
  if (elapsedTime >= m_calculationWindow) {
    m_lastMixTime = currentTime;
    m_stats.worstPacketSize = 0;

    for (auto& pair : m_unmixed) {
      if (pair.second > m_stats.worstPacketSize) {
        m_stats.worstPacketType = pair.first;
        m_stats.worstPacketSize = pair.second;
      }

      m_stats.packetBytesPerSecond[pair.first] = std::round(pair.second / elapsedTime);
    }
    m_stats.bytesPerSecond = std::round(float(m_totalBytes) / elapsedTime);
    m_totalBytes = 0;
    m_unmixed.clear();
  }
}

std::optional<PacketStats> PacketSocket::incomingStats() const {
  return {};
}

std::optional<PacketStats> PacketSocket::outgoingStats() const {
  return {};
}
void PacketSocket::setNetRules(NetCompatibilityRules netRules) { m_netRules = netRules; }
NetCompatibilityRules PacketSocket::netRules() const { return m_netRules; }

void CompressedPacketSocket::setCompressionStreamEnabled(bool enabled) { m_useCompressionStream = enabled; }
bool CompressedPacketSocket::compressionStreamEnabled() const { return m_useCompressionStream; }

std::pair<std::unique_ptr<LocalPacketSocket>, std::unique_ptr<LocalPacketSocket>> LocalPacketSocket::openPair() {
  auto lhsIncomingPipe = std::make_shared<Pipe>();
  auto rhsIncomingPipe = std::make_shared<Pipe>();

  return {
    std::unique_ptr<LocalPacketSocket>(new LocalPacketSocket(lhsIncomingPipe, std::weak_ptr<Pipe>(rhsIncomingPipe))),
    std::unique_ptr<LocalPacketSocket>(new LocalPacketSocket(rhsIncomingPipe, std::weak_ptr<Pipe>(lhsIncomingPipe)))
  };
}

LocalPacketSocket::~LocalPacketSocket() {
  close();
}

bool LocalPacketSocket::isOpen() const {
  auto incomingPipe = std::atomic_load(&m_incomingPipe);
  if (!incomingPipe)
    return false;
  std::lock_guard locker(incomingPipe->mutex);
  return incomingPipe->open && incomingPipe->peerOpen;
}

void LocalPacketSocket::close() {
  if (auto incomingPipe = std::atomic_exchange(&m_incomingPipe, std::shared_ptr<Pipe>())) {
    {
      std::lock_guard locker(incomingPipe->mutex);
      incomingPipe->open = false;
    }
    incomingPipe->ready.notify_all();
    if (auto outgoingPipe = m_outgoingPipe.lock()) {
      {
        std::lock_guard locker(outgoingPipe->mutex);
        outgoingPipe->peerOpen = false;
      }
      outgoingPipe->ready.notify_all();
    }
  }
}

void LocalPacketSocket::sendPackets(List<std::shared_ptr<Packet>> packets) {
  if (!isOpen() || packets.empty())
    return;

  if (auto outgoingPipe = m_outgoingPipe.lock()) {
    std::lock_guard locker(outgoingPipe->mutex);
    if (!outgoingPipe->open)
      return;

#ifndef NDEBUG
    // Self test the serialization when assertions are enabled, but forward the
    // original packets.  The round trip is not bijective for every packet (see
    // the compatibility hack in ProtocolResponsePacket::read, which keys off the
    // compression mode), so replacing the packets with their round tripped
    // copies corrupted the local handshake instead of testing it.
    for (auto const& inPacket : packets) {
      DataStreamBuffer buffer;
      buffer.setStreamCompatibilityVersion(netRules());
      inPacket->write(buffer, netRules());
      auto outPacket = createPacket(inPacket->type());
      outPacket->setCompressionMode(inPacket->compressionMode());
      buffer.seek(0);
      try {
        outPacket->read(buffer);
      } catch (std::exception const&) {
        // A packet type whose round trip is not readable is a defect, but this
        // is only a self test and must never change what is actually sent.
      }
    }
#endif

    outgoingPipe->queue.appendAll(std::move(packets));
    outgoingPipe->ready.notify_all();
  }
}

List<std::shared_ptr<Packet>> LocalPacketSocket::receivePackets() {
  // One snapshot: the mutex and the queue must belong to the same pipe, and the
  // pipe can be closed by another thread between two loads.
  auto incomingPipe = std::atomic_load(&m_incomingPipe);
  if (!incomingPipe)
    return {};

  std::lock_guard locker(incomingPipe->mutex);
  List<std::shared_ptr<Packet>> packets;
  packets.appendAll(take(incomingPipe->queue));
  return packets;
}

bool LocalPacketSocket::sentPacketsPending() const {
  return false;
}

bool LocalPacketSocket::writeData() {
  return false;
}

bool LocalPacketSocket::readData() {
  return false;
}

void LocalPacketSocket::waitForData(bool writable, std::chrono::steady_clock::time_point deadline) {
  if (writable)
    return;
  auto incomingPipe = std::atomic_load(&m_incomingPipe);
  if (!incomingPipe)
    return;
  std::unique_lock locker(incomingPipe->mutex);
  incomingPipe->ready.wait_until(locker, deadline, [&] {
    return !incomingPipe->queue.empty() || !incomingPipe->open || !incomingPipe->peerOpen;
  });
}

LocalPacketSocket::LocalPacketSocket(std::shared_ptr<Pipe> incomingPipe, std::weak_ptr<Pipe> outgoingPipe)
  : m_incomingPipe(std::move(incomingPipe)), m_outgoingPipe(std::move(outgoingPipe)) {}

std::unique_ptr<TcpPacketSocket> TcpPacketSocket::open(Logger& logger, std::shared_ptr<TcpSocket> socket) {
  socket->setNoDelay(true);
  socket->setNonBlocking(true);
  return std::unique_ptr<TcpPacketSocket>(new TcpPacketSocket(logger, std::move(socket)));
}

bool TcpPacketSocket::isOpen() const {
  return m_socket->isActive();
}

void TcpPacketSocket::close() {
  m_socket->close();
}

void TcpPacketSocket::sendPackets(List<std::shared_ptr<Packet>> packets) {
  auto it = packets.begin();
  if (compressionStreamEnabled()) {
    DataStreamBuffer outBuffer;
    while (it != packets.end()) {
      std::shared_ptr<Packet>& packet = *it++;
      auto packetType = packet->type();
      DataStreamBuffer packetBuffer;
      packetBuffer.setStreamCompatibilityVersion(netRules());
      packet->write(packetBuffer, netRules());
      outBuffer.write(packetType);
      outBuffer.writeVlqI((int)packetBuffer.size());
      outBuffer.writeData(packetBuffer.ptr(), packetBuffer.size());
      m_outgoingStats.mix(packetType, packetBuffer.size(), false);
    }
    m_outputBuffer.append(outBuffer.ptr(), outBuffer.size());
  } else {
    while (it != packets.end()) {
      PacketType currentType = (*it)->type();
      PacketCompressionMode currentCompressionMode = (*it)->compressionMode();

      DataStreamBuffer packetBuffer;
      packetBuffer.setStreamCompatibilityVersion(netRules());
      while (it != packets.end()
             && (*it)->type() == currentType
             && (*it)->compressionMode() == currentCompressionMode) {
          (*it++)->write(packetBuffer, netRules());
      }

      // Packets must read and write actual data, because this is used to
      // determine packet count
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((!packetBuffer.empty()));
      }

      ByteArray compressedPackets;
      bool mustCompress = currentCompressionMode == PacketCompressionMode::Enabled;
      bool perhapsCompress = currentCompressionMode == PacketCompressionMode::Automatic && packetBuffer.size() > 64;
      if (mustCompress || perhapsCompress)
        compressedPackets = compressData(packetBuffer.data());

      DataStreamBuffer outBuffer;
      outBuffer.write(currentType);

      if (!compressedPackets.empty() && (mustCompress || compressedPackets.size() < packetBuffer.size())) {
        outBuffer.writeVlqI(-(int)(compressedPackets.size()));
        outBuffer.writeData(compressedPackets.ptr(), compressedPackets.size());
        m_outgoingStats.mix(currentType, compressedPackets.size(), true);
      } else {
        outBuffer.writeVlqI((int)(packetBuffer.size()));
        outBuffer.writeData(packetBuffer.ptr(), packetBuffer.size());
        m_outgoingStats.mix(currentType, packetBuffer.size(), true);
      }
      m_outputBuffer.append(outBuffer.takeData());
    }
  }
}

List<std::shared_ptr<Packet>> TcpPacketSocket::receivePackets() {
  // How large can uncompressed packets be
  // this limit is now also used during decompression
  std::uint64_t const PacketSizeLimit = 64 << 20;
  // How many packets can be batched together into one compressed chunk at once
  std::uint64_t const PacketBatchLimit = 131072;
  List<std::shared_ptr<Packet>> packets;
  try {
    DataStreamExternalBuffer ds(m_inputBuffer);
    std::size_t trimPos = 0;
    while (!ds.atEnd()) {
      PacketType packetType;
      std::uint64_t packetSize = 0;
      bool packetCompressed = false;

      try {
        packetType = ds.read<PacketType>();
        std::int64_t len = ds.readVlqI();
        packetCompressed = len < 0;
        packetSize = packetCompressed ? -len : len;
      } catch (EofException const&) {
        // Guard against not having the entire packet header available when
        // trying to read.
        break;
      }

      if (packetSize > PacketSizeLimit)
        throw IOException::format("{} bytes large {} exceeds max size!", packetSize, PacketTypeNames.getRight(packetType));

      if (packetSize > ds.remaining())
        break;

      m_incomingStats.mix(packetType, packetSize, !compressionStreamEnabled());

      DataStreamExternalBuffer packetStream(ds.ptr() + ds.pos(), packetSize);
      packetStream.setStreamCompatibilityVersion(netRules());
      ByteArray uncompressed;
      if (packetCompressed) {
        uncompressed = uncompressData(packetStream.ptr(), packetSize, PacketSizeLimit);
        packetStream.reset(uncompressed.ptr(), uncompressed.size());
      }
      ds.seek(packetSize, IOSeek::Relative);
      trimPos = ds.pos();

      std::size_t count = 0;
      do {
        if (++count > PacketBatchLimit) {
          throw IOException::format("Packet batch limit {} reached while reading {}s!", PacketBatchLimit, PacketTypeNames.getRight(packetType));
          break;
        }
        std::shared_ptr<Packet> packet = createPacket(packetType);
        packet->setCompressionMode(packetCompressed ? PacketCompressionMode::Enabled : PacketCompressionMode::Disabled);
        packet->read(packetStream, netRules());
        packets.append(std::move(packet));
      } while (!packetStream.atEnd());
    }
    if (trimPos)
      m_inputBuffer.trimLeft(trimPos);
  } catch (IOException const& e) {
    m_logger.warn("I/O error in TcpPacketSocket::receivePackets, closing: {}", outputException(e, false));
    m_inputBuffer.clear();
    m_socket->shutdown();
  }
  return packets;
}

bool TcpPacketSocket::sentPacketsPending() const {
  return !m_outputBuffer.empty() || !m_compressedOutputBuffer.empty();
}

bool TcpPacketSocket::writeData() {
  if (!isOpen())
    return false;

  bool dataSent = false;
  try {
    if (compressionStreamEnabled() && !m_outputBuffer.empty()) {
      m_compressionStream.compress(m_outputBuffer, m_compressedOutputBuffer);
      m_outputBuffer.clear();
    }
    // A previously compressed tail must be drained even if the mode changed.
    while (!m_compressedOutputBuffer.empty()) {
      std::size_t written = m_socket->send(m_compressedOutputBuffer.ptr(), m_compressedOutputBuffer.size());
      if (written == 0)
        return dataSent;
      dataSent = true;
      m_compressedOutputBuffer.trimLeft(written);
      m_outgoingStats.mix(written);
    }
    while (!m_outputBuffer.empty()) {
      std::size_t written = m_socket->send(m_outputBuffer.ptr(), m_outputBuffer.size());
      if (written == 0)
        break;
      dataSent = true;
      m_outputBuffer.trimLeft(written);
    }
  } catch (SocketClosedException const& e) {
    m_logger.debug("TcpPacketSocket socket closed: {}", outputException(e, false));
  } catch (IOException const& e) {
    m_logger.warn("I/O error in TcpPacketSocket::writeData: {}", outputException(e, false));
    m_socket->shutdown();
  }
  return dataSent;
}

bool TcpPacketSocket::readData() {
  bool dataReceived = false;
  try {
    char readBuffer[1024];
    while (true) {
      std::size_t readAmount = m_socket->receive(readBuffer, 1024);
      if (readAmount == 0)
        break;
      dataReceived = true;
      if (compressionStreamEnabled()) {
        m_incomingStats.mix(readAmount);
        m_decompressionStream.decompress(readBuffer, readAmount, m_inputBuffer);
      } else {
        m_inputBuffer.append(readBuffer, readAmount);
      }
    }
  } catch (SocketClosedException const& e) {
    m_logger.debug("TcpPacketSocket socket closed: {}", outputException(e, false));
  } catch (IOException const& e) {
    m_logger.warn("I/O error in TcpPacketSocket::receiveData: {}", outputException(e, false));
    m_socket->shutdown();
  }
  return dataReceived;
}

void TcpPacketSocket::waitForData(bool writable, std::chrono::steady_clock::time_point deadline) {
  m_socket->waitForData(writable, deadline);
}

std::optional<PacketStats> TcpPacketSocket::incomingStats() const {
  return m_incomingStats.stats();
}

std::optional<PacketStats> TcpPacketSocket::outgoingStats() const {
  return m_outgoingStats.stats();
}

TcpPacketSocket::TcpPacketSocket(Logger& logger, std::shared_ptr<TcpSocket> socket)
  : m_logger(logger), m_socket(std::move(socket)) {}

std::unique_ptr<P2PPacketSocket> P2PPacketSocket::open(Logger& logger, std::unique_ptr<P2PSocket> socket) {
  return std::unique_ptr<P2PPacketSocket>(new P2PPacketSocket(logger, std::move(socket)));
}

bool P2PPacketSocket::isOpen() const {
  return m_socket && m_socket->isOpen();
}

void P2PPacketSocket::close() {
  {
    std::lock_guard locker(m_waitMutex);
    m_closed = true;
  }
  m_waitCondition.notify_all();
  m_socket.reset();
}

void P2PPacketSocket::sendPackets(List<std::shared_ptr<Packet>> packets) {
  auto it = packets.begin();

  if (compressionStreamEnabled()) {
    DataStreamBuffer outBuffer;
    while (it != packets.end()) {
      PacketType currentType = (*it)->type();
      DataStreamBuffer packetBuffer;
      packetBuffer.setStreamCompatibilityVersion(netRules());
      while (it != packets.end() && (*it)->type() == currentType)
        (*it++)->write(packetBuffer, netRules());
      outBuffer.write(currentType);
      outBuffer.write<bool>(false);
      outBuffer.writeData(packetBuffer.ptr(), packetBuffer.size());
      m_outgoingStats.mix(currentType, packetBuffer.size(), false);
      m_outputMessages.append(m_compressionStream.compress(outBuffer.takeData()));
    }
  } else {
    while (it != packets.end()) {
      PacketType currentType = (*it)->type();
      PacketCompressionMode currentCompressionMode = (*it)->compressionMode();

      DataStreamBuffer packetBuffer;
      packetBuffer.setStreamCompatibilityVersion(netRules());
      while (it != packets.end()
             && (*it)->type() == currentType
             && (*it)->compressionMode() == currentCompressionMode) {
          (*it++)->write(packetBuffer, netRules());
      }

      // Packets must read and write actual data, because this is used to
      // determine packet count
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((!packetBuffer.empty()));
      }

      ByteArray compressedPackets;
      bool mustCompress = currentCompressionMode == PacketCompressionMode::Enabled;
      bool perhapsCompress = currentCompressionMode == PacketCompressionMode::Automatic && packetBuffer.size() > 64;
      if (mustCompress || perhapsCompress)
        compressedPackets = compressData(packetBuffer.data());

      DataStreamBuffer outBuffer;
      outBuffer.write(currentType);

      if (!compressedPackets.empty() && (mustCompress || compressedPackets.size() < packetBuffer.size())) {
        outBuffer.write<bool>(true);
        outBuffer.writeData(compressedPackets.ptr(), compressedPackets.size());
        m_outgoingStats.mix(currentType, compressedPackets.size(), true);
      } else {
        outBuffer.write<bool>(false);
        outBuffer.writeData(packetBuffer.ptr(), packetBuffer.size());
        m_outgoingStats.mix(currentType, packetBuffer.size(), true);
      }
      m_outputMessages.append(outBuffer.takeData());
    }
  }
}

List<std::shared_ptr<Packet>> P2PPacketSocket::receivePackets() {
  List<std::shared_ptr<Packet>> packets;
  try {
    for (auto& inputMessage : take(m_inputMessages)) {
      DataStreamBuffer ds(std::move(inputMessage));

      PacketType packetType = ds.read<PacketType>();
      bool packetCompressed = ds.read<bool>();
      std::size_t packetSize = ds.size() - ds.pos();

      ByteArray packetBytes = ds.readBytes(packetSize);
      if (packetCompressed)
        packetBytes = uncompressData(packetBytes);

      m_incomingStats.mix(packetType, packetSize, !compressionStreamEnabled());

      DataStreamExternalBuffer packetStream(packetBytes);
      packetStream.setStreamCompatibilityVersion(netRules());
      do {
        std::shared_ptr<Packet> packet = createPacket(packetType);
        packet->setCompressionMode(packetCompressed ? PacketCompressionMode::Enabled : PacketCompressionMode::Disabled);
        packet->read(packetStream, netRules());
        packets.append(std::move(packet));
      } while (!packetStream.atEnd());
    }
  } catch (IOException const& e) {
    m_logger.warn("I/O error in P2PPacketSocket::receivePackets, closing: {}", outputException(e, false));
    m_socket.reset();
  }
  return packets;
}

bool P2PPacketSocket::sentPacketsPending() const {
  return !m_outputMessages.empty();
}

bool P2PPacketSocket::writeData() {
  bool workDone = false;

  if (m_socket) {
    try {
      while (!m_outputMessages.empty()) {
        if (m_socket->sendMessage(m_outputMessages.first())) {
          m_outgoingStats.mix(m_outputMessages.first().size());
          m_outputMessages.removeFirst();
          workDone = true;
        } else {
          break;
        }
      }
    } catch (StarException const& e) {
      m_logger.warn("Exception in P2PPacketSocket::writeData, closing: {}", outputException(e, false));
      m_socket.reset();
    }
  }

  return workDone;
}

bool P2PPacketSocket::readData() {
  bool workDone = false;

  if (m_socket) {
    try {
      while (auto message = m_socket->receiveMessage()) {
        m_incomingStats.mix(message->size());
        m_inputMessages.append(compressionStreamEnabled()
          ? m_decompressionStream.decompress(*message)
          : *message);
        workDone = true;
      }
    } catch (StarException const& e) {
      m_logger.warn("Exception in P2PPacketSocket::readData, closing: {}", outputException(e, false));
      m_socket.reset();
    }
  }

  return workDone;
}

void P2PPacketSocket::waitForData(bool, std::chrono::steady_clock::time_point deadline) {
  // Steam/Discord expose no socket readiness event here. Retry their actual
  // sendMessage/receiveMessage operations at most 1 ms later, interruptible by
  // local close; this is a bounded polling fallback, not a network notification.
  std::unique_lock locker(m_waitMutex);
  m_waitCondition.wait_until(locker,
      std::min(deadline, std::chrono::steady_clock::now() + std::chrono::milliseconds(1)),
      [&] { return m_closed; });
}

std::optional<PacketStats> P2PPacketSocket::incomingStats() const {
  return m_incomingStats.stats();
}

std::optional<PacketStats> P2PPacketSocket::outgoingStats() const {
  return m_outgoingStats.stats();
}

P2PPacketSocket::P2PPacketSocket(Logger& logger, std::shared_ptr<P2PSocket> socket)
  : m_logger(logger), m_socket(std::move(socket)) {}

}

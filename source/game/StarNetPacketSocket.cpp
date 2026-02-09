#include "StarNetPacketSocket.hpp"
#include "StarCompression.hpp"
#include "StarConfig.hpp"
#include "StarIterator.hpp"
#include "StarLogging.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

PacketStatCollector::PacketStatCollector(float calculationWindow)
    : m_calculationWindow(calculationWindow), m_stats(), m_totalBytes(0), m_lastMixTime(0) {}

void PacketStatCollector::mix(size_t size) {
  calculate();
  m_totalBytes += size;
}

void PacketStatCollector::mix(PacketType type, size_t size, bool addToTotal) {
  calculate();
  m_unmixed[type] += size;
  if (addToTotal)
    m_totalBytes += size;
}

void PacketStatCollector::mix(HashMap<PacketType, size_t> const& sizes, bool addToTotal) {
  calculate();
  for (auto const& p : sizes) {
    if (addToTotal)
      m_totalBytes += p.second;
    m_unmixed[p.first] += p.second;
  }
}

auto PacketStatCollector::stats() const -> PacketStats {
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

auto PacketSocket::incomingStats() const -> std::optional<PacketStats> {
  return {};
}

auto PacketSocket::outgoingStats() const -> std::optional<PacketStats> {
  return {};
}
void PacketSocket::setNetRules(NetCompatibilityRules netRules) { m_netRules = netRules; }
auto PacketSocket::netRules() const -> NetCompatibilityRules { return m_netRules; }

void CompressedPacketSocket::setCompressionStreamEnabled(bool enabled) { m_useCompressionStream = enabled; }
auto CompressedPacketSocket::compressionStreamEnabled() const -> bool { return m_useCompressionStream; }

auto LocalPacketSocket::openPair() -> std::pair<UPtr<LocalPacketSocket>, UPtr<LocalPacketSocket>> {
  auto lhsIncomingPipe = std::make_shared<Pipe>();
  auto rhsIncomingPipe = std::make_shared<Pipe>();

  return {
    UPtr<LocalPacketSocket>(new LocalPacketSocket(lhsIncomingPipe, std::weak_ptr<Pipe>(rhsIncomingPipe))),
    UPtr<LocalPacketSocket>(new LocalPacketSocket(rhsIncomingPipe, std::weak_ptr<Pipe>(lhsIncomingPipe)))};
}

auto LocalPacketSocket::isOpen() const -> bool {
  return m_incomingPipe && !m_outgoingPipe.expired();
}

void LocalPacketSocket::close() {
  m_incomingPipe.reset();
}

void LocalPacketSocket::sendPackets(List<Ptr<Packet>> packets) {
  if (!isOpen() || packets.empty())
    return;

  if (auto outgoingPipe = m_outgoingPipe.lock()) {
    MutexLocker locker(outgoingPipe->mutex);

#ifdef STAR_DEBUG
    // Test serialization if STAR_DEBUG is enabled
    DataStreamBuffer buffer;
    for (auto inPacket : take(packets)) {
      buffer.clear();
      inPacket->write(buffer);
      auto outPacket = createPacket(inPacket->type());
      buffer.seek(0);
      outPacket->read(buffer);
      packets.append(outPacket);
    }
#endif

    outgoingPipe->queue.appendAll(std::move(packets));
  }
}

auto LocalPacketSocket::receivePackets() -> List<Ptr<Packet>> {
  MutexLocker locker(m_incomingPipe->mutex);
  List<Ptr<Packet>> packets;
  packets.appendAll(take(m_incomingPipe->queue));
  return packets;
}

auto LocalPacketSocket::sentPacketsPending() const -> bool {
  return false;
}

auto LocalPacketSocket::writeData() -> bool {
  return false;
}

auto LocalPacketSocket::readData() -> bool {
  return false;
}

LocalPacketSocket::LocalPacketSocket(std::shared_ptr<Pipe> incomingPipe, std::weak_ptr<Pipe> outgoingPipe)
    : m_incomingPipe(std::move(incomingPipe)), m_outgoingPipe(std::move(outgoingPipe)) {}

auto TcpPacketSocket::open(Ptr<TcpSocket> socket) -> UPtr<TcpPacketSocket> {
  socket->setNoDelay(true);
  socket->setNonBlocking(true);
  return UPtr<TcpPacketSocket>(new TcpPacketSocket(std::move(socket)));
}

auto TcpPacketSocket::isOpen() const -> bool {
  return m_socket->isActive();
}

void TcpPacketSocket::close() {
  m_socket->close();
}

void TcpPacketSocket::sendPackets(List<Ptr<Packet>> packets) {
  auto it = makeSMutableIterator(packets);
  if (compressionStreamEnabled()) {
    DataStreamBuffer outBuffer;
    while (it.hasNext()) {
      Ptr<Packet>& packet = it.next();
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
    while (it.hasNext()) {
      PacketType currentType = it.peekNext()->type();
      PacketCompressionMode currentCompressionMode = it.peekNext()->compressionMode();

      DataStreamBuffer packetBuffer;
      packetBuffer.setStreamCompatibilityVersion(netRules());
      while (it.hasNext()
             && it.peekNext()->type() == currentType
             && it.peekNext()->compressionMode() == currentCompressionMode) {
        it.next()->write(packetBuffer, netRules());
      }

      // Packets must read and write actual data, because this is used to
      // determine packet count

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
        m_outgoingStats.mix(currentType, compressedPackets.size());
      } else {
        outBuffer.writeVlqI((int)(packetBuffer.size()));
        outBuffer.writeData(packetBuffer.ptr(), packetBuffer.size());
        m_outgoingStats.mix(currentType, packetBuffer.size());
      }
      m_outputBuffer.append(outBuffer.takeData());
    }
  }
}

auto TcpPacketSocket::receivePackets() -> List<Ptr<Packet>> {
  // How large can uncompressed packets be
  // this limit is now also used during decompression
  std::uint64_t const PacketSizeLimit = 64 << 20;
  // How many packets can be batched together into one compressed chunk at once
  std::uint64_t const PacketBatchLimit = 131072;
  List<Ptr<Packet>> packets;
  try {
    DataStreamExternalBuffer ds(m_inputBuffer);
    size_t trimPos = 0;
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

      size_t count = 0;
      do {
        if (++count > PacketBatchLimit) {
          throw IOException::format("Packet batch limit {} reached while reading {}s!", PacketBatchLimit, PacketTypeNames.getRight(packetType));
          break;
        }
        Ptr<Packet> packet = createPacket(packetType);
        packet->setCompressionMode(packetCompressed ? PacketCompressionMode::Enabled : PacketCompressionMode::Disabled);
        packet->read(packetStream, netRules());
        packets.append(std::move(packet));
      } while (!packetStream.atEnd());
    }
    if (trimPos)
      m_inputBuffer.trimLeft(trimPos);
  } catch (IOException const& e) {
    Logger::warn("I/O error in TcpPacketSocket::receivePackets, closing: {}", outputException(e, false));
    m_inputBuffer.clear();
    m_socket->shutdown();
  }
  return packets;
}

auto TcpPacketSocket::sentPacketsPending() const -> bool {
  return !m_outputBuffer.empty() || (compressionStreamEnabled() && !m_compressedOutputBuffer.empty());
}

auto TcpPacketSocket::writeData() -> bool {
  if (!isOpen())
    return false;

  bool dataSent = false;
  try {
    if (!m_outputBuffer.empty() || !m_compressedOutputBuffer.empty()) {
      if (compressionStreamEnabled()) {
        if (!m_outputBuffer.empty()) {
          m_compressionStream.compress(m_outputBuffer, m_compressedOutputBuffer);
          m_outputBuffer.clear();
        }
        do {
          size_t written = m_socket->send(m_compressedOutputBuffer.ptr(), m_compressedOutputBuffer.size());
          if (written == 0)
            break;
          dataSent = true;
          m_compressedOutputBuffer.trimLeft(written);
          m_outgoingStats.mix(written);
        } while (!m_compressedOutputBuffer.empty());
      } else {
        do {
          size_t written = m_socket->send(m_outputBuffer.ptr(), m_outputBuffer.size());
          if (written == 0)
            break;
          dataSent = true;
          m_outputBuffer.trimLeft(written);
        } while (!m_outputBuffer.empty());
      }
    }
  } catch (SocketClosedException const& e) {
    Logger::debug("TcpPacketSocket socket closed: {}", outputException(e, false));
  } catch (IOException const& e) {
    Logger::warn("I/O error in TcpPacketSocket::writeData: {}", outputException(e, false));
    m_socket->shutdown();
  }
  return dataSent;
}

auto TcpPacketSocket::readData() -> bool {
  bool dataReceived = false;
  try {
    std::array<char, 1024> readBuffer;
    while (true) {
      size_t readAmount = m_socket->receive(readBuffer.data(), 1024);
      if (readAmount == 0)
        break;
      dataReceived = true;
      if (compressionStreamEnabled()) {
        m_incomingStats.mix(readAmount);
        m_decompressionStream.decompress(readBuffer.data(), readAmount, m_inputBuffer);
      } else {
        m_inputBuffer.append(readBuffer.data(), readAmount);
      }
    }
  } catch (SocketClosedException const& e) {
    Logger::debug("TcpPacketSocket socket closed: {}", outputException(e, false));
  } catch (IOException const& e) {
    Logger::warn("I/O error in TcpPacketSocket::receiveData: {}", outputException(e, false));
    m_socket->shutdown();
  }
  return dataReceived;
}

auto TcpPacketSocket::incomingStats() const -> std::optional<PacketStats> {
  return m_incomingStats.stats();
}

auto TcpPacketSocket::outgoingStats() const -> std::optional<PacketStats> {
  return m_outgoingStats.stats();
}

TcpPacketSocket::TcpPacketSocket(Ptr<TcpSocket> socket) : m_socket(std::move(socket)) {}

auto P2PPacketSocket::open(UPtr<P2PSocket> socket) -> UPtr<P2PPacketSocket> {
  return UPtr<P2PPacketSocket>(new P2PPacketSocket(std::move(socket)));
}

auto P2PPacketSocket::isOpen() const -> bool {
  return m_socket && m_socket->isOpen();
}

void P2PPacketSocket::close() {
  m_socket.reset();
}

void P2PPacketSocket::sendPackets(List<Ptr<Packet>> packets) {
  auto it = makeSMutableIterator(packets);

  if (compressionStreamEnabled()) {
    DataStreamBuffer outBuffer;
    while (it.hasNext()) {
      PacketType currentType = it.peekNext()->type();
      DataStreamBuffer packetBuffer;
      packetBuffer.setStreamCompatibilityVersion(netRules());
      while (it.hasNext() && it.peekNext()->type() == currentType)
        it.next()->write(packetBuffer, netRules());
      outBuffer.write(currentType);
      outBuffer.write<bool>(false);
      outBuffer.writeData(packetBuffer.ptr(), packetBuffer.size());
      m_outgoingStats.mix(currentType, packetBuffer.size(), false);
      m_outputMessages.append(m_compressionStream.compress(outBuffer.takeData()));
    }
  } else {
    while (it.hasNext()) {
      PacketType currentType = it.peekNext()->type();
      PacketCompressionMode currentCompressionMode = it.peekNext()->compressionMode();

      DataStreamBuffer packetBuffer;
      packetBuffer.setStreamCompatibilityVersion(netRules());
      while (it.hasNext()
             && it.peekNext()->type() == currentType
             && it.peekNext()->compressionMode() == currentCompressionMode) {
        it.next()->write(packetBuffer, netRules());
      }

      // Packets must read and write actual data, because this is used to
      // determine packet count

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
        m_outgoingStats.mix(currentType, compressedPackets.size());
      } else {
        outBuffer.write<bool>(false);
        outBuffer.writeData(packetBuffer.ptr(), packetBuffer.size());
        m_outgoingStats.mix(currentType, packetBuffer.size());
      }
      m_outputMessages.append(outBuffer.takeData());
    }
  }
}

auto P2PPacketSocket::receivePackets() -> List<Ptr<Packet>> {
  List<Ptr<Packet>> packets;
  try {
    for (auto& inputMessage : take(m_inputMessages)) {
      DataStreamBuffer ds(std::move(inputMessage));

      auto packetType = ds.read<PacketType>();
      bool packetCompressed = ds.read<bool>();
      size_t packetSize = ds.size() - ds.pos();

      ByteArray packetBytes = ds.readBytes(packetSize);
      if (packetCompressed)
        packetBytes = uncompressData(packetBytes);

      m_incomingStats.mix(packetType, packetSize, !compressionStreamEnabled());

      DataStreamExternalBuffer packetStream(packetBytes);
      packetStream.setStreamCompatibilityVersion(netRules());
      do {
        Ptr<Packet> packet = createPacket(packetType);
        packet->setCompressionMode(packetCompressed ? PacketCompressionMode::Enabled : PacketCompressionMode::Disabled);
        packet->read(packetStream, netRules());
        packets.append(std::move(packet));
      } while (!packetStream.atEnd());
    }
  } catch (IOException const& e) {
    Logger::warn("I/O error in P2PPacketSocket::receivePackets, closing: {}", outputException(e, false));
    m_socket.reset();
  }
  return packets;
}

auto P2PPacketSocket::sentPacketsPending() const -> bool {
  return !m_outputMessages.empty();
}

auto P2PPacketSocket::writeData() -> bool {
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
      Logger::warn("Exception in P2PPacketSocket::writeData, closing: {}", outputException(e, false));
      m_socket.reset();
    }
  }

  return workDone;
}

auto P2PPacketSocket::readData() -> bool {
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
      Logger::warn("Exception in P2PPacketSocket::readData, closing: {}", outputException(e, false));
      m_socket.reset();
    }
  }

  return workDone;
}

auto P2PPacketSocket::incomingStats() const -> std::optional<PacketStats> {
  return m_incomingStats.stats();
}

auto P2PPacketSocket::outgoingStats() const -> std::optional<PacketStats> {
  return m_outgoingStats.stats();
}

P2PPacketSocket::P2PPacketSocket(Ptr<P2PSocket> socket)
    : m_socket(std::move(socket)) {}

}// namespace Star

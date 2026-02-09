#include "StarUniverseConnection.hpp"
#include "StarConfig.hpp"
#include "StarFormat.hpp"
#include "StarLogging.hpp"
#include "StarTime.hpp"

import std;

namespace Star {

static constexpr std::int32_t PacketSocketPollSleep = 1;

UniverseConnection::UniverseConnection(UPtr<PacketSocket> packetSocket)
    : m_packetSocket(std::move(packetSocket)) {}

UniverseConnection::UniverseConnection(UniverseConnection&& rhs) {
  operator=(std::move(rhs));
}

UniverseConnection::~UniverseConnection() {
  if (m_packetSocket)
    m_packetSocket->close();
}

auto UniverseConnection::operator=(UniverseConnection&& rhs) -> UniverseConnection& {
  MutexLocker locker(m_mutex);
  m_sendQueue = take(rhs.m_sendQueue);
  m_receiveQueue = take(rhs.m_receiveQueue);
  m_packetSocket = take(rhs.m_packetSocket);
  return *this;
}

auto UniverseConnection::isOpen() const -> bool {
  MutexLocker locker(m_mutex);
  return m_packetSocket->isOpen();
}

void UniverseConnection::close() {
  MutexLocker locker(m_mutex);
  m_packetSocket->close();
}

void UniverseConnection::push(List<Ptr<Packet>> packets) {
  MutexLocker locker(m_mutex);
  m_sendQueue.appendAll(std::move(packets));
}

void UniverseConnection::pushSingle(Ptr<Packet> packet) {
  MutexLocker locker(m_mutex);
  m_sendQueue.append(std::move(packet));
}

auto UniverseConnection::pull() -> List<Ptr<Packet>> {
  MutexLocker locker(m_mutex);
  return List<Ptr<Packet>>::from(take(m_receiveQueue));
}

auto UniverseConnection::pullSingle() -> Ptr<Packet> {
  MutexLocker locker(m_mutex);
  if (m_receiveQueue.empty())
    return {};
  return m_receiveQueue.takeFirst();
}

auto UniverseConnection::send() -> bool {
  MutexLocker locker(m_mutex);
  m_packetSocket->sendPackets(take(m_sendQueue));
  return m_packetSocket->writeData();
}

auto UniverseConnection::sendAll(unsigned timeout) -> bool {
  MutexLocker locker(m_mutex);

  m_packetSocket->sendPackets(take(m_sendQueue));

  auto timer = Timer::withMilliseconds(timeout);
  while (true) {
    m_packetSocket->writeData();
    if (!m_packetSocket->sentPacketsPending())
      return true;

    if (timer.timeUp() || !m_packetSocket->isOpen())
      return false;

    Thread::sleep(PacketSocketPollSleep);
  }
}

auto UniverseConnection::receive() -> bool {
  MutexLocker locker(m_mutex);
  bool received = m_packetSocket->readData();
  m_receiveQueue.appendAll(m_packetSocket->receivePackets());
  return received;
}

auto UniverseConnection::receiveAny(unsigned timeout) -> bool {
  MutexLocker locker(m_mutex);
  if (!m_receiveQueue.empty())
    return true;

  auto timer = Timer::withMilliseconds(timeout);
  while (true) {
    m_packetSocket->readData();
    m_receiveQueue.appendAll(m_packetSocket->receivePackets());
    if (!m_receiveQueue.empty())
      return true;

    if (timer.timeUp() || !m_packetSocket->isOpen())
      return false;

    Thread::sleep(PacketSocketPollSleep);
  }
}

auto UniverseConnection::packetSocket() -> PacketSocket& {
  return *m_packetSocket;
}

auto UniverseConnection::incomingStats() const -> std::optional<PacketStats> {
  MutexLocker locker(m_mutex);
  return m_packetSocket->incomingStats();
}

auto UniverseConnection::outgoingStats() const -> std::optional<PacketStats> {
  MutexLocker locker(m_mutex);
  return m_packetSocket->outgoingStats();
}

UniverseConnectionServer::UniverseConnectionServer(PacketReceiveCallback packetReceiver, size_t numWorkerThreads)
    : m_packetReceiver(std::move(packetReceiver)), m_shutdown(false) {
  if (numWorkerThreads == 0)
    m_numWorkerThreads = std::max<size_t>(2, std::thread::hardware_concurrency() / 4);
  else
    m_numWorkerThreads = numWorkerThreads;

  Logger::info("UniverseConnectionServer: Starting {} network worker threads", m_numWorkerThreads);

  m_workerStats.resize(m_numWorkerThreads);

  for (size_t i = 0; i < m_numWorkerThreads; ++i) {
    m_processingThreads.append(Thread::invoke(strf("UniverseConnectionServer::worker_{}", i), [this, i]() -> void {
      RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
      try {
        while (!m_shutdown) {
          connectionsLocker.lock();
          auto connections = m_connections.pairs();
          connectionsLocker.unlock();

          bool dataTransmitted = false;
          size_t handledCount = 0;
          for (auto& p : connections) {
            if (p.second->workerIndex != i)
              continue;

            handledCount++;
            MutexLocker connectionLocker(p.second->mutex);
            if (!p.second->packetSocket || !p.second->packetSocket->isOpen())
              continue;

            p.second->packetSocket->sendPackets(take(p.second->sendQueue));
            dataTransmitted |= p.second->packetSocket->writeData();

            dataTransmitted |= p.second->packetSocket->readData();
            List<Ptr<Packet>> receivePackets = p.second->packetSocket->receivePackets();
            if (!receivePackets.empty()) {
              p.second->lastActivityTime = Time::monotonicMilliseconds();
              m_workerStats[i].packetsProcessed += receivePackets.size();
              p.second->receiveQueue.appendAll(take(receivePackets));
            }

            if (!p.second->receiveQueue.empty()) {
              List<Ptr<Packet>> toReceive = List<Ptr<Packet>>::from(take(p.second->receiveQueue));
              connectionLocker.unlock();

              try {
                m_packetReceiver(this, p.first, std::move(toReceive));
              } catch (std::exception const& e) {
                Logger::error("Exception caught handling incoming server packets, disconnecting client '{}' {}", p.first, outputException(e, true));

                connectionLocker.lock();
                p.second->packetSocket->close();
              }
            }
          }
          m_workerStats[i].connectionsHandled = handledCount;

          if (!dataTransmitted)
            Thread::sleep(PacketSocketPollSleep);
        }
      } catch (std::exception const& e) {
        Logger::error("Exception caught in UniverseConnectionServer::worker_{}, closing assigned connections: {}", i, e.what());
        connectionsLocker.lock();
        for (auto& p : m_connections)
          if (p.second->workerIndex == i)
            p.second->packetSocket->close();
      }
    }));
  }
}

UniverseConnectionServer::~UniverseConnectionServer() {
  m_shutdown = true;
  for (auto& thread : m_processingThreads)
    thread.finish();
  removeAllConnections();
}

auto UniverseConnectionServer::hasConnection(ConnectionId clientId) const -> bool {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  return m_connections.contains(clientId);
}

auto UniverseConnectionServer::allConnections() const -> List<ConnectionId> {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  return m_connections.keys();
}

auto UniverseConnectionServer::connectionIsOpen(ConnectionId clientId) const -> bool {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (auto conn = m_connections.value(clientId)) {
    connectionsLocker.unlock();
    MutexLocker connectionLocker(conn->mutex);
    return conn->packetSocket->isOpen();
  }

  throw UniverseConnectionException::format("No such client '{}' in UniverseConnectionServer::connectionIsOpen", clientId);
}

auto UniverseConnectionServer::lastActivityTime(ConnectionId clientId) const -> std::int64_t {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (auto conn = m_connections.value(clientId)) {
    connectionsLocker.unlock();
    MutexLocker connectionLocker(conn->mutex);
    return conn->lastActivityTime;
  }
  throw UniverseConnectionException::format("No such client '{}' in UniverseConnectionServer::lastRemoteActivityTime", clientId);
}

void UniverseConnectionServer::addConnection(ConnectionId clientId, UniverseConnection uc) {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (m_connections.contains(clientId))
    throw UniverseConnectionException::format("Client '{}' already exists in UniverseConnectionServer::addConnection", clientId);

  auto connection = std::make_shared<Connection>();
  connection->packetSocket = std::move(uc.m_packetSocket);
  connection->sendQueue = std::move(uc.m_sendQueue);
  connection->receiveQueue = std::move(uc.m_receiveQueue);
  connection->lastActivityTime = Time::monotonicMilliseconds();
  connection->workerIndex = clientId % m_numWorkerThreads;
  m_connections.add(clientId, std::move(connection));
}

auto UniverseConnectionServer::removeConnection(ConnectionId clientId) -> UniverseConnection {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (!m_connections.contains(clientId))
    throw UniverseConnectionException::format("Client '{}' does not exist in UniverseConnectionServer::removeConnection", clientId);

  auto conn = m_connections.take(clientId);
  connectionsLocker.unlock();
  MutexLocker connectionLocker(conn->mutex);

  UniverseConnection uc;
  uc.m_packetSocket = take(conn->packetSocket);
  uc.m_sendQueue = std::move(conn->sendQueue);
  uc.m_receiveQueue = std::move(conn->receiveQueue);
  return uc;
}

auto UniverseConnectionServer::removeAllConnections() -> List<UniverseConnection> {
  List<UniverseConnection> removedConnections;
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  for (auto connectionId : m_connections.keys())
    removedConnections.append(removeConnection(connectionId));
  return removedConnections;
}

void UniverseConnectionServer::sendPackets(ConnectionId clientId, List<Ptr<Packet>> packets) {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (auto conn = m_connections.value(clientId)) {
    connectionsLocker.unlock();
    MutexLocker connectionLocker(conn->mutex);
    conn->sendQueue.appendAll(std::move(packets));

    if (conn->packetSocket->isOpen()) {
      conn->packetSocket->sendPackets(take(conn->sendQueue));
      conn->packetSocket->writeData();
    }
  } else {
    throw UniverseConnectionException::format("No such client '{}' in UniverseConnectionServer::sendPackets", clientId);
  }
}

auto UniverseConnectionServer::totalPacketsProcessed() const -> std::uint64_t {
  std::uint64_t total = 0;
  for (auto const& stats : m_workerStats)
    total += stats.packetsProcessed.load();
  return total;
}

auto UniverseConnectionServer::numWorkerThreads() const -> size_t {
  return m_numWorkerThreads;
}

}// namespace Star

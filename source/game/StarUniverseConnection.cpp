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
import star.time;
import star.version;
import star.ordered_map;
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
import star.format;
import star.logging;

import star.lua_root;
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
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;

import star.algorithm;
namespace Star {

static constexpr auto PacketSocketPollInterval = std::chrono::milliseconds(1);

UniverseConnection::UniverseConnection(std::unique_ptr<PacketSocket> packetSocket)
    : m_packetSocket(std::move(packetSocket)) {}

UniverseConnection::UniverseConnection(UniverseConnection&& rhs) {
  operator=(std::move(rhs));
}

UniverseConnection::~UniverseConnection() {
  if (m_packetSocket)
    m_packetSocket->close();
}

UniverseConnection& UniverseConnection::operator=(UniverseConnection&& rhs) {
  MutexLocker locker(m_mutex);
  m_sendQueue = take(rhs.m_sendQueue);
  m_receiveQueue = take(rhs.m_receiveQueue);
  m_packetSocket = take(rhs.m_packetSocket);
  return *this;
}

bool UniverseConnection::isOpen() const {
  MutexLocker locker(m_mutex);
  return m_packetSocket->isOpen();
}

void UniverseConnection::close() {
  MutexLocker locker(m_mutex);
  m_packetSocket->close();
}

void UniverseConnection::push(List<std::shared_ptr<Packet>> packets) {
  MutexLocker locker(m_mutex);
  m_sendQueue.appendAll(std::move(packets));
}

void UniverseConnection::pushSingle(std::shared_ptr<Packet> packet) {
  MutexLocker locker(m_mutex);
  m_sendQueue.append(std::move(packet));
}

List<std::shared_ptr<Packet>> UniverseConnection::pull() {
  MutexLocker locker(m_mutex);
  return List<std::shared_ptr<Packet>>::from(take(m_receiveQueue));
}

std::shared_ptr<Packet> UniverseConnection::pullSingle() {
  MutexLocker locker(m_mutex);
  if (m_receiveQueue.empty())
    return {};
  return m_receiveQueue.takeFirst();
}

bool UniverseConnection::send() {
  MutexLocker locker(m_mutex);
  m_packetSocket->sendPackets(take(m_sendQueue));
  return m_packetSocket->writeData();
}

bool UniverseConnection::sendAll(unsigned timeout) {
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
  MutexLocker locker(m_mutex);
  m_packetSocket->sendPackets(take(m_sendQueue));
  while (true) {
    m_packetSocket->writeData();
    if (!m_packetSocket->sentPacketsPending())
      return true;
    if (std::chrono::steady_clock::now() >= deadline || !m_packetSocket->isOpen())
      return false;

    auto* socket = m_packetSocket.get();
    locker.unlock();
    socket->waitForData(true, deadline);
    locker.lock();
  }
}

bool UniverseConnection::receive() {
  MutexLocker locker(m_mutex);
  bool received = m_packetSocket->readData();
  m_receiveQueue.appendAll(m_packetSocket->receivePackets());
  return received;
}

bool UniverseConnection::receiveAny(unsigned timeout) {
  auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
  MutexLocker locker(m_mutex);
  if (!m_receiveQueue.empty())
    return true;
  while (true) {
    m_packetSocket->readData();
    m_receiveQueue.appendAll(m_packetSocket->receivePackets());
    if (!m_receiveQueue.empty())
      return true;
    if (std::chrono::steady_clock::now() >= deadline || !m_packetSocket->isOpen())
      return false;

    auto* socket = m_packetSocket.get();
    locker.unlock();
    socket->waitForData(false, deadline);
    locker.lock();
  }
}

PacketSocket& UniverseConnection::packetSocket() {
  return *m_packetSocket;
}

std::optional<PacketStats> UniverseConnection::incomingStats() const {
  MutexLocker locker(m_mutex);
  return m_packetSocket->incomingStats();
}

std::optional<PacketStats> UniverseConnection::outgoingStats() const {
  MutexLocker locker(m_mutex);
  return m_packetSocket->outgoingStats();
}

UniverseConnectionServer::UniverseConnectionServer(Logger& logger, PacketReceiveCallback packetReceiver, std::size_t numWorkerThreads)
    : m_logger(logger), m_packetReceiver(std::move(packetReceiver)), m_shutdown(false) {
  if (numWorkerThreads == 0)
    m_numWorkerThreads = std::max<std::size_t>(2, std::thread::hardware_concurrency() / 4);
  else
    m_numWorkerThreads = numWorkerThreads;

  m_logger.info("UniverseConnectionServer: Starting {} network worker threads", m_numWorkerThreads);

  m_connections.resize(m_numWorkerThreads);
  m_workerStats.resize(m_numWorkerThreads);
  m_processingThreads.reserve(m_numWorkerThreads);

  try {
    for (std::size_t i = 0; i < m_numWorkerThreads; ++i) {
      m_processingThreads.append(Thread::invoke(strf("UniverseConnectionServer::worker_{}", i), [this, i]() {
      RecursiveMutexLocker connectionsLocker(m_connectionsMutex, false);
      try {
        while (!m_shutdown) {
          connectionsLocker.lock();
          auto connections = m_connections[i].pairs();
          connectionsLocker.unlock();

          bool dataTransmitted = false;
          std::size_t handledCount = 0;
          for (auto& p : connections) {
            handledCount++;
            MutexLocker connectionLocker(p.second->mutex);
            if (!p.second->packetSocket || !p.second->packetSocket->isOpen())
              continue;

            try {
              p.second->packetSocket->sendPackets(take(p.second->sendQueue));
              dataTransmitted |= p.second->packetSocket->writeData();

              dataTransmitted |= p.second->packetSocket->readData();
              List<std::shared_ptr<Packet>> receivePackets = p.second->packetSocket->receivePackets();
              if (!receivePackets.empty()) {
                p.second->lastActivityTime = Time::monotonicMilliseconds();
                m_workerStats[i].packetsProcessed += receivePackets.size();
                p.second->receiveQueue.appendAll(take(receivePackets));
              }

              if (!p.second->receiveQueue.empty()) {
                List<std::shared_ptr<Packet>> toReceive = List<std::shared_ptr<Packet>>::from(take(p.second->receiveQueue));
                connectionLocker.unlock();

                try {
                  m_packetReceiver(this, p.first, std::move(toReceive));
                } catch (std::exception const&) {
                  ExceptionDiagnostic const e(std::current_exception());
                  m_logger.error("Exception caught handling incoming server packets, disconnecting client '{}' {}", p.first, outputException(e, true));

                  connectionLocker.lock();
                  if (p.second->packetSocket)
                    p.second->packetSocket->close();
                }
              }
            } catch (std::exception const&) {
              ExceptionDiagnostic const e(std::current_exception());
              m_logger.error("Exception caught receiving incoming server packets, disconnecting client '{}' {}", p.first, outputException(e, true));
              
              connectionLocker.lock();
              if (p.second->packetSocket)
                p.second->packetSocket->close();
            }
          }
          m_workerStats[i].connectionsHandled = handledCount;

          if (!dataTransmitted) {
            // Heterogeneous server shards are serviced with a bounded scan.
            // This scheduler wake handles registry/work/stop changes, not
            // network readiness; synchronous waits use each transport's native
            // readiness interface instead.
            std::unique_lock workLocker(m_workMutex);
            if (!m_shutdown)
              m_workCondition.wait_for(workLocker, PacketSocketPollInterval);
          }
        }
      } catch (std::exception const& e) {
        m_logger.error("Exception caught in UniverseConnectionServer::worker_{}, closing assigned connections: {}", i, e.what());
        connectionsLocker.lock();
        for (auto& p : m_connections[i]) {
          MutexLocker connectionLocker(p.second->mutex);
          if (p.second->packetSocket)
            p.second->packetSocket->close();
        }
      }
      }));
    }
  } catch (...) {
    auto failure = std::current_exception();
    {
      std::lock_guard workLocker(m_workMutex);
      m_shutdown = true;
    }
    m_workCondition.notify_all();
    for (auto& thread : m_processingThreads) {
      try {
        thread.finish();
      } catch (...) {
        // Join every started thread, then preserve the startup failure.
      }
    }
    std::rethrow_exception(failure);
  }
}

UniverseConnectionServer::~UniverseConnectionServer() {
  {
    std::lock_guard workLocker(m_workMutex);
    m_shutdown = true;
  }
  m_workCondition.notify_all();
  for (auto& thread : m_processingThreads)
    thread.finish();
  removeAllConnections();
}

bool UniverseConnectionServer::hasConnection(ConnectionId clientId) const {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  return m_connections[clientId % m_numWorkerThreads].contains(clientId);
}

List<ConnectionId> UniverseConnectionServer::allConnections() const {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  List<ConnectionId> connections;
  for (auto const& shard : m_connections)
    connections.appendAll(shard.keys());
  return connections;
}

bool UniverseConnectionServer::connectionIsOpen(ConnectionId clientId) const {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (auto conn = m_connections[clientId % m_numWorkerThreads].value(clientId)) {
    MutexLocker connectionLocker(conn->mutex);
    connectionsLocker.unlock();
    return conn->packetSocket && conn->packetSocket->isOpen();
  }

  throw UniverseConnectionException::format("No such client '{}' in UniverseConnectionServer::connectionIsOpen", clientId);
}

std::int64_t UniverseConnectionServer::lastActivityTime(ConnectionId clientId) const {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (auto conn = m_connections[clientId % m_numWorkerThreads].value(clientId)) {
    MutexLocker connectionLocker(conn->mutex);
    connectionsLocker.unlock();
    return conn->lastActivityTime;
  }
  throw UniverseConnectionException::format("No such client '{}' in UniverseConnectionServer::lastRemoteActivityTime", clientId);
}

void UniverseConnectionServer::addConnection(ConnectionId clientId, UniverseConnection uc) {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  auto& shard = m_connections[clientId % m_numWorkerThreads];
  if (shard.contains(clientId))
    throw UniverseConnectionException::format("Client '{}' already exists in UniverseConnectionServer::addConnection", clientId);

  auto connection = std::make_shared<Connection>();
  connection->packetSocket = std::move(uc.m_packetSocket);
  connection->sendQueue = std::move(uc.m_sendQueue);
  connection->receiveQueue = std::move(uc.m_receiveQueue);
  connection->lastActivityTime = Time::monotonicMilliseconds();
  shard.add(clientId, std::move(connection));
  m_workCondition.notify_all();
}

UniverseConnection UniverseConnectionServer::removeConnection(ConnectionId clientId) {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  auto& shard = m_connections[clientId % m_numWorkerThreads];
  if (!shard.contains(clientId))
    throw UniverseConnectionException::format("Client '{}' does not exist in UniverseConnectionServer::removeConnection", clientId);

  auto conn = shard.take(clientId);
  MutexLocker connectionLocker(conn->mutex);
  connectionsLocker.unlock();

  UniverseConnection uc;
  uc.m_packetSocket = take(conn->packetSocket);
  uc.m_sendQueue = std::move(conn->sendQueue);
  uc.m_receiveQueue = std::move(conn->receiveQueue);
  return uc;
}

List<UniverseConnection> UniverseConnectionServer::removeAllConnections() {
  List<UniverseConnection> removedConnections;
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  for (auto& shard : m_connections)
    for (auto connectionId : shard.keys())
      removedConnections.append(removeConnection(connectionId));
  return removedConnections;
}

void UniverseConnectionServer::sendPackets(ConnectionId clientId, List<std::shared_ptr<Packet>> packets) {
  RecursiveMutexLocker connectionsLocker(m_connectionsMutex);
  if (auto conn = m_connections[clientId % m_numWorkerThreads].value(clientId)) {
    MutexLocker connectionLocker(conn->mutex);
    connectionsLocker.unlock();
    conn->sendQueue.appendAll(std::move(packets));

    if (conn->packetSocket && conn->packetSocket->isOpen()) {
      conn->packetSocket->sendPackets(take(conn->sendQueue));
      conn->packetSocket->writeData();
    }
    m_workCondition.notify_all();
  } else {
    throw UniverseConnectionException::format("No such client '{}' in UniverseConnectionServer::sendPackets", clientId);
  }
}

std::uint64_t UniverseConnectionServer::totalPacketsProcessed() const {
  std::uint64_t total = 0;
  for (auto const& stats : m_workerStats)
    total += stats.packetsProcessed.load();
  return total;
}

std::size_t UniverseConnectionServer::numWorkerThreads() const {
  return m_numWorkerThreads;
}

}// namespace Star

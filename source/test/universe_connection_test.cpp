#ifdef STAR_SYSTEM_FAMILY_WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

import star.xx_hash;
import star.json;
import star.io_device;
import star.string;
import star.either;
import star.thread;
import star.logging;
import std;
import star.id_map;
import star.data_stream;
import star.list;
import star.bi_map;
import star.game_types;
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

import star.host_address;
import star.socket;
import star.tcp;
import star.net_impl;

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
import star.universe_connection;
import star.game_test;
import star.root;

#include "gtest/gtest.h"

#ifdef STAR_TEST_WRAP_PTHREAD_CREATE
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

namespace {
thread_local int threadStartsBeforeFailure = -1;
thread_local std::atomic<unsigned>* completedInjectedThreads = nullptr;

struct InjectedThreadStart {
  void* (*function)(void*);
  void* argument;
  std::atomic<unsigned>* completed;
};
}

extern "C" int __real_pthread_create(pthread_t*, pthread_attr_t const*, void* (*)(void*), void*);

extern "C" int __wrap_pthread_create(pthread_t* thread, pthread_attr_t const* attributes,
    void* (*function)(void*), void* argument) {
  if (threadStartsBeforeFailure < 0)
    return __real_pthread_create(thread, attributes, function, argument);
  if (threadStartsBeforeFailure-- == 0) {
    threadStartsBeforeFailure = -1;
    return EAGAIN;
  }
  auto start = std::unique_ptr<InjectedThreadStart>(
      new (std::nothrow) InjectedThreadStart{function, argument, completedInjectedThreads});
  if (!start) {
    threadStartsBeforeFailure = -1;
    return EAGAIN;
  }
  auto run = [](void* argument) -> void* {
    std::unique_ptr<InjectedThreadStart> start(static_cast<InjectedThreadStart*>(argument));
    auto result = start->function(start->argument);
    ++*start->completed;
    return result;
  };
  int result = __real_pthread_create(thread, attributes, run, start.get());
  if (result == 0)
    start.release();
  return result;
}
#endif

using namespace Star;

namespace Star {
// Only this real-socket fixture changes native buffer sizes; production sockets
// retain their normal settings and expose no test/failure control API.
struct SocketTestAccess {
  static void setBufferSize(Socket& socket, int option, int bytes) {
    ReadLocker locker(socket.m_mutex);
    socket.m_impl->setSockOpt(SOL_SOCKET, option, &bytes, sizeof(bytes));
  }
};
}

unsigned const PacketCount = 20;
std::uint16_t const ServerPort = 55555;

unsigned const NumLocalASyncConnections = 5;
unsigned const NumRemoteASyncConnections = 5;
unsigned const ASyncSleepMillis = 5;

unsigned const NumLocalSyncConnections = 5;
unsigned const NumRemoteSyncConnections = 5;
unsigned const SyncWaitMillis = 10000;

class ASyncClientThread : public Thread {
public:
  ASyncClientThread(UniverseConnection conn)
    : Thread("UniverseConnectionTestClientThread"), m_connection(std::move(conn)) {
    start();
  }

  ~ASyncClientThread() override {
    join();
  }

  void run() override {
    try {
      unsigned read = 0;
      unsigned written = 0;
      while (read < PacketCount || written < PacketCount) {
        m_connection.receive();
        if (read < PacketCount) {
          if (auto packet = m_connection.pullSingle()) {
            EXPECT_TRUE(convert<ProtocolRequestPacket>(packet)->requestProtocolVersion == read);
            ++read;
          }
        }

        if (written < PacketCount) {
          m_connection.push({std::make_shared<ProtocolRequestPacket>(written)});
          ++written;
        }
        m_connection.send();

        Thread::sleep(ASyncSleepMillis);

        if (!m_connection.isOpen())
          break;
      }

      EXPECT_EQ(PacketCount, read);
      EXPECT_EQ(PacketCount, written);
      m_connection.close();
      EXPECT_TRUE(m_connection.pull().empty());
    } catch (std::exception const& e) {
      ADD_FAILURE() << "Exception: " << outputException(e, true);
    } catch (...) {
      ADD_FAILURE();
    }
  }

private:
  UniverseConnection m_connection;
};

class SyncClientThread : public Thread {
public:
  SyncClientThread(UniverseConnection conn)
    : Thread("UniverseConnectionTestClientThread"), m_connection(std::move(conn)) {
    start();
  }

  ~SyncClientThread() override {
    join();
  }

  void run() override {
    try {
      for (unsigned i = 0; i < PacketCount; ++i) {
        m_connection.pushSingle(std::make_shared<ProtocolRequestPacket>(i));
        EXPECT_TRUE(m_connection.sendAll(SyncWaitMillis));
        EXPECT_TRUE(m_connection.receiveAny(SyncWaitMillis));
        EXPECT_EQ(convert<ProtocolRequestPacket>(m_connection.pullSingle())->requestProtocolVersion, i);

        if (!m_connection.isOpen())
          break;
      }

      m_connection.close();
      EXPECT_TRUE(m_connection.pull().empty());
    } catch (std::exception const& e) {
      ADD_FAILURE() << "Exception: " << outputException(e, true);
    } catch (...) {
      ADD_FAILURE();
    }
  }

private:
  UniverseConnection m_connection;
};

const GameTestRegistration universeConnectionsAll("UniverseConnections", "All", [](GameTest& fixture) {
  auto& logger = fixture.root().logger();
  UniverseConnectionServer server(logger, [](UniverseConnectionServer* server, ConnectionId clientId, List<std::shared_ptr<::Star::Packet>> packets) {
      server->sendPackets(clientId, packets);
    });

  ConnectionId clientId = ServerConnectionId;
  TcpServer tcpServer(logger, HostAddressWithPort(HostAddress::localhost(), ServerPort));
  tcpServer.setAcceptCallback([&logger, &server, &clientId](std::shared_ptr<::Star::TcpSocket> socket) {
      socket->setNonBlocking(true);
      auto conn = UniverseConnection(TcpPacketSocket::open(logger, std::move(socket)));
      server.addConnection(++clientId, std::move(conn));
    });

  LinkedList<ASyncClientThread> localASyncClients;
  for (unsigned i = 0; i < NumLocalASyncConnections; ++i) {
    auto pair = LocalPacketSocket::openPair();
    server.addConnection(++clientId, UniverseConnection(std::move(pair.first)));
    localASyncClients.emplaceAppend(UniverseConnection(std::move(pair.second)));
  }

  LinkedList<SyncClientThread> localSyncClients;
  for (unsigned i = 0; i < NumLocalSyncConnections; ++i) {
    auto pair = LocalPacketSocket::openPair();
    server.addConnection(++clientId, UniverseConnection(std::move(pair.first)));
    localSyncClients.emplaceAppend(UniverseConnection(std::move(pair.second)));
  }

  LinkedList<ASyncClientThread> remoteASyncClients;
  for (unsigned i = 0; i < NumRemoteASyncConnections; ++i) {
    auto socket = TcpSocket::connectTo({HostAddress::localhost(), ServerPort});
    socket->setNonBlocking(true);
    remoteASyncClients.emplaceAppend(UniverseConnection(TcpPacketSocket::open(logger, std::move(socket))));
  }

  LinkedList<SyncClientThread> remoteSyncClients;
  for (unsigned i = 0; i < NumRemoteSyncConnections; ++i) {
    auto socket = TcpSocket::connectTo({HostAddress::localhost(), ServerPort});
    socket->setNonBlocking(true);
    remoteSyncClients.emplaceAppend(UniverseConnection(TcpPacketSocket::open(logger, std::move(socket))));
  }

  for (auto& c : localASyncClients)
    c.join();

  for (auto& c : remoteASyncClients)
    c.join();

  for (auto& c : localSyncClients)
    c.join();

  for (auto& c : remoteSyncClients)
    c.join();

  server.removeAllConnections();
});

namespace {

std::pair<std::shared_ptr<TcpSocket>, std::shared_ptr<TcpSocket>> connectionTestTcpPair(Logger& logger) {
  auto listener = TcpSocket::listen(logger, {HostAddress::localhost(), 0});
  listener->setNonBlocking(true);
  auto client = TcpSocket::connectTo(listener->localAddress());
  listener->waitForData(false, std::chrono::steady_clock::now() + std::chrono::seconds(2));
  auto server = listener->accept(logger);
  if (!server)
    throw std::runtime_error("Loopback connection was not accepted");
  return {std::move(client), std::move(server)};
}

ByteArray connectionTestFrame(VersionNumber version) {
  ProtocolRequestPacket packet(version);
  DataStreamBuffer payload;
  packet.write(payload);
  DataStreamBuffer frame;
  frame.write(PacketType::ProtocolRequest);
  frame.writeVlqI(payload.size());
  frame.writeData(payload.ptr(), payload.size());
  return frame.takeData();
}

}

const GameTestRegistration universeConnectionsLocalZeroDeadlineAndQueuedPacketAfterPeerClose(
    "UniverseConnections", "LocalZeroDeadlineAndQueuedPacketAfterPeerClose", [](GameTest&) {
  auto pair = LocalPacketSocket::openPair();
  UniverseConnection sender(std::move(pair.first));
  UniverseConnection receiver(std::move(pair.second));
  EXPECT_FALSE(receiver.receiveAny(0));
  sender.pushSingle(std::make_shared<ProtocolRequestPacket>(41));
  EXPECT_TRUE(sender.sendAll(0));
  sender.close();
  EXPECT_FALSE(receiver.isOpen());
  ASSERT_TRUE(receiver.receiveAny(0));
  EXPECT_EQ(convert<ProtocolRequestPacket>(receiver.pullSingle())->requestProtocolVersion, 41u);
  EXPECT_FALSE(receiver.receiveAny(0));
});

const GameTestRegistration universeConnectionsLocalDeliveryAndPeerCloseWakeReadiness(
    "UniverseConnections", "LocalDeliveryAndPeerCloseWakeReadiness", [](GameTest&) {
  auto pair = LocalPacketSocket::openPair();
  UniverseConnection sender(std::move(pair.first));
  UniverseConnection receiver(std::move(pair.second));
  std::promise<void> starting;
  auto started = starting.get_future();
  auto received = std::async(std::launch::async, [&] {
    starting.set_value();
    return receiver.receiveAny(2000);
  });
  started.wait();
  sender.pushSingle(std::make_shared<ProtocolRequestPacket>(42));
  EXPECT_TRUE(sender.sendAll(0));
  EXPECT_EQ(received.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
  EXPECT_TRUE(received.get());
  ASSERT_TRUE(receiver.pullSingle());

  auto closed = std::async(std::launch::async, [&] { return receiver.receiveAny(2000); });
  sender.close();
  EXPECT_EQ(closed.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
  EXPECT_FALSE(closed.get());
});

const GameTestRegistration universeConnectionsLocalCloseDoesNotWaitForReceiveDeadline(
    "UniverseConnections", "LocalCloseDoesNotWaitForReceiveDeadline", [](GameTest&) {
  auto pair = LocalPacketSocket::openPair();
  UniverseConnection receiver(std::move(pair.first));
  std::promise<void> starting;
  auto started = starting.get_future();
  auto received = std::async(std::launch::async, [&] {
    starting.set_value();
    return receiver.receiveAny(2000);
  });
  started.wait();
  EXPECT_EQ(received.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);
  auto before = std::chrono::steady_clock::now();
  receiver.close();
  EXPECT_LT(std::chrono::steady_clock::now() - before, std::chrono::milliseconds(500));
  EXPECT_EQ(received.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
  EXPECT_FALSE(received.get());
});

const GameTestRegistration universeConnectionsTcpPartialHeaderBodyAndZeroDeadline(
    "UniverseConnections", "TcpPartialHeaderBodyAndZeroDeadline", [](GameTest& fixture) {
  auto [sender, socket] = connectionTestTcpPair(fixture.root().logger());
  UniverseConnection receiver(TcpPacketSocket::open(fixture.root().logger(), socket));
  auto frame = connectionTestFrame(43);
  ASSERT_GT(frame.size(), 2u);
  ASSERT_EQ(sender->send(frame.ptr(), 1), 1u);
  socket->waitForData(false, std::chrono::steady_clock::now() + std::chrono::seconds(2));
  EXPECT_FALSE(receiver.receiveAny(0));
  ASSERT_EQ(sender->send(frame.ptr() + 1, frame.size() - 2), frame.size() - 2);
  socket->waitForData(false, std::chrono::steady_clock::now() + std::chrono::seconds(2));
  EXPECT_FALSE(receiver.receiveAny(0));
  ASSERT_EQ(sender->send(frame.ptr() + frame.size() - 1, 1), 1u);
  socket->waitForData(false, std::chrono::steady_clock::now() + std::chrono::seconds(2));
  ASSERT_TRUE(receiver.receiveAny(0));
  EXPECT_EQ(convert<ProtocolRequestPacket>(receiver.pullSingle())->requestProtocolVersion, 43u);
  EXPECT_FALSE(receiver.receiveAny(0));
});

const GameTestRegistration universeConnectionsTcpCompletePacketBeforeEofAndTruncatedPacketAtEof(
    "UniverseConnections", "TcpCompletePacketBeforeEofAndTruncatedPacketAtEof", [](GameTest& fixture) {
  {
    auto [sender, socket] = connectionTestTcpPair(fixture.root().logger());
    UniverseConnection receiver(TcpPacketSocket::open(fixture.root().logger(), socket));
    auto frame = connectionTestFrame(44);
    ASSERT_EQ(sender->send(frame.ptr(), frame.size()), frame.size());
    sender->close();
    ASSERT_TRUE(receiver.receiveAny(1000));
    EXPECT_EQ(convert<ProtocolRequestPacket>(receiver.pullSingle())->requestProtocolVersion, 44u);
    EXPECT_FALSE(receiver.receiveAny(1000));
    EXPECT_FALSE(receiver.isOpen());
  }
  {
    auto [sender, socket] = connectionTestTcpPair(fixture.root().logger());
    UniverseConnection receiver(TcpPacketSocket::open(fixture.root().logger(), socket));
    auto frame = connectionTestFrame(45);
    ASSERT_EQ(sender->send(frame.ptr(), frame.size() - 1), frame.size() - 1);
    sender->close();
    EXPECT_FALSE(receiver.receiveAny(1000));
    EXPECT_FALSE(receiver.isOpen());
    EXPECT_FALSE(receiver.pullSingle());
  }
});

const GameTestRegistration universeConnectionsTcpCloseWakesNativeReadinessWithoutHoldingConnectionLock(
    "UniverseConnections", "TcpCloseWakesNativeReadinessWithoutHoldingConnectionLock", [](GameTest& fixture) {
  auto [sender, socket] = connectionTestTcpPair(fixture.root().logger());
  UniverseConnection receiver(TcpPacketSocket::open(fixture.root().logger(), socket));
  std::promise<void> starting;
  auto started = starting.get_future();
  auto received = std::async(std::launch::async, [&] {
    starting.set_value();
    return receiver.receiveAny(2000);
  });
  started.wait();
  EXPECT_EQ(received.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);
  auto before = std::chrono::steady_clock::now();
  receiver.close();
  EXPECT_LT(std::chrono::steady_clock::now() - before, std::chrono::milliseconds(500));
  EXPECT_EQ(received.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
  EXPECT_FALSE(received.get());
});

const GameTestRegistration universeConnectionsTcpShutdownWakesEveryNativeReadinessWaiter(
    "UniverseConnections", "TcpShutdownWakesEveryNativeReadinessWaiter", [](GameTest& fixture) {
  auto [sender, socket] = connectionTestTcpPair(fixture.root().logger());
  auto wait = [&] {
    socket->waitForData(false, std::chrono::steady_clock::now() + std::chrono::seconds(2));
  };
  auto first = std::async(std::launch::async, wait);
  auto second = std::async(std::launch::async, wait);
  EXPECT_EQ(first.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);
  EXPECT_EQ(second.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);
  auto before = std::chrono::steady_clock::now();
  socket->shutdown();
  EXPECT_LT(std::chrono::steady_clock::now() - before, std::chrono::milliseconds(500));
  EXPECT_EQ(first.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
  EXPECT_EQ(second.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
  EXPECT_NO_THROW(first.get());
  EXPECT_NO_THROW(second.get());
  before = std::chrono::steady_clock::now();
  socket->waitForData(false, before + std::chrono::seconds(2));
  EXPECT_LT(std::chrono::steady_clock::now() - before, std::chrono::milliseconds(500));
});

const GameTestRegistration universeConnectionsTcpPeerCloseWakesReceiveDeadline(
    "UniverseConnections", "TcpPeerCloseWakesReceiveDeadline", [](GameTest& fixture) {
  auto [sender, socket] = connectionTestTcpPair(fixture.root().logger());
  UniverseConnection receiver(TcpPacketSocket::open(fixture.root().logger(), socket));
  auto received = std::async(std::launch::async, [&] { return receiver.receiveAny(2000); });
  EXPECT_EQ(received.wait_for(std::chrono::milliseconds(20)), std::future_status::timeout);
  sender->close();
  EXPECT_EQ(received.wait_for(std::chrono::milliseconds(500)), std::future_status::ready);
  EXPECT_FALSE(received.get());
  EXPECT_FALSE(receiver.isOpen());
});

const GameTestRegistration universeConnectionsLocalAndTcpReadinessRespectFiniteDeadline(
    "UniverseConnections", "LocalAndTcpReadinessRespectFiniteDeadline", [](GameTest& fixture) {
  {
    auto pair = LocalPacketSocket::openPair();
    UniverseConnection receiver(std::move(pair.first));
    auto before = std::chrono::steady_clock::now();
    EXPECT_FALSE(receiver.receiveAny(25));
    auto elapsed = std::chrono::steady_clock::now() - before;
    EXPECT_GE(elapsed, std::chrono::milliseconds(20));
    EXPECT_LT(elapsed, std::chrono::milliseconds(500));
  }
  {
    auto [sender, socket] = connectionTestTcpPair(fixture.root().logger());
    UniverseConnection receiver(TcpPacketSocket::open(fixture.root().logger(), socket));
    auto before = std::chrono::steady_clock::now();
    EXPECT_FALSE(receiver.receiveAny(25));
    auto elapsed = std::chrono::steady_clock::now() - before;
    EXPECT_GE(elapsed, std::chrono::milliseconds(20));
    EXPECT_LT(elapsed, std::chrono::milliseconds(500));
  }
});

const GameTestRegistration universeConnectionsTcpCompressedBackpressureRetainsAndDrainsPendingTail(
    "UniverseConnections", "TcpCompressedBackpressureRetainsAndDrainsPendingTail", [](GameTest& fixture) {
  auto [outgoing, incoming] = connectionTestTcpPair(fixture.root().logger());
  SocketTestAccess::setBufferSize(*outgoing, SO_SNDBUF, 16384);
  SocketTestAccess::setBufferSize(*incoming, SO_RCVBUF, 65536);
  auto sendSocket = TcpPacketSocket::open(fixture.root().logger(), outgoing);
  auto* sendTransport = sendSocket.get();
  sendSocket->setCompressionStreamEnabled(true);
  auto receiveSocket = TcpPacketSocket::open(fixture.root().logger(), incoming);
  receiveSocket->setCompressionStreamEnabled(true);
  UniverseConnection sender(std::move(sendSocket));
  UniverseConnection receiver(std::move(receiveSocket));
  auto nextPayload = [](std::uint32_t& state) {
    ByteArray payload(256 << 10, 0);
    for (auto& byte : payload) {
      state ^= state << 13;
      state ^= state >> 17;
      state ^= state << 5;
      byte = static_cast<char>(state);
    }
    return payload;
  };
  std::uint32_t sendState = 0x12345678;
  std::size_t packetCount = 0;
  bool backpressured = false;
  // Stop on observed native backpressure, not an assumed OS buffer capacity.
  // The cap is a runaway-fixture guard; buffers above are local to this pair.
  while (!backpressured && packetCount < 512) {
    sender.pushSingle(std::make_shared<ClientContextUpdatePacket>(nextPayload(sendState)));
    ++packetCount;
    backpressured = !sender.sendAll(0);
  }
  ASSERT_TRUE(backpressured) << "Native socket never reported backpressure after " << packetCount << " packets";
  ASSERT_TRUE(sendTransport->sentPacketsPending());
  // Already-compressed bytes retain their wire encoding even if no new stream
  // compression is requested. Disabling the mode must not hide that tail.
  sendTransport->setCompressionStreamEnabled(false);
  EXPECT_TRUE(sendTransport->sentPacketsPending());
  auto received = std::async(std::launch::async, [&] {
    std::uint32_t receiveState = 0x12345678;
    for (std::size_t i = 0; i < packetCount; ++i) {
      if (!receiver.receiveAny(5000))
        return false;
      auto packet = convert<ClientContextUpdatePacket>(receiver.pullSingle());
      if (!packet || packet->updateData != nextPayload(receiveState))
        return false;
    }
    return true;
  });
  EXPECT_TRUE(sender.sendAll(5000));
  EXPECT_TRUE(received.get());
  EXPECT_FALSE(sendTransport->sentPacketsPending());
});

const GameTestRegistration universeConnectionsShardCallbacksReenterAndRemovalReturnsUnprocessedQueues(
    "UniverseConnections", "ShardCallbacksReenterAndRemovalReturnsUnprocessedQueues", [](GameTest& fixture) {
  std::promise<void> entered;
  auto callbackEntered = entered.get_future();
  std::promise<void> release;
  auto callbackRelease = release.get_future().share();
  std::atomic<unsigned> calls = 0;
  std::atomic<unsigned> active = 0;
  UniverseConnectionServer server(fixture.root().logger(), [&](UniverseConnectionServer* server, ConnectionId id,
      List<std::shared_ptr<Packet>> packets) {
    if (id == 2) {
      EXPECT_EQ(active.fetch_add(1), 0u);
      if (calls++ == 0) {
        EXPECT_TRUE(server->hasConnection(id));
        auto connection = server->removeConnection(id);
        server->addConnection(id, std::move(connection));
        entered.set_value();
        callbackRelease.wait_for(std::chrono::seconds(2));
      }
      server->sendPackets(id, std::move(packets));
      --active;
    } else {
      server->sendPackets(id, std::move(packets));
    }
  }, 2);
  auto gatePair = LocalPacketSocket::openPair();
  server.addConnection(2, UniverseConnection(std::move(gatePair.first)));
  UniverseConnection gateClient(std::move(gatePair.second));
  gateClient.pushSingle(std::make_shared<ProtocolRequestPacket>(1));
  ASSERT_TRUE(gateClient.sendAll(0));
  ASSERT_EQ(callbackEntered.wait_for(std::chrono::seconds(1)), std::future_status::ready);

  // Same shard: the worker is inside the callback, so neither queue can have
  // been processed before removal. The returned connection owns both queues.
  auto queuedPair = LocalPacketSocket::openPair();
  UniverseConnection queued(std::move(queuedPair.first));
  UniverseConnection queuedClient(std::move(queuedPair.second));
  queued.pushSingle(std::make_shared<ProtocolRequestPacket>(46));
  queuedClient.pushSingle(std::make_shared<ProtocolRequestPacket>(47));
  ASSERT_TRUE(queuedClient.sendAll(0));
  ASSERT_TRUE(queued.receiveAny(0));
  server.addConnection(4, std::move(queued));
  auto returned = server.removeConnection(4);
  ASSERT_TRUE(returned.receiveAny(0));
  EXPECT_EQ(convert<ProtocolRequestPacket>(returned.pullSingle())->requestProtocolVersion, 47u);
  ASSERT_TRUE(returned.sendAll(0));
  ASSERT_TRUE(queuedClient.receiveAny(0));
  EXPECT_EQ(convert<ProtocolRequestPacket>(queuedClient.pullSingle())->requestProtocolVersion, 46u);

  // Another shard continues while the first shard is in its reentrant callback.
  auto otherPair = LocalPacketSocket::openPair();
  server.addConnection(3, UniverseConnection(std::move(otherPair.first)));
  UniverseConnection otherClient(std::move(otherPair.second));
  otherClient.pushSingle(std::make_shared<ProtocolRequestPacket>(48));
  ASSERT_TRUE(otherClient.sendAll(0));
  ASSERT_TRUE(otherClient.receiveAny(1000));
  EXPECT_EQ(convert<ProtocolRequestPacket>(otherClient.pullSingle())->requestProtocolVersion, 48u);
  gateClient.pushSingle(std::make_shared<ProtocolRequestPacket>(2));
  ASSERT_TRUE(gateClient.sendAll(0));
  EXPECT_FALSE(gateClient.receiveAny(0));
  release.set_value();
  for (VersionNumber version : {1u, 2u}) {
    ASSERT_TRUE(gateClient.receiveAny(1000));
    EXPECT_EQ(convert<ProtocolRequestPacket>(gateClient.pullSingle())->requestProtocolVersion, version);
  }
});

#ifdef STAR_TEST_WRAP_PTHREAD_CREATE
const GameTestRegistration universeConnectionsStartupFailureJoinsEveryPreviouslyStartedWorker(
    "UniverseConnectionsDeathTest", "StartupFailureJoinsEveryPreviouslyStartedWorker", [](GameTest& fixture) {
  auto& logger = fixture.root().logger();
  auto oldStyle = testing::GTEST_FLAG(death_test_style);
  testing::GTEST_FLAG(death_test_style) = "threadsafe";
  EXPECT_EXIT(([&logger] {
    // A regression must fail this subprocess, not hang the whole asset suite.
    alarm(5);
    for (int failAfter : {0, 1, 3}) {
      std::atomic<unsigned> completed = 0;
      completedInjectedThreads = &completed;
      threadStartsBeforeFailure = failAfter;
      bool threw = false;
      try {
        UniverseConnectionServer server(logger, [](auto*, auto, auto) {}, 4);
      } catch (StarException const&) {
        threw = true;
      }
      threadStartsBeforeFailure = -1;
      completedInjectedThreads = nullptr;
      if (!threw || completed.load() != static_cast<unsigned>(failAfter))
        std::_Exit(1);
    }
    // Injection resets on failure; a subsequent ordinary startup still works.
    {
      UniverseConnectionServer server(logger, [](auto*, auto, auto) {}, 2);
    }
    std::_Exit(0);
  }()), testing::ExitedWithCode(0), "");
  testing::GTEST_FLAG(death_test_style) = oldStyle;
});
#endif

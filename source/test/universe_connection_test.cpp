#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarIODevice.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"
#include "StarThread.hpp"
#include <atomic>
#include <memory>
#include "StarIdMap.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarVariant.hpp"
#include "StarColor.hpp"
#include "StarMaybe.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarArray.hpp"
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include <functional>
#include "StarSet.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarBTree.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
#include "StarOrderedSet.hpp"
#include "StarRpcPromise.hpp"
#include "StarSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarByteArray.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
#include "StarNetCompatibility.hpp"
#include "StarIODevice.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"
#include "StarThread.hpp"

import star.host_address;
import star.socket;
import star.tcp;

#include "StarP2PNetworkingService.hpp"
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
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
#include "StarLuaRoot.hpp"
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.host_address;
import star.socket;
import star.tcp;

#include "gtest/gtest.h"

using namespace Star;

unsigned const PacketCount = 20;
uint16_t const ServerPort = 55555;

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

  virtual void run() {
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
          m_connection.push({make_shared<ProtocolRequestPacket>(written)});
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

  virtual void run() {
    try {
      for (unsigned i = 0; i < PacketCount; ++i) {
        m_connection.pushSingle(make_shared<ProtocolRequestPacket>(i));
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

TEST(UniverseConnections, All) {
  UniverseConnectionServer server([](UniverseConnectionServer* server, ConnectionId clientId, List<PacketPtr> packets) {
      server->sendPackets(clientId, packets);
    });

  ConnectionId clientId = ServerConnectionId;
  TcpServer tcpServer(HostAddressWithPort(HostAddress::localhost(), ServerPort));
  tcpServer.setAcceptCallback([&server, &clientId](TcpSocketPtr socket) {
      socket->setNonBlocking(true);
      auto conn = UniverseConnection(TcpPacketSocket::open(std::move(socket)));
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
    remoteASyncClients.emplaceAppend(UniverseConnection(TcpPacketSocket::open(std::move(socket))));
  }

  LinkedList<SyncClientThread> remoteSyncClients;
  for (unsigned i = 0; i < NumRemoteSyncConnections; ++i) {
    auto socket = TcpSocket::connectTo({HostAddress::localhost(), ServerPort});
    socket->setNonBlocking(true);
    remoteSyncClients.emplaceAppend(UniverseConnection(TcpPacketSocket::open(std::move(socket))));
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
}

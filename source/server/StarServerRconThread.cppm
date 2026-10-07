module;
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarGameTypes.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarLexicalCast.hpp"
#include "StarLogging.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAssetPath.hpp"
#include "StarRefPtr.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarVersion.hpp"
#include "StarIdMap.hpp"
#include "StarConfig.hpp"
#include "StarLockFile.hpp"
#include "StarVector.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarRpcPromise.hpp"
#include "StarZSTDCompression.hpp"
#include "StarNetCompatibility.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarCasting.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarPerlin.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarDirectives.hpp"
#include "StarVariant.hpp"
#include "StarSet.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include "StarLruCache.hpp"
#include <atomic>
#include <memory>
#include "StarIterator.hpp"


import star.host_address;
import star.socket;
import star.tcp;
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
import star.configuration;
import star.universe;
import star.worker_pool;
import star.celestial_coordinate;
import star.uuid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.interaction_types;
import star.item_descriptor;
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
import star.tile_modification;
import star.world;
import star.celestial_types;
import star.chat_types;
import star.warping;
import star.wiring;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.cellular_light_array;
import star.cellular_lighting;
import star.cellular_liquid;
import star.world_structure;
#include "StarLuaComponents.hpp"
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.world_render_data;
import star.world_client_state;
import star.interpolation_tracker;
import star.spawn_type_database;
import star.spawner;
import star.world_server;
import star.world_server_thread;
import star.world_template;
import star.system_world_server;
import star.system_world_server_thread;
#include "StarP2PNetworkingService.hpp"
import star.net_packet_socket;
import star.universe_connection;
import star.universe_settings;
import star.universe_server;

export module star.server_rcon;

export namespace Star {

class ServerRconClient : public Thread {
public:
  static const uint32_t SERVERDATA_AUTH = 0x03;
  static const uint32_t SERVERDATA_EXECCOMMAND = 0x02;
  static const uint32_t SERVERDATA_RESPONSE_VALUE = 0x00;
  static const uint32_t SERVERDATA_AUTH_RESPONSE = 0x02;
  static const uint32_t SERVERDATA_AUTH_FAILURE = 0xffffffff;
  ServerRconClient(UniverseServer* universe, TcpSocketPtr socket);
  ~ServerRconClient();

  void start();
  void stop();

protected:
  virtual void run();

private:
  static constexpr size_t MaxPacketSize = 4096;
  // Requests are small (a command line); anything larger is refused instead of
  // being allocated, since the length is client supplied.
  static constexpr size_t MaxReceivePacketSize = MaxPacketSize * 4;
  struct NoMoreRequestsTag {
    static constexpr char const* name() { return "NoMoreRequests"; }
  };
  using NoMoreRequests = StarError<NoMoreRequestsTag, StarException>;
  struct OversizedPacketTag {
    static constexpr char const* name() { return "OversizedPacket"; }
  };
  using OversizedPacket = StarError<OversizedPacketTag, StarException>;

  void receive(size_t size);
  void send(uint32_t requestId, uint32_t cmd, String str = "");
  void sendAuthFailure();
  void sendCmdResponse(uint32_t requestId, String response);
  void closeSocket();
  void processRequest();
  ServerCommandResult handleCommand(String commandLine);

  UniverseServer* m_universe;
  TcpSocketPtr m_socket;
  DataStreamBuffer m_packetBuffer;
  bool m_stop;
  bool m_authed;
  String m_rconPassword;

  HashMap<uint32_t,RpcPromise<String>> m_commandPromises;
};
typedef shared_ptr<ServerRconClient> ServerRconClientPtr;

STAR_CLASS(ServerRconThread);

class ServerRconThread : public Thread {
public:
  ServerRconThread(UniverseServer* universe, HostAddressWithPort const& address);
  ~ServerRconThread();

  void start();
  void stop();

protected:
  virtual void run();

private:
  void clearClients(bool all = false);

  UniverseServer* m_universe;
  TcpServer m_rconServer;
  bool m_stop;
  HashMap<HostAddress, ServerRconClientPtr> m_clients;
};

}

namespace Star {

ServerRconClient::ServerRconClient(UniverseServer* universe, TcpSocketPtr socket)
  : Thread("RconClient"),
    m_universe(universe),
    m_socket(socket),
    m_packetBuffer(MaxPacketSize),
    m_stop(true),
    m_authed(false) {
  auto& root = Root::singleton();
  auto cfg = root.configuration();

  m_packetBuffer.setByteOrder(ByteOrder::LittleEndian);
  m_packetBuffer.setNullTerminatedStrings(true);

  m_rconPassword = cfg->get("rconServerPassword").toString();
}

ServerRconClient::~ServerRconClient() {
  stop();
  join();
}

ServerCommandResult ServerRconClient::handleCommand(String commandLine) {
  String command = commandLine.extract();

  if (command == "echo") {
    return commandLine;
  } else if (command == "broadcast" || command == "say") {
    m_universe->adminBroadcast(commandLine);
    return String(strf("OK: said {}", commandLine));
  } else if (command == "stop") {
    m_universe->stop();
    return String("OK: shutting down");
  } else {
    return m_universe->adminCommand(strf("{} {}", command, commandLine));
  }
}

void ServerRconClient::receive(size_t size) {
  // The size comes straight from the client and is used as an allocation size,
  // so it has to be refused before the buffer is resized for it.
  if (size > MaxReceivePacketSize)
    throw OversizedPacket(strf("RCON packet of {} bytes is above the {} byte limit", size, MaxReceivePacketSize));

  m_packetBuffer.reset(size);
  auto ptr = m_packetBuffer.ptr();
  while (size > 0) {
    size_t r = m_socket->receive(ptr, size);
    if (r == 0)
      throw NoMoreRequests();
    size -= r;
    ptr += r;
  }
}

void ServerRconClient::send(uint32_t requestId, uint32_t cmd, String str) {
  m_packetBuffer.clear();
  m_packetBuffer << (uint32_t)(str.utf8Size() + 10) << requestId << cmd << str << (uint8_t)0x00;
  m_socket->send(m_packetBuffer.ptr(), m_packetBuffer.size());
}

void ServerRconClient::sendAuthFailure() {
  send(SERVERDATA_AUTH_FAILURE, SERVERDATA_AUTH_RESPONSE, "");
}

void ServerRconClient::sendCmdResponse(uint32_t requestId, String response) {
  size_t len = response.length();
  // Always send at least one packet even if the response was blank
  do {
    auto dataLen = (len >= MaxPacketSize) ? MaxPacketSize : len;
    send(requestId, SERVERDATA_RESPONSE_VALUE, response.substr(0, dataLen));
    response = response.substr(dataLen);
    len = response.length();
  } while (len > 0);
}

void ServerRconClient::start() {
  m_stop = false;
  Thread::start();
}

void ServerRconClient::stop() {
  m_stop = true;
  m_socket->close();
}

void ServerRconClient::processRequest() {
  receive(4);
  uint32_t size = m_packetBuffer.read<uint32_t>();

  receive(size);
  uint32_t requestId;
  m_packetBuffer >> requestId;

  uint32_t cmd;
  m_packetBuffer >> cmd;

  switch (cmd) {
    case SERVERDATA_AUTH: {
      String password;
      m_packetBuffer >> password;
      if (!m_rconPassword.empty() && m_rconPassword.equals(password)) {
        m_authed = true;
        send(requestId, SERVERDATA_RESPONSE_VALUE);
        send(requestId, SERVERDATA_AUTH_RESPONSE);
      } else {
        m_authed = false;
        sendAuthFailure();
      }
      break;
    }
    case SERVERDATA_EXECCOMMAND:
      if (m_authed) {
        String command;
        m_packetBuffer >> command;
        try {
          Logger::info("RCON {}: {}", m_socket->remoteAddress(), command);
          auto res = handleCommand(command);
          if (res.is<String>())
            sendCmdResponse(requestId, res.get<String>());
          else
            m_commandPromises[requestId] = res.get<RpcPromise<String>>();
        } catch (std::exception const& e) {
          sendCmdResponse(requestId, strf("RCON: Error executing: {}: {}", command, outputException(e, true)));
        }
      } else {
        sendAuthFailure();
      }
      break;
    default:
      sendCmdResponse(requestId, strf("Unknown request {:06x}", cmd));
  }
  
  for (auto const& requestId : m_commandPromises.keys()) {
    if (m_commandPromises[requestId].finished()) {
      if (m_commandPromises[requestId].succeeded()) {
        sendCmdResponse(requestId, *m_commandPromises[requestId].result());
      } else {
        sendCmdResponse(requestId, strf("RCON: Command promise failed: {}", *m_commandPromises[requestId].error()));
      }
      m_commandPromises.remove(requestId);
    }
  }
}

void ServerRconClient::run() {
  try {
    while (!m_stop)
      processRequest();
  } catch (NoMoreRequests const&) {
  } catch (std::exception const& e) {
    Logger::error("ServerRconClient exception caught: {}", outputException(e, false));
  }
}


ServerRconThread::ServerRconThread(UniverseServer* universe, HostAddressWithPort const& address)
  : Thread("RconServer"), m_universe(universe), m_rconServer(address), m_stop(true) {
  if (Root::singleton().configuration()->get("rconServerPassword").toString().empty())
    Logger::warn("rconServerPassword is not configured requests will NOT be processed");
}

ServerRconThread::~ServerRconThread() {
  stop();
  join();
}

void ServerRconThread::clearClients(bool all) {
  auto it = makeSMutableMapIterator(m_clients);
  while (it.hasNext()) {
    auto const& pair = it.next();
    auto client = pair.second;
    if (all)
      client->stop();
    else if (!client->isRunning())
      it.remove();
  }
}

void ServerRconThread::start() {
  m_stop = false;
  Thread::start();
}

void ServerRconThread::stop() {
  m_stop = true;
  m_rconServer.stop();
  clearClients(true);
}

void ServerRconThread::run() {
  try {
    auto timeout = Root::singleton().configuration()->get("rconServerTimeout").toInt();
    while (!m_stop) {
      if (auto client = m_rconServer.accept(100)) {
        client->setTimeout(timeout);
        auto rconClient = make_shared<ServerRconClient>(m_universe, client);
        rconClient->start();
        m_clients[client->remoteAddress().address()] = rconClient;
        clearClients();
      }
    }
  } catch (std::exception const& e) {
    Logger::error("ServerRconThread exception caught: {}", e.what());
  }
}

}

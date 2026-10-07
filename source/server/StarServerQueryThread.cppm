module;
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarThread.hpp"
#include "StarString.hpp"
#include "StarEither.hpp"
#include "StarMap.hpp"
#include "StarDataStreamDevices.hpp"
#include <random>
#include "StarLogging.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarIdMap.hpp"
#include "StarConfig.hpp"
#include "StarMaybe.hpp"
import star.lock_file;
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarRpcPromise.hpp"
#include "StarByteArray.hpp"

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
#include "StarNetCompatibility.hpp"
#include "StarSet.hpp"
#include "StarBTree.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
import star.btree_database;
#include "StarCasting.hpp"
#include "StarOrderedSet.hpp"
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;
import star.weighted_pool;
#include "StarStrongTypedef.hpp"
#include <functional>
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarMaybe.hpp"
#include "StarColor.hpp"
#include "StarVariant.hpp"
#include "StarSet.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarRandom.hpp"
#include "StarBlockAllocator.hpp"
#include <atomic>
#include <memory>
#include "StarIterator.hpp"


import star.host_address;


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
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.net_packet_socket;
import star.universe_connection;
import star.universe_settings;
import star.universe_server;

export module star.server_query;

import star.udp;

export namespace Star {

STAR_CLASS(ServerQueryThread);

class ServerQueryThread : public Thread {
public:
  ServerQueryThread(UniverseServer* universe, HostAddressWithPort const& bindAddress);
  ~ServerQueryThread();

  void start();
  void stop();

protected:
  virtual void run();

private:
  static const uint8_t A2A_PING_REQUEST = 0x69;
  static const uint8_t A2A_PING_REPLY = 0x6a;
  static const uint8_t A2S_CHALLENGE_REQUEST = 0x57;
  static const uint8_t A2S_CHALLENGE_RESPONSE = 0x41;
  static const uint8_t A2S_INFO_REQUEST = 0x54;
  static const uint8_t A2S_INFO_REPLY = 0x49;
  static const uint8_t A2S_PLAYER_REQUEST = 0x55;
  static const uint8_t A2S_PLAYER_REPLY = 0x44;
  static const uint8_t A2S_RULES_REQUEST = 0x56;
  static const uint8_t A2S_RULES_REPLY = 0x45;
  static const uint8_t A2S_VERSION = 0x07;
  static const uint8_t A2S_STR_TERM = 0x00;
  static const uint8_t A2S_EDF_GID = 0x01;
  static const uint8_t A2S_EDF_SID = 0x10;
  static const uint8_t A2S_EDF_TAGS = 0x20;
  static const uint8_t A2S_EDF_STV = 0x40;
  static const uint8_t A2S_EDF_PORT = 0x80;
  static const uint8_t A2S_ENV_WINDOWS = 'w';
  static const uint8_t A2S_ENV_LINUX = 'l';
  static const uint8_t A2S_ENV_MAC = 'm';
  static const uint8_t A2S_TYPE_DEDICATED = 'd';
  static const uint8_t A2S_TYPE_LISTEN = 'l';
  static const uint8_t A2S_TYPE_TV = 'p';
  static const uint8_t A2S_VAC_OFF = 0x00;
  static const uint8_t A2S_VAC_ON = 0x01;
  static constexpr const char* A2S_INFO_REQUEST_STRING = "Source Engine Query";
  static const uint16_t A2S_APPID = (uint16_t)0xfffe;
  static const uint16_t A2S_PACKET_SIZE = (uint16_t)0x4e0;
  static const uint32_t A2S_HEAD_INT = 0xffffffff;
  static constexpr const char* GAME_DIR = "starbound";
  static constexpr const char* GAME_DESC = "Starbound";
  static constexpr const char* GAME_TYPE = "SMP";
  static const int32_t challengeCheckInterval = 30000;
  static const int32_t responseCacheTime = 5000;

  void sendTo(HostAddressWithPort const& address, DataStreamBuffer* ds);
  bool processPacket(HostAddressWithPort const& address, char const* data, size_t length);
  void buildPlayerResponse();
  void buildRuleResponse();
  bool validChallenge(HostAddressWithPort const& address, char const* data, size_t length);
  void sendChallenge(HostAddressWithPort const& address);
  void pruneChallenges();
  bool challengeRequest(HostAddressWithPort const& address, char const* data, size_t length);

  // Server API
  uint8_t serverPlayerCount();
  bool serverPassworded();
  const char* serverPlugins();
  String serverWorldNames();

  UniverseServer* m_universe;
  UdpServer m_queryServer;
  bool m_stop;
  DataStreamBuffer m_playersResponse;
  DataStreamBuffer m_rulesResponse;
  DataStreamBuffer m_generalResponse;

  class RequestChallenge {
  public:
    RequestChallenge();
    bool before(uint64_t time);
    int32_t getChallenge();

  private:
    uint64_t m_time;
    int32_t m_challenge;
  };

  uint16_t m_serverPort;
  uint8_t m_maxPlayers;
  String m_serverName;
  HashMap<HostAddress, shared_ptr<RequestChallenge>> m_validChallenges;
  int64_t m_lastChallengeCheck;
  int64_t m_lastPlayersResponse;
  int64_t m_lastRulesResponse;
  int64_t m_lastActiveTime;
};

}

namespace Star {

ServerQueryThread::ServerQueryThread(UniverseServer* universe, HostAddressWithPort const& bindAddress)
  : Thread("QueryServer"),
    m_universe(universe),
    m_queryServer(bindAddress),
    m_stop(true),
    m_lastChallengeCheck(Time::monotonicMilliseconds()) {
  m_playersResponse.resize(A2S_PACKET_SIZE);
  m_playersResponse.setByteOrder(ByteOrder::LittleEndian);
  m_playersResponse.setNullTerminatedStrings(true);

  m_rulesResponse.resize(A2S_PACKET_SIZE);
  m_rulesResponse.setByteOrder(ByteOrder::LittleEndian);
  m_rulesResponse.setNullTerminatedStrings(true);

  m_generalResponse.resize(A2S_PACKET_SIZE);
  m_generalResponse.setByteOrder(ByteOrder::LittleEndian);
  m_generalResponse.setNullTerminatedStrings(true);

  m_serverPort = 0;
  m_lastActiveTime = 0;

  auto& root = Root::singleton();
  auto cfg = root.configuration();

  m_maxPlayers = cfg->get("maxPlayers").toUInt();
  m_serverName = cfg->get("serverName").toString();

  m_lastPlayersResponse = 0;
  m_lastRulesResponse = 0;
}

ServerQueryThread::~ServerQueryThread() {
  stop();
  join();
}

void ServerQueryThread::start() {
  m_stop = false;
  Thread::start();
  m_lastActiveTime = Time::monotonicMilliseconds();
}

void ServerQueryThread::stop() {
  m_stop = true;
  m_queryServer.close();
}

void ServerQueryThread::sendTo(HostAddressWithPort const& address, DataStreamBuffer* ds) {
  m_queryServer.send(address, ds->ptr(), ds->size());
}

uint8_t ServerQueryThread::serverPlayerCount() {
  return m_universe->numberOfClients();
}

bool ServerQueryThread::serverPassworded() {
  // TODO: implement
  return false;
}

String ServerQueryThread::serverWorldNames() {
  auto activeWorlds = m_universe->activeWorlds();
  if (activeWorlds.empty())
    return String("Unknown");

  return StringList(activeWorlds.transformed(printWorldId)).join(",");
}

const char* ServerQueryThread::serverPlugins() {
  // TODO: implement
  return "none";
}

bool ServerQueryThread::processPacket(HostAddressWithPort const& address, char const* data, size_t length) {
  uint8_t* buf = (uint8_t*)data;
  if (length < 5 || buf[0] != 0xff || buf[1] != 0xff || buf[2] != 0xff || buf[3] != 0xff) {
    // short packet or missing header
    return false;
  }

  // Process packet
  switch (buf[4]) {
    case A2S_INFO_REQUEST: {
      // We use -6 and not -5 as the string should be NULL terminated
      // but instead of the std::string constructor stopping at the NULL
      // it includes it :(
      if (length < 6) {
        // Without the trailing NULL byte the subtraction below would wrap around
        return false;
      }
      std::string str((const char*)(buf + 5), length - 6);
      if (str.compare(A2S_INFO_REQUEST_STRING) != 0) {
        // Invalid request
        return false;
      }

      m_generalResponse.clear();
      m_generalResponse << A2S_HEAD_INT << A2S_INFO_REPLY << A2S_VERSION << m_serverName << serverWorldNames()
                        << GAME_DIR << GAME_DESC << A2S_APPID // Should be SteamAppId but this isn't a short :(
                        << serverPlayerCount() << m_maxPlayers << (uint8_t)0x00 // bots
                        << A2S_TYPE_DEDICATED // dedicated
#ifdef STAR_SYSTEM_FAMILY_WINDOWS
                        << A2S_ENV_WINDOWS // os
#elif defined(STAR_SYSTEM_MACOS)
                        << A2S_ENV_MAC // os
#else
                        << A2S_ENV_LINUX // os
#endif
                        << serverPassworded() << A2S_VAC_OFF // secure
                        << StarVersionString << A2S_EDF_PORT // EDF
                        << m_serverPort;

      sendTo(address, &m_generalResponse);
      return true;
    }
    case A2S_CHALLENGE_REQUEST:
      sendChallenge(address);
      return true;

    case A2S_PLAYER_REQUEST:
      if (challengeRequest(address, data, length))
        return true;

      if (!validChallenge(address, data, length))
        return false;

      buildPlayerResponse();
      sendTo(address, &m_playersResponse);
      return true;

    case A2S_RULES_REQUEST:
      if (challengeRequest(address, data, length))
        return true;

      if (!validChallenge(address, data, length))
        return false;

      buildRuleResponse();
      sendTo(address, &m_rulesResponse);
      return true;
  }

  return false;
}

void ServerQueryThread::buildPlayerResponse() {
  int64_t now = Time::monotonicMilliseconds();
  if (now < m_lastPlayersResponse + responseCacheTime) {
    return;
  }

  auto clientIds = m_universe->clientIdsAndCreationTime();
  uint8_t cnt = (uint8_t)clientIds.count();
  int32_t kills = 0; // Not currently supported

  m_playersResponse.clear();
  m_playersResponse << A2S_HEAD_INT << A2S_PLAYER_REPLY << cnt;

  uint8_t i = 0;
  for (auto& pair : clientIds) {
    auto timeConnected = float(now - pair.second) / 1000.f;
    m_playersResponse << i++ << m_universe->clientNick(pair.first) << kills << timeConnected;
  }

  m_lastPlayersResponse = now;
}

void ServerQueryThread::buildRuleResponse() {
  int64_t now = Time::monotonicMilliseconds();
  if (now < m_lastRulesResponse + responseCacheTime) {
    return;
  }

  uint16_t cnt = 1;
  m_rulesResponse.clear();
  m_rulesResponse << A2S_HEAD_INT << A2S_RULES_REPLY << cnt << "plugins" << serverPlugins();

  m_lastRulesResponse = now;
}

void ServerQueryThread::sendChallenge(HostAddressWithPort const& address) {
  auto challenge = make_shared<RequestChallenge>();

  m_validChallenges[address.address()] = challenge;
  m_generalResponse.clear();
  m_generalResponse << A2S_HEAD_INT << A2S_CHALLENGE_RESPONSE << challenge->getChallenge();

  sendTo(address, &m_generalResponse);
}

void ServerQueryThread::pruneChallenges() {
  int64_t now = Time::monotonicMilliseconds();
  if (now < m_lastChallengeCheck + challengeCheckInterval) {
    return;
  }

  auto expire = now - challengeCheckInterval;
  auto it = makeSMutableMapIterator(m_validChallenges);
  while (it.hasNext()) {
    auto const& pair = it.next();
    if (pair.second->before(expire)) {
      it.remove();
    }
  }
  m_lastChallengeCheck = now;
}

void ServerQueryThread::run() {
  HostAddressWithPort udpAddress;
  char udpData[MaxUdpData];
  while (!m_stop) {
    try {
      auto len = m_queryServer.receive(&udpAddress, udpData, MaxUdpData, 100);
      pruneChallenges();
      if (len != 0)
        processPacket(udpAddress, udpData, len);
    } catch (SocketClosedException const&) {
    } catch (std::exception const& e) {
      Logger::error("ServerQueryThread exception caught: {}", outputException(e, true));
    }
  }
}

ServerQueryThread::RequestChallenge::RequestChallenge()
  : m_time(Time::monotonicMilliseconds()), m_challenge(Random::randi32()) {}

bool ServerQueryThread::RequestChallenge::before(uint64_t time) {
  return m_time < time;
}

int ServerQueryThread::RequestChallenge::getChallenge() {
  return m_challenge;
}

bool ServerQueryThread::validChallenge(HostAddressWithPort const& address, char const* data, size_t len) {
  if (len != 9) {
    // too much or too little data
    return false;
  }

  if (m_validChallenges.count(address.address()) == 0) {
    // Don't know this source address ignore
    return false;
  }

  uint8_t const* b = (uint8_t const*)data;
  int32_t challenge = ((int32_t)b[8] & 0xff) << 24 | ((int32_t)b[7] & 0xff) << 16 | ((int32_t)b[6] & 0xff) << 8
      | ((int32_t)b[5] & 0xff);
  // Note: No byte order swapping needed as protcol performs no conversion
  if (m_validChallenges.get(address.address())->getChallenge() != challenge) {
    // Challenges didnt match ignore
    return false;
  }

  // All good
  return true;
}

bool ServerQueryThread::challengeRequest(HostAddressWithPort const& address, char const* data, size_t len) {
  if (len != 9) {
    // too much or too little data
    return false;
  }

  uint8_t const* buf = (uint8_t const*)data;
  if ((buf[5] == 0xff) && (buf[6] == 0xff) && (buf[7] == 0xff) && (buf[8] == 0xff)) {
    sendChallenge(address);
    return true;
  }

  return false;
}

}

module;
import star.json;
import star.xx_hash;
import star.thread;
import star.string;
import star.either;
import star.map;
import star.data_stream_devices;
import std;
import star.logging;
import star.ordered_map;
import star.bi_map;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.listener;
import star.time;
import star.format;
import star.output_proxy;
import star.version;
import star.id_map;
import star.lock_file;
import star.game_types;
import star.rpc_promise;
import star.byte_array;

import star.bytes;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.set;
import star.btree;
import star.lru_cache;
import star.btree_database;
import star.casting;
import star.ordered_set;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.strong_typedef;
import star.worker_pool;

import star.sector_array_2d;
import star.color;
import star.image;


import star.host_address;


import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
import star.configuration;
import star.universe;
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
import star.lua_root;
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
import star.lua_components;
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
import star.p2p_networking_service;
import star.net_packet_socket;
import star.universe_connection;
import star.universe_settings;
import star.universe_server;

export module star.server_query;

import star.udp;

export namespace Star {

class ServerQueryThread : public Thread {
public:
  [[nodiscard]] static std::expected<std::unique_ptr<ServerQueryThread>, StarException> create(UniverseServer* universe, HostAddressWithPort const& bindAddress);
  ~ServerQueryThread();

  [[nodiscard]] std::expected<void, StarException> start();
  void stop();

protected:
  [[nodiscard]] std::expected<void, StarException> run() override;

private:
  ServerQueryThread(UniverseServer* universe, std::unique_ptr<UdpServer> server, std::uint8_t maxPlayers, String serverName);
  static constexpr std::uint8_t A2A_PING_REQUEST = 0x69;
  static constexpr std::uint8_t A2A_PING_REPLY = 0x6a;
  static constexpr std::uint8_t A2S_CHALLENGE_REQUEST = 0x57;
  static constexpr std::uint8_t A2S_CHALLENGE_RESPONSE = 0x41;
  static constexpr std::uint8_t A2S_INFO_REQUEST = 0x54;
  static constexpr std::uint8_t A2S_INFO_REPLY = 0x49;
  static constexpr std::uint8_t A2S_PLAYER_REQUEST = 0x55;
  static constexpr std::uint8_t A2S_PLAYER_REPLY = 0x44;
  static constexpr std::uint8_t A2S_RULES_REQUEST = 0x56;
  static constexpr std::uint8_t A2S_RULES_REPLY = 0x45;
  static constexpr std::uint8_t A2S_VERSION = 0x07;
  static constexpr std::uint8_t A2S_STR_TERM = 0x00;
  static constexpr std::uint8_t A2S_EDF_GID = 0x01;
  static constexpr std::uint8_t A2S_EDF_SID = 0x10;
  static constexpr std::uint8_t A2S_EDF_TAGS = 0x20;
  static constexpr std::uint8_t A2S_EDF_STV = 0x40;
  static constexpr std::uint8_t A2S_EDF_PORT = 0x80;
  static constexpr std::uint8_t A2S_ENV_WINDOWS = 'w';
  static constexpr std::uint8_t A2S_ENV_LINUX = 'l';
  static constexpr std::uint8_t A2S_ENV_MAC = 'm';
  static constexpr std::uint8_t A2S_TYPE_DEDICATED = 'd';
  static constexpr std::uint8_t A2S_TYPE_LISTEN = 'l';
  static constexpr std::uint8_t A2S_TYPE_TV = 'p';
  static constexpr std::uint8_t A2S_VAC_OFF = 0x00;
  static constexpr std::uint8_t A2S_VAC_ON = 0x01;
  static constexpr const char* A2S_INFO_REQUEST_STRING = "Source Engine Query";
  static constexpr std::uint16_t A2S_APPID = (std::uint16_t)0xfffe;
  static constexpr std::uint16_t A2S_PACKET_SIZE = (std::uint16_t)0x4e0;
  static constexpr std::uint32_t A2S_HEAD_INT = 0xffffffff;
  static constexpr const char* GAME_DIR = "starbound";
  static constexpr const char* GAME_DESC = "Starbound";
  static constexpr const char* GAME_TYPE = "SMP";
  static constexpr std::int32_t challengeCheckInterval = 30000;
  static constexpr std::int32_t responseCacheTime = 5000;

  [[nodiscard]] std::expected<void, StarException> sendTo(HostAddressWithPort const& address, DataStreamBuffer* ds);
  [[nodiscard]] std::expected<bool, StarException> processPacket(HostAddressWithPort const& address, std::span<std::uint8_t const> data);
  [[nodiscard]] std::expected<void, StarException> buildPlayerResponse();
  [[nodiscard]] std::expected<void, StarException> buildRuleResponse();
  bool validChallenge(HostAddressWithPort const& address, std::span<std::uint8_t const> data);
  [[nodiscard]] std::expected<void, StarException> sendChallenge(HostAddressWithPort const& address);
  void pruneChallenges();
  [[nodiscard]] std::expected<bool, StarException> challengeRequest(HostAddressWithPort const& address, std::span<std::uint8_t const> data);

  // Server API
  std::uint8_t serverPlayerCount();
  bool serverPassworded();
  const char* serverPlugins();
  [[nodiscard]] std::expected<String, StarException> serverWorldNames();

  UniverseServer* m_universe;
  std::unique_ptr<UdpServer> m_queryServer;
  bool m_stop;
  DataStreamBuffer m_playersResponse;
  DataStreamBuffer m_rulesResponse;
  DataStreamBuffer m_generalResponse;

  class RequestChallenge {
  public:
    explicit RequestChallenge(RandomService& random);
    bool before(std::uint64_t time);
    std::int32_t getChallenge();

  private:
    std::uint64_t m_time;
    std::int32_t m_challenge;
  };

  std::uint16_t m_serverPort;
  std::uint8_t m_maxPlayers;
  String m_serverName;
  HashMap<HostAddress, std::shared_ptr<RequestChallenge>> m_validChallenges;
  std::int64_t m_lastChallengeCheck;
  std::int64_t m_lastPlayersResponse;
  std::int64_t m_lastRulesResponse;
  std::int64_t m_lastActiveTime;
};

}

namespace Star {

std::expected<std::unique_ptr<ServerQueryThread>, StarException> ServerQueryThread::create(UniverseServer* universe, HostAddressWithPort const& bindAddress) {
  auto server = UdpServer::create(universe->root().logger(), bindAddress);
  if (!server)
    return std::unexpected(std::move(server.error()));
  auto configuration = universe->root().configuration();
  if (!configuration)
    return std::unexpected(std::move(configuration.error()));
  auto maxPlayers = (*configuration)->get("maxPlayers");
  if (!maxPlayers)
    return std::unexpected(std::move(maxPlayers.error()));
  auto count = maxPlayers->toUInt();
  if (!count)
    return std::unexpected(std::move(count.error()));
  auto name = (*configuration)->get("serverName");
  if (!name)
    return std::unexpected(std::move(name.error()));
  auto text = name->toString();
  if (!text)
    return std::unexpected(std::move(text.error()));
  return std::unique_ptr<ServerQueryThread>(new ServerQueryThread(universe, std::move(*server), *count, std::move(*text)));
}

ServerQueryThread::ServerQueryThread(UniverseServer* universe, std::unique_ptr<UdpServer> server, std::uint8_t maxPlayers, String serverName)
  : Thread("QueryServer"), m_universe(universe), m_queryServer(std::move(server)), m_stop(true),
    m_playersResponse(A2S_PACKET_SIZE), m_rulesResponse(A2S_PACKET_SIZE), m_generalResponse(A2S_PACKET_SIZE),
    m_serverPort(0), m_maxPlayers(maxPlayers), m_serverName(std::move(serverName)),
    m_lastChallengeCheck(Time::monotonicMilliseconds()), m_lastPlayersResponse(0), m_lastRulesResponse(0), m_lastActiveTime(0) {
  m_playersResponse.setByteOrder(ByteOrder::LittleEndian);
  m_playersResponse.setNullTerminatedStrings(true);
  m_rulesResponse.setByteOrder(ByteOrder::LittleEndian);
  m_rulesResponse.setNullTerminatedStrings(true);
  m_generalResponse.setByteOrder(ByteOrder::LittleEndian);
  m_generalResponse.setNullTerminatedStrings(true);
}

ServerQueryThread::~ServerQueryThread() {
  stop();
  if (auto joined = join(); !joined)
    EmergencyLog::error("Query server shutdown failed: {}", joined.error().what());
}

std::expected<void, StarException> ServerQueryThread::start() {
  m_stop = false;
  auto started = Thread::start();
  if (!started) {
    m_stop = true;
    return std::unexpected(std::move(started.error()));
  }
  m_lastActiveTime = Time::monotonicMilliseconds();
  return {};
}

void ServerQueryThread::stop() {
  m_stop = true;
  m_queryServer->close();
}

std::expected<void, StarException> ServerQueryThread::sendTo(HostAddressWithPort const& address, DataStreamBuffer* ds) {
  auto sent = m_queryServer->send(address, ds->ptr(), ds->size());
  if (!sent)
    return std::unexpected(std::move(sent.error()));
  return {};
}

std::uint8_t ServerQueryThread::serverPlayerCount() {
  return m_universe->numberOfClients();
}

bool ServerQueryThread::serverPassworded() {
  // TODO: implement
  return false;
}

std::expected<String, StarException> ServerQueryThread::serverWorldNames() {
  auto activeWorlds = m_universe->activeWorlds();
  if (!activeWorlds)
    return std::unexpected(std::move(activeWorlds.error()));
  if (activeWorlds->empty())
    return String("Unknown");
  StringList names;
  for (auto const& world : *activeWorlds) {
    auto name = printWorldId(world);
    if (!name)
      return std::unexpected(std::move(name.error()));
    names.append(std::move(*name));
  }
  return names.join(",");
}

const char* ServerQueryThread::serverPlugins() {
  // TODO: implement
  return "none";
}

std::expected<bool, StarException> ServerQueryThread::processPacket(HostAddressWithPort const& address, std::span<std::uint8_t const> buf) {
  if (buf.size() < 5 || buf[0] != 0xff || buf[1] != 0xff || buf[2] != 0xff || buf[3] != 0xff) {
    // short packet or missing header
    return false;
  }

  // Process packet
  switch (buf[4]) {
    case A2S_INFO_REQUEST: {
      // Exclude the trailing byte, preserving the existing accepted payload.
      if (buf.size() < 6)
        return false;
      std::string_view str(reinterpret_cast<char const*>(buf.data() + 5), buf.size() - 6);
      if (str.compare(A2S_INFO_REQUEST_STRING) != 0) {
        // Invalid request
        return false;
      }

      auto worlds = serverWorldNames();
      if (!worlds)
        return std::unexpected(std::move(worlds.error()));
      m_generalResponse.clear();
      auto response = m_generalResponse << A2S_HEAD_INT << A2S_INFO_REPLY << A2S_VERSION << m_serverName << *worlds
                        << GAME_DIR << GAME_DESC << A2S_APPID // Should be SteamAppId but this isn't a short :(
                        << serverPlayerCount() << m_maxPlayers << (std::uint8_t)0x00 // bots
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
      if (!response)
        return std::unexpected(std::move(response.error()));

      auto sent = sendTo(address, &m_generalResponse);
      if (!sent)
        return std::unexpected(std::move(sent.error()));
      return true;
    }
    case A2S_CHALLENGE_REQUEST: {
      auto sent = sendChallenge(address);
      if (!sent)
        return std::unexpected(std::move(sent.error()));
      return true;
    }

    case A2S_PLAYER_REQUEST: {
      auto challenge = challengeRequest(address, buf);
      if (!challenge)
        return std::unexpected(std::move(challenge.error()));
      if (*challenge)
        return true;

      if (!validChallenge(address, buf))
        return false;

      auto response = buildPlayerResponse();
      if (!response)
        return std::unexpected(std::move(response.error()));
      auto sent = sendTo(address, &m_playersResponse);
      if (!sent)
        return std::unexpected(std::move(sent.error()));
      return true;
    }

    case A2S_RULES_REQUEST: {
      auto challenge = challengeRequest(address, buf);
      if (!challenge)
        return std::unexpected(std::move(challenge.error()));
      if (*challenge)
        return true;

      if (!validChallenge(address, buf))
        return false;

      auto response = buildRuleResponse();
      if (!response)
        return std::unexpected(std::move(response.error()));
      auto sent = sendTo(address, &m_rulesResponse);
      if (!sent)
        return std::unexpected(std::move(sent.error()));
      return true;
    }
  }

  return false;
}

std::expected<void, StarException> ServerQueryThread::buildPlayerResponse() {
  std::int64_t now = Time::monotonicMilliseconds();
  if (now < m_lastPlayersResponse + responseCacheTime)
    return {};
  auto clientIds = m_universe->clientIdsAndCreationTime();
  if (!clientIds)
    return std::unexpected(std::move(clientIds.error()));
  std::uint8_t count = static_cast<std::uint8_t>(clientIds->count());
  std::int32_t kills = 0;
  m_playersResponse.clear();
  auto header = m_playersResponse << A2S_HEAD_INT << A2S_PLAYER_REPLY << count;
  if (!header)
    return std::unexpected(std::move(header.error()));
  std::uint8_t i = 0;
  for (auto const& pair : *clientIds) {
    auto name = m_universe->clientNick(pair.first);
    if (!name)
      return std::unexpected(std::move(name.error()));
    auto timeConnected = float(now - pair.second) / 1000.f;
    auto player = m_playersResponse << i++ << *name << kills << timeConnected;
    if (!player)
      return std::unexpected(std::move(player.error()));
  }
  m_lastPlayersResponse = now;
  return {};
}

std::expected<void, StarException> ServerQueryThread::buildRuleResponse() {
  std::int64_t now = Time::monotonicMilliseconds();
  if (now < m_lastRulesResponse + responseCacheTime)
    return {};
  std::uint16_t count = 1;
  m_rulesResponse.clear();
  auto rules = m_rulesResponse << A2S_HEAD_INT << A2S_RULES_REPLY << count << "plugins" << serverPlugins();
  if (!rules)
    return std::unexpected(std::move(rules.error()));
  m_lastRulesResponse = now;
  return {};
}

std::expected<void, StarException> ServerQueryThread::sendChallenge(HostAddressWithPort const& address) {
  auto challenge = std::make_shared<RequestChallenge>(m_universe->root().random());
  m_validChallenges[address.address()] = challenge;
  m_generalResponse.clear();
  auto response = m_generalResponse << A2S_HEAD_INT << A2S_CHALLENGE_RESPONSE << challenge->getChallenge();
  if (!response)
    return std::unexpected(std::move(response.error()));
  return sendTo(address, &m_generalResponse);
}

void ServerQueryThread::pruneChallenges() {
  std::int64_t now = Time::monotonicMilliseconds();
  if (now < m_lastChallengeCheck + challengeCheckInterval) {
    return;
  }

  auto expire = now - challengeCheckInterval;
  for (auto it = m_validChallenges.begin(); it != m_validChallenges.end();) {
    if (it->second->before(expire))
      it = m_validChallenges.erase(it);
    else
      ++it;
  }
  m_lastChallengeCheck = now;
}

std::expected<void, StarException> ServerQueryThread::run() {
  HostAddressWithPort udpAddress;
  char udpData[MaxUdpData];
  auto report = [this](StarException const& error) -> std::expected<void, StarException> {
    if (!error.is<SocketClosedException>())
      return m_universe->root().logger().error("ServerQueryThread error: {}", outputException(ExceptionDiagnostic(error), true));
    return {};
  };
  while (!m_stop) {
    auto received = m_queryServer->receive(&udpAddress, udpData, MaxUdpData, 100);
    if (!received) {
      auto logged = report(received.error());
      if (!logged)
        return std::unexpected(std::move(logged.error()));
      continue;
    }
    pruneChallenges();
    if (*received != 0) {
      auto processed = processPacket(udpAddress, std::span(reinterpret_cast<std::uint8_t const*>(udpData), *received));
      if (!processed) {
        auto logged = report(processed.error());
        if (!logged)
          return std::unexpected(std::move(logged.error()));
      }
    }
  }
  return {};
}

ServerQueryThread::RequestChallenge::RequestChallenge(RandomService& random)
  : m_time(Time::monotonicMilliseconds()), m_challenge(random.randi32()) {}

bool ServerQueryThread::RequestChallenge::before(std::uint64_t time) {
  return m_time < time;
}

int ServerQueryThread::RequestChallenge::getChallenge() {
  return m_challenge;
}

bool ServerQueryThread::validChallenge(HostAddressWithPort const& address, std::span<std::uint8_t const> b) {
  if (b.size() != 9) {
    // too much or too little data
    return false;
  }

  if (m_validChallenges.count(address.address()) == 0) {
    // Don't know this source address ignore
    return false;
  }

  std::int32_t challenge = ((std::int32_t)b[8] & 0xff) << 24 | ((std::int32_t)b[7] & 0xff) << 16 | ((std::int32_t)b[6] & 0xff) << 8
      | ((std::int32_t)b[5] & 0xff);
  // Note: No byte order swapping needed as protcol performs no conversion
  if (m_validChallenges.ptr(address.address())->get()->getChallenge() != challenge) {
    // Challenges didnt match ignore
    return false;
  }

  // All good
  return true;
}

std::expected<bool, StarException> ServerQueryThread::challengeRequest(HostAddressWithPort const& address, std::span<std::uint8_t const> buf) {
  if (buf.size() != 9) {
    // too much or too little data
    return false;
  }

  if ((buf[5] == 0xff) && (buf[6] == 0xff) && (buf[7] == 0xff) && (buf[8] == 0xff)) {
    auto sent = sendChallenge(address);
    if (!sent)
      return std::unexpected(std::move(sent.error()));
    return true;
  }

  return false;
}

}

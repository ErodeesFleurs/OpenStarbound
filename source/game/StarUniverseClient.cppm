module;
import star.xx_hash;
import star.json;
import star.game_types;
import star.maybe;
import star.string;
import star.either;
import star.vector;
import star.variant;
import star.color;
import star.bi_map;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.strong_typedef;
import star.ordered_set;
import star.io_device;
import star.byte_array;
import star.data_stream_devices;

import std;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.lua;
import star.id_map;
import star.version;
import star.ordered_map;
import star.casting;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.btree_database;
import star.rpc_promise;
import star.image;
import star.audio;
import star.map;

import star.host_address;
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
import star.uuid;
import star.warping;
import star.item_descriptor;
import star.quest_descriptor;
import star.ai_types;
import star.socket;
import star.tcp;
import star.p2p_networking_service;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.world_geometry;
import star.wiring;
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
import star.drawable;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.mixer;
import star.entity_rendering;
import star.world;
import star.lua_components;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe

namespace Star {

class WorldTemplate;
class ClientContext;
class Sky;
struct Packet;
class WorldClient;
class SystemWorldClient;
class Player;
class PlayerStorage;
class Statistics;
class Clock;
class CelestialLog;
class CelestialSlaveDatabase;
class CelestialDatabase;
class JsonRpcInterface;
class TeamClient;
class QuestManager;
class LuaRoot;

class UniverseClient : public Universe {
public:
  typedef LuaMessageHandlingComponent<LuaUpdatableComponent<LuaBaseComponent>> ScriptComponent;
  
  UniverseClient(std::shared_ptr<PlayerStorage> playerStorage, std::shared_ptr<Statistics> statistics, String const& customWorldStorageDir);
  ~UniverseClient() override;

  void setMainPlayer(std::shared_ptr<Player> player);
  std::shared_ptr<Player> mainPlayer() const;

  // Returns error if connection failed
  Maybe<String> connect(UniverseConnection connection, bool allowAssetsMismatch, String const& account = "", String const& password = "", bool const& forceLegacy = false);
  bool isConnected() const;
  void disconnect();
  Maybe<String> disconnectReason() const;
  
  unsigned connectionVersion();

  // WorldClient may be null if the UniverseClient is not connected.
  std::shared_ptr<WorldClient> worldClient() const;
  std::shared_ptr<SystemWorldClient> systemWorldClient() const;

  // Updates internal world client in addition to handling universe level
  // commands.
  void update(float dt);

  Maybe<BeamUpRule> beamUpRule() const;
  bool canBeamUp() const;
  bool canBeamDown(bool deploy = false) const;
  bool canBeamToTeamShip() const;
  bool canTeleport() const;

  void warpPlayer(WarpAction const& warpAction, bool animate = true, String const& animationType = "default", bool deploy = false);
  void flyShip(Vec3I const& system, SystemLocation const& destination, Json const& settings = {});

  std::shared_ptr<CelestialDatabase> celestialDatabase() override;

  CelestialCoordinate shipCoordinate() const;

  bool playerOnOwnShip() const;
  bool playerIsOriginal() const;

  WorldId playerWorld() const;
  bool isAdmin() const;
  // If the player is in a multi person team returns the team uuid, or if the
  // player is by themselves returns the player uuid.
  Uuid teamUuid() const;

  std::shared_ptr<WorldTemplate const> currentTemplate() const;
  std::shared_ptr<Sky const> currentSky() const;
  bool flying() const;

  void sendChat(String const& text, ChatSendMode sendMode, Maybe<bool> speak = {}, Maybe<JsonObject> data = {});
  List<ChatReceivedMessage> pullChatMessages();

  std::uint16_t players();
  std::uint16_t maxPlayers();

  void setLuaCallbacks(String const& groupName, LuaCallbacks const& callbacks);
  void restartLua();
  void startLuaScripts();
  void stopLua();
  std::shared_ptr<LuaRoot> luaRoot();

  bool reloadPlayer(Json const& data, Uuid const& uuid, bool resetInterfaces = false, bool showIndicator = false);
  bool switchPlayer(Uuid const& uuid);
  bool switchPlayer(std::size_t index);
  bool switchPlayer(String const& name);

  typedef std::function<void()> Callback;
  typedef std::function<void(bool)> ReloadPlayerCallback;
  ReloadPlayerCallback& playerReloadPreCallback();
  ReloadPlayerCallback& playerReloadCallback();

  std::shared_ptr<Clock const> universeClock() const;
  std::shared_ptr<CelestialLog const> celestialLog() const;
  std::shared_ptr<JsonRpcInterface> rpcInterface() const;
  std::shared_ptr<ClientContext> clientContext() const;
  std::shared_ptr<TeamClient> teamClient() const;
  std::shared_ptr<QuestManager> questManager() const;
  std::shared_ptr<PlayerStorage> playerStorage() const;
  std::shared_ptr<Statistics> statistics() const;

  std::shared_ptr<UniverseClient::ScriptComponent> scriptContext(String const& contextName);
  
  void createCustomWorld(String const& name, Json templateData);
  
  ClientSubWorldId createSubWorld();
  void setSubWorldWorld(ClientSubWorldId subWorldId, WorldId worldId = WorldId());
  bool subWorldExistsOnWorld(WorldId worldId) const;
  ClientSubWorldId getSubWorldOnWorld(WorldId worldId);
  void destroySubWorldOnWorld(WorldId worldId);

  RpcPromise<Json> sendSubWorldOnWorldMessage(WorldId const& worldId, String const& message, JsonArray const& args = {});
  RpcPromise<Json> sendMainWorldMessage(String const& message, JsonArray const& args = {});

  bool paused() const;
  
  void passMessage(Universe::Message&& message) override;
  RpcPromise<Json> sendUniverseMessage(ConnectionId const& connectionId, String const& message, JsonArray const& args = {}) override;
  
  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool const& local, JsonArray const& args) override;

private:
  struct ServerInfo {
    std::uint16_t players;
    std::uint16_t maxPlayers;
  };

  void setPause(bool pause);

  void handlePackets(List<std::shared_ptr<Packet>> const& packets);
  void reset();
  
  mutable RecursiveMutex m_mutex;

  std::shared_ptr<PlayerStorage> m_playerStorage;
  std::shared_ptr<Statistics> m_statistics;
  std::shared_ptr<Player> m_mainPlayer;
  
  String m_customWorldStorageDirectory;

  std::shared_ptr<std::atomic<bool>> m_pause;
  std::shared_ptr<Clock> m_universeClock;
  std::shared_ptr<WorldClient> m_worldClient;
  std::shared_ptr<SystemWorldClient> m_systemWorldClient;
  Maybe<UniverseConnection> m_connection;
  Maybe<ServerInfo> m_serverInfo;

  std::shared_ptr<CelestialSlaveDatabase> m_celestialDatabase;
  std::shared_ptr<ClientContext> m_clientContext;
  std::shared_ptr<TeamClient> m_teamClient;

  std::shared_ptr<QuestManager> m_questManager;

  WarpAction m_pendingWarp;
  GameTimer m_warpDelay;
  Maybe<GameTimer> m_warpCinemaCancelTimer;

  Maybe<WarpAction> m_warping;
  bool m_respawnWarped;
  bool m_respawning;
  GameTimer m_respawnTimer;

  std::int64_t m_storageTriggerDeadline;

  List<ChatReceivedMessage> m_pendingMessages;

  Maybe<String> m_disconnectReason;

  std::shared_ptr<LuaRoot> m_luaRoot;
  
  StringMap<std::shared_ptr<UniverseClient::ScriptComponent>> m_scriptContexts;
  
  BiMap<ClientSubWorldId, WorldId> m_subWorlds;
  IdMap<ClientSubWorldId,std::shared_ptr<WorldClientThread>> m_subWorldThreads;

  ReloadPlayerCallback m_playerReloadPreCallback;
  ReloadPlayerCallback m_playerReloadCallback;

  mutable RecursiveMutex m_messageMutex;
  List<Universe::Message> m_universeMessages;
  
  HashMap<Uuid, RpcPromiseKeeper<Json>> m_universeMessageResponses;
  HashMap<Uuid, RpcPromise<Json>> m_universeMessagePromises;
};

}


export module star.universe_client;

export namespace Star {
  using ::Star::WorldTemplate;
  using ::Star::ClientContext;
  using ::Star::Sky;
  using ::Star::Packet;
  using ::Star::WorldClient;
  using ::Star::SystemWorldClient;
  using ::Star::Player;
  using ::Star::PlayerStorage;
  using ::Star::Statistics;
  using ::Star::Clock;
  using ::Star::CelestialLog;
  using ::Star::CelestialSlaveDatabase;
  using ::Star::CelestialDatabase;
  using ::Star::JsonRpcInterface;
  using ::Star::TeamClient;
  using ::Star::QuestManager;
  using ::Star::UniverseClient;
  using ::Star::LuaRoot;
}


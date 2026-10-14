#pragma once
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
#include "StarStrongTypedef.hpp"
import star.ordered_set;
import star.io_device;
import star.byte_array;
#include "StarDataStreamDevices.hpp"

import std;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
#include "StarConfig.hpp"
import star.lua;
import star.id_map;
import star.version;
import star.ordered_map;
#include "StarCasting.hpp"
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
#include "StarBTree.hpp"
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
#include "StarLuaComponents.hpp"
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

STAR_CLASS(WorldTemplate);
STAR_CLASS(ClientContext);
STAR_CLASS(Sky);
STAR_STRUCT(Packet);
STAR_CLASS(WorldClient);
STAR_CLASS(SystemWorldClient);
STAR_CLASS(Player);
STAR_CLASS(PlayerStorage);
STAR_CLASS(Statistics);
STAR_CLASS(Clock);
STAR_CLASS(CelestialLog);
STAR_CLASS(CelestialSlaveDatabase);
STAR_CLASS(CelestialDatabase);
STAR_CLASS(JsonRpcInterface);
STAR_CLASS(TeamClient);
STAR_CLASS(QuestManager);
STAR_CLASS(UniverseClient);
STAR_CLASS(LuaRoot);

class UniverseClient : public Universe {
public:
  typedef LuaMessageHandlingComponent<LuaUpdatableComponent<LuaBaseComponent>> ScriptComponent;
  typedef shared_ptr<ScriptComponent> ScriptComponentPtr;
  
  UniverseClient(PlayerStoragePtr playerStorage, StatisticsPtr statistics, String const& customWorldStorageDir);
  ~UniverseClient() override;

  void setMainPlayer(PlayerPtr player);
  PlayerPtr mainPlayer() const;

  // Returns error if connection failed
  Maybe<String> connect(UniverseConnection connection, bool allowAssetsMismatch, String const& account = "", String const& password = "", bool const& forceLegacy = false);
  bool isConnected() const;
  void disconnect();
  Maybe<String> disconnectReason() const;
  
  unsigned connectionVersion();

  // WorldClient may be null if the UniverseClient is not connected.
  WorldClientPtr worldClient() const;
  SystemWorldClientPtr systemWorldClient() const;

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

  CelestialDatabasePtr celestialDatabase() override;

  CelestialCoordinate shipCoordinate() const;

  bool playerOnOwnShip() const;
  bool playerIsOriginal() const;

  WorldId playerWorld() const;
  bool isAdmin() const;
  // If the player is in a multi person team returns the team uuid, or if the
  // player is by themselves returns the player uuid.
  Uuid teamUuid() const;

  WorldTemplateConstPtr currentTemplate() const;
  SkyConstPtr currentSky() const;
  bool flying() const;

  void sendChat(String const& text, ChatSendMode sendMode, Maybe<bool> speak = {}, Maybe<JsonObject> data = {});
  List<ChatReceivedMessage> pullChatMessages();

  uint16_t players();
  uint16_t maxPlayers();

  void setLuaCallbacks(String const& groupName, LuaCallbacks const& callbacks);
  void restartLua();
  void startLuaScripts();
  void stopLua();
  LuaRootPtr luaRoot();

  bool reloadPlayer(Json const& data, Uuid const& uuid, bool resetInterfaces = false, bool showIndicator = false);
  bool switchPlayer(Uuid const& uuid);
  bool switchPlayer(size_t index);
  bool switchPlayer(String const& name);

  typedef std::function<void()> Callback;
  typedef std::function<void(bool)> ReloadPlayerCallback;
  ReloadPlayerCallback& playerReloadPreCallback();
  ReloadPlayerCallback& playerReloadCallback();

  ClockConstPtr universeClock() const;
  CelestialLogConstPtr celestialLog() const;
  JsonRpcInterfacePtr rpcInterface() const;
  ClientContextPtr clientContext() const;
  TeamClientPtr teamClient() const;
  QuestManagerPtr questManager() const;
  PlayerStoragePtr playerStorage() const;
  StatisticsPtr statistics() const;

  ScriptComponentPtr scriptContext(String const& contextName);
  
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
    uint16_t players;
    uint16_t maxPlayers;
  };

  void setPause(bool pause);

  void handlePackets(List<PacketPtr> const& packets);
  void reset();
  
  mutable RecursiveMutex m_mutex;

  PlayerStoragePtr m_playerStorage;
  StatisticsPtr m_statistics;
  PlayerPtr m_mainPlayer;
  
  String m_customWorldStorageDirectory;

  shared_ptr<atomic<bool>> m_pause;
  ClockPtr m_universeClock;
  WorldClientPtr m_worldClient;
  SystemWorldClientPtr m_systemWorldClient;
  Maybe<UniverseConnection> m_connection;
  Maybe<ServerInfo> m_serverInfo;

  CelestialSlaveDatabasePtr m_celestialDatabase;
  ClientContextPtr m_clientContext;
  TeamClientPtr m_teamClient;

  QuestManagerPtr m_questManager;

  WarpAction m_pendingWarp;
  GameTimer m_warpDelay;
  Maybe<GameTimer> m_warpCinemaCancelTimer;

  Maybe<WarpAction> m_warping;
  bool m_respawnWarped;
  bool m_respawning;
  GameTimer m_respawnTimer;

  int64_t m_storageTriggerDeadline;

  List<ChatReceivedMessage> m_pendingMessages;

  Maybe<String> m_disconnectReason;

  LuaRootPtr m_luaRoot;
  
  StringMap<ScriptComponentPtr> m_scriptContexts;
  
  BiMap<ClientSubWorldId, WorldId> m_subWorlds;
  IdMap<ClientSubWorldId,WorldClientThreadPtr> m_subWorldThreads;

  ReloadPlayerCallback m_playerReloadPreCallback;
  ReloadPlayerCallback m_playerReloadCallback;

  mutable RecursiveMutex m_messageMutex;
  List<Universe::Message> m_universeMessages;
  
  HashMap<Uuid, RpcPromiseKeeper<Json>> m_universeMessageResponses;
  HashMap<Uuid, RpcPromise<Json>> m_universeMessagePromises;
};

}


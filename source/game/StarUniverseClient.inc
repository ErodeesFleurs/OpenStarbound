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

class WorldTemplate;
using WorldTemplatePtr = std::shared_ptr<WorldTemplate>;
using WorldTemplateConstPtr = std::shared_ptr<const WorldTemplate>;
using WorldTemplateWeakPtr = std::weak_ptr<WorldTemplate>;
using WorldTemplateConstWeakPtr = std::weak_ptr<const WorldTemplate>;
using WorldTemplateUPtr = std::unique_ptr<WorldTemplate>;
using WorldTemplateConstUPtr = std::unique_ptr<const WorldTemplate>;
class ClientContext;
using ClientContextPtr = std::shared_ptr<ClientContext>;
using ClientContextConstPtr = std::shared_ptr<const ClientContext>;
using ClientContextWeakPtr = std::weak_ptr<ClientContext>;
using ClientContextConstWeakPtr = std::weak_ptr<const ClientContext>;
using ClientContextUPtr = std::unique_ptr<ClientContext>;
using ClientContextConstUPtr = std::unique_ptr<const ClientContext>;
class Sky;
using SkyPtr = std::shared_ptr<Sky>;
using SkyConstPtr = std::shared_ptr<const Sky>;
using SkyWeakPtr = std::weak_ptr<Sky>;
using SkyConstWeakPtr = std::weak_ptr<const Sky>;
using SkyUPtr = std::unique_ptr<Sky>;
using SkyConstUPtr = std::unique_ptr<const Sky>;
struct Packet;
using PacketPtr = std::shared_ptr<Packet>;
using PacketConstPtr = std::shared_ptr<const Packet>;
using PacketWeakPtr = std::weak_ptr<Packet>;
using PacketConstWeakPtr = std::weak_ptr<const Packet>;
using PacketUPtr = std::unique_ptr<Packet>;
using PacketConstUPtr = std::unique_ptr<const Packet>;
class WorldClient;
using WorldClientPtr = std::shared_ptr<WorldClient>;
using WorldClientConstPtr = std::shared_ptr<const WorldClient>;
using WorldClientWeakPtr = std::weak_ptr<WorldClient>;
using WorldClientConstWeakPtr = std::weak_ptr<const WorldClient>;
using WorldClientUPtr = std::unique_ptr<WorldClient>;
using WorldClientConstUPtr = std::unique_ptr<const WorldClient>;
class SystemWorldClient;
using SystemWorldClientPtr = std::shared_ptr<SystemWorldClient>;
using SystemWorldClientConstPtr = std::shared_ptr<const SystemWorldClient>;
using SystemWorldClientWeakPtr = std::weak_ptr<SystemWorldClient>;
using SystemWorldClientConstWeakPtr = std::weak_ptr<const SystemWorldClient>;
using SystemWorldClientUPtr = std::unique_ptr<SystemWorldClient>;
using SystemWorldClientConstUPtr = std::unique_ptr<const SystemWorldClient>;
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<const Player>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<const Player>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<const Player>;
class PlayerStorage;
using PlayerStoragePtr = std::shared_ptr<PlayerStorage>;
using PlayerStorageConstPtr = std::shared_ptr<const PlayerStorage>;
using PlayerStorageWeakPtr = std::weak_ptr<PlayerStorage>;
using PlayerStorageConstWeakPtr = std::weak_ptr<const PlayerStorage>;
using PlayerStorageUPtr = std::unique_ptr<PlayerStorage>;
using PlayerStorageConstUPtr = std::unique_ptr<const PlayerStorage>;
class Statistics;
using StatisticsPtr = std::shared_ptr<Statistics>;
using StatisticsConstPtr = std::shared_ptr<const Statistics>;
using StatisticsWeakPtr = std::weak_ptr<Statistics>;
using StatisticsConstWeakPtr = std::weak_ptr<const Statistics>;
using StatisticsUPtr = std::unique_ptr<Statistics>;
using StatisticsConstUPtr = std::unique_ptr<const Statistics>;
class Clock;
using ClockPtr = std::shared_ptr<Clock>;
using ClockConstPtr = std::shared_ptr<const Clock>;
using ClockWeakPtr = std::weak_ptr<Clock>;
using ClockConstWeakPtr = std::weak_ptr<const Clock>;
using ClockUPtr = std::unique_ptr<Clock>;
using ClockConstUPtr = std::unique_ptr<const Clock>;
class CelestialLog;
using CelestialLogPtr = std::shared_ptr<CelestialLog>;
using CelestialLogConstPtr = std::shared_ptr<const CelestialLog>;
using CelestialLogWeakPtr = std::weak_ptr<CelestialLog>;
using CelestialLogConstWeakPtr = std::weak_ptr<const CelestialLog>;
using CelestialLogUPtr = std::unique_ptr<CelestialLog>;
using CelestialLogConstUPtr = std::unique_ptr<const CelestialLog>;
class CelestialSlaveDatabase;
using CelestialSlaveDatabasePtr = std::shared_ptr<CelestialSlaveDatabase>;
using CelestialSlaveDatabaseConstPtr = std::shared_ptr<const CelestialSlaveDatabase>;
using CelestialSlaveDatabaseWeakPtr = std::weak_ptr<CelestialSlaveDatabase>;
using CelestialSlaveDatabaseConstWeakPtr = std::weak_ptr<const CelestialSlaveDatabase>;
using CelestialSlaveDatabaseUPtr = std::unique_ptr<CelestialSlaveDatabase>;
using CelestialSlaveDatabaseConstUPtr = std::unique_ptr<const CelestialSlaveDatabase>;
class CelestialDatabase;
using CelestialDatabasePtr = std::shared_ptr<CelestialDatabase>;
using CelestialDatabaseConstPtr = std::shared_ptr<const CelestialDatabase>;
using CelestialDatabaseWeakPtr = std::weak_ptr<CelestialDatabase>;
using CelestialDatabaseConstWeakPtr = std::weak_ptr<const CelestialDatabase>;
using CelestialDatabaseUPtr = std::unique_ptr<CelestialDatabase>;
using CelestialDatabaseConstUPtr = std::unique_ptr<const CelestialDatabase>;
class JsonRpcInterface;
using JsonRpcInterfacePtr = std::shared_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstPtr = std::shared_ptr<const JsonRpcInterface>;
using JsonRpcInterfaceWeakPtr = std::weak_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstWeakPtr = std::weak_ptr<const JsonRpcInterface>;
using JsonRpcInterfaceUPtr = std::unique_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstUPtr = std::unique_ptr<const JsonRpcInterface>;
class TeamClient;
using TeamClientPtr = std::shared_ptr<TeamClient>;
using TeamClientConstPtr = std::shared_ptr<const TeamClient>;
using TeamClientWeakPtr = std::weak_ptr<TeamClient>;
using TeamClientConstWeakPtr = std::weak_ptr<const TeamClient>;
using TeamClientUPtr = std::unique_ptr<TeamClient>;
using TeamClientConstUPtr = std::unique_ptr<const TeamClient>;
class QuestManager;
using QuestManagerPtr = std::shared_ptr<QuestManager>;
using QuestManagerConstPtr = std::shared_ptr<const QuestManager>;
using QuestManagerWeakPtr = std::weak_ptr<QuestManager>;
using QuestManagerConstWeakPtr = std::weak_ptr<const QuestManager>;
using QuestManagerUPtr = std::unique_ptr<QuestManager>;
using QuestManagerConstUPtr = std::unique_ptr<const QuestManager>;
class UniverseClient;
using UniverseClientPtr = std::shared_ptr<UniverseClient>;
using UniverseClientConstPtr = std::shared_ptr<const UniverseClient>;
using UniverseClientWeakPtr = std::weak_ptr<UniverseClient>;
using UniverseClientConstWeakPtr = std::weak_ptr<const UniverseClient>;
using UniverseClientUPtr = std::unique_ptr<UniverseClient>;
using UniverseClientConstUPtr = std::unique_ptr<const UniverseClient>;
class LuaRoot;
using LuaRootPtr = std::shared_ptr<LuaRoot>;
using LuaRootConstPtr = std::shared_ptr<const LuaRoot>;
using LuaRootWeakPtr = std::weak_ptr<LuaRoot>;
using LuaRootConstWeakPtr = std::weak_ptr<const LuaRoot>;
using LuaRootUPtr = std::unique_ptr<LuaRoot>;
using LuaRootConstUPtr = std::unique_ptr<const LuaRoot>;

class UniverseClient : public Universe {
public:
  typedef LuaMessageHandlingComponent<LuaUpdatableComponent<LuaBaseComponent>> ScriptComponent;
  typedef std::shared_ptr<ScriptComponent> ScriptComponentPtr;
  
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

  std::uint16_t players();
  std::uint16_t maxPlayers();

  void setLuaCallbacks(String const& groupName, LuaCallbacks const& callbacks);
  void restartLua();
  void startLuaScripts();
  void stopLua();
  LuaRootPtr luaRoot();

  bool reloadPlayer(Json const& data, Uuid const& uuid, bool resetInterfaces = false, bool showIndicator = false);
  bool switchPlayer(Uuid const& uuid);
  bool switchPlayer(std::size_t index);
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
    std::uint16_t players;
    std::uint16_t maxPlayers;
  };

  void setPause(bool pause);

  void handlePackets(List<PacketPtr> const& packets);
  void reset();
  
  mutable RecursiveMutex m_mutex;

  PlayerStoragePtr m_playerStorage;
  StatisticsPtr m_statistics;
  PlayerPtr m_mainPlayer;
  
  String m_customWorldStorageDirectory;

  std::shared_ptr<std::atomic<bool>> m_pause;
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

  std::int64_t m_storageTriggerDeadline;

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


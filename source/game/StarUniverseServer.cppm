module;
import star.xx_hash;
import star.json;
import star.id_map;
import star.format;
import star.maybe;
import star.string;
import star.lock_file;
import star.thread;
import star.game_types;
import star.vector;
import star.either;
import star.data_stream;
import star.rpc_promise;
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
import star.set;
import star.btree;
import star.ordered_map;
import star.lru_cache;
import star.btree_database;
import star.casting;
import star.ordered_set;
import star.list;
import star.bi_map;
import star.version;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.strong_typedef;
import star.worker_pool;

import star.sector_array_2d;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.variant;
import star.image;
import star.map;

import star.universe;
import star.celestial_coordinate;
import star.host_address;
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

namespace Star {

class Clock;
class File;
class Player;
class ChatProcessor;
class CommandProcessor;
class TeamManager;
class WorldTemplate;
class CelestialMasterDatabase;
class WorldServer;
class UniverseSettings;
class ServerClientContext;

struct UniverseServerExceptionTag {
  static constexpr char const* name() { return "UniverseServerException"; }
};
using UniverseServerException = StarError<UniverseServerExceptionTag, StarException>;

// Manages all running worlds, listens for new client connections and marshalls
// between all the different worlds and all the different client connections
// and routes packets between them.
class UniverseServer : public Universe, public Thread {
public:
  UniverseServer(String const& storageDir, bool const& isLocal = false);
  ~UniverseServer() override;

  // If enabled, will listen on the configured server port for incoming
  // connections.
  void setListeningTcp(bool listenTcp);

  // Connects an arbitrary UniverseConnection to this server
  void addClient(UniverseConnection remoteConnection);
  // Constructs an in-process connection to a UniverseServer for a
  // UniverseClient, and returns the other side of the connection.
  UniverseConnection addLocalClient();

  // Signals the UniverseServer to stop and then joins the thread.
  void stop();

  void setPause(bool pause);
  void setTimescale(float timescale);
  void setTickRate(float tickRate);

  List<WorldId> activeWorlds() const;
  bool isWorldActive(WorldId const& worldId) const;

  List<ConnectionId> clientIds() const;
  List<std::pair<ConnectionId, std::int64_t>> clientIdsAndCreationTime() const;
  std::size_t numberOfClients() const;
  std::uint32_t maxClients() const;
  bool isConnectedClient(ConnectionId clientId) const;

  String clientDescriptor(ConnectionId clientId) const;
  unsigned clientConnectionVersion(ConnectionId clientId) const;

  String clientNick(ConnectionId clientId) const;
  Maybe<ConnectionId> findNick(String const& nick) const;

  String clientAccount(ConnectionId clientId) const;

  Maybe<Uuid> uuidForClient(ConnectionId clientId) const;
  Maybe<ConnectionId> clientForUuid(Uuid const& uuid) const;

  void adminBroadcast(String const& text);
  void adminWhisper(ConnectionId clientId, String const& text);
  ServerCommandResult adminCommand(String text);

  bool isAdmin(ConnectionId clientId) const;
  bool canBecomeAdmin(ConnectionId clientId) const;
  void setAdmin(ConnectionId clientId, bool admin);

  bool serverDebug(ConnectionId clientId) const;
  void setServerDebug(ConnectionId clientId, bool serverDebug);

  bool isLocal(ConnectionId clientId) const;

  bool isPvp(ConnectionId clientId) const;
  void setPvp(ConnectionId clientId, bool pvp);

  RpcPromise<Json> sendWorldMessage(WorldId const& worldId, String const& message, JsonArray const& args = {});

  void clientWarpPlayer(ConnectionId clientId, WarpAction action, bool deploy = false);
  void clientFlyShip(ConnectionId clientId, Vec3I const& system, SystemLocation const& location, Json const& settings = {});
  WorldId clientWorld(ConnectionId clientId) const;
  CelestialCoordinate clientShipCoordinate(ConnectionId clientId) const;

  std::shared_ptr<Clock> universeClock() const;
  std::shared_ptr<UniverseSettings> universeSettings() const;

  std::shared_ptr<CelestialDatabase> celestialDatabase() override;

  // If the client exists and is in a valid connection state, executes the
  // given function on the client world and player object in a thread safe way.
  // Returns true if function was called, false if client was not found or in
  // an invalid connection state.
  bool executeForClient(ConnectionId clientId, std::function<void(WorldServer*, std::shared_ptr<Player>)> action);
  void disconnectClient(ConnectionId clientId, String const& reason);
  void banUser(ConnectionId clientId, String const& reason, std::pair<bool, bool> banType, Maybe<int> timeout);
  bool unbanIp(String const& addressString);
  bool unbanUuid(String const& uuidString);

  bool updatePlanetType(CelestialCoordinate const& coordinate, String const& newType, String const& weatherBiome);

  bool setWeather(CelestialCoordinate const& coordinate, String const& weatherName, bool force = false);

  StringList weatherList(CelestialCoordinate const& coordinate);
  
  void createCustomWorld(CustomWorldId const& customWorld, std::shared_ptr<WorldTemplate> worldTemplate);

  bool sendPacket(ConnectionId clientId, std::shared_ptr<Packet> packet);
  
  void passMessage(Universe::Message&& message) override;
  
  RpcPromise<Json> sendUniverseMessage(ConnectionId const& connectionId, String const& message, JsonArray const& args = {}) override;
  
  Maybe<ChainableJsonMessageResponse> receiveMessage(String const& message, bool const& local, JsonArray const& args) override;

protected:
  virtual void run() override;

private:
  struct TimeoutBan {
    std::int64_t banExpiry;
    String reason;
    Maybe<HostAddress> ip;
    Maybe<Uuid> uuid;
  };
  
  struct WorldServerPromise {
    Variant<WorkerPoolPromise<std::shared_ptr<WorldServerThread>>,RpcPromise<WorldChunks>> currentPromise;
    Maybe<std::function<WorkerPoolPromise<std::shared_ptr<WorldServerThread>>(WorldChunks)>> producer;
    
    double startTime;
    
    WorldServerPromise(std::function<WorkerPoolPromise<std::shared_ptr<WorldServerThread>>(WorldChunks)> producer, RpcPromise<WorldChunks> promise);
    WorldServerPromise(WorkerPoolPromise<std::shared_ptr<WorldServerThread>> promise);
    
    bool done() const;
    bool poll();
    
    std::shared_ptr<WorldServerThread> get();
  };

  enum class TcpState : std::uint8_t { No, Yes, Fuck };

  void processUniverseFlags();
  void sendPendingChat();
  void updateTeams();
  void updateShips();
  void sendClockUpdates();
  void sendClientContextUpdate(std::shared_ptr<ServerClientContext> clientContext);
  void sendClientContextUpdates();
  void kickErroredPlayers();
  void reapConnections();
  void processPlanetTypeChanges();
  void warpPlayers();
  void flyShips();
  void arriveShips();
  void respondToCelestialRequests();
  void processChat();
  void clearBrokenWorlds();
  void handleWorldMessages();
  void handleUniverseMessages();
  void shutdownInactiveWorlds();
  void doTriggeredStorage();

  void saveSettings();
  void loadSettings();

  void startLuaScripts();
  void updateLua();
  void stopLua();

  // Either returns the default configured starter world, or a new randomized
  // starter world, or if a randomized world is not yet available, starts a job
  // to find a randomized starter world and returns nothing until it is ready.
  Maybe<CelestialCoordinate> nextStarterWorld();

  void loadTempWorldIndex();
  void saveTempWorldIndex();
  String tempWorldFile(InstanceWorldId const& worldId) const;

  Maybe<String> isBannedUser(Maybe<HostAddress> hostAddress, Uuid playerUuid) const;
  void doTempBan(ConnectionId clientId, String const& reason, std::pair<bool, bool> banType, int timeout);
  void doPermBan(ConnectionId clientId, String const& reason, std::pair<bool, bool> banType);
  void removeTimedBan();

  void addCelestialRequests(ConnectionId clientId, List<CelestialRequest> requests);

  void worldUpdated(WorldServerThread* worldServer);
  void systemWorldUpdated(SystemWorldServerThread* systemWorldServer);
  void packetsReceived(UniverseConnectionServer* connectionServer, ConnectionId clientId, List<std::shared_ptr<Packet>> packets);

  void acceptConnection(UniverseConnection connection, Maybe<HostAddress> remoteAddress);

  // Main lock and clients read lock must be held when calling
  WarpToWorld resolveWarpAction(WarpAction warpAction, ConnectionId clientId, bool deploy) const;
  bool canWarpToShip(ConnectionId clientId, Uuid const& targetShipUuid) const;

  void doDisconnection(ConnectionId clientId, String const& reason);

  // Clients read lock must be held when calling
  Maybe<ConnectionId> getClientForUuid(Uuid const& uuid) const;

  // Get the world only if it is already loaded, Main lock must be held when
  // calling.
  std::shared_ptr<WorldServerThread> getWorld(WorldId const& worldId);

  // If the world is not created, block and load it, otherwise just return the
  // loaded world.  Main lock and Clients read lock must be held when calling.
  std::shared_ptr<WorldServerThread> createWorld(WorldId const& worldId);

  // Trigger off-thread world creation, returns a value when the creation is
  // finished, either successfully or with an error.  Main lock and Clients
  // read lock must be held when calling.
  Maybe<std::shared_ptr<WorldServerThread>> triggerWorldCreation(WorldId const& worldId);

  // Main lock and clients read lock must be held when calling world promise
  // generators
  Maybe<WorldServerPromise> makeWorldPromise(WorldId const& worldId);
  Maybe<WorldServerPromise> shipWorldPromise(ClientShipWorldId const& uuid);
  Maybe<WorldServerPromise> celestialWorldPromise(CelestialWorldId const& coordinate);
  Maybe<WorldServerPromise> instanceWorldPromise(InstanceWorldId const& instanceWorld);
  Maybe<WorldServerPromise> customWorldPromise(CustomWorldId const& customWorld, Maybe<std::shared_ptr<WorldTemplate>> worldTemplate = {});
  Maybe<WorldServerPromise> clientCustomWorldPromise(ClientCustomWorldId const& clientCustomWorld, Maybe<std::shared_ptr<WorldTemplate>> worldTemplate = {});
  
  void notifyWorldCreated(WorldId const& worldId);

  // If the system world is not created, initialize it, otherwise return the
  // already initialized one
  std::shared_ptr<SystemWorldServerThread> createSystemWorld(Vec3I const& location);

  bool instanceWorldStoredOrActive(InstanceWorldId const& worldId) const;

  // Signal that a world either failed to load, or died due to an exception,
  // kicks clients if that world is a ship world.  Main lock and clients read
  // lock must be held when calling.
  void worldDiedWithError(WorldId world);

  // Get SkyParameters if the coordinate is a valid world, and empty
  // SkyParameters otherwise.
  SkyParameters celestialSkyParameters(CelestialCoordinate const& coordinate) const;

  mutable RecursiveMutex m_mainLock;
  
  bool m_isLocal;

  String m_storageDirectory;
  ByteArray m_assetsDigest;
  Maybe<LockFile> m_storageDirectoryLock;
  StringMap<StringList> m_speciesShips;
  std::shared_ptr<CelestialMasterDatabase> m_celestialDatabase;
  std::shared_ptr<Clock> m_universeClock;
  std::shared_ptr<UniverseSettings> m_universeSettings;
  WorkerPool m_workerPool;

  std::int64_t m_storageTriggerDeadline;
  std::int64_t m_clearBrokenWorldsDeadline;
  std::int64_t m_lastClockUpdateSent;
  std::atomic<bool> m_stop;
  std::atomic<TcpState> m_tcpState;

  mutable ReadersWriterMutex m_clientsLock;
  unsigned m_maxPlayers;
  IdMap<ConnectionId, std::shared_ptr<ServerClientContext>> m_clients;

  std::shared_ptr<std::atomic<bool>> m_pause;
  bool m_secureWarps;
  Map<WorldId, Maybe<WorldServerPromise>> m_worlds;
  Map<InstanceWorldId, std::pair<std::int64_t, std::int64_t>> m_tempWorldIndex;
  Map<Vec3I, std::shared_ptr<SystemWorldServerThread>> m_systemWorlds;
  std::shared_ptr<UniverseConnectionServer> m_connectionServer;

  RecursiveMutex m_connectionAcceptThreadsMutex;
  List<ThreadFunction<void>> m_connectionAcceptThreads;
  LinkedList<std::pair<UniverseConnection, std::int64_t>> m_deadConnections;

  std::shared_ptr<ChatProcessor> m_chatProcessor;
  std::shared_ptr<CommandProcessor> m_commandProcessor;
  std::shared_ptr<TeamManager> m_teamManager;

  HashMap<ConnectionId, std::pair<WarpAction, bool>> m_pendingPlayerWarps;
  HashMap<ConnectionId, HashMap<ClientSubWorldId, WorldId>> m_pendingSubWorlds;
  HashMap<ConnectionId, std::pair<std::tuple<Vec3I, SystemLocation, Json>, Maybe<double>>> m_queuedFlights;
  HashMap<ConnectionId, std::tuple<Vec3I, SystemLocation, Json>> m_pendingFlights;
  HashMap<ConnectionId, CelestialCoordinate> m_pendingArrivals;
  HashMap<ConnectionId, String> m_pendingDisconnections;
  HashMap<ConnectionId, List<WorkerPoolPromise<CelestialResponse>>> m_pendingCelestialRequests;
  List<std::pair<WorldId, UniverseFlagAction>> m_pendingFlagActions;
  HashMap<ConnectionId, List<std::tuple<String, ChatSendMode, JsonObject>>> m_pendingChat;
  Maybe<WorkerPoolPromise<CelestialCoordinate>> m_nextRandomizedStarterWorld;
  Map<WorldId, List<WorldServerThread::Message>> m_pendingWorldMessages;

  List<TimeoutBan> m_tempBans;

  std::shared_ptr<LuaRoot> m_luaRoot;

  typedef LuaMessageHandlingComponent<LuaUpdatableComponent<LuaBaseComponent>> ScriptComponent;
  StringMap<std::shared_ptr<UniverseServer::ScriptComponent>> m_scriptContexts;

  mutable RecursiveMutex m_messageLock;
  List<Universe::Message> m_universeMessages;
  
  HashMap<Uuid, std::pair<ConnectionId, MVariant<ConnectionId, RpcPromiseKeeper<Json>>>> m_universeMessageResponses;
  HashMap<Uuid, std::pair<ConnectionId, RpcPromise<Json>>> m_universeMessagePromises;
};

}


export module star.universe_server;

export namespace Star {
  using ::Star::Clock;
  using ::Star::File;
  using ::Star::Player;
  using ::Star::ChatProcessor;
  using ::Star::CommandProcessor;
  using ::Star::TeamManager;
  using ::Star::UniverseServer;
  using ::Star::WorldTemplate;
  using ::Star::CelestialMasterDatabase;
  using ::Star::WorldServer;
  using ::Star::UniverseSettings;
  using ::Star::ServerClientContext;
  using ::Star::UniverseServerExceptionTag;
  using ::Star::UniverseServerException;
}


#pragma once

#include "StarAssets.hpp"
#include "StarBiomeDatabase.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarConfiguration.hpp"
#include "StarEntityFactory.hpp"
#include "StarGameTypes.hpp"
#include "StarIdMap.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarItemDatabase.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarLockFile.hpp"
#include "StarLuaRoot.hpp"
#include "StarMaterialDatabase.hpp"
#include "StarMonsterDatabase.hpp"
#include "StarNameGenerator.hpp"
#include "StarNpcDatabase.hpp"
#include "StarPlantDatabase.hpp"
#include "StarProjectileDatabase.hpp"
#include "StarServerClientContext.hpp"
#include "StarSpawnTypeDatabase.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarStagehandDatabase.hpp"
#include "StarSystemWorldServerThread.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarTreasure.hpp"
#include "StarUniverseConnection.hpp"
#include "StarUniverseSettings.hpp"
#include "StarVehicleDatabase.hpp"
#include "StarVersioningDatabase.hpp"
#include "StarWorkerPool.hpp"
#include "StarWorldServerThread.hpp"

namespace Star {

class ItemDatabase;
using ItemDatabaseConstPtr = SharedPtr<ItemDatabase const>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class VehicleDatabase;
using VehicleDatabaseConstPtr = SharedPtr<VehicleDatabase const>;
class DungeonDefinitions;
using DungeonDefinitionsConstPtr = SharedPtr<DungeonDefinitions const>;
class BehaviorDatabase;
using BehaviorDatabaseConstPtr = SharedPtr<BehaviorDatabase const>;
class Clock;
class File;
class Player;
using PlayerPtr = SharedPtr<Player>;
class ChatProcessor;
using ChatProcessorPtr = SharedPtr<ChatProcessor>;
class CommandProcessor;
using CommandProcessorPtr = SharedPtr<CommandProcessor>;
class TeamManager;
using TeamManagerPtr = SharedPtr<TeamManager>;
class UniverseServer;
using UniverseServerPtr = SharedPtr<UniverseServer>;
class WorldTemplate;
class WorldServer;
class UniverseSettings;

struct UniverseServerExceptionTag {
  static constexpr char const* typeName = "UniverseServerException";
};
using UniverseServerException = TypedException<StarException, UniverseServerExceptionTag>;

// Manages all running worlds, listens for new client connections and marshals
// between all the different worlds and all the different client connections
// and routes packets between them.
class UniverseServer : public Thread {
public:
  UniverseServer(String const& storageDir,
                 AssetsConstPtr assets,
                 ConfigurationPtr configuration,
                 MaterialDatabaseConstPtr materialDatabase,
                 ImageMetadataDatabaseConstPtr imageMetadataDatabase,
                 ItemDatabaseConstPtr itemDatabase,
                 ObjectDatabaseConstPtr objectDatabase,
                 ProjectileDatabaseConstPtr projectileDatabase,
                 PlantDatabaseConstPtr plantDatabase,
                 TreasureDatabaseConstPtr treasureDatabase,
                 NpcDatabaseConstPtr npcDatabase,
                 MonsterDatabaseConstPtr monsterDatabase,
                 SpawnTypeDatabaseConstPtr spawnTypeDatabase,
                 StagehandDatabaseConstPtr stagehandDatabase,
                 VehicleDatabaseConstPtr vehicleDatabase,
                 SpeciesDatabaseConstPtr speciesDatabase,
                 EntityFactoryConstPtr entityFactory,
                 LiquidsDatabaseConstPtr liquidsDatabase,
                 TerrainDatabaseConstPtr terrainDatabase,
                 BiomeDatabaseConstPtr biomeDatabase,
                 PatternedNameGeneratorConstPtr nameGenerator,
                 VersioningDatabaseConstPtr versioningDatabase,
                 FunctionDatabaseConstPtr functionDatabase,
                 EffectSourceDatabaseConstPtr effectSourceDatabase,
                 ParticleDatabaseConstPtr particleDatabase,
                 TechDatabaseConstPtr techDatabase,
                 StatusEffectDatabaseConstPtr statusEffectDatabase,
                 DungeonDefinitionsConstPtr dungeonDefinitions,
                 BehaviorDatabaseConstPtr behaviorDatabase,
                 LuaRootServices luaRootServices,
                 function<void()> reloadRoot);
  ~UniverseServer();

  // If enabled, will listen on the configured server port for incoming
  // connections.
  void setListeningTcp(bool listenTcp);

  // Connects an arbitrary UniverseConnection to this server
  void addClient(UniverseConnection remoteConnection);
  // Constructs an in-process connection to a UniverseServer for a
  // UniverseClient, and returns the other side of the connection.
  [[nodiscard]] UniverseConnection addLocalClient();

  // Signals the UniverseServer to stop and then joins the thread.
  void stop();

  void setPause(bool pause);
  void setTimescale(float timescale);
  void setTickRate(float tickRate);

  [[nodiscard]] List<WorldId> activeWorlds() const;
  [[nodiscard]] bool isWorldActive(WorldId const& worldId) const;

  [[nodiscard]] List<ConnectionId> clientIds() const;
  [[nodiscard]] List<pair<ConnectionId, int64_t>> clientIdsAndCreationTime() const;
  [[nodiscard]] size_t numberOfClients() const;
  [[nodiscard]] uint32_t maxClients() const;
  [[nodiscard]] bool isConnectedClient(ConnectionId clientId) const;

  [[nodiscard]] String clientDescriptor(ConnectionId clientId) const;

  [[nodiscard]] String clientNick(ConnectionId clientId) const;
  [[nodiscard]] Maybe<ConnectionId> findNick(String const& nick) const;

  [[nodiscard]] Maybe<Uuid> uuidForClient(ConnectionId clientId) const;
  [[nodiscard]] Maybe<ConnectionId> clientForUuid(Uuid const& uuid) const;

  void adminBroadcast(String const& text);
  void adminWhisper(ConnectionId clientId, String const& text);
  [[nodiscard]] String adminCommand(String text);

  [[nodiscard]] bool isAdmin(ConnectionId clientId) const;
  [[nodiscard]] bool canBecomeAdmin(ConnectionId clientId) const;
  void setAdmin(ConnectionId clientId, bool admin);

  [[nodiscard]] bool isLocal(ConnectionId clientId) const;

  [[nodiscard]] bool isPvp(ConnectionId clientId) const;
  void setPvp(ConnectionId clientId, bool pvp);

  [[nodiscard]] RpcThreadPromise<Json> sendWorldMessage(WorldId const& worldId, String const& message, JsonArray const& args = {});

  void clientWarpPlayer(ConnectionId clientId, WarpAction action, bool deploy = false);
  void clientFlyShip(ConnectionId clientId, Vec3I const& system, SystemLocation const& location, Json const& settings = {});
  [[nodiscard]] WorldId clientWorld(ConnectionId clientId) const;
  [[nodiscard]] CelestialCoordinate clientShipCoordinate(ConnectionId clientId) const;

  [[nodiscard]] ClockPtr universeClock() const;
  [[nodiscard]] UniverseSettingsPtr universeSettings() const;

  [[nodiscard]] CelestialDatabase& celestialDatabase();

  // If the client exists and is in a valid connection state, executes the
  // given function on the client world and player object in a thread safe way.
  // Returns true if function was called, false if client was not found or in
  // an invalid connection state.
  [[nodiscard]] bool executeForClient(ConnectionId clientId, function<void(WorldServer*, PlayerPtr)> action);
  void disconnectClient(ConnectionId clientId, String const& reason);
  void banUser(ConnectionId clientId, String const& reason, pair<bool, bool> banType, Maybe<int> timeout);
  [[nodiscard]] bool unbanIp(String const& addressString);
  [[nodiscard]] bool unbanUuid(String const& uuidString);

  [[nodiscard]] bool updatePlanetType(CelestialCoordinate const& coordinate, String const& newType, String const& weatherBiome);

  [[nodiscard]] bool setWeather(CelestialCoordinate const& coordinate, String const& weatherName, bool force = false);

  [[nodiscard]] StringList weatherList(CelestialCoordinate const& coordinate);

  [[nodiscard]] bool sendPacket(ConnectionId clientId, PacketPtr packet);

protected:
  virtual void run();

private:
  [[nodiscard]] WorldServerServices worldServerServices() const;

  struct TimeoutBan {
    int64_t banExpiry;
    String reason;
    Maybe<HostAddress> ip;
    Maybe<Uuid> uuid;
  };

  enum class TcpState : uint8_t { No,
                                  Yes,
                                  Fuck };

  void processUniverseFlags();
  void sendPendingChat();
  void updateTeams();
  void updateShips();
  void sendClockUpdates();
  void sendClientContextUpdate(ServerClientContextPtr clientContext);
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
  [[nodiscard]] Maybe<CelestialCoordinate> nextStarterWorld();

  void loadTempWorldIndex();
  void saveTempWorldIndex();
  [[nodiscard]] String tempWorldFile(InstanceWorldId const& worldId) const;

  [[nodiscard]] Maybe<String> isBannedUser(Maybe<HostAddress> hostAddress, Uuid playerUuid) const;
  void doTempBan(ConnectionId clientId, String const& reason, pair<bool, bool> banType, int timeout);
  void doPermBan(ConnectionId clientId, String const& reason, pair<bool, bool> banType);
  void removeTimedBan();

  void addCelestialRequests(ConnectionId clientId, List<CelestialRequest> requests);

  void worldUpdated(WorldServerThread* worldServer);
  void systemWorldUpdated(SystemWorldServerThread* systemWorldServer);
  void packetsReceived(UniverseConnectionServer* connectionServer, ConnectionId clientId, List<PacketPtr> packets);

  void acceptConnection(UniverseConnection connection, Maybe<HostAddress> remoteAddress);

  // Main lock and clients read lock must be held when calling
  [[nodiscard]] WarpToWorld resolveWarpAction(WarpAction warpAction, ConnectionId clientId, bool deploy) const;
  [[nodiscard]] bool canWarpToShip(ConnectionId clientId, Uuid const& targetShipUuid) const;

  void doDisconnection(ConnectionId clientId, String const& reason);

  // Clients read lock must be held when calling
  [[nodiscard]] Maybe<ConnectionId> getClientForUuid(Uuid const& uuid) const;

  // Get the world only if it is already loaded, Main lock must be held when
  // calling.
  [[nodiscard]] WorldServerThreadPtr getWorld(WorldId const& worldId);

  // If the world is not created, block and load it, otherwise just return the
  // loaded world.  Main lock and Clients read lock must be held when calling.
  [[nodiscard]] WorldServerThreadPtr createWorld(WorldId const& worldId);

  // Trigger off-thread world creation, returns a value when the creation is
  // finished, either successfully or with an error.  Main lock and Clients
  // read lock must be held when calling.
  [[nodiscard]] Maybe<WorldServerThreadPtr> triggerWorldCreation(WorldId const& worldId);

  // Main lock and clients read lock must be held when calling world promise
  // generators
  [[nodiscard]] Maybe<WorkerPoolPromise<WorldServerThreadPtr>> makeWorldPromise(WorldId const& worldId);
  [[nodiscard]] Maybe<WorkerPoolPromise<WorldServerThreadPtr>> shipWorldPromise(ClientShipWorldId const& uuid);
  [[nodiscard]] Maybe<WorkerPoolPromise<WorldServerThreadPtr>> celestialWorldPromise(CelestialWorldId const& coordinate);
  [[nodiscard]] Maybe<WorkerPoolPromise<WorldServerThreadPtr>> instanceWorldPromise(InstanceWorldId const& instanceWorld);

  // If the system world is not created, initialize it, otherwise return the
  // already initialized one
  [[nodiscard]] SystemWorldServerThreadPtr createSystemWorld(Vec3I const& location);

  [[nodiscard]] bool instanceWorldStoredOrActive(InstanceWorldId const& worldId) const;

  // Signal that a world either failed to load, or died due to an exception,
  // kicks clients if that world is a ship world.  Main lock and clients read
  // lock must be held when calling.
  void worldDiedWithError(WorldId world);

  // Get SkyParameters if the coordinate is a valid world, and empty
  // SkyParameters otherwise.
  [[nodiscard]] SkyParameters celestialSkyParameters(CelestialCoordinate const& coordinate) const;

  mutable RecursiveMutex m_mainLock;

  String m_storageDirectory;
  ByteArray m_assetsDigest;
  Maybe<LockFile> m_storageDirectoryLock;
  StringMap<StringList> m_speciesShips;
  CelestialMasterDatabasePtr m_celestialDatabase;
  ClockPtr m_universeClock;
  UniverseSettingsPtr m_universeSettings;
  AssetsConstPtr m_assets;
  ConfigurationPtr m_configuration;
  LuaRootServices m_luaRootServices;
  MaterialDatabaseConstPtr m_materialDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  EffectSourceDatabaseConstPtr m_effectSourceDatabase;
  ParticleDatabaseConstPtr m_particleDatabase;
  TechDatabaseConstPtr m_techDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
  DungeonDefinitionsConstPtr m_dungeonDefinitions;
  BehaviorDatabaseConstPtr m_behaviorDatabase;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  ProjectileDatabaseConstPtr m_projectileDatabase;
  PlantDatabaseConstPtr m_plantDatabase;
  TreasureDatabaseConstPtr m_treasureDatabase;
  NpcDatabaseConstPtr m_npcDatabase;
  MonsterDatabaseConstPtr m_monsterDatabase;
  SpawnTypeDatabaseConstPtr m_spawnTypeDatabase;
  StagehandDatabaseConstPtr m_stagehandDatabase;
  VehicleDatabaseConstPtr m_vehicleDatabase;
  SpeciesDatabaseConstPtr m_speciesDatabase;
  EntityFactoryConstPtr m_entityFactory;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  TerrainDatabaseConstPtr m_terrainDatabase;
  BiomeDatabaseConstPtr m_biomeDatabase;
  PatternedNameGeneratorConstPtr m_nameGenerator;
  VersioningDatabaseConstPtr m_versioningDatabase;
  FunctionDatabaseConstPtr m_functionDatabase;
  function<void()> m_reloadRoot;
  WorkerPool m_workerPool;

  int64_t m_storageTriggerDeadline = 0;
  int64_t m_clearBrokenWorldsDeadline = 0;
  int64_t m_lastClockUpdateSent = 0;
  atomic<bool> m_stop = false;
  atomic<TcpState> m_tcpState = TcpState::No;

  mutable ReadersWriterMutex m_clientsLock;
  unsigned m_maxPlayers;
  IdMap<ConnectionId, ServerClientContextPtr> m_clients;

  shared_ptr<atomic<bool>> m_pause = make_shared<atomic<bool>>(false);
  bool m_secureWarps;
  Map<WorldId, Maybe<WorkerPoolPromise<WorldServerThreadPtr>>> m_worlds;
  Map<InstanceWorldId, pair<int64_t, int64_t>> m_tempWorldIndex;
  Map<Vec3I, SystemWorldServerThreadPtr> m_systemWorlds;
  UniverseConnectionServerPtr m_connectionServer;

  RecursiveMutex m_connectionAcceptThreadsMutex;
  List<ThreadFunction<void>> m_connectionAcceptThreads;
  LinkedList<pair<UniverseConnection, int64_t>> m_deadConnections;

  ChatProcessorPtr m_chatProcessor;
  CommandProcessorPtr m_commandProcessor;
  TeamManagerPtr m_teamManager;

  HashMap<ConnectionId, pair<WarpAction, bool>> m_pendingPlayerWarps;
  HashMap<ConnectionId, pair<tuple<Vec3I, SystemLocation, Json>, Maybe<double>>> m_queuedFlights;
  HashMap<ConnectionId, tuple<Vec3I, SystemLocation, Json>> m_pendingFlights;
  HashMap<ConnectionId, CelestialCoordinate> m_pendingArrivals;
  HashMap<ConnectionId, String> m_pendingDisconnections;
  HashMap<ConnectionId, List<WorkerPoolPromise<CelestialResponse>>> m_pendingCelestialRequests;
  List<pair<WorldId, UniverseFlagAction>> m_pendingFlagActions;
  HashMap<ConnectionId, List<tuple<String, ChatSendMode, JsonObject>>> m_pendingChat;
  Maybe<WorkerPoolPromise<CelestialCoordinate>> m_nextRandomizedStarterWorld;
  Map<WorldId, List<WorldServerThread::Message>> m_pendingWorldMessages;

  List<TimeoutBan> m_tempBans;

  LuaRootPtr m_luaRoot;

  using ScriptComponent = LuaUpdatableComponent<LuaBaseComponent>;
  using ScriptComponentPtr = shared_ptr<ScriptComponent>;
  StringMap<ScriptComponentPtr> m_scriptContexts;
};

}// namespace Star

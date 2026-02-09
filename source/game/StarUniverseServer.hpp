#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarCelestialDatabase.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarIdMap.hpp"
#include "StarLockFile.hpp"
#include "StarServerClientContext.hpp"
#include "StarSystemWorldServerThread.hpp"
#include "StarUniverseConnection.hpp"
#include "StarUniverseSettings.hpp"
#include "StarWorkerPool.hpp"
#include "StarWorldServerThread.hpp"

import std;

namespace Star {

class ChatProcessor;
class CommandProcessor;
class TeamManager;

using UniverseServerException = ExceptionDerived<"UniverseServerException">;

// Manages all running worlds, listens for new client connections and marshalls
// between all the different worlds and all the different client connections
// and routes packets between them.
class UniverseServer : public Thread {
public:
  UniverseServer(String const& storageDir);
  ~UniverseServer() override;

  // If enabled, will listen on the configured server port for incoming
  // connections.
  void setListeningTcp(bool listenTcp);

  // Connects an arbitrary UniverseConnection to this server
  void addClient(UniverseConnection remoteConnection);
  // Constructs an in-process connection to a UniverseServer for a
  // UniverseClient, and returns the other side of the connection.
  auto addLocalClient() -> UniverseConnection;

  // Signals the UniverseServer to stop and then joins the thread.
  void stop();

  void setPause(bool pause);
  void setTimescale(float timescale);
  void setTickRate(float tickRate);

  auto activeWorlds() const -> List<WorldId>;
  auto isWorldActive(WorldId const& worldId) const -> bool;

  auto clientIds() const -> List<ConnectionId>;
  auto clientIdsAndCreationTime() const -> List<std::pair<ConnectionId, std::int64_t>>;
  auto numberOfClients() const -> size_t;
  auto maxClients() const -> std::uint32_t;
  auto isConnectedClient(ConnectionId clientId) const -> bool;

  auto clientDescriptor(ConnectionId clientId) const -> String;

  auto clientNick(ConnectionId clientId) const -> String;
  auto findNick(String const& nick) const -> std::optional<ConnectionId>;

  auto uuidForClient(ConnectionId clientId) const -> std::optional<Uuid>;
  auto clientForUuid(Uuid const& uuid) const -> std::optional<ConnectionId>;

  void adminBroadcast(String const& text);
  void adminWhisper(ConnectionId clientId, String const& text);
  auto adminCommand(String text) -> String;

  auto isAdmin(ConnectionId clientId) const -> bool;
  auto canBecomeAdmin(ConnectionId clientId) const -> bool;
  void setAdmin(ConnectionId clientId, bool admin);

  auto isLocal(ConnectionId clientId) const -> bool;

  auto isPvp(ConnectionId clientId) const -> bool;
  void setPvp(ConnectionId clientId, bool pvp);

  auto sendWorldMessage(WorldId const& worldId, String const& message, JsonArray const& args = {}) -> RpcThreadPromise<Json>;

  void clientWarpPlayer(ConnectionId clientId, WarpAction action, bool deploy = false);
  void clientFlyShip(ConnectionId clientId, Vec3I const& system, SystemLocation const& location, Json const& settings = {});
  auto clientWorld(ConnectionId clientId) const -> WorldId;
  auto clientShipCoordinate(ConnectionId clientId) const -> CelestialCoordinate;

  auto universeClock() const -> Ptr<Clock>;
  auto universeSettings() const -> Ptr<UniverseSettings>;

  auto celestialDatabase() -> CelestialDatabase&;

  // If the client exists and is in a valid connection state, executes the
  // given function on the client world and player object in a thread safe way.
  // Returns true if function was called, false if client was not found or in
  // an invalid connection state.
  auto executeForClient(ConnectionId clientId, std::function<void(WorldServer*, Ptr<Player>)> action) -> bool;
  void disconnectClient(ConnectionId clientId, String const& reason);
  void banUser(ConnectionId clientId, String const& reason, std::pair<bool, bool> banType, std::optional<int> timeout);
  auto unbanIp(String const& addressString) -> bool;
  auto unbanUuid(String const& uuidString) -> bool;

  auto updatePlanetType(CelestialCoordinate const& coordinate, String const& newType, String const& weatherBiome) -> bool;

  auto setWeather(CelestialCoordinate const& coordinate, String const& weatherName, bool force = false) -> bool;

  auto weatherList(CelestialCoordinate const& coordinate) -> StringList;

  auto sendPacket(ConnectionId clientId, Ptr<Packet> packet) -> bool;

protected:
  void run() override;

private:
  struct TimeoutBan {
    std::int64_t banExpiry;
    String reason;
    std::optional<HostAddress> ip;
    std::optional<Uuid> uuid;
  };

  enum class TcpState : std::uint8_t { No,
                                       Yes,
                                       Fuck };

  void processUniverseFlags();
  void sendPendingChat();
  void updateTeams();
  void updateShips();
  void sendClockUpdates();
  void sendClientContextUpdate(Ptr<ServerClientContext> clientContext);
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
  auto nextStarterWorld() -> std::optional<CelestialCoordinate>;

  void loadTempWorldIndex();
  void saveTempWorldIndex();
  auto tempWorldFile(InstanceWorldId const& worldId) const -> String;

  auto isBannedUser(std::optional<HostAddress> hostAddress, Uuid playerUuid) const -> std::optional<String>;
  void doTempBan(ConnectionId clientId, String const& reason, std::pair<bool, bool> banType, int timeout);
  void doPermBan(ConnectionId clientId, String const& reason, std::pair<bool, bool> banType);
  void removeTimedBan();

  void addCelestialRequests(ConnectionId clientId, List<CelestialRequest> requests);

  void worldUpdated(WorldServerThread* worldServer);
  void systemWorldUpdated(SystemWorldServerThread* systemWorldServer);
  void packetsReceived(UniverseConnectionServer* connectionServer, ConnectionId clientId, List<Ptr<Packet>> packets);

  void acceptConnection(UniverseConnection connection, std::optional<HostAddress> remoteAddress);

  // Main lock and clients read lock must be held when calling
  auto resolveWarpAction(WarpAction warpAction, ConnectionId clientId, bool deploy) const -> WarpToWorld;

  void doDisconnection(ConnectionId clientId, String const& reason);

  // Clients read lock must be held when calling
  auto getClientForUuid(Uuid const& uuid) const -> std::optional<ConnectionId>;

  // Get the world only if it is already loaded, Main lock must be held when
  // calling.
  auto getWorld(WorldId const& worldId) -> Ptr<WorldServerThread>;

  // If the world is not created, block and load it, otherwise just return the
  // loaded world.  Main lock and Clients read lock must be held when calling.
  auto createWorld(WorldId const& worldId) -> Ptr<WorldServerThread>;

  // Trigger off-thread world creation, returns a value when the creation is
  // finished, either successfully or with an error.  Main lock and Clients
  // read lock must be held when calling.
  auto triggerWorldCreation(WorldId const& worldId) -> std::optional<Ptr<WorldServerThread>>;

  // Main lock and clients read lock must be held when calling world promise
  // generators
  auto makeWorldPromise(WorldId const& worldId) -> std::optional<WorkerPoolPromise<Ptr<WorldServerThread>>>;
  auto shipWorldPromise(ClientShipWorldId const& uuid) -> std::optional<WorkerPoolPromise<Ptr<WorldServerThread>>>;
  auto celestialWorldPromise(CelestialWorldId const& coordinate) -> std::optional<WorkerPoolPromise<Ptr<WorldServerThread>>>;
  auto instanceWorldPromise(InstanceWorldId const& instanceWorld) -> std::optional<WorkerPoolPromise<Ptr<WorldServerThread>>>;

  // If the system world is not created, initialize it, otherwise return the
  // already initialized one
  auto createSystemWorld(Vec3I const& location) -> Ptr<SystemWorldServerThread>;

  auto instanceWorldStoredOrActive(InstanceWorldId const& worldId) const -> bool;

  // Signal that a world either failed to load, or died due to an exception,
  // kicks clients if that world is a ship world.  Main lock and clients read
  // lock must be held when calling.
  void worldDiedWithError(WorldId world);

  // Get SkyParameters if the coordinate is a valid world, and empty
  // SkyParameters otherwise.
  auto celestialSkyParameters(CelestialCoordinate const& coordinate) const -> SkyParameters;

  mutable RecursiveMutex m_mainLock;

  String m_storageDirectory;
  ByteArray m_assetsDigest;
  std::optional<LockFile> m_storageDirectoryLock;
  StringMap<StringList> m_speciesShips;
  Ptr<CelestialMasterDatabase> m_celestialDatabase;
  Ptr<Clock> m_universeClock;
  Ptr<UniverseSettings> m_universeSettings;
  WorkerPool m_workerPool;

  std::int64_t m_storageTriggerDeadline;
  std::int64_t m_clearBrokenWorldsDeadline;
  std::int64_t m_lastClockUpdateSent;
  std::atomic<bool> m_stop;
  std::atomic<TcpState> m_tcpState;

  mutable ReadersWriterMutex m_clientsLock;
  unsigned m_maxPlayers;
  IdMap<ConnectionId, Ptr<ServerClientContext>> m_clients;

  std::shared_ptr<std::atomic<bool>> m_pause;
  Map<WorldId, std::optional<WorkerPoolPromise<Ptr<WorldServerThread>>>> m_worlds;
  Map<InstanceWorldId, std::pair<std::int64_t, std::int64_t>> m_tempWorldIndex;
  Map<Vec3I, Ptr<SystemWorldServerThread>> m_systemWorlds;
  Ptr<UniverseConnectionServer> m_connectionServer;

  RecursiveMutex m_connectionAcceptThreadsMutex;
  List<ThreadFunction<void>> m_connectionAcceptThreads;
  LinkedList<std::pair<UniverseConnection, std::int64_t>> m_deadConnections;

  Ptr<ChatProcessor> m_chatProcessor;
  Ptr<CommandProcessor> m_commandProcessor;
  Ptr<TeamManager> m_teamManager;

  HashMap<ConnectionId, std::pair<WarpAction, bool>> m_pendingPlayerWarps;
  HashMap<ConnectionId, std::pair<std::tuple<Vec3I, SystemLocation, Json>, std::optional<double>>> m_queuedFlights;
  HashMap<ConnectionId, std::tuple<Vec3I, SystemLocation, Json>> m_pendingFlights;
  HashMap<ConnectionId, CelestialCoordinate> m_pendingArrivals;
  HashMap<ConnectionId, String> m_pendingDisconnections;
  HashMap<ConnectionId, List<WorkerPoolPromise<CelestialResponse>>> m_pendingCelestialRequests;
  List<std::pair<WorldId, UniverseFlagAction>> m_pendingFlagActions;
  HashMap<ConnectionId, List<std::tuple<String, ChatSendMode, JsonObject>>> m_pendingChat;
  std::optional<WorkerPoolPromise<CelestialCoordinate>> m_nextRandomizedStarterWorld;
  Map<WorldId, List<WorldServerThread::Message>> m_pendingWorldMessages;

  List<TimeoutBan> m_tempBans;

  Ptr<LuaRoot> m_luaRoot;

  using ScriptComponent = LuaUpdatableComponent<LuaBaseComponent>;
  StringMap<Ptr<ScriptComponent>> m_scriptContexts;
};

}// namespace Star

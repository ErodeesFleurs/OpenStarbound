#pragma once

#include "StarMaybe.hpp"
#include "StarHostAddress.hpp"
#include "StarGameTimers.hpp"
#include "StarCelestialParameters.hpp"
#include "StarChatTypes.hpp"
#include "StarWarping.hpp"
#include "StarAiTypes.hpp"
#include "StarSky.hpp"
#include "StarUniverseConnection.hpp"
#include "StarLuaComponents.hpp"

namespace Star {

class WorldTemplate;
using WorldTemplateConstPtr = SharedPtr<WorldTemplate const>;
class ClientContext;
using ClientContextPtr = SharedPtr<ClientContext>;
class Sky;
using SkyConstPtr = SharedPtr<Sky const>;
struct Packet;
using PacketPtr = SharedPtr<Packet>;
class WorldClient;
using WorldClientPtr = SharedPtr<WorldClient>;
class SystemWorldClient;
using SystemWorldClientPtr = SharedPtr<SystemWorldClient>;
class Player;
using PlayerPtr = SharedPtr<Player>;
class PlayerStorage;
using PlayerStoragePtr = SharedPtr<PlayerStorage>;
class Statistics;
using StatisticsPtr = SharedPtr<Statistics>;
class Clock;
using ClockPtr = SharedPtr<Clock>;
using ClockConstPtr = SharedPtr<Clock const>;
class CelestialLog;
using CelestialLogConstPtr = SharedPtr<CelestialLog const>;
class CelestialSlaveDatabase;
using CelestialSlaveDatabasePtr = SharedPtr<CelestialSlaveDatabase>;
class CelestialDatabase;
using CelestialDatabasePtr = SharedPtr<CelestialDatabase>;
class JsonRpcInterface;
using JsonRpcInterfacePtr = SharedPtr<JsonRpcInterface>;
class TeamClient;
using TeamClientPtr = SharedPtr<TeamClient>;
class QuestManager;
using QuestManagerPtr = SharedPtr<QuestManager>;
class UniverseClient;
using UniverseClientPtr = SharedPtr<UniverseClient>;
class LuaRoot;
using LuaRootPtr = SharedPtr<LuaRoot>;

class UniverseClient {
public:
  UniverseClient(PlayerStoragePtr playerStorage, StatisticsPtr statistics);
  ~UniverseClient();

  void setMainPlayer(PlayerPtr player);
  PlayerPtr mainPlayer() const;

  // Returns error if connection failed
  Maybe<String> connect(UniverseConnection connection, bool allowAssetsMismatch, String const& account = "", String const& password = "", bool const& forceLegacy = false);
  bool isConnected() const;
  void disconnect();
  Maybe<String> disconnectReason() const;

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

  CelestialDatabasePtr celestialDatabase() const;

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

  using Callback = std::function<void()>;
  using ReloadPlayerCallback = std::function<void(bool)>;
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

  bool paused() const;

private:
  struct ServerInfo {
    uint16_t players;
    uint16_t maxPlayers;
  };

  void setPause(bool pause);

  void handlePackets(List<PacketPtr> const& packets);
  void reset();

  PlayerStoragePtr m_playerStorage;
  StatisticsPtr m_statistics;
  PlayerPtr m_mainPlayer;

  bool m_pause;
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
  bool m_respawning;
  GameTimer m_respawnTimer;

  int64_t m_storageTriggerDeadline;

  List<ChatReceivedMessage> m_pendingMessages;

  Maybe<String> m_disconnectReason;

  LuaRootPtr m_luaRoot;

  using ScriptComponent = LuaUpdatableComponent<LuaBaseComponent>;
  using ScriptComponentPtr = shared_ptr<ScriptComponent>;
  StringMap<ScriptComponentPtr> m_scriptContexts;

  ReloadPlayerCallback m_playerReloadPreCallback;
  ReloadPlayerCallback m_playerReloadCallback;
};

}

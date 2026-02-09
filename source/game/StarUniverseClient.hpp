#pragma once

#include "StarChatTypes.hpp"
#include "StarConfig.hpp"
#include "StarGameTimers.hpp"
#include "StarLuaComponents.hpp"
#include "StarSky.hpp"
#include "StarUniverseConnection.hpp"
#include "StarWarping.hpp"

import std;

namespace Star {

class PlayerStorage;
class Statistics;
class Player;
class SystemWorldClient;
class WorldTemplate;
class CelestialLog;
class JsonRpcInterface;
class ClientContext;
class TeamClient;
class QuestManager;
class CelestialSlaveDatabase;

class UniverseClient {
public:
  UniverseClient(Ptr<PlayerStorage> playerStorage, Ptr<Statistics> statistics);
  ~UniverseClient();

  void setMainPlayer(Ptr<Player> player);
  auto mainPlayer() const -> Ptr<Player>;

  // Returns error if connection failed
  auto connect(UniverseConnection connection, bool allowAssetsMismatch, String const& account = "", String const& password = "", bool const& forceLegacy = false) -> std::optional<String>;
  auto isConnected() const -> bool;
  void disconnect();
  auto disconnectReason() const -> std::optional<String>;

  // WorldClient may be null if the UniverseClient is not connected.
  auto worldClient() const -> Ptr<WorldClient>;
  auto systemWorldClient() const -> Ptr<SystemWorldClient>;

  // Updates internal world client in addition to handling universe level
  // commands.
  void update(float dt);

  auto beamUpRule() const -> std::optional<BeamUpRule>;
  auto canBeamUp() const -> bool;
  auto canBeamDown(bool deploy = false) const -> bool;
  auto canBeamToTeamShip() const -> bool;
  auto canTeleport() const -> bool;

  void warpPlayer(WarpAction const& warpAction, bool animate = true, String const& animationType = "default", bool deploy = false);
  void flyShip(Vec3I const& system, SystemLocation const& destination, Json const& settings = {});

  auto celestialDatabase() const -> Ptr<CelestialDatabase>;

  auto shipCoordinate() const -> CelestialCoordinate;

  auto playerOnOwnShip() const -> bool;
  auto playerIsOriginal() const -> bool;

  auto playerWorld() const -> WorldId;
  auto isAdmin() const -> bool;
  // If the player is in a multi person team returns the team uuid, or if the
  // player is by themselves returns the player uuid.
  auto teamUuid() const -> Uuid;

  auto currentTemplate() const -> ConstPtr<WorldTemplate>;
  auto currentSky() const -> ConstPtr<Sky>;
  auto flying() const -> bool;

  void sendChat(String const& text, ChatSendMode sendMode, std::optional<bool> speak = {}, std::optional<JsonObject> data = {});
  auto pullChatMessages() -> List<ChatReceivedMessage>;

  auto players() -> std::uint16_t;
  auto maxPlayers() -> std::uint16_t;

  void setLuaCallbacks(String const& groupName, LuaCallbacks const& callbacks);
  void restartLua();
  void startLuaScripts();
  void stopLua();
  auto luaRoot() -> Ptr<LuaRoot>;

  auto reloadPlayer(Json const& data, Uuid const& uuid, bool resetInterfaces = false, bool showIndicator = false) -> bool;
  auto switchPlayer(Uuid const& uuid) -> bool;
  auto switchPlayer(size_t index) -> bool;
  auto switchPlayer(String const& name) -> bool;

  using Callback = std::function<void()>;
  using ReloadPlayerCallback = std::function<void(bool)>;
  auto playerReloadPreCallback() -> ReloadPlayerCallback&;
  auto playerReloadCallback() -> ReloadPlayerCallback&;

  auto universeClock() const -> ConstPtr<Clock>;
  auto celestialLog() const -> ConstPtr<CelestialLog>;
  auto rpcInterface() const -> Ptr<JsonRpcInterface>;
  auto clientContext() const -> Ptr<ClientContext>;
  auto teamClient() const -> Ptr<TeamClient>;
  auto questManager() const -> Ptr<QuestManager>;
  auto playerStorage() const -> Ptr<PlayerStorage>;
  auto statistics() const -> Ptr<Statistics>;

  auto paused() const -> bool;

private:
  struct ServerInfo {
    std::uint16_t players;
    std::uint16_t maxPlayers;
  };

  void setPause(bool pause);

  void handlePackets(List<Ptr<Packet>> const& packets);
  void reset();

  Ptr<PlayerStorage> m_playerStorage;
  Ptr<Statistics> m_statistics;
  Ptr<Player> m_mainPlayer;

  bool m_pause;
  Ptr<Clock> m_universeClock;
  Ptr<WorldClient> m_worldClient;
  Ptr<SystemWorldClient> m_systemWorldClient;
  std::optional<UniverseConnection> m_connection;
  std::optional<ServerInfo> m_serverInfo;

  Ptr<CelestialSlaveDatabase> m_celestialDatabase;
  Ptr<ClientContext> m_clientContext;
  Ptr<TeamClient> m_teamClient;

  Ptr<QuestManager> m_questManager;

  WarpAction m_pendingWarp;
  GameTimer m_warpDelay;
  std::optional<GameTimer> m_warpCinemaCancelTimer;

  std::optional<WarpAction> m_warping;
  bool m_respawning;
  GameTimer m_respawnTimer;

  std::int64_t m_storageTriggerDeadline;

  List<ChatReceivedMessage> m_pendingMessages;

  std::optional<String> m_disconnectReason;

  Ptr<LuaRoot> m_luaRoot;

  using ScriptComponent = LuaUpdatableComponent<LuaBaseComponent>;
  StringMap<Ptr<ScriptComponent>> m_scriptContexts;

  ReloadPlayerCallback m_playerReloadPreCallback;
  ReloadPlayerCallback m_playerReloadCallback;
};

}// namespace Star

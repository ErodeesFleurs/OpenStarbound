#pragma once

#include "StarConfig.hpp"
#include "StarDamageTypes.hpp"
#include "StarGameTypes.hpp"
#include "StarHostAddress.hpp"
#include "StarJsonRpc.hpp"
#include "StarNetElementSystem.hpp"
#include "StarPlayerTypes.hpp"
#include "StarSystemWorld.hpp"
#include "StarThread.hpp"
#include "StarUuid.hpp"
#include "StarWorldStorage.hpp"

import std;

namespace Star {

class WorldServerThread;
class SystemWorldServerThread;

class ServerClientContext {
public:
  ServerClientContext(ConnectionId clientId, std::optional<HostAddress> remoteAddress, NetCompatibilityRules netRules, Uuid playerUuid,
                      String playerName, String shipSpecies, bool canBecomeAdmin, WorldChunks initialShipChunks);

  auto clientId() const -> ConnectionId;
  auto remoteAddress() const -> std::optional<HostAddress> const&;
  auto playerUuid() const -> Uuid const&;
  auto playerName() const -> String const&;
  auto shipSpecies() const -> String const&;
  auto canBecomeAdmin() const -> bool;
  auto netRules() const -> NetCompatibilityRules;
  auto descriptiveName() const -> String;

  // Register additional rpc methods from other server side services.
  void registerRpcHandlers(JsonRpcHandlers const& rpcHandlers);

  // The coordinate for the world which the *player's* ship is currently
  // orbiting, if it is currently orbiting a world.
  auto shipCoordinate() const -> CelestialCoordinate;
  void setShipCoordinate(CelestialCoordinate shipCoordinate);

  auto shipLocation() const -> SystemLocation;
  void setShipLocation(SystemLocation location);

  // Warp action and warp mode to the planet the player is currently orbiting
  // valid when the player is on any ship world orbiting a location
  auto orbitWarpAction() const -> std::optional<std::pair<WarpAction, WarpMode>>;
  void setOrbitWarpAction(std::optional<std::pair<WarpAction, WarpMode>> warpAction);

  auto isAdmin() const -> bool;
  void setAdmin(bool admin);

  auto team() const -> EntityDamageTeam;
  void setTeam(EntityDamageTeam team);

  auto shipUpgrades() const -> ShipUpgrades;
  void setShipUpgrades(ShipUpgrades shipUpgrades);
  void setShipSpecies(String shipSpecies);

  auto shipChunks() const -> WorldChunks;
  void updateShipChunks(WorldChunks newShipChunks);

  auto writeInitialState() const -> ByteArray;

  void readUpdate(ByteArray data);
  auto writeUpdate() -> ByteArray;

  void setPlayerWorld(Ptr<WorldServerThread> worldThread);
  auto playerWorld() const -> Ptr<WorldServerThread>;
  auto playerWorldId() const -> WorldId;
  void clearPlayerWorld();

  void setSystemWorld(Ptr<SystemWorldServerThread> systemWorldThread);
  auto systemWorld() const -> Ptr<SystemWorldServerThread>;
  void clearSystemWorld();

  auto playerReturnWarp() const -> WarpToWorld;
  void setPlayerReturnWarp(WarpToWorld warp);

  auto playerReviveWarp() const -> WarpToWorld;
  void setPlayerReviveWarp(WarpToWorld warp);

  // Store and load the data for this client that should be persisted on the
  // server, such as celestial log data, admin state, team, and current ship
  // location, and warp history.  Does not store ship data or ship upgrades.
  void loadServerData(Json const& store);
  auto storeServerData() -> Json;

  auto creationTime() const -> std::int64_t;

private:
  ConnectionId const m_clientId;
  std::optional<HostAddress> const m_remoteAddress;
  NetCompatibilityRules m_netRules;
  Uuid const m_playerUuid;
  String const m_playerName;
  String m_shipSpecies;
  bool const m_canBecomeAdmin;

  mutable RecursiveMutex m_mutex;

  WorldChunks m_shipChunks;
  WorldChunks m_shipChunksUpdate;

  SystemLocation m_shipSystemLocation;
  JsonRpc m_rpc;
  Ptr<WorldServerThread> m_worldThread;
  WarpToWorld m_returnWarp;
  WarpToWorld m_reviveWarp;

  Ptr<SystemWorldServerThread> m_systemWorldThread;

  NetElementTopGroup m_netGroup;
  std::uint64_t m_netVersion = 0;
  std::int64_t m_creationTime;

  NetElementData<std::optional<std::pair<WarpAction, WarpMode>>> m_orbitWarpActionNetState;
  NetElementData<WorldId> m_playerWorldIdNetState;
  NetElementBool m_isAdminNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementData<ShipUpgrades> m_shipUpgrades;
  NetElementData<CelestialCoordinate> m_shipCoordinate;
};

}// namespace Star

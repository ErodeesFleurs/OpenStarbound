#pragma once

#include "StarNetElementSystem.hpp"
#include "StarThread.hpp"
#include "StarUuid.hpp"
#include "StarJsonRpc.hpp"
#include "StarDamageTypes.hpp"
#include "StarGameTypes.hpp"
#include "StarHostAddress.hpp"
#include "StarClientContext.hpp"
#include "StarWorldStorage.hpp"
#include "StarSystemWorld.hpp"
#include "StarItemDatabase.hpp"

namespace Star {

class WorldServerThread;
using WorldServerThreadPtr = SharedPtr<WorldServerThread>;
class SystemWorldServerThread;
using SystemWorldServerThreadPtr = SharedPtr<SystemWorldServerThread>;
class ServerClientContext;
using ServerClientContextPtr = SharedPtr<ServerClientContext>;

class ServerClientContext {
public:
  ServerClientContext(ConnectionId clientId, Maybe<HostAddress> remoteAddress, NetCompatibilityRules netRules, Uuid playerUuid,
      String playerName, String shipSpecies, bool canBecomeAdmin, WorldChunks initialShipChunks, ItemDatabaseConstPtr itemDatabase);

  [[nodiscard]] ConnectionId clientId() const;
  [[nodiscard]] Maybe<HostAddress> const& remoteAddress() const;
  [[nodiscard]] Uuid const& playerUuid() const;
  [[nodiscard]] String const& playerName() const;
  [[nodiscard]] String const& shipSpecies() const;
  [[nodiscard]] bool canBecomeAdmin() const;
  [[nodiscard]] NetCompatibilityRules netRules() const;
  [[nodiscard]] String descriptiveName() const;

  // Register additional rpc methods from other server side services.
  void registerRpcHandlers(JsonRpcHandlers const& rpcHandlers);

  // The coordinate for the world which the *player's* ship is currently
  // orbiting, if it is currently orbiting a world.
  [[nodiscard]] CelestialCoordinate shipCoordinate() const;
  void setShipCoordinate(CelestialCoordinate shipCoordinate);

  [[nodiscard]] SystemLocation shipLocation() const;
  void setShipLocation(SystemLocation location);

  // Warp action and warp mode to the planet the player is currently orbiting
  // valid when the player is on any ship world orbiting a location
  [[nodiscard]] Maybe<pair<WarpAction, WarpMode>> orbitWarpAction() const;
  void setOrbitWarpAction(Maybe<pair<WarpAction, WarpMode>> warpAction);

  [[nodiscard]] bool isAdmin() const;
  void setAdmin(bool admin);

  [[nodiscard]] EntityDamageTeam team() const;
  void setTeam(EntityDamageTeam team);

  [[nodiscard]] ShipUpgrades shipUpgrades() const;
  void setShipUpgrades(ShipUpgrades shipUpgrades);
  void setShipSpecies(String shipSpecies);

  [[nodiscard]] WorldChunks shipChunks() const;
  void updateShipChunks(WorldChunks newShipChunks);

  [[nodiscard]] ByteArray writeInitialState() const;

  void readUpdate(ByteArray data);
  [[nodiscard]] ByteArray writeUpdate();

  void setPlayerWorld(WorldServerThreadPtr worldThread);
  [[nodiscard]] WorldServerThreadPtr playerWorld() const;
  [[nodiscard]] WorldId playerWorldId() const;
  void clearPlayerWorld();

  void setSystemWorld(SystemWorldServerThreadPtr systemWorldThread);
  [[nodiscard]] SystemWorldServerThreadPtr systemWorld() const;
  void clearSystemWorld();

  [[nodiscard]] WarpToWorld playerReturnWarp() const;
  void setPlayerReturnWarp(WarpToWorld warp);

  [[nodiscard]] WarpToWorld playerReviveWarp() const;
  void setPlayerReviveWarp(WarpToWorld warp);

  // Store and load the data for this client that should be persisted on the
  // server, such as celestial log data, admin state, team, and current ship
  // location, and warp history.  Does not store ship data or ship upgrades.
  void loadServerData(Json const& store);
  [[nodiscard]] Json storeServerData();

  [[nodiscard]] int64_t creationTime() const;

private:
  ConnectionId const m_clientId;
  Maybe<HostAddress> const m_remoteAddress;
  NetCompatibilityRules m_netRules;
  Uuid const m_playerUuid;
  String const m_playerName;
  String m_shipSpecies;
  bool const m_canBecomeAdmin;
  ItemDatabaseConstPtr m_itemDatabase;

  mutable RecursiveMutex m_mutex;

  WorldChunks m_shipChunks;
  WorldChunks m_shipChunksUpdate;

  SystemLocation m_shipSystemLocation;
  JsonRpc m_rpc;
  WorldServerThreadPtr m_worldThread;
  WarpToWorld m_returnWarp;
  WarpToWorld m_reviveWarp;

  SystemWorldServerThreadPtr m_systemWorldThread;

  NetElementTopGroup m_netGroup;
  uint64_t m_netVersion = 0;
  int64_t m_creationTime;

  NetElementData<Maybe<pair<WarpAction, WarpMode>>> m_orbitWarpActionNetState;
  NetElementData<WorldId> m_playerWorldIdNetState;
  NetElementBool m_isAdminNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementData<ShipUpgrades> m_shipUpgrades;
  NetElementData<CelestialCoordinate> m_shipCoordinate;
};

}

#pragma once

#include "StarNetElementSystem.hpp"
#include "StarJsonRpc.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarDamageTypes.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarWarping.hpp"
#include "StarWorldStorage.hpp"
#include "StarPlayerTypes.hpp"

namespace Star {

class ClientContext;
using ClientContextPtr = SharedPtr<ClientContext>;

class ClientContext {
public:
  ClientContext(Uuid serverUuid, Uuid playerUuid);

  [[nodiscard]] Uuid serverUuid() const;
  // The player Uuid can differ from the mainPlayer's Uuid
  //  if the player has swapped character - use this for ship saving.
  [[nodiscard]] Uuid playerUuid() const;

  // The coordinate for the world which the player's ship is currently
  // orbiting.
  [[nodiscard]] CelestialCoordinate shipCoordinate() const;

  [[nodiscard]] Maybe<pair<WarpAction, WarpMode>> orbitWarpAction() const;

  // The current world id of the player
  [[nodiscard]] WorldId playerWorldId() const;

  [[nodiscard]] bool isAdmin() const;
  [[nodiscard]] EntityDamageTeam team() const;

  [[nodiscard]] JsonRpcInterfacePtr rpcInterface() const;

  [[nodiscard]] WorldChunks newShipUpdates();
  [[nodiscard]] ShipUpgrades shipUpgrades() const;

  void readUpdate(ByteArray data, NetCompatibilityRules rules);
  [[nodiscard]] ByteArray writeUpdate(NetCompatibilityRules rules);

  void setConnectionId(ConnectionId connectionId);
  [[nodiscard]] ConnectionId connectionId() const;

  void setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules);
  [[nodiscard]] NetCompatibilityRules netCompatibilityRules() const;

private:
  Uuid m_serverUuid;
  Uuid m_playerUuid;
  ConnectionId m_connectionId = 0;
  NetCompatibilityRules m_netCompatibilityRules;

  mutable RecursiveMutex m_mutex;

  JsonRpcPtr m_rpc;

  NetElementTopGroup m_netGroup;
  NetElementData<Maybe<pair<WarpAction, WarpMode>>> m_orbitWarpActionNetState;
  NetElementData<WorldId> m_playerWorldIdNetState;
  NetElementBool m_isAdminNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementData<ShipUpgrades> m_shipUpgrades;
  NetElementData<CelestialCoordinate> m_shipCoordinate;
  WorldChunks m_newShipUpdates;
};

}

#pragma once

#include "StarConfig.hpp"
#include "StarNetElementSystem.hpp"
#include "StarJsonRpc.hpp"
#include "StarGameTypes.hpp"
#include "StarDamageTypes.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarWarping.hpp"
#include "StarWorldStorage.hpp"
#include "StarPlayerTypes.hpp"

import std;

namespace Star {

class ClientContext {
public:
  ClientContext(Uuid serverUuid, Uuid playerUuid);

  auto serverUuid() const -> Uuid;
  // The player Uuid can differ from the mainPlayer's Uuid
  //  if the player has swapped character - use this for ship saving.
  auto playerUuid() const -> Uuid;

  // The coordinate for the world which the player's ship is currently
  // orbiting.
  auto shipCoordinate() const -> CelestialCoordinate;

  auto orbitWarpAction() const -> std::optional<std::pair<WarpAction, WarpMode>>;

  // The current world id of the player
  auto playerWorldId() const -> WorldId;

  auto isAdmin() const -> bool;
  auto team() const -> EntityDamageTeam;

  auto rpcInterface() const -> Ptr<JsonRpcInterface>;

  auto newShipUpdates() -> WorldChunks;
  auto shipUpgrades() const -> ShipUpgrades;

  void readUpdate(ByteArray data, NetCompatibilityRules rules);
  auto writeUpdate(NetCompatibilityRules rules) -> ByteArray;

  void setConnectionId(ConnectionId connectionId);
  auto connectionId() const -> ConnectionId;

  void setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules);
  auto netCompatibilityRules() const -> NetCompatibilityRules;

private:
  Uuid m_serverUuid;
  Uuid m_playerUuid;
  ConnectionId m_connectionId = 0;
  NetCompatibilityRules m_netCompatibilityRules;

  Ptr<JsonRpc> m_rpc;

  NetElementTopGroup m_netGroup;
  NetElementData<std::optional<std::pair<WarpAction, WarpMode>>> m_orbitWarpActionNetState;
  NetElementData<WorldId> m_playerWorldIdNetState;
  NetElementBool m_isAdminNetState;
  NetElementData<EntityDamageTeam> m_teamNetState;
  NetElementData<ShipUpgrades> m_shipUpgrades;
  NetElementData<CelestialCoordinate> m_shipCoordinate;
  WorldChunks m_newShipUpdates;
};

}

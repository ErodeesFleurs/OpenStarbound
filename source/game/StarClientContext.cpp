#include "StarClientContext.hpp"

import std;

namespace Star {

auto operator>>(DataStream& ds, ShipUpgrades& upgrades) -> DataStream& {
  ds.read(upgrades.shipLevel);
  ds.read(upgrades.maxFuel);
  ds.read(upgrades.crewSize);
  ds.read(upgrades.fuelEfficiency);
  ds.read(upgrades.shipSpeed);
  ds.read(upgrades.capabilities);
  return ds;
}

auto operator<<(DataStream& ds, ShipUpgrades const& upgrades) -> DataStream& {
  ds.write(upgrades.shipLevel);
  ds.write(upgrades.maxFuel);
  ds.write(upgrades.crewSize);
  ds.write(upgrades.fuelEfficiency);
  ds.write(upgrades.shipSpeed);
  ds.write(upgrades.capabilities);
  return ds;
}

ClientContext::ClientContext(Uuid serverUuid, Uuid playerUuid) {
  m_serverUuid = std::move(serverUuid);
  m_playerUuid = std::move(playerUuid);
  m_rpc = std::make_shared<JsonRpc>();

  m_netGroup.addNetElement(&m_orbitWarpActionNetState);
  m_netGroup.addNetElement(&m_playerWorldIdNetState);
  m_netGroup.addNetElement(&m_isAdminNetState);
  m_netGroup.addNetElement(&m_teamNetState);
  m_netGroup.addNetElement(&m_shipUpgrades);
  m_netGroup.addNetElement(&m_shipCoordinate);
}

auto ClientContext::serverUuid() const -> Uuid {
  return m_serverUuid;
}

auto ClientContext::playerUuid() const -> Uuid {
  return m_playerUuid;
}

auto ClientContext::shipCoordinate() const -> CelestialCoordinate {
  return m_shipCoordinate.get();
}

auto ClientContext::orbitWarpAction() const -> std::optional<std::pair<WarpAction, WarpMode>> {
  return m_orbitWarpActionNetState.get();
}

auto ClientContext::playerWorldId() const -> WorldId {
  return m_playerWorldIdNetState.get();
}

auto ClientContext::isAdmin() const -> bool {
  return m_isAdminNetState.get();
}

auto ClientContext::team() const -> EntityDamageTeam {
  return m_teamNetState.get();
}

auto ClientContext::rpcInterface() const -> Ptr<JsonRpcInterface> {
  return m_rpc;
}

auto ClientContext::newShipUpdates() -> WorldChunks {
  return take(m_newShipUpdates);
}

auto ClientContext::shipUpgrades() const -> ShipUpgrades {
  return m_shipUpgrades.get();
}

void ClientContext::readUpdate(ByteArray data, NetCompatibilityRules rules) {
  if (data.empty())
    return;

  DataStreamBuffer ds(std::move(data));
  ds.setStreamCompatibilityVersion(rules);

  m_rpc->receive(ds.read<ByteArray>());

  auto shipUpdates = ds.read<ByteArray>();
  if (!shipUpdates.empty())
    m_newShipUpdates.merge(DataStreamBuffer::deserialize<WorldChunks>(std::move(shipUpdates)), true);

  m_netGroup.readNetState(ds.read<ByteArray>(), 0.0f, rules);
}

auto ClientContext::writeUpdate(NetCompatibilityRules) -> ByteArray {
  return m_rpc->send();
}

void ClientContext::setConnectionId(ConnectionId connectionId) {
  m_connectionId = connectionId;
}

auto ClientContext::connectionId() const -> ConnectionId {
  return m_connectionId;
}

void ClientContext::setNetCompatibilityRules(NetCompatibilityRules netCompatibilityRules) {
  m_netCompatibilityRules = netCompatibilityRules;
}

auto ClientContext::netCompatibilityRules() const -> NetCompatibilityRules {
  return m_netCompatibilityRules;
}

}// namespace Star

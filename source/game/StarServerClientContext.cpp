#include "StarServerClientContext.hpp"

#include "StarCasting.hpp"
#include "StarContainerEntity.hpp"
#include "StarItemDatabase.hpp"
#include "StarRoot.hpp"
#include "StarUniverseSettings.hpp"// IWYU pragma: export
#include "StarWorldServerThread.hpp"

import std;

namespace Star {

ServerClientContext::ServerClientContext(ConnectionId clientId, std::optional<HostAddress> remoteAddress, NetCompatibilityRules netRules, Uuid playerUuid,
                                         String playerName, String shipSpecies, bool canBecomeAdmin, WorldChunks initialShipChunks)
    : m_clientId(clientId),
      m_remoteAddress(remoteAddress),
      m_netRules(netRules),
      m_playerUuid(playerUuid),
      m_playerName(std::move(playerName)),
      m_shipSpecies(std::move(shipSpecies)),
      m_canBecomeAdmin(canBecomeAdmin),
      m_shipChunks(std::move(initialShipChunks)) {
  m_rpc.registerHandler("ship.applyShipUpgrades", [this](Json const& args) -> Json {
    RecursiveMutexLocker locker(m_mutex);
    setShipUpgrades(shipUpgrades().apply(args));
    return true;
  });

  m_rpc.registerHandler("ship.setShipSpecies", [this](Json const& species) -> Json {
    RecursiveMutexLocker locker(m_mutex);
    setShipSpecies(species.toString());
    return true;
  });

  m_rpc.registerHandler("world.containerPutItems", [this](Json const& args) -> Json {
    List<ItemDescriptor> overflow = args.getArray("items").transformed(construct<ItemDescriptor>());
    RecursiveMutexLocker locker(m_mutex);
    if (m_worldThread) {
      m_worldThread->executeAction([args, &overflow](WorldServerThread*, WorldServer* server) -> void {
        EntityId entityId = args.getInt("entityId");
        Json items = args.get("items");
        ConstPtr<ItemDatabase> itemDatabase = Root::singleton().itemDatabase();
        if (auto containerEntity = as<ContainerEntity>(server->entity(entityId))) {
          overflow.clear();
          for (auto const& itemDescriptor : items.iterateArray()) {
            if (auto left = containerEntity->addItems(itemDatabase->item(ItemDescriptor(itemDescriptor))).result().value())
              overflow.append(left->descriptor());
          }
        }
      });
    }
    return overflow.transformed(std::mem_fn(&ItemDescriptor::toJson));
  });

  m_rpc.registerHandler("universe.setFlag", [this](Json const& args) -> Json {
    auto flagName = args.toString();
    RecursiveMutexLocker locker(m_mutex);
    if (m_worldThread) {
      m_worldThread->executeAction([flagName](WorldServerThread*, WorldServer* server) -> void {
        server->universeSettings()->setFlag(flagName);
      });
    }
    return {};
  });

  m_netGroup.addNetElement(&m_orbitWarpActionNetState);
  m_netGroup.addNetElement(&m_playerWorldIdNetState);
  m_netGroup.addNetElement(&m_isAdminNetState);
  m_netGroup.addNetElement(&m_teamNetState);
  m_netGroup.addNetElement(&m_shipUpgrades);
  m_netGroup.addNetElement(&m_shipCoordinate);

  m_creationTime = Time::monotonicMilliseconds();
}

auto ServerClientContext::clientId() const -> ConnectionId {
  return m_clientId;
}

auto ServerClientContext::remoteAddress() const -> std::optional<HostAddress> const& {
  return m_remoteAddress;
}

auto ServerClientContext::playerUuid() const -> Uuid const& {
  return m_playerUuid;
}

auto ServerClientContext::playerName() const -> String const& {
  return m_playerName;
}

auto ServerClientContext::shipSpecies() const -> String const& {
  return m_shipSpecies;
}

auto ServerClientContext::canBecomeAdmin() const -> bool {
  return m_canBecomeAdmin;
}

auto ServerClientContext::netRules() const -> NetCompatibilityRules {
  return m_netRules;
}

auto ServerClientContext::descriptiveName() const -> String {
  RecursiveMutexLocker locker(m_mutex);
  String hostName = m_remoteAddress ? toString(*m_remoteAddress) : "local";
  return strf("'{}' <{}> ({})", m_playerName, m_clientId, hostName);
}

void ServerClientContext::registerRpcHandlers(JsonRpcHandlers const& rpcHandlers) {
  m_rpc.registerHandlers(rpcHandlers);
}

auto ServerClientContext::shipCoordinate() const -> CelestialCoordinate {
  RecursiveMutexLocker locker(m_mutex);
  return m_shipCoordinate.get();
}

void ServerClientContext::setShipCoordinate(CelestialCoordinate system) {
  RecursiveMutexLocker locker(m_mutex);
  m_shipCoordinate.set(system);
}

auto ServerClientContext::shipLocation() const -> SystemLocation {
  RecursiveMutexLocker locker(m_mutex);
  return m_shipSystemLocation;
}

void ServerClientContext::setShipLocation(SystemLocation location) {
  RecursiveMutexLocker locker(m_mutex);
  m_shipSystemLocation = location;
}

auto ServerClientContext::orbitWarpAction() const -> std::optional<std::pair<WarpAction, WarpMode>> {
  RecursiveMutexLocker locker(m_mutex);
  return m_orbitWarpActionNetState.get();
}

void ServerClientContext::setOrbitWarpAction(std::optional<std::pair<WarpAction, WarpMode>> warpAction) {
  RecursiveMutexLocker locker(m_mutex);
  m_orbitWarpActionNetState.set(std::move(warpAction));
}

auto ServerClientContext::isAdmin() const -> bool {
  RecursiveMutexLocker locker(m_mutex);
  return m_isAdminNetState.get();
}

void ServerClientContext::setAdmin(bool admin) {
  RecursiveMutexLocker locker(m_mutex);
  m_isAdminNetState.set(admin);
}

auto ServerClientContext::team() const -> EntityDamageTeam {
  RecursiveMutexLocker locker(m_mutex);
  return m_teamNetState.get();
}

void ServerClientContext::setTeam(EntityDamageTeam team) {
  RecursiveMutexLocker locker(m_mutex);
  m_teamNetState.set(team);
}

auto ServerClientContext::shipUpgrades() const -> ShipUpgrades {
  RecursiveMutexLocker locker(m_mutex);
  return m_shipUpgrades.get();
}

void ServerClientContext::setShipUpgrades(ShipUpgrades upgrades) {
  RecursiveMutexLocker locker(m_mutex);
  m_shipUpgrades.set(upgrades);
}

void ServerClientContext::setShipSpecies(String shipSpecies) {
  m_shipSpecies = shipSpecies;
}

auto ServerClientContext::shipChunks() const -> WorldChunks {
  RecursiveMutexLocker locker(m_mutex);
  return m_shipChunks;
}

void ServerClientContext::updateShipChunks(WorldChunks newShipChunks) {
  RecursiveMutexLocker locker(m_mutex);
  m_shipChunksUpdate.merge(WorldStorage::getWorldChunksUpdate(m_shipChunks, newShipChunks), true);
  m_shipChunks = std::move(newShipChunks);
}

void ServerClientContext::readUpdate(ByteArray data) {
  RecursiveMutexLocker locker(m_mutex);
  m_rpc.receive(data);
}

auto ServerClientContext::writeUpdate() -> ByteArray {
  RecursiveMutexLocker locker(m_mutex);

  ByteArray rpcUpdate = m_rpc.send();

  ByteArray shipChunksUpdate;
  if (!m_shipChunksUpdate.empty())
    shipChunksUpdate = DataStreamBuffer::serialize(take(m_shipChunksUpdate));

  ByteArray netGroupUpdate;
  std::tie(netGroupUpdate, m_netVersion) = m_netGroup.writeNetState(m_netVersion, m_netRules);

  if (rpcUpdate.empty() && shipChunksUpdate.empty() && netGroupUpdate.empty())
    return {};

  DataStreamBuffer ds;
  ds.write(rpcUpdate);
  ds.write(shipChunksUpdate);
  ds.write(netGroupUpdate);

  return ds.takeData();
}

void ServerClientContext::setSystemWorld(Ptr<SystemWorldServerThread> systemWorldThread) {
  RecursiveMutexLocker locker(m_mutex);
  if (m_systemWorldThread == systemWorldThread)
    return;

  m_systemWorldThread = std::move(systemWorldThread);
}

auto ServerClientContext::systemWorld() const -> Ptr<SystemWorldServerThread> {
  RecursiveMutexLocker locker(m_mutex);
  return m_systemWorldThread;
}

void ServerClientContext::clearSystemWorld() {
  RecursiveMutexLocker locker(m_mutex);
  setSystemWorld({});
}

void ServerClientContext::setPlayerWorld(Ptr<WorldServerThread> worldThread) {
  RecursiveMutexLocker locker(m_mutex);
  if (m_worldThread == worldThread)
    return;

  m_worldThread = std::move(worldThread);
  if (m_worldThread)
    m_playerWorldIdNetState.set(m_worldThread->worldId());
  else
    m_playerWorldIdNetState.set(WorldId());
}

auto ServerClientContext::playerWorld() const -> Ptr<WorldServerThread> {
  RecursiveMutexLocker locker(m_mutex);
  return m_worldThread;
}

auto ServerClientContext::playerWorldId() const -> WorldId {
  RecursiveMutexLocker locker(m_mutex);
  return m_playerWorldIdNetState.get();
}

void ServerClientContext::clearPlayerWorld() {
  setPlayerWorld({});
}

auto ServerClientContext::playerReturnWarp() const -> WarpToWorld {
  RecursiveMutexLocker locker(m_mutex);
  return m_returnWarp;
}

void ServerClientContext::setPlayerReturnWarp(WarpToWorld warp) {
  RecursiveMutexLocker locker(m_mutex);
  m_returnWarp = std::move(warp);
}

auto ServerClientContext::playerReviveWarp() const -> WarpToWorld {
  RecursiveMutexLocker locker(m_mutex);
  return m_reviveWarp;
}

void ServerClientContext::setPlayerReviveWarp(WarpToWorld warp) {
  RecursiveMutexLocker locker(m_mutex);
  m_reviveWarp = std::move(warp);
}

void ServerClientContext::loadServerData(Json const& store) {
  RecursiveMutexLocker locker(m_mutex);
  m_shipCoordinate.set(CelestialCoordinate(store.get("shipCoordinate")));
  m_shipSystemLocation = jsonToSystemLocation(store.get("systemLocation"));
  setAdmin(store.getBool("isAdmin"));
  setTeam(EntityDamageTeam(store.get("team")));
  m_reviveWarp = WarpToWorld(store.get("reviveWarp"));
  m_returnWarp = WarpToWorld(store.get("returnWarp"));
}

auto ServerClientContext::storeServerData() -> Json {
  RecursiveMutexLocker locker(m_mutex);
  auto store = JsonObject{
    {"shipCoordinate", m_shipCoordinate.get().toJson()},
    {"systemLocation", jsonFromSystemLocation(m_shipSystemLocation)},
    {"isAdmin", m_isAdminNetState.get()},
    {"team", team().toJson()},
    {"reviveWarp", m_reviveWarp.toJson()},
    {"returnWarp", m_returnWarp.toJson()}};
  return store;
}

auto ServerClientContext::creationTime() const -> int64_t {
  return m_creationTime;
}

}// namespace Star

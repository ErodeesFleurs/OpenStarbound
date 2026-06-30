#include "StarWorldServerDungeonProtection.hpp"
#include "StarWorldServer.hpp"
#include "StarNetPackets.hpp"
#include "StarLogging.hpp"
#include "StarIterator.hpp"

namespace Star {

WorldServerDungeonProtection::WorldServerDungeonProtection(WorldServer& worldServer)
  : m_worldServer(worldServer) {}

bool WorldServerDungeonProtection::isTileProtected(Vec2I const& pos) const {
  if (!m_tileProtectionEnabled)
    return false;

  auto const& tile = m_worldServer.m_tileArray->tile(pos);
  return m_protectedDungeonIds.contains(tile.dungeonId);
}

bool WorldServerDungeonProtection::getTileProtection(DungeonId dungeonId) const {
  return m_protectedDungeonIds.contains(dungeonId);
}

void WorldServerDungeonProtection::setTileProtection(DungeonId dungeonId, bool isProtected) {
  bool updated = false;
  if (isProtected) {
    updated = m_protectedDungeonIds.add(dungeonId);
  } else {
    updated = m_protectedDungeonIds.remove(dungeonId);
  }

  if (updated) {
    for (auto const& [_, clientInfo] : m_worldServer.m_clientInfo)
      clientInfo->outgoingPackets.append(make_shared<UpdateTileProtectionPacket>(dungeonId, isProtected));

    Logger::info("Protected dungeonIds for world set to {}", m_protectedDungeonIds);
  }
}

size_t WorldServerDungeonProtection::setTileProtection(List<DungeonId> const& dungeonIds, bool isProtected) {
  List<PacketPtr> updates;
  updates.reserve(dungeonIds.size());
  for (auto const& dungeonId : dungeonIds)
    if (isProtected ? m_protectedDungeonIds.add(dungeonId) : m_protectedDungeonIds.remove(dungeonId))
      updates.append(make_shared<UpdateTileProtectionPacket>(dungeonId, isProtected));

  if (updates.empty())
    return 0;

  for (auto const& [_, clientInfo] : m_worldServer.m_clientInfo)
    clientInfo->outgoingPackets.appendAll(updates);

  auto newDungeonIds = m_protectedDungeonIds.values();
  sort(newDungeonIds);
  Logger::info("Protected dungeonIds for world set to {}", newDungeonIds);
  return updates.size();
}

void WorldServerDungeonProtection::setTileProtectionEnabled(bool enabled) {
  m_tileProtectionEnabled = enabled;
}

void WorldServerDungeonProtection::setDungeonId(RectI const& tileArea, DungeonId dungeonId) {
  for (int x = tileArea.xMin(); x < tileArea.xMax(); ++x) {
    for (int y = tileArea.yMin(); y < tileArea.yMax(); ++y) {
      auto pos = Vec2I{x, y};
      if (auto tile = m_worldServer.m_tileArray->modifyTile(pos)) {
        tile->dungeonId = dungeonId;
        m_worldServer.queueTileUpdates(pos);
      }
    }
  }
}

DungeonId WorldServerDungeonProtection::dungeonId(Vec2I const& pos) const {
  return m_worldServer.m_tileArray->tile(pos).dungeonId;
}

void WorldServerDungeonProtection::setDungeonGravity(DungeonId dungeonId, Maybe<float> gravity) {
  Maybe<float> current = m_dungeonIdGravity.maybe(dungeonId);
  if (gravity != current) {
    if (gravity)
      m_dungeonIdGravity[dungeonId] = *gravity;
    else
      m_dungeonIdGravity.remove(dungeonId);

    for (auto const& [_, clientInfo] : m_worldServer.m_clientInfo)
      clientInfo->outgoingPackets.append(make_shared<SetDungeonGravityPacket>(dungeonId, gravity));
  }
}

void WorldServerDungeonProtection::setDungeonBreathable(DungeonId dungeonId, Maybe<bool> breathable) {
  Maybe<bool> current = m_dungeonIdBreathable.maybe(dungeonId);
  if (breathable != current) {
    if (breathable)
      m_dungeonIdBreathable[dungeonId] = *breathable;
    else
      m_dungeonIdBreathable.remove(dungeonId);

    for (auto const& [_, clientInfo] : m_worldServer.m_clientInfo)
      clientInfo->outgoingPackets.append(make_shared<SetDungeonBreathablePacket>(dungeonId, breathable));
  }
}

bool WorldServerDungeonProtection::isPlayerModified(RectI const& region) const {
  return m_worldServer.m_tileArray->tileSatisfies(region, [](Vec2I const&, ServerTile const& tile) {
      return tile.dungeonId == ConstructionDungeonId || tile.dungeonId == DestroyedBlockDungeonId;
    });
}

}

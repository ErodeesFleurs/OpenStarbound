#include "StarWorldServerSpawnFinder.hpp"
#include "StarWorldServer.hpp"
#include "StarWorldImpl.hpp"
#include "StarWorldGeneration.hpp"
#include "StarLogging.hpp"
#include "StarRandom.hpp"
#include "StarJsonExtra.hpp"
#include "StarGameTypes.hpp"
#include "StarNetPackets.hpp"

namespace Star {

WorldServerSpawnFinder::WorldServerSpawnFinder(WorldServer& worldServer)
  : m_worldServer(worldServer) {}

Vec2F WorldServerSpawnFinder::findPlayerStart(Maybe<Vec2F> firstTry) {
  Vec2F spawnRectSize = jsonToVec2F(m_worldServer.m_serverConfig.get("playerStartRegionSize"));
  auto maximumVerticalSearch = m_worldServer.m_serverConfig.getInt("playerStartRegionMaximumVerticalSearch");
  auto maximumTries = m_worldServer.m_serverConfig.getInt("playerStartRegionMaximumTries");

  static const Set<DungeonId> allowedSpawnDungeonIds = {NoDungeonId, SpawnDungeonId, ConstructionDungeonId, DestroyedBlockDungeonId};

  Vec2F pos;
  if (firstTry)
    pos = *firstTry;
  else
    pos = Vec2F(m_worldServer.m_worldTemplate->findSensiblePlayerStart().value(Vec2I(0, m_worldServer.m_worldTemplate->surfaceLevel())));

  CollisionSet collideWithAnything{CollisionKind::Null, CollisionKind::Block, CollisionKind::Dynamic, CollisionKind::Platform, CollisionKind::Slippery};
  for (int t = 0; t < maximumTries; ++t) {
    bool foundGround = false;
    for (int i = 0; i < maximumVerticalSearch; ++i) {
      RectF spawnRect = RectF(pos[0] - spawnRectSize[0] / 2, pos[1], pos[0] + spawnRectSize[0] / 2, pos[1] + spawnRectSize[1]);
      m_worldServer.generateRegion(RectI::integral(spawnRect));
      if (WorldImpl::rectTileCollision(m_worldServer.m_tileArray, RectI::integral(spawnRect), collideWithAnything)) {
        foundGround = true;
        break;
      }
      --pos[1];
    }

    if (foundGround) {
      for (int i = 0; i < maximumVerticalSearch; ++i) {
        if (m_worldServer.m_tileArray->tile(Vec2I::floor(pos)).liquid.liquid != EmptyLiquidId)
          break;

        RectF spawnRect = RectF(pos[0] - spawnRectSize[0] / 2, pos[1], pos[0] + spawnRectSize[0] / 2, pos[1] + spawnRectSize[1]);

        m_worldServer.generateRegion(RectI::integral(spawnRect));

        auto tileDungeonId = m_worldServer.getServerTile(Vec2I::floor(pos)).dungeonId;

        if (!allowedSpawnDungeonIds.contains(tileDungeonId))
          break;

        if (!WorldImpl::rectTileCollision(m_worldServer.m_tileArray, RectI::integral(spawnRect), collideWithAnything) && spawnRect.yMax() < m_worldServer.m_geometry.height())
          return pos;

        ++pos[1];
      }
    }

    pos = Vec2F(m_worldServer.m_worldTemplate->findSensiblePlayerStart().value(Vec2I(0, m_worldServer.m_worldTemplate->surfaceLevel())));
  }

  return pos;
}

Vec2F WorldServerSpawnFinder::findPlayerSpaceStart(float targetX) {
  Vec2F testRectSize = jsonToVec2F(m_worldServer.m_serverConfig.get("playerSpaceStartRegionSize"));
  auto distanceIncrement = m_worldServer.m_serverConfig.getFloat("playerSpaceStartDistanceIncrement");
  auto maximumTries = m_worldServer.m_serverConfig.getInt("playerSpaceStartMaximumTries");

  Vec2F basePos = Vec2F(targetX, m_worldServer.m_geometry.height() * 0.5);

  CollisionSet collideWithAnything{CollisionKind::Null, CollisionKind::Block, CollisionKind::Dynamic, CollisionKind::Platform, CollisionKind::Slippery};
  for (int t = 0; t < maximumTries; ++t) {
    Vec2F testPos = m_worldServer.m_geometry.limit(basePos + Vec2F::withAngle(Random::randf() * 2 * Constants::pi, t * distanceIncrement));
    RectF testRect = RectF::withCenter(testPos, testRectSize);
    m_worldServer.generateRegion(RectI::integral(testRect));
    if (!WorldImpl::rectTileCollision(m_worldServer.m_tileArray, RectI::integral(testRect), collideWithAnything))
      return testPos;
  }

  return basePos;
}

void WorldServerSpawnFinder::setPlayerStart(Vec2F const& startPosition, bool respawnInWorld) {
  m_playerStart = startPosition;
  m_respawnInWorld = respawnInWorld;
  m_adjustPlayerStart = false;
  for (auto const& pair : m_worldServer.m_clientInfo)
    pair.second->outgoingPackets.append(make_shared<SetPlayerStartPacket>(m_playerStart, m_respawnInWorld));
}

Vec2F WorldServerSpawnFinder::playerStart() const {
  return m_playerStart;
}

bool WorldServerSpawnFinder::adjustPlayerStart() const {
  return m_adjustPlayerStart;
}

bool WorldServerSpawnFinder::respawnInWorld() const {
  return m_respawnInWorld;
}

}

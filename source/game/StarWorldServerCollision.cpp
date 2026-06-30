#include "StarWorldServerCollision.hpp"
#include "StarWorldServer.hpp"
#include "StarWorldImpl.hpp"
#include "StarWorldTiles.hpp"

namespace Star {

WorldServerCollision::WorldServerCollision(WorldServer& worldServer)
  : m_worldServer(worldServer) {}

bool WorldServerCollision::pointTileCollision(Vec2F const& point, CollisionSet const& collisionSet) const {
  return m_worldServer.m_tileArray->tile(Vec2I(point.floor())).isColliding(collisionSet);
}

bool WorldServerCollision::lineTileCollision(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet) const {
  return WorldImpl::lineTileCollision(m_worldServer.m_geometry, m_worldServer.m_tileArray, begin, end, collisionSet);
}

Maybe<pair<Vec2F, Vec2I>> WorldServerCollision::lineTileCollisionPoint(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet) const {
  return WorldImpl::lineTileCollisionPoint(m_worldServer.m_geometry, m_worldServer.m_tileArray, begin, end, collisionSet);
}

List<Vec2I> WorldServerCollision::collidingTilesAlongLine(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet, int maxSize, bool includeEdges) const {
  return WorldImpl::collidingTilesAlongLine(m_worldServer.m_geometry, m_worldServer.m_tileArray, begin, end, collisionSet, maxSize, includeEdges);
}

bool WorldServerCollision::rectTileCollision(RectI const& region, CollisionSet const& collisionSet) const {
  return WorldImpl::rectTileCollision(m_worldServer.m_tileArray, region, collisionSet);
}

bool WorldServerCollision::tileIsOccupied(Vec2I const& pos, TileLayer layer, bool includeEphemeral, bool checkCollision) const {
  return WorldImpl::tileIsOccupied(m_worldServer.m_tileArray, m_worldServer.m_entityMap, pos, layer, includeEphemeral, checkCollision);
}

CollisionKind WorldServerCollision::tileCollisionKind(Vec2I const& pos) const {
  return WorldImpl::tileCollisionKind(m_worldServer.m_tileArray, m_worldServer.m_entityMap, pos);
}

void WorldServerCollision::forEachCollisionBlock(RectI const& region, function<void(CollisionBlock const&)> const& iterator) const {
  const_cast<WorldServerCollision*>(this)->freshenCollision(region);
  m_worldServer.m_tileArray->tileEach(region, [&](Vec2I const& pos, ServerTile const& tile) {
      if (tile.getCollision() == CollisionKind::Null) {
        iterator(CollisionBlock::nullBlock(pos));
      } else {
        assert(!tile.collisionCacheDirty);
        if (auto cache = m_collisionCache.ptr(pos)) {
          for (auto const& block : *cache)
            iterator(block);
        }
      }
    });
}

bool WorldServerCollision::isTileConnectable(Vec2I const& pos, TileLayer layer, bool tilesOnly) const {
  return m_worldServer.m_tileArray->tile(pos).isConnectable(layer, tilesOnly);
}

void WorldServerCollision::dirtyCollision(RectI const& region) {
  dirtyCollisionImpl(m_worldServer.m_tileArray, region);
}

void WorldServerCollision::freshenCollision(RectI const& region) {
  freshenCollisionImpl(m_worldServer.m_tileArray, m_collisionGenerator, m_collisionCache, region);
}

}

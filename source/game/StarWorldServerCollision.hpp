#pragma once

#include "StarCollisionGenerator.hpp"
#include "StarMap.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarList.hpp"
#include "StarCollisionBlock.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class WorldServer;

class WorldServerCollision {
public:
  friend class WorldServer;

  WorldServerCollision() = default;
  explicit WorldServerCollision(WorldServer* worldServer);

  bool pointTileCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const;
  bool lineTileCollision(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const;
  Maybe<pair<Vec2F, Vec2I>> lineTileCollisionPoint(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const;
  List<Vec2I> collidingTilesAlongLine(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet, int maxSize = -1, bool includeEdges = true) const;
  bool rectTileCollision(RectI const& region, CollisionSet const& collisionSet = DefaultCollisionSet) const;
  bool tileIsOccupied(Vec2I const& pos, TileLayer layer, bool includeEphemeral = false, bool checkCollision = false) const;
  CollisionKind tileCollisionKind(Vec2I const& pos) const;
  void forEachCollisionBlock(RectI const& region, function<void(CollisionBlock const&)> const& iterator) const;
  bool isTileConnectable(Vec2I const& pos, TileLayer layer, bool tilesOnly = false) const;
  void dirtyCollision(RectI const& region);
  void freshenCollision(RectI const& region);

private:
  WorldServer* m_worldServer = nullptr;

  CollisionGenerator m_collisionGenerator;
  HashMap<Vec2I, StaticList<CollisionBlock, CollisionGenerator::MaximumCollisionsPerSpace>> m_collisionCache;
  List<CollisionBlock> m_workingCollisionBlocks;
};

}

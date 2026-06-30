#pragma once

#include "StarInteractionTypes.hpp"
#include "StarCollisionBlock.hpp"
#include "StarTileModification.hpp"

namespace Star {

// Interface for tile/block/material queries and modifications.
// Extracted from World for use where only tile-level operations are needed.
class TileWorldInterface {
public:
  virtual ~TileWorldInterface() = default;

  [[nodiscard]] virtual MaterialId material(Vec2I const& position, TileLayer layer) const = 0;
  virtual std::tuple<MaterialId, ModId> materialAndMod(Vec2I const& position, TileLayer layer) const = 0;
  [[nodiscard]] virtual MaterialHue materialHueShift(Vec2I const& position, TileLayer layer) const = 0;
  [[nodiscard]] virtual ModId mod(Vec2I const& position, TileLayer layer) const = 0;
  [[nodiscard]] virtual MaterialHue modHueShift(Vec2I const& position, TileLayer layer) const = 0;
  [[nodiscard]] virtual MaterialColorVariant colorVariant(Vec2I const& position, TileLayer layer) const = 0;
  [[nodiscard]] virtual LiquidLevel liquidLevel(Vec2I const& pos) const = 0;
  [[nodiscard]] virtual LiquidLevel liquidLevel(RectF const& region) const = 0;

  [[nodiscard]] virtual TileModificationList validTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) const = 0;
  [[nodiscard]] virtual TileModificationList applyTileModifications(TileModificationList const& modificationList, bool allowEntityOverlap) = 0;
  [[nodiscard]] virtual TileModificationList replaceTiles(TileModificationList const& modificationList, TileDamage const& tileDamage, bool applyDamage = false) = 0;
  [[nodiscard]] virtual bool damageWouldDestroy(Vec2I const& pos, TileLayer layer, TileDamage const& tileDamage) const = 0;

  [[nodiscard]] virtual bool isTileProtected(Vec2I const& pos) const = 0;

  [[nodiscard]] virtual bool tileIsOccupied(Vec2I const& pos, TileLayer layer, bool includeEphemeral = false, bool checkCollision = false) const = 0;

  [[nodiscard]] virtual CollisionKind tileCollisionKind(Vec2I const& pos) const = 0;
  virtual void forEachCollisionBlock(RectI const& region, function<void(CollisionBlock const&)> const& iterator) const = 0;
  [[nodiscard]] virtual bool isTileConnectable(Vec2I const& pos, TileLayer layer, bool tilesOnly = false) const = 0;

  [[nodiscard]] virtual bool pointTileCollision(Vec2F const& point, CollisionSet const& collisionSet = DefaultCollisionSet) const = 0;
  [[nodiscard]] virtual bool lineTileCollision(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const = 0;
  [[nodiscard]] virtual Maybe<pair<Vec2F, Vec2I>> lineTileCollisionPoint(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet) const = 0;
  [[nodiscard]] virtual List<Vec2I> collidingTilesAlongLine(Vec2F const& begin, Vec2F const& end, CollisionSet const& collisionSet = DefaultCollisionSet, int maxSize = -1, bool includeEdges = true) const = 0;
  [[nodiscard]] virtual bool rectTileCollision(RectI const& region, CollisionSet const& collisionSet = DefaultCollisionSet) const = 0;

  [[nodiscard]] virtual TileDamageResult damageTiles(List<Vec2I> const& tilePositions, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, Maybe<EntityId> sourceEntity = {}) = 0;
};

}

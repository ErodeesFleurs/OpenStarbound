#pragma once

#include "StarBiMap.hpp"
#include "StarPoly.hpp"

import std;

namespace Star {

enum class CollisionKind : std::uint8_t {
  // Special collision block that is used for unloaded / un-generated tiles.
  // Collides the same as "Block", but does not tile with it.
  Null,
  None,
  Platform,
  Dynamic,
  Slippery,
  Block
};

enum class TileCollisionOverride : std::uint8_t {
  None,
  Empty,
  Platform,
  Block
};

inline auto collisionKindFromOverride(TileCollisionOverride const& over) -> CollisionKind {
  switch (over) {
  case TileCollisionOverride::Empty:
    return CollisionKind::None;
  case TileCollisionOverride::Platform:
    return CollisionKind::Platform;
  case TileCollisionOverride::Block:
    return CollisionKind::Block;
  default:
    return CollisionKind::Null;
  }
}

class CollisionSet {
public:
  CollisionSet();
  CollisionSet(std::initializer_list<CollisionKind> kinds);

  void insert(CollisionKind kind);
  void remove(CollisionKind kind);
  [[nodiscard]] auto contains(CollisionKind kind) const -> bool;

private:
  static auto kindBit(CollisionKind kind) -> std::uint8_t;

  std::uint8_t m_kinds;
};

// The default CollisionSet consists of Null, Slippery, Dynamic and Block
extern CollisionSet const DefaultCollisionSet;

// Defines what can be "blocks" e.g. for tile rendering: Block and Slippery
extern CollisionSet const BlockCollisionSet;

extern EnumMap<TileCollisionOverride> const TileCollisionOverrideNames;

extern EnumMap<CollisionKind> const CollisionKindNames;

auto isColliding(CollisionKind kind, CollisionSet const& collisionSet) -> bool;
auto isSolidColliding(CollisionKind kind) -> bool;

// Returns the highest priority collision kind, where Block > Slippery >
// Dynamic > Platform > None > Null
auto maxCollision(CollisionKind first, CollisionKind second) -> CollisionKind;

struct CollisionBlock {
  // Make a null collision block for the given space.
  static auto nullBlock(Vec2I const& space) -> CollisionBlock;

  CollisionKind kind;
  Vec2I space;
  PolyF poly;
  RectF polyBounds;
};

inline CollisionSet::CollisionSet()
    : m_kinds(0) {}

inline CollisionSet::CollisionSet(std::initializer_list<CollisionKind> kinds)
    : CollisionSet() {
  for (auto kind : kinds) {
    insert(kind);
  }
}

inline void CollisionSet::insert(CollisionKind kind) {
  m_kinds = m_kinds | kindBit(kind);
}

inline void CollisionSet::remove(CollisionKind kind) {
  m_kinds = m_kinds & ~kindBit(kind);
}

inline auto CollisionSet::contains(CollisionKind kind) const -> bool {
  return m_kinds & kindBit(kind);
}

inline auto CollisionSet::kindBit(CollisionKind kind) -> std::uint8_t {
  return 1 << ((std::uint8_t)kind + 1);
}

inline auto isColliding(CollisionKind kind, CollisionSet const& collisionSet) -> bool {
  return collisionSet.contains(kind);
}

inline auto isSolidColliding(CollisionKind kind) -> bool {
  return isColliding(kind, DefaultCollisionSet);
}

inline auto maxCollision(CollisionKind first, CollisionKind second) -> CollisionKind {
  return std::max(first, second);
}

inline auto CollisionBlock::nullBlock(Vec2I const& space) -> CollisionBlock {
  CollisionBlock block;
  block.kind = CollisionKind::Null;
  block.space = space;
  block.poly = {
    Vec2F(space) + Vec2F(0, 0),
    Vec2F(space) + Vec2F(1, 0),
    Vec2F(space) + Vec2F(1, 1),
    Vec2F(space) + Vec2F(0, 1)};
  block.polyBounds = RectF::withSize(Vec2F(space), Vec2F(1, 1));
  return block;
}

}// namespace Star

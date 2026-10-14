#include "StarConfig.hpp"
import star.list;
import star.bi_map;
import star.collision_block;

namespace Star {

extern CollisionSet const DefaultCollisionSet({CollisionKind::Null, CollisionKind::Slippery, CollisionKind::Dynamic, CollisionKind::Block});
extern CollisionSet const BlockCollisionSet({CollisionKind::Block, CollisionKind::Slippery});

extern EnumMap<TileCollisionOverride> const TileCollisionOverrideNames = {
    {TileCollisionOverride::None, "None"},
    {TileCollisionOverride::Empty, "Empty"},
    {TileCollisionOverride::Platform, "Platform"},
    {TileCollisionOverride::Block, "Block"}
  };

extern EnumMap<CollisionKind> const CollisionKindNames{
  {CollisionKind::Null, "Null"},
  {CollisionKind::None, "None"},
  {CollisionKind::Platform, "Platform"},
  {CollisionKind::Dynamic, "Dynamic"},
  {CollisionKind::Slippery, "Slippery"},
  {CollisionKind::Block, "Block"}
};

}

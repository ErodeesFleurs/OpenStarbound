import star.format;
import std;
import star.list;
import star.bi_map;
import star.collision_block;

namespace Star {

extern CollisionSet const DefaultCollisionSet({CollisionKind::Null, CollisionKind::Slippery, CollisionKind::Dynamic, CollisionKind::Block});
extern CollisionSet const BlockCollisionSet({CollisionKind::Block, CollisionKind::Slippery});

extern std::expected<EnumMap<TileCollisionOverride>, StarException> const TileCollisionOverrideNames = EnumMap<TileCollisionOverride>::create({
    {TileCollisionOverride::None, "None"},
    {TileCollisionOverride::Empty, "Empty"},
    {TileCollisionOverride::Platform, "Platform"},
    {TileCollisionOverride::Block, "Block"}
  });

extern std::expected<EnumMap<CollisionKind>, StarException> const CollisionKindNames = EnumMap<CollisionKind>::create({
  {CollisionKind::Null, "Null"},
  {CollisionKind::None, "None"},
  {CollisionKind::Platform, "Platform"},
  {CollisionKind::Dynamic, "Dynamic"},
  {CollisionKind::Slippery, "Slippery"},
  {CollisionKind::Block, "Block"}
});

}

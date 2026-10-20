import std;
import star.json;
import star.id_map;
import star.poly;
import star.variant;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.list;
import star.rpc_promise;
import star.lua;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.lua_root;
import star.force_regions;
import star.world;
import star.scripted_entity;

import star.line;
import star.rect;
import star.math_common;
namespace Star {

bool World::isServer() const {
  return connection() == ServerConnectionId;
}

bool World::isClient() const {
  return !isServer();
}

List<std::shared_ptr<Entity>> World::entityQuery(RectF const& boundBox, EntityFilter selector) const {
  List<std::shared_ptr<Entity>> list;
  forEachEntity(boundBox, [&](std::shared_ptr<Entity> const& entity) {
      if (!selector || selector(entity))
        list.append(entity);
    });
  return list;
}

List<std::shared_ptr<Entity>> World::entityLineQuery(Vec2F const& begin, Vec2F const& end, EntityFilter selector) const {
  List<std::shared_ptr<Entity>> list;
  forEachEntityLine(begin, end, [&](std::shared_ptr<Entity> const& entity) {
      if (!selector || selector(entity))
        list.append(entity);
    });
  return list;
}

List<std::shared_ptr<TileEntity>> World::entitiesAtTile(Vec2I const& pos, EntityFilter selector) const {
  List<std::shared_ptr<TileEntity>> list;
  forEachEntityAtTile(pos, [&](std::shared_ptr<TileEntity> entity) {
      if (!selector || selector(entity))
        list.append(std::move(entity));
    });
  return list;
}

List<Vec2I> World::findEmptyTiles(Vec2I pos, unsigned maxDist, std::size_t maxAmount, bool excludeEphemeral) const {
  List<Vec2I> res;
  if (!tileIsOccupied(pos, TileLayer::Foreground, excludeEphemeral))
    res.append(pos);

  if (res.size() >= maxAmount)
    return res;

  // searches manhattan distance counterclockwise from right
  for (int distance = 1; distance <= (int)maxDist; distance++) {
    const int totalSpots = 4 * distance;
    int xDiff = distance;
    int yDiff = 0;
    int dx = -1;
    int dy = 1;
    for (int i = 0; i < totalSpots; i++) {
      if (!tileIsOccupied(pos + Vec2I(xDiff, yDiff), TileLayer::Foreground)) {
        res.append(pos + Vec2I(xDiff, yDiff));
        if (res.size() >= maxAmount) {
          return res;
        }
      }
      xDiff += dx;
      yDiff += dy;
      if (std::abs(xDiff) == distance)
        dx *= -1;
      if (std::abs(yDiff) == distance)
        dy *= -1;
    }
  }

  return res;
}

bool World::canModifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap) const {
  return !validTileModifications({{pos, modification}}, allowEntityOverlap).empty();
}

bool World::modifyTile(Vec2I const& pos, TileModification const& modification, bool allowEntityOverlap) {
  return applyTileModifications({{pos, modification}}, allowEntityOverlap).empty();
}

TileDamageResult World::damageTile(Vec2I const& tilePosition, TileLayer layer, Vec2F const& sourcePosition, TileDamage const& tileDamage, std::optional<EntityId> sourceEntity) {
  return damageTiles({tilePosition}, layer, sourcePosition, tileDamage, sourceEntity);
}

std::shared_ptr<Entity> World::closestEntityInSight(Vec2F const& center, float radius, CollisionSet const& collisionSet, EntityFilter selector) const {
  return closestEntity(center, radius, [=, this](std::shared_ptr<Entity> const& entity) {
      return selector(entity) && !lineTileCollision(center, entity->position(), collisionSet);
    });
}

bool World::pointCollision(Vec2F const& point, CollisionSet const& collisionSet) const {
  bool collided = false;

  forEachCollisionBlock(RectI::withCenter(Vec2I(point), {3, 3}), [&](CollisionBlock const& block) {
      if (collided || !isColliding(block.kind, collisionSet))
        return;

      if (block.poly.contains(point))
        collided = true;
    });

  return collided;
}

std::optional<std::pair<Vec2F, std::optional<Vec2F>>> World::lineCollision(Line2F const& line, CollisionSet const& collisionSet) const {
  auto geometry = this->geometry();
  std::optional<PolyF> intersectPoly;
  std::optional<PolyF::LineIntersectResult> closestIntersection;

  forEachCollisionBlock(RectI::integral(RectF::boundBoxOf(line.min(), line.max()).padded(1)), [&](CollisionBlock const& block) {
      if (block.poly.isNull() || !isColliding(block.kind, collisionSet))
        return;

      Vec2F nearMin = geometry.nearestTo(block.poly.center(), line.min());
      auto intersection = block.poly.lineIntersection(Line2F(nearMin, nearMin + line.diff()));
      if (intersection && (!closestIntersection || intersection->along < closestIntersection->along)) {
        intersectPoly = block.poly;
        closestIntersection = intersection;
      }
    });

  if (closestIntersection) {
    auto point = line.eval(closestIntersection->along);
    auto normal = closestIntersection->intersectedSide.transform([&](std::uint64_t side) { return intersectPoly->normal(side); });
    return std::make_pair(point, normal);
  }
  return {};
}

bool World::polyCollision(PolyF const& poly, CollisionSet const& collisionSet) const {
  auto geometry = this->geometry();
  Vec2F polyCenter = poly.center();
  PolyF translatedPoly;
  bool collided = false;

  forEachCollisionBlock(RectI::integral(poly.boundBox()).padded(1), [&](CollisionBlock const& block) {
      if (collided || !isColliding(block.kind, collisionSet))
        return;

      Vec2F center = block.poly.center();
      Vec2F newCenter = geometry.nearestTo(polyCenter, center);
      translatedPoly = block.poly;
      translatedPoly.translate(newCenter - center);
      if (poly.intersects(translatedPoly))
        collided = true;
    });

  return collided;
}

}

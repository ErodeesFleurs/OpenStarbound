import std;
import star.json;
import star.json_extra;
import star.id_map;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.list;
import star.data_stream_extra;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.collision_block;
import star.force_regions;
import star.physics_entity;

import star.rect;
namespace Star {

PhysicsMovingCollision PhysicsMovingCollision::fromJson(Json const& json) {
  PhysicsMovingCollision pmc;
  pmc.position = json.opt("position").transform(jsonToVec2F).value_or(Vec2F{});
  pmc.collision = jsonToPolyF(json.get("collision"));
  pmc.collisionKind = CollisionKindNames.getLeft(json.getString("collisionKind", "block"));
  pmc.categoryFilter = jsonToPhysicsCategoryFilter(json);
  return pmc;
}

RectF PhysicsMovingCollision::boundBox() const {
  return collision.boundBox().translated(position);
}

void PhysicsMovingCollision::translate(Vec2F const& pos) {
  position += pos;
}

bool PhysicsMovingCollision::operator==(PhysicsMovingCollision const& rhs) const {
  return std::tie(position, collision, collisionKind, categoryFilter) == std::tie(rhs.position, rhs.collision, rhs.collisionKind, rhs.categoryFilter);
}

DataStream& operator>>(DataStream& ds, PhysicsMovingCollision& pmc) {
  ds >> pmc.position;
  ds >> pmc.collision;
  ds >> pmc.collisionKind;
  ds >> pmc.categoryFilter;
  return ds;
}

DataStream& operator<<(DataStream& ds, PhysicsMovingCollision const& pmc) {
  ds << pmc.position;
  ds << pmc.collision;
  ds << pmc.collisionKind;
  ds << pmc.categoryFilter;
  return ds;
}

MovingCollisionId::MovingCollisionId() : physicsEntityId(NullEntityId), collisionIndex(0) {}

MovingCollisionId::MovingCollisionId(EntityId physicsEntityId, std::size_t collisionIndex)
  : physicsEntityId(physicsEntityId), collisionIndex(collisionIndex) {}

bool MovingCollisionId::operator==(MovingCollisionId const& rhs) {
  return std::tie(physicsEntityId, collisionIndex) == std::tie(rhs.physicsEntityId, rhs.collisionIndex);
}

bool MovingCollisionId::valid() const {
  return physicsEntityId != NullEntityId;
}

MovingCollisionId::operator bool() const {
  return valid();
}

DataStream& operator>>(DataStream& ds, MovingCollisionId& mci) {
  ds.read(mci.physicsEntityId);
  ds.readVlqS(mci.collisionIndex);
  return ds;
}

DataStream& operator<<(DataStream& ds, MovingCollisionId const& mci) {
  ds.write(mci.physicsEntityId);
  ds.writeVlqS(mci.collisionIndex);
  return ds;
}

List<PhysicsForceRegion> PhysicsEntity::forceRegions() const {
  return {};
}

std::size_t PhysicsEntity::movingCollisionCount() const {
  return 0;
}

std::optional<PhysicsMovingCollision> PhysicsEntity::movingCollision(std::size_t) const {
  return {};
}

}

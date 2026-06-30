#include "StarPhysicsEntity.hpp"
#include "StarJsonExtra.hpp"
#include "StarDataStreamExtra.hpp"

namespace Star {

[[nodiscard]] PhysicsMovingCollision PhysicsMovingCollision::fromJson(Json const& json) {
  PhysicsMovingCollision pmc;
  pmc.position = json.opt("position").apply(jsonToVec2F).value();
  pmc.collision = jsonToPolyF(json.get("collision"));
  pmc.collisionKind = CollisionKindNames.getLeft(json.getString("collisionKind", "block"));
  pmc.categoryFilter = jsonToPhysicsCategoryFilter(json);
  return pmc;
}

[[nodiscard]] RectF PhysicsMovingCollision::boundBox() const {
  return collision.boundBox().translated(position);
}

void PhysicsMovingCollision::translate(Vec2F const& pos) {
  position += pos;
}

[[nodiscard]] bool PhysicsMovingCollision::operator==(PhysicsMovingCollision const& rhs) const {
  return tie(position, collision, collisionKind, categoryFilter) == tie(rhs.position, rhs.collision, rhs.collisionKind, rhs.categoryFilter);
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

MovingCollisionId::MovingCollisionId(EntityId physicsEntityId, size_t collisionIndex)
  : physicsEntityId(physicsEntityId), collisionIndex(collisionIndex) {}

[[nodiscard]] bool MovingCollisionId::operator==(MovingCollisionId const& rhs) const {
  return tie(physicsEntityId, collisionIndex) == tie(rhs.physicsEntityId, rhs.collisionIndex);
}

[[nodiscard]] bool MovingCollisionId::valid() const {
  return physicsEntityId != NullEntityId;
}

[[nodiscard]] MovingCollisionId::operator bool() const {
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

[[nodiscard]] List<PhysicsForceRegion> PhysicsEntity::forceRegions() const {
  return {};
}

[[nodiscard]] size_t PhysicsEntity::movingCollisionCount() const {
  return 0;
}

[[nodiscard]] Maybe<PhysicsMovingCollision> PhysicsEntity::movingCollision(size_t) const {
  return {};
}

}

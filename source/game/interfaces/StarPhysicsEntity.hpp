#pragma once

#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"
#include "StarEntity.hpp"
#include "StarForceRegions.hpp"
#include "StarCollisionBlock.hpp"

namespace Star {

class PhysicsEntity;

struct PhysicsMovingCollision {
  [[nodiscard]] static PhysicsMovingCollision fromJson(Json const& json);

  [[nodiscard]] RectF boundBox() const;

  void translate(Vec2F const& pos);

  [[nodiscard]] bool operator==(PhysicsMovingCollision const& rhs) const;

  Vec2F position;
  PolyF collision;
  CollisionKind collisionKind;
  PhysicsCategoryFilter categoryFilter;
};

DataStream& operator>>(DataStream& ds, PhysicsMovingCollision& pmc);
DataStream& operator<<(DataStream& ds, PhysicsMovingCollision const& pmc);

struct MovingCollisionId {
  MovingCollisionId() = default;
  MovingCollisionId(EntityId physicsEntityId, size_t collisionIndex);

  [[nodiscard]] bool operator==(MovingCollisionId const& rhs) const;

  // Returns true if the MovingCollisionId is not empty, i.e. default
  // constructed
  [[nodiscard]] bool valid() const;
  [[nodiscard]] explicit operator bool() const;

  EntityId physicsEntityId = NullEntityId;
  size_t collisionIndex = 0;
};

DataStream& operator>>(DataStream& ds, MovingCollisionId& mci);
DataStream& operator<<(DataStream& ds, MovingCollisionId const& mci);

class PhysicsEntity : public virtual Entity {
public:
  [[nodiscard]] virtual List<PhysicsForceRegion> forceRegions() const;

  [[nodiscard]] virtual size_t movingCollisionCount() const;
  [[nodiscard]] virtual Maybe<PhysicsMovingCollision> movingCollision(size_t positionIndex) const;
};

}

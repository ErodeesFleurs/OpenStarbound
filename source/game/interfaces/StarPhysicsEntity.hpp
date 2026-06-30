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
  static PhysicsMovingCollision fromJson(Json const& json);

  RectF boundBox() const;

  void translate(Vec2F const& pos);

  bool operator==(PhysicsMovingCollision const& rhs) const;

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

  bool operator==(MovingCollisionId const& rhs) const;

  // Returns true if the MovingCollisionId is not empty, i.e. default
  // constructed
  bool valid() const;
  explicit operator bool() const;

  EntityId physicsEntityId = NullEntityId;
  size_t collisionIndex = 0;
};

DataStream& operator>>(DataStream& ds, MovingCollisionId& mci);
DataStream& operator<<(DataStream& ds, MovingCollisionId const& mci);

class PhysicsEntity : public virtual Entity {
public:
  virtual List<PhysicsForceRegion> forceRegions() const;

  virtual size_t movingCollisionCount() const;
  virtual Maybe<PhysicsMovingCollision> movingCollision(size_t positionIndex) const;
};

}

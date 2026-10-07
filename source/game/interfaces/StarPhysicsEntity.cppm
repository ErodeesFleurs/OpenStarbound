module;
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarList.hpp"


import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.collision_block;

import star.force_regions;

namespace Star {

STAR_CLASS(PhysicsEntity);

struct PhysicsMovingCollision {
  friend DataStream& operator>>(DataStream& ds, PhysicsMovingCollision& pmc);
  friend DataStream& operator<<(DataStream& ds, PhysicsMovingCollision const& pmc);
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
  friend DataStream& operator>>(DataStream& ds, MovingCollisionId& mci);
  friend DataStream& operator<<(DataStream& ds, MovingCollisionId const& mci);
  MovingCollisionId();
  MovingCollisionId(EntityId physicsEntityId, size_t collisionIndex);

  bool operator==(MovingCollisionId const& rhs);

  // Returns true if the MovingCollisionId is not empty, i.e. default
  // constructed
  bool valid() const;
  operator bool() const;

  EntityId physicsEntityId;
  size_t collisionIndex;
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

export module star.physics_entity;

export namespace Star {
  using ::Star::PhysicsEntity;
  using ::Star::PhysicsEntityPtr;
  using ::Star::PhysicsEntityConstPtr;
  using ::Star::PhysicsEntityWeakPtr;
  using ::Star::PhysicsEntityConstWeakPtr;
  using ::Star::PhysicsEntityUPtr;
  using ::Star::PhysicsEntityConstUPtr;
  using ::Star::PhysicsMovingCollision;
  using ::Star::MovingCollisionId;
}

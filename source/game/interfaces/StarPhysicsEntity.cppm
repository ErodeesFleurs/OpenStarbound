module;
import std;
import star.type_identity;
import star.json;
import star.id_map;
import star.poly;
import star.casting;
import star.vector;
import star.data_stream;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.list;


import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.collision_block;

import star.force_regions;

import star.rect;
namespace Star {

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
  MovingCollisionId(EntityId physicsEntityId, std::size_t collisionIndex);

  bool operator==(MovingCollisionId const& rhs);

  // Returns true if the MovingCollisionId is not empty, i.e. default
  // constructed
  bool valid() const;
  operator bool() const;

  EntityId physicsEntityId;
  std::size_t collisionIndex;
};

DataStream& operator>>(DataStream& ds, MovingCollisionId& mci);
DataStream& operator<<(DataStream& ds, MovingCollisionId const& mci);

class PhysicsEntity : public virtual Entity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<PhysicsEntity, Entity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<PhysicsEntity>();
  }

  virtual List<PhysicsForceRegion> forceRegions() const;

  virtual std::size_t movingCollisionCount() const;
  virtual std::optional<PhysicsMovingCollision> movingCollision(std::size_t positionIndex) const;
};

}

export module star.physics_entity;

export namespace Star {
  using ::Star::PhysicsEntity;
  using ::Star::PhysicsMovingCollision;
  using ::Star::MovingCollisionId;
}

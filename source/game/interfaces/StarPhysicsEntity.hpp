#pragma once

#include "StarCollisionBlock.hpp"
#include "StarEntity.hpp"
#include "StarForceRegions.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"

import std;

namespace Star {

struct PhysicsMovingCollision {
  static auto fromJson(Json const& json) -> PhysicsMovingCollision;

  [[nodiscard]] auto boundBox() const -> RectF;

  void translate(Vec2F const& pos);

  auto operator==(PhysicsMovingCollision const& rhs) const -> bool;

  Vec2F position;
  PolyF collision;
  CollisionKind collisionKind;
  PhysicsCategoryFilter categoryFilter;
};

auto operator>>(DataStream& ds, PhysicsMovingCollision& pmc) -> DataStream&;
auto operator<<(DataStream& ds, PhysicsMovingCollision const& pmc) -> DataStream&;

struct MovingCollisionId {
  MovingCollisionId();
  MovingCollisionId(EntityId physicsEntityId, std::size_t collisionIndex);

  auto operator==(MovingCollisionId const& rhs) -> bool;

  // Returns true if the MovingCollisionId is not empty, i.e. default
  // constructed
  [[nodiscard]] auto valid() const -> bool;
  operator bool() const;

  EntityId physicsEntityId;
  std::size_t collisionIndex;
};

auto operator>>(DataStream& ds, MovingCollisionId& mci) -> DataStream&;
auto operator<<(DataStream& ds, MovingCollisionId const& mci) -> DataStream&;

class PhysicsEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto forceRegions() const -> List<PhysicsForceRegion>;

  [[nodiscard]] virtual auto movingCollisionCount() const -> std::size_t;
  [[nodiscard]] virtual auto movingCollision(std::size_t positionIndex) const -> std::optional<PhysicsMovingCollision>;
};

}// namespace Star

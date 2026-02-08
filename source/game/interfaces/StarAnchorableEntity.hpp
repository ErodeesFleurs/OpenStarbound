#pragma once

#include "StarConfig.hpp"
#include "StarEntity.hpp"

import std;

namespace Star {

struct EntityAnchor {
  virtual ~EntityAnchor() = default;

  Vec2F position;
  // If set, the entity should place the bottom center of its collision poly on
  // the given position at exit
  std::optional<Vec2F> exitBottomPosition;
  Direction direction;
  float angle;
};

struct EntityAnchorState {
  EntityId entityId;
  std::size_t positionIndex;

  auto operator==(EntityAnchorState const& eas) const -> bool;
};

auto operator>>(DataStream& ds, EntityAnchorState& anchorState) -> DataStream&;
auto operator<<(DataStream& ds, EntityAnchorState const& anchorState) -> DataStream&;

class AnchorableEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto anchorCount() const -> std::size_t = 0;
  [[nodiscard]] virtual auto anchor(std::size_t anchorPositionIndex) const -> ConstPtr<EntityAnchor> = 0;
};

}// namespace Star

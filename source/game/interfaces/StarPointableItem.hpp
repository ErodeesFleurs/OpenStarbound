#pragma once

#include "StarDrawable.hpp"
#include "StarGameTypes.hpp"

namespace Star {

class PointableItem {
public:
  virtual ~PointableItem() = default;

  virtual auto getAngleDir(float aimAngle, Direction facingDirection) -> float;
  virtual auto getAngle(float angle) -> float;
  [[nodiscard]] virtual auto drawables() const -> List<Drawable> = 0;
};

}// namespace Star

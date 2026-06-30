#pragma once

#include "StarGameTypes.hpp"
#include "StarDrawable.hpp"

namespace Star {

class PointableItem;

class PointableItem {
public:
  virtual ~PointableItem() = default;

  [[nodiscard]] virtual float getAngleDir(float aimAngle, Direction facingDirection);
  [[nodiscard]] virtual float getAngle(float angle);
  [[nodiscard]] virtual List<Drawable> drawables() const = 0;
};

}

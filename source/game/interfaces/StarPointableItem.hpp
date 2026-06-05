#pragma once

#include "StarGameTypes.hpp"
#include "StarDrawable.hpp"

namespace Star {

class PointableItem;

class PointableItem {
public:
  virtual ~PointableItem() {}

  virtual float getAngleDir(float aimAngle, Direction facingDirection);
  virtual float getAngle(float angle);
  virtual List<Drawable> drawables() const = 0;
};

}

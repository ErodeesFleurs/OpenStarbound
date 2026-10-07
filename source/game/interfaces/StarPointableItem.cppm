module;
#include "StarGameTypes.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"

import star.drawable;

namespace Star {

STAR_CLASS(PointableItem);

class PointableItem {
public:
  virtual ~PointableItem() {}

  virtual float getAngleDir(float aimAngle, Direction facingDirection);
  virtual float getAngle(float angle);
  virtual List<Drawable> drawables() const = 0;
};

}

export module star.pointable_item;

export namespace Star {
  using ::Star::PointableItem;
  using ::Star::PointableItemPtr;
  using ::Star::PointableItemConstPtr;
  using ::Star::PointableItemWeakPtr;
  using ::Star::PointableItemConstWeakPtr;
  using ::Star::PointableItemUPtr;
  using ::Star::PointableItemConstUPtr;
}

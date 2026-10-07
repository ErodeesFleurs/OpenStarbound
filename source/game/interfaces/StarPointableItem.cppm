module;
#include "StarGameTypes.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;

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

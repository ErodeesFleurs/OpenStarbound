module;
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

STAR_CLASS(NonRotatedDrawablesItem);

class NonRotatedDrawablesItem {
public:
  virtual ~NonRotatedDrawablesItem() {}
  virtual List<Drawable> nonRotatedDrawables() const = 0;
};

}

export module star.non_rotated_drawables_item;

export namespace Star {
  using ::Star::NonRotatedDrawablesItem;
  using ::Star::NonRotatedDrawablesItemPtr;
  using ::Star::NonRotatedDrawablesItemConstPtr;
  using ::Star::NonRotatedDrawablesItemWeakPtr;
  using ::Star::NonRotatedDrawablesItemConstWeakPtr;
  using ::Star::NonRotatedDrawablesItemUPtr;
  using ::Star::NonRotatedDrawablesItemConstUPtr;
}

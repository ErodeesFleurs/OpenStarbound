module;
#include "StarConfig.hpp"
import star.json;
import star.string;
import star.data_stream;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
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

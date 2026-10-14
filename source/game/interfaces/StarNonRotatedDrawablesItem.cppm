module;
import std;
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

class NonRotatedDrawablesItem;
using NonRotatedDrawablesItemPtr = std::shared_ptr<NonRotatedDrawablesItem>;
using NonRotatedDrawablesItemConstPtr = std::shared_ptr<const NonRotatedDrawablesItem>;
using NonRotatedDrawablesItemWeakPtr = std::weak_ptr<NonRotatedDrawablesItem>;
using NonRotatedDrawablesItemConstWeakPtr = std::weak_ptr<const NonRotatedDrawablesItem>;
using NonRotatedDrawablesItemUPtr = std::unique_ptr<NonRotatedDrawablesItem>;
using NonRotatedDrawablesItemConstUPtr = std::unique_ptr<const NonRotatedDrawablesItem>;

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

module;
import std;
import star.json;
import star.game_types;
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

class PointableItem;
using PointableItemPtr = std::shared_ptr<PointableItem>;
using PointableItemConstPtr = std::shared_ptr<const PointableItem>;
using PointableItemWeakPtr = std::weak_ptr<PointableItem>;
using PointableItemConstWeakPtr = std::weak_ptr<const PointableItem>;
using PointableItemUPtr = std::unique_ptr<PointableItem>;
using PointableItemConstUPtr = std::unique_ptr<const PointableItem>;

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

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
}

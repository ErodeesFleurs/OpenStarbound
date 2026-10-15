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

class Player;

class PreviewableItem {
public:
  virtual ~PreviewableItem() {}
  virtual List<Drawable> preview(std::shared_ptr<Player> const& viewer = {}) const = 0;
};

}

export module star.previewable_item;

export namespace Star {
  using ::Star::Player;
  using ::Star::PreviewableItem;
}

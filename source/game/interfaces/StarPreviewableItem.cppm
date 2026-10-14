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

STAR_CLASS(Player);
STAR_CLASS(PreviewableItem);

class PreviewableItem {
public:
  virtual ~PreviewableItem() {}
  virtual List<Drawable> preview(PlayerPtr const& viewer = {}) const = 0;
};

}

export module star.previewable_item;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::PreviewableItem;
  using ::Star::PreviewableItemPtr;
  using ::Star::PreviewableItemConstPtr;
  using ::Star::PreviewableItemWeakPtr;
  using ::Star::PreviewableItemConstWeakPtr;
  using ::Star::PreviewableItemUPtr;
  using ::Star::PreviewableItemConstUPtr;
}

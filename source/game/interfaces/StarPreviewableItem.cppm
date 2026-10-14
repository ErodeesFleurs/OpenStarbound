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
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<const Player>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<const Player>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<const Player>;
class PreviewableItem;
using PreviewableItemPtr = std::shared_ptr<PreviewableItem>;
using PreviewableItemConstPtr = std::shared_ptr<const PreviewableItem>;
using PreviewableItemWeakPtr = std::weak_ptr<PreviewableItem>;
using PreviewableItemConstWeakPtr = std::weak_ptr<const PreviewableItem>;
using PreviewableItemUPtr = std::unique_ptr<PreviewableItem>;
using PreviewableItemConstUPtr = std::unique_ptr<const PreviewableItem>;

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

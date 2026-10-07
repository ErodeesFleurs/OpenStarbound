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

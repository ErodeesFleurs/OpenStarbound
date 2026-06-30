#pragma once

#include "StarDrawable.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class PreviewableItem;

class PreviewableItem {
public:
  virtual ~PreviewableItem() {}
  virtual List<Drawable> preview(PlayerPtr const& viewer = {}) const = 0;
};

}

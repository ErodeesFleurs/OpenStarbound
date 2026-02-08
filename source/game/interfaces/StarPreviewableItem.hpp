#pragma once

#include "StarConfig.hpp"
#include "StarDrawable.hpp"

namespace Star {

class Player;
// STAR_CLASS(PreviewableItem);

class PreviewableItem {
public:
  virtual ~PreviewableItem() = default;
  [[nodiscard]] virtual auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> = 0;
};

}// namespace Star

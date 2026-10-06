#pragma once

#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
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

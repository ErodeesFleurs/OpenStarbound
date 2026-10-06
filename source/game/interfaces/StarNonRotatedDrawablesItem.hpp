#pragma once

#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;

namespace Star {

STAR_CLASS(NonRotatedDrawablesItem);

class NonRotatedDrawablesItem {
public:
  virtual ~NonRotatedDrawablesItem() {}
  virtual List<Drawable> nonRotatedDrawables() const = 0;
};

}

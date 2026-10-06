#pragma once

#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
import star.drawable;
#include "StarEntity.hpp"

namespace Star {

STAR_CLASS(PortraitEntity);

class PortraitEntity : public virtual Entity {
public:
  virtual List<Drawable> portrait(PortraitMode mode) const = 0;
};

}

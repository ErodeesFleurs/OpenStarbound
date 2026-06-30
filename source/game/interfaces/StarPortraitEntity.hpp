#pragma once

#include "StarDrawable.hpp"
#include "StarEntity.hpp"

namespace Star {

class PortraitEntity;
using PortraitEntityPtr = SharedPtr<PortraitEntity>;

class PortraitEntity : public virtual Entity {
public:
  virtual List<Drawable> portrait(PortraitMode mode) const = 0;
};

}

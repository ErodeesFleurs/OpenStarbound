#pragma once

#include "StarDrawable.hpp"
#include "StarEntity.hpp"

namespace Star {

class PortraitEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto portrait(PortraitMode mode) const -> List<Drawable> = 0;
};

}// namespace Star

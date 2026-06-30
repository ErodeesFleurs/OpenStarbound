#pragma once

#include "StarDrawable.hpp"

namespace Star {

class NonRotatedDrawablesItem;

class NonRotatedDrawablesItem {
public:
  virtual ~NonRotatedDrawablesItem() = default;
  [[nodiscard]] virtual List<Drawable> nonRotatedDrawables() const = 0;
};

}

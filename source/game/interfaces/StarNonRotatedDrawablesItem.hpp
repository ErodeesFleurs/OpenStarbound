#pragma once

#include "StarDrawable.hpp"

namespace Star {

class NonRotatedDrawablesItem {
public:
  virtual ~NonRotatedDrawablesItem() = default;
  [[nodiscard]] virtual auto nonRotatedDrawables() const -> List<Drawable> = 0;
};

}// namespace Star

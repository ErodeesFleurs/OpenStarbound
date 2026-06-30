#pragma once

#include "StarString.hpp"

namespace Star {

class EffectSourceItem;

class EffectSourceItem {
public:
  virtual ~EffectSourceItem() = default;
  [[nodiscard]] virtual StringSet effectSources() const = 0;
};

}

#pragma once

#include "StarString.hpp"

namespace Star {

class EffectSourceItem {
public:
  virtual ~EffectSourceItem() = default;
  [[nodiscard]] virtual auto effectSources() const -> StringSet = 0;
};

}// namespace Star

#pragma once

#include "StarString.hpp"

namespace Star {

class EffectSourceItem;

class EffectSourceItem {
public:
  virtual ~EffectSourceItem() {}
  virtual StringSet effectSources() const = 0;
};

}

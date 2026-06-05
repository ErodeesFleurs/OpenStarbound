#pragma once

#include "StarStatusTypes.hpp"

namespace Star {

class StatusEffectItem;

class StatusEffectItem {
public:
  virtual ~StatusEffectItem() {}
  virtual List<PersistentStatusEffect> statusEffects() const = 0;
};

}

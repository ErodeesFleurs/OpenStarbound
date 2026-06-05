#pragma once

#include "StarEntity.hpp"

namespace Star {

class StatusEffectEntity;

class StatusEffectEntity : public virtual Entity {
public:
  virtual List<PersistentStatusEffect> statusEffects() const = 0;
  virtual PolyF statusEffectArea() const = 0;
};

}

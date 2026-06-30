#pragma once

#include "StarEntity.hpp"

namespace Star {

class StatusEffectEntity;

class StatusEffectEntity : public virtual Entity {
public:
  [[nodiscard]] virtual List<PersistentStatusEffect> statusEffects() const = 0;
  [[nodiscard]] virtual PolyF statusEffectArea() const = 0;
};

}

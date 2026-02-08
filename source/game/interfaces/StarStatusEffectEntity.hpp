#pragma once

#include "StarEntity.hpp"

namespace Star {

class StatusEffectEntity : public virtual Entity {
public:
  [[nodiscard]] virtual auto statusEffects() const -> List<PersistentStatusEffect> = 0;
  [[nodiscard]] virtual auto statusEffectArea() const -> PolyF = 0;
};

}// namespace Star

#pragma once

#include "StarStatusTypes.hpp"

namespace Star {

class StatusEffectItem {
public:
  virtual ~StatusEffectItem() = default;
  [[nodiscard]] virtual auto statusEffects() const -> List<PersistentStatusEffect> = 0;
};

}// namespace Star

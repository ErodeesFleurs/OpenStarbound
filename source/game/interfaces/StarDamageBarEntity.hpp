#pragma once

#include "StarPortraitEntity.hpp"

import std;

namespace Star {

enum class DamageBarType : std::uint8_t {
  Default,
  None,
  Special
};
extern EnumMap<DamageBarType> const DamageBarTypeNames;

class DamageBarEntity : public virtual PortraitEntity {
public:
  [[nodiscard]] virtual auto health() const -> float = 0;
  [[nodiscard]] virtual auto maxHealth() const -> float = 0;
  [[nodiscard]] virtual auto damageBar() const -> DamageBarType = 0;
};

}// namespace Star

#pragma once

#include "StarPortraitEntity.hpp"

namespace Star {

class DamageBarEntity;
using DamageBarEntityPtr = SharedPtr<DamageBarEntity>;

enum class DamageBarType : uint8_t {
  Default,
  None,
  Special
};
extern EnumMap<DamageBarType> const DamageBarTypeNames;

class DamageBarEntity : public virtual PortraitEntity {
public:
  [[nodiscard]] virtual float health() const = 0;
  [[nodiscard]] virtual float maxHealth() const = 0;
  [[nodiscard]] virtual DamageBarType damageBar() const = 0;
};

}

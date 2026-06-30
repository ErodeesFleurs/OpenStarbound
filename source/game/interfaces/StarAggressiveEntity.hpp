#pragma once

#include "StarEntity.hpp"

namespace Star {

class AggressiveEntity;

class AggressiveEntity : public virtual Entity {
public:
  [[nodiscard]] virtual bool aggressive() const = 0;
};

}

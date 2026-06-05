#pragma once

#include "StarEntity.hpp"

namespace Star {

class AggressiveEntity;

class AggressiveEntity : public virtual Entity {
public:
  virtual bool aggressive() const = 0;
};

}

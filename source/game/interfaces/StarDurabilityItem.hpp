#pragma once

#include "StarConfig.hpp"

namespace Star {

class DurabilityItem;

class DurabilityItem {
public:
  virtual ~DurabilityItem() = default;
  virtual float durabilityStatus() = 0;
};

}

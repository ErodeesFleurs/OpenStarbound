#pragma once

#include "StarEntity.hpp"

namespace Star {

// Interface for tool hand positioning and item access.
// Extracted from ToolUserEntity.
class ToolHandInterface {
public:
  virtual ~ToolHandInterface() = default;

  virtual Vec2F armPosition(ToolHand hand, Direction facingDirection, float armAngle, Vec2F offset = {}) const = 0;
  virtual Vec2F handOffset(ToolHand hand, Direction facingDirection) const = 0;
  virtual Vec2F handPosition(ToolHand hand, Vec2F const& handOffset = Vec2F()) const = 0;
  virtual Vec2F armAdjustment() const = 0;

  virtual Vec2F aimPosition() const = 0;
};

}

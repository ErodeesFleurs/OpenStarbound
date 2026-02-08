#pragma once

#include "StarFireableItem.hpp"

import std;

namespace Star {

class SwingableItem : public FireableItem {
public:
  SwingableItem();
  SwingableItem(Json const& params);
  ~SwingableItem() override = default;

  // These can be different
  // Default implementation is the same though
  virtual auto getAngleDir(float aimAngle, Direction facingDirection) -> float;
  virtual auto getAngle(float aimAngle) -> float;
  virtual auto getItemAngle(float aimAngle) -> float;
  virtual auto getArmFrame() -> String;

  virtual auto drawables() const -> List<Drawable> = 0;

  void setParams(Json const& params);

protected:
  float m_swingStart;
  float m_swingFinish;
  float m_swingAimFactor;
  std::optional<float> m_coolingDownAngle;
};

}// namespace Star

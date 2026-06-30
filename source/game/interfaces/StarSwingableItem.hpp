#pragma once

#include "StarFireableItem.hpp"

namespace Star {

class SwingableItem;

class SwingableItem : public FireableItem {
public:
  SwingableItem() = default;
  SwingableItem(Json const& params);
  virtual ~SwingableItem() = default;

  // These can be different
  // Default implementation is the same though
  [[nodiscard]] virtual float getAngleDir(float aimAngle, Direction facingDirection);
  [[nodiscard]] virtual float getAngle(float aimAngle);
  [[nodiscard]] virtual float getItemAngle(float aimAngle);
  [[nodiscard]] virtual String getArmFrame();

  [[nodiscard]] virtual List<Drawable> drawables() const = 0;

  void setParams(Json const& params);

protected:
  float m_swingStart = 0.0f;
  float m_swingFinish = 0.0f;
  float m_swingAimFactor = 0.0f;
  Maybe<float> m_coolingDownAngle;
};

}

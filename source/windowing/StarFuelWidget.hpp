#pragma once

#include "StarWidget.hpp"

namespace Star {

class FuelWidget;
using FuelWidgetPtr = SharedPtr<FuelWidget>;

class FuelWidget : public Widget {
public:
  explicit FuelWidget(GuiContext& context);
  virtual ~FuelWidget() = default;

  virtual void update(float dt) override;

  void setCurrentFuelLevel(float amount);
  void setMaxFuelLevel(float amount);
  void setPotentialFuelAmount(float amount);
  void setRequestedFuelAmount(float amount);

  void ping();

protected:
  virtual void renderImpl() override;

  float m_fuelLevel = 0.0f;
  float m_maxLevel = 0.0f;
  float m_potential = 0.0f;
  float m_requested = 0.0f;

  float m_pingTimeout = 0.0f;

  TextStyle m_textStyle;

private:
};

}

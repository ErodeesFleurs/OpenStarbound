#pragma once

#include "StarWidget.hpp"

namespace Star {

class ProgressWidget;
using ProgressWidgetPtr = SharedPtr<ProgressWidget>;
class ProgressWidget : public Widget {
public:
  ProgressWidget(GuiContext& context,
      String const& background,
      String const& overlay,
      ImageStretchSet const& progressSet,
      GuiDirection direction);
  virtual ~ProgressWidget() = default;

  void setCurrentProgressLevel(float amount);
  void setMaxProgressLevel(float amount);

  void setColor(Color const& color);
  void setOverlay(String const& overlay);

protected:
  virtual void renderImpl() override;
  RectI shift(float begin, float end, RectI templ);

  float m_progressLevel = 0.0f;
  float m_maxLevel = 1.0f;

  Color m_color = Color::White;

  String m_background;
  String m_overlay;
  ImageStretchSet m_bar;
  GuiDirection m_direction;

private:
};

}

#pragma once

#include "StarWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarImageWidget.hpp"

namespace Star {

// This class only does left right right now.  If you need advanced
// multiorientation physics please
// implement it in.
class SliderBarWidget : public Widget {
public:
  SliderBarWidget(GuiContext& context, String const& grid, bool showSpinner = true);

  void setJogImages(String const& baseImage, String const& hoverImage = "", String const& pressedImage = "", String const& disabledImage = "");

  void setRange(int low, int high, int delta);
  void setRange(Vec2I const& range, int delta);
  void setVal(int val, bool callbackIfChanged = true);
  [[nodiscard]] int val() const;

  void setEnabled(bool enabled);

  void setCallback(WidgetCallbackFunc callback);

  void update(float dt) override;

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;

private:
  void leftCallback();
  void rightCallback();

  ButtonWidgetPtr m_leftButton;
  ButtonWidgetPtr m_rightButton;
  ImageWidgetPtr m_grid;
  ButtonWidgetPtr m_jog;
  int m_low = 0;
  int m_high = 1;
  int m_delta = 1;
  int m_val = 0;

  bool m_updateJog = true;

  Vec2I m_savedJogPos;
  Vec2I m_jogDragPos;
  bool m_jogDragActive = false;

  bool m_enabled = true;

  WidgetCallbackFunc m_callback;
};
using SliderBarWidgetPtr = shared_ptr<SliderBarWidget>;
}

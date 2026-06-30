#pragma once

#include "StarWidget.hpp"
#include "StarButtonWidget.hpp"
#include "StarImageWidget.hpp"

namespace Star {

class ScrollThumb : public Widget {
public:
  ScrollThumb(GuiContext& context, GuiDirection direction);
  virtual ~ScrollThumb() = default;

  void setDirection(GuiDirection direction);
  void setImages(ImageStretchSet const& base,
      ImageStretchSet const& hover = ImageStretchSet(),
      ImageStretchSet const& pressed = ImageStretchSet());
  void setImages(Json const& images);

  [[nodiscard]] bool isHovered() const;
  [[nodiscard]] bool isPressed() const;

  void setHovered(bool hovered);
  void setPressed(bool pressed);

  void mouseOver() override;
  void mouseOut() override;

  [[nodiscard]] Vec2U baseSize() const;

protected:
  void renderImpl() override;

private:
  void readDefaults();

  GuiDirection m_direction;

  ImageStretchSet m_baseThumb;
  ImageStretchSet m_hoverThumb;
  ImageStretchSet m_pressedThumb;

  bool m_hovered = false;
  bool m_pressed = false;
};
using ScrollThumbPtr = shared_ptr<ScrollThumb>;

class ScrollBar : public Widget {
public:
  ScrollBar(GuiContext& context, GuiDirection direction, WidgetCallbackFunc forwardFunc, WidgetCallbackFunc backwardFunc);

  void setButtonImages(Json const& images);

  [[nodiscard]] int trackSize() const;
  [[nodiscard]] float sizeRatio() const;
  [[nodiscard]] Vec2I size() const override;
  [[nodiscard]] float scrollRatio() const;
  [[nodiscard]] Vec2I offsetFromThumbPosition(Vec2I const& thumbPosition) const;

  [[nodiscard]] ButtonWidgetPtr forwardButton() const;
  [[nodiscard]] ButtonWidgetPtr backwardButton() const;
  [[nodiscard]] ScrollThumbPtr thumb() const;

protected:
  void drawChildren() override;

private:
  GuiDirection m_direction;

  ButtonWidgetPtr m_forward; // up or right, makes the offset higher
  ButtonWidgetPtr m_backward; // down or left, makes the offset lower
  ScrollThumbPtr m_thumb;

  ImageStretchSet m_track;
};
using ScrollBarPtr = shared_ptr<ScrollBar>;

class ScrollArea : public Widget {
public:
  explicit ScrollArea(GuiContext& context);

  void setButtonImages(Json const& images);
  void setThumbImages(Json const& images);

  [[nodiscard]] RectI contentBoundRect() const;
  [[nodiscard]] Vec2I contentSize() const;

  [[nodiscard]] Vec2I areaSize() const;

  void scrollAreaBy(Vec2I const& offset);

  [[nodiscard]] Vec2I scrollOffset() const;
  [[nodiscard]] Vec2I maxScrollPosition() const;

  [[nodiscard]] bool horizontalScroll() const;
  void setHorizontalScroll(bool horizontal);

  [[nodiscard]] bool verticalScroll() const;
  void setVerticalScroll(bool vertical);

  void setUpdatesChildren(bool slop);

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;
  void update(float dt) override;

protected:
  void drawChildren() override;

private:
  [[nodiscard]] int advanceFactorHelper();

  int m_buttonAdvance;
  int64_t m_advanceLimiter;

  Vec2I m_scrollOffset;
  Vec2I m_lastMaxScroll;
  Vec2I m_contentSize;

  bool m_dragActive = false;
  GuiDirection m_dragDirection;
  Vec2I m_dragOffset;

  ScrollBarPtr m_vBar;
  ScrollBarPtr m_hBar;
  ImageWidgetPtr m_cornerBlock;

  bool m_horizontalScroll = false;
  bool m_verticalScroll = true;

  bool m_updatesChildren = false;
};
using ScrollAreaPtr = shared_ptr<ScrollArea>;
}

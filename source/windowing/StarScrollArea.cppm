module;
import std;
import star.json;

import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.rect;
import star.application_controller;
import star.color;
import star.font;
import star.variant;
import star.bi_map;
import star.ref_ptr;
import star.renderer;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.button_group;
import star.button_widget;
import star.image_widget;

namespace Star {

class ScrollThumb : public Widget {
public:
  ScrollThumb(GuiDirection direction);
  virtual ~ScrollThumb() {}

  void setDirection(GuiDirection direction);
  void setImages(ImageStretchSet const& base,
      ImageStretchSet const& hover = ImageStretchSet(),
      ImageStretchSet const& pressed = ImageStretchSet());
  void setImages(Json const& images);

  bool isHovered() const;
  bool isPressed() const;

  void setHovered(bool hovered);
  void setPressed(bool pressed);

  void mouseOver() override;
  void mouseOut() override;

  Vec2U baseSize() const;

protected:
  virtual void renderImpl() override;

private:
  void readDefaults();

  GuiDirection m_direction;

  ImageStretchSet m_baseThumb;
  ImageStretchSet m_hoverThumb;
  ImageStretchSet m_pressedThumb;

  bool m_hovered;
  bool m_pressed;
};
typedef std::shared_ptr<ScrollThumb> ScrollThumbPtr;

class ScrollBar : public Widget {
public:
  ScrollBar(GuiDirection direction, WidgetCallbackFunc forwardFunc, WidgetCallbackFunc backwardFunc);

  void setButtonImages(Json const& images);

  int trackSize() const;
  float sizeRatio() const;
  virtual Vec2I size() const override;
  float scrollRatio() const;
  Vec2I offsetFromThumbPosition(Vec2I const& thumbPosition) const;

  ButtonWidgetPtr forwardButton() const;
  ButtonWidgetPtr backwardButton() const;
  ScrollThumbPtr thumb() const;

protected:
  void drawChildren() override;

private:
  GuiDirection m_direction;

  ButtonWidgetPtr m_forward; // up or right, makes the offset higher
  ButtonWidgetPtr m_backward; // down or left, makes the offset lower
  ScrollThumbPtr m_thumb;

  ImageStretchSet m_track;
};
typedef std::shared_ptr<ScrollBar> ScrollBarPtr;

class ScrollArea : public Widget {
public:
  ScrollArea();

  void setButtonImages(Json const& images);
  void setThumbImages(Json const& images);

  RectI contentBoundRect() const;
  Vec2I contentSize() const;

  Vec2I areaSize() const;

  void scrollAreaBy(Vec2I const& offset);

  Vec2I scrollOffset() const;
  Vec2I maxScrollPosition() const;

  bool horizontalScroll() const;
  void setHorizontalScroll(bool horizontal);

  bool verticalScroll() const;
  void setVerticalScroll(bool vertical);

  void setUpdatesChildren(bool slop);

  virtual bool sendEvent(InputEvent const& event) override;
  virtual void update(float dt) override;

protected:
  void drawChildren() override;

private:
  int advanceFactorHelper();

  int m_buttonAdvance;
  std::int64_t m_advanceLimiter;

  Vec2I m_scrollOffset;
  Vec2I m_lastMaxScroll;
  Vec2I m_contentSize;

  bool m_dragActive;
  GuiDirection m_dragDirection;
  Vec2I m_dragOffset;

  ScrollBarPtr m_vBar;
  ScrollBarPtr m_hBar;
  ImageWidgetPtr m_cornerBlock;

  bool m_horizontalScroll;
  bool m_verticalScroll;

  bool m_updatesChildren;
};
typedef std::shared_ptr<ScrollArea> ScrollAreaPtr;
}

export module star.scroll_area;

export namespace Star {
  using ::Star::ScrollThumb;
  using ::Star::ScrollBar;
  using ::Star::ScrollArea;
  using ::Star::ScrollThumbPtr;
  using ::Star::ScrollBarPtr;
  using ::Star::ScrollAreaPtr;
}

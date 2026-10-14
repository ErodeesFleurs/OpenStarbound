module;
#include "StarConfig.hpp"
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
import star.layout;

namespace Star {

STAR_CLASS(VerticalLayout);

class VerticalLayout : public Layout {
public:
  VerticalLayout();
  VerticalLayout(VerticalAnchor verticalAnchor, int verticalSpacing = 0);

  void update(float dt) override;
  Vec2I size() const override;
  RectI relativeBoundRect() const override;

  void setHorizontalAnchor(HorizontalAnchor horizontalAnchor);
  void setVerticalAnchor(VerticalAnchor verticalAnchor);
  void setVerticalSpacing(int verticalSpacing);
  void setFillDown(bool fillDown);

private:
  RectI contentBoundRect() const;

  HorizontalAnchor m_horizontalAnchor;
  VerticalAnchor m_verticalAnchor;
  int m_verticalSpacing;
  bool m_fillDown;
  Vec2I m_size;
};

}

export module star.vertical_layout;

export namespace Star {
  using ::Star::VerticalLayout;
  using ::Star::VerticalLayoutPtr;
  using ::Star::VerticalLayoutConstPtr;
  using ::Star::VerticalLayoutWeakPtr;
  using ::Star::VerticalLayoutConstWeakPtr;
  using ::Star::VerticalLayoutUPtr;
  using ::Star::VerticalLayoutConstUPtr;
}

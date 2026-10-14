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
import star.widget_parsing;
import star.gui_reader;

namespace Star {

STAR_CLASS(ListWidget);

class ListWidget : public Widget {
public:
  ListWidget(Json const& schema);
  ListWidget();

  RectI relativeBoundRect() const override;

  // Callback is called when the selection changes
  void setCallback(WidgetCallbackFunc callback);

  bool sendEvent(InputEvent const& event) override;
  void setSchema(Json const& schema);
  WidgetPtr constructWidget();
  WidgetPtr addItem();
  WidgetPtr addItem(size_t at);
  WidgetPtr addItem(WidgetPtr existingItem);
  void removeItem(size_t at);
  void removeItem(WidgetPtr item);
  void clear();
  size_t selectedItem() const;
  size_t itemPosition(WidgetPtr item) const;
  WidgetPtr itemAt(size_t n) const;
  WidgetPtr selectedWidget() const;
  List<WidgetPtr> const& list() const;
  size_t listSize() const;
  void setEnabled(size_t pos, bool enabled);
  void setHovered(size_t pos, bool hovered);
  void setSelected(size_t pos);
  void clearSelected();
  void setSelectedWidget(WidgetPtr selected);

  void registerMemberCallback(String const& name, WidgetCallbackFunc const& callback);

  void setFillDown(bool fillDown);
  void setColumns(uint64_t columns);

private:
  void updateSizeAndPosition();

  Json m_schema;
  GuiReader m_reader;

  Set<size_t> m_disabledItems;
  size_t m_selectedItem;
  WidgetCallbackFunc m_callback;

  String m_selectedBG;
  String m_unselectedBG;
  String m_hoverBG;
  String m_disabledBG;
  Vec2I m_spacing;

  bool m_fillDown;
  uint64_t m_columns;
};

}

export module star.list_widget;

export namespace Star {
  using ::Star::ListWidget;
  using ::Star::ListWidgetPtr;
  using ::Star::ListWidgetConstPtr;
  using ::Star::ListWidgetWeakPtr;
  using ::Star::ListWidgetConstWeakPtr;
  using ::Star::ListWidgetUPtr;
  using ::Star::ListWidgetConstUPtr;
}

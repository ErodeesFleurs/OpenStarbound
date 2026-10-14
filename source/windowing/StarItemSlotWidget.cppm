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
import star.progress_widget;
import star.animation;

namespace Star {

class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<Item const>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<Item const>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<Item const>;
class ItemSlotWidget;
using ItemSlotWidgetPtr = std::shared_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstPtr = std::shared_ptr<ItemSlotWidget const>;
using ItemSlotWidgetWeakPtr = std::weak_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstWeakPtr = std::weak_ptr<ItemSlotWidget const>;
using ItemSlotWidgetUPtr = std::unique_ptr<ItemSlotWidget>;
using ItemSlotWidgetConstUPtr = std::unique_ptr<ItemSlotWidget const>;

inline constexpr float ItemIndicateNewTime = 1.5f;

class ItemSlotWidget : public Widget {
public:
  ItemSlotWidget(ItemPtr const& item, String const& backingImage);

  virtual void update(float dt) override;
  bool sendEvent(InputEvent const& event) override;
  void setCallback(WidgetCallbackFunc callback);
  void setRightClickCallback(WidgetCallbackFunc callback);
  void setMiddleClickCallback(WidgetCallbackFunc callback);
  void setItem(ItemPtr const& item);
  ItemPtr item() const;
  void setProgress(float progress);
  void setBackingImageAffinity(bool full, bool empty);
  void setCountPosition(TextPositioning textPositioning);
  void setCountFontMode(FontMode fontMode);

  void showDurability(bool show);
  void showCount(bool show);
  void showRarity(bool showRarity);
  void showLinkIndicator(bool showLinkIndicator);
  void showSecondaryIcon(bool show);

  void indicateNew();

  void setHighlightEnabled(bool highlight);

protected:
  virtual void renderImpl() override;

private:
  ItemPtr m_item;

  String m_backingImage;
  bool m_drawBackingImageWhenFull;
  bool m_drawBackingImageWhenEmpty;
  bool m_showDurability;
  bool m_showCount;
  bool m_showRarity;
  bool m_showLinkIndicator;
  bool m_showSecondaryIcon;

  TextPositioning m_countPosition;
  FontMode m_countFontMode;

  Vec2I m_durabilityOffset;
  RectI m_itemDraggableArea;

  TextStyle m_textStyle;

  WidgetCallbackFunc m_callback;
  WidgetCallbackFunc m_rightClickCallback;
  WidgetCallbackFunc m_middleClickCallback;
  float m_progress;

  ProgressWidgetPtr m_durabilityBar;

  Animation m_newItemIndicator;

  bool m_highlightEnabled;
  Animation m_highlightAnimation;
};

}

export module star.item_slot_widget;

export namespace Star {
  using ::Star::ItemSlotWidget;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::ItemSlotWidgetPtr;
  using ::Star::ItemSlotWidgetConstPtr;
  using ::Star::ItemSlotWidgetWeakPtr;
  using ::Star::ItemSlotWidgetConstWeakPtr;
  using ::Star::ItemSlotWidgetUPtr;
  using ::Star::ItemSlotWidgetConstUPtr;
  using ::Star::ItemIndicateNewTime;
}

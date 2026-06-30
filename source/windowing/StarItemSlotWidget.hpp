#pragma once

#include "StarWidget.hpp"
#include "StarProgressWidget.hpp"
#include "StarAnimation.hpp"

namespace Star {

class Item;
using ItemPtr = SharedPtr<Item>;
class ItemSlotWidget;
using ItemSlotWidgetPtr = SharedPtr<ItemSlotWidget>;

static constexpr float ItemIndicateNewTime = 1.5f;

class ItemSlotWidget : public Widget {
public:
  ItemSlotWidget(GuiContext& context, ItemPtr const& item, String const& backingImage);

  void update(float dt) override;
  [[nodiscard]] bool sendEvent(InputEvent const& event) override;
  void setCallback(WidgetCallbackFunc callback);
  void setRightClickCallback(WidgetCallbackFunc callback);
  void setMiddleClickCallback(WidgetCallbackFunc callback);
  void setItem(ItemPtr const& item);
  [[nodiscard]] ItemPtr item() const;
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
  void renderImpl() override;

private:
  ItemPtr m_item;

  String m_backingImage;
  bool m_drawBackingImageWhenFull = false;
  bool m_drawBackingImageWhenEmpty = true;
  bool m_showDurability = false;
  bool m_showCount = true;
  bool m_showRarity = true;
  bool m_showLinkIndicator = false;
  bool m_showSecondaryIcon = false;

  TextPositioning m_countPosition;
  FontMode m_countFontMode;

  Vec2I m_durabilityOffset;
  RectI m_itemDraggableArea;

  TextStyle m_textStyle;

  WidgetCallbackFunc m_callback;
  WidgetCallbackFunc m_rightClickCallback;
  WidgetCallbackFunc m_middleClickCallback;
  float m_progress = 1.0f;

  ProgressWidgetPtr m_durabilityBar;

  Animation m_newItemIndicator;

  bool m_highlightEnabled = false;
  Animation m_highlightAnimation;
};

}

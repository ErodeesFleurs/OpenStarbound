#pragma once

#include "StarItemBag.hpp"
#include "StarWidget.hpp"
#include "StarItemSlotWidget.hpp"
#include "StarItem.hpp"

namespace Star {

class ItemGridWidget;
using ItemGridWidgetPtr = SharedPtr<ItemGridWidget>;

class ItemGridWidget : public Widget {
public:
  ItemGridWidget(GuiContext& context, ItemBagConstPtr bag, Vec2I const& dimensions, Vec2I const& spacing, String const& backingImage, unsigned bagOffset);
  ItemGridWidget(GuiContext& context, ItemBagConstPtr bag, Vec2I const& dimensions, Vec2I const& rowSpacing, Vec2I const& columnSpacing, String const& backingImage, unsigned bagOffset);

  [[nodiscard]] ItemBagConstPtr bag() const;

  [[nodiscard]] ItemPtr itemAt(Vec2I const& position) const;
  [[nodiscard]] ItemPtr itemAt(size_t index) const;
  [[nodiscard]] ItemPtr selectedItem() const;

  [[nodiscard]] ItemSlotWidgetPtr itemWidgetAt(Vec2I const& position) const;
  [[nodiscard]] ItemSlotWidgetPtr itemWidgetAt(size_t index) const;

  // Returns the dimensions of the item grid
  [[nodiscard]] Vec2I dimensions() const;

  // Returns the number of item slots in the grid (dimensions.x() * dimensions.y())
  [[nodiscard]] size_t itemSlots() const;

  // Returns the size of the underlying bag.
  [[nodiscard]] size_t bagSize() const;

  // Returns the min of bagSize() and itemSlots()
  [[nodiscard]] size_t effectiveSize() const;

  [[nodiscard]] size_t bagLocationAt(Vec2I const& position) const;
  [[nodiscard]] Vec2I positionOfSlot(size_t slotNumber);

  [[nodiscard]] bool sendEvent(InputEvent const& event) override;
  void setCallback(WidgetCallbackFunc callback);
  void setRightClickCallback(WidgetCallbackFunc callback);
  void setMiddleClickCallback(WidgetCallbackFunc callback);
  void setItemBag(ItemBagConstPtr bag);
  void setProgress(float progress);

  [[nodiscard]] size_t selectedIndex() const;

  void updateAllItemSlots();

  // Item states, keeping track of new items
  void updateItemState();
  void clearChangedSlots();
  [[nodiscard]] bool slotsChanged();
  void indicateChangedSlots();

  void setHighlightEmpty(bool highlight);

  void setBackingImageAffinity(bool full, bool empty);
  void showDurability(bool show);

  [[nodiscard]] RectI getScissorRect() const override;

protected:
  void renderImpl() override;
  [[nodiscard]] HashSet<ItemDescriptor> uniqueItemState();
  [[nodiscard]] List<String> slotItemNames();

private:
  [[nodiscard]] Vec2I locOfItemSlot(unsigned slot) const;

  ItemBagConstPtr m_bag;
  List<ItemSlotWidgetPtr> m_slots;
  unsigned m_bagOffset;
  Vec2I m_dimensions;
  Vec2I m_rowSpacing;
  Vec2I m_columnSpacing;

  List<String> m_itemNames;
  Set<size_t> m_changedSlots;

  RectI m_itemDraggableArea;

  String m_backingImage;
  bool m_drawBackingImageWhenFull = false;
  bool m_drawBackingImageWhenEmpty = true;
  bool m_showDurability = false;

  float m_progress = 1.0f;

  bool m_highlightEmpty = false;

  unsigned m_selectedIndex = 0;
  WidgetCallbackFunc m_callback;
  WidgetCallbackFunc m_rightClickCallback;
  WidgetCallbackFunc m_middleClickCallback;
};

}

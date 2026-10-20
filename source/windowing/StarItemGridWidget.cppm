module;
import std;
import star.type_identity;
import star.json;

import star.vector;
import star.casting;
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
import star.item_slot_widget;
import star.item_descriptor;

namespace Star {

class ItemBag;

class ItemGridWidget : public Widget {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<ItemGridWidget, Widget>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<ItemGridWidget>();
  }

  ItemGridWidget(GuiContext& context, std::shared_ptr<ItemBag const> bag, Vec2I const& dimensions, Vec2I const& spacing, String const& backingImage, unsigned bagOffset);
  ItemGridWidget(GuiContext& context, std::shared_ptr<ItemBag const> bag, Vec2I const& dimensions, Vec2I const& rowSpacing, Vec2I const& columnSpacing, String const& backingImage, unsigned bagOffset);

  std::shared_ptr<ItemBag const> bag() const;

  std::shared_ptr<Item> itemAt(Vec2I const& position) const;
  std::shared_ptr<Item> itemAt(std::size_t index) const;
  std::shared_ptr<Item> selectedItem() const;

  std::shared_ptr<ItemSlotWidget> itemWidgetAt(Vec2I const& position) const;
  std::shared_ptr<ItemSlotWidget> itemWidgetAt(std::size_t index) const;

  // Returns the dimensions of the item grid
  Vec2I dimensions() const;

  // Returns the number of item slots in the grid (dimensions.x() * dimensions.y())
  std::size_t itemSlots() const;

  // Returns the size of the underlying bag.
  std::size_t bagSize() const;

  // Returns the min of bagSize() and itemSlots()
  std::size_t effectiveSize() const;

  std::size_t bagLocationAt(Vec2I const& position) const;
  Vec2I positionOfSlot(std::size_t slotNumber);

  bool sendEvent(InputEvent const& event) override;
  void setCallback(WidgetCallbackFunc callback);
  void setRightClickCallback(WidgetCallbackFunc callback);
  void setMiddleClickCallback(WidgetCallbackFunc callback);
  void setItemBag(std::shared_ptr<ItemBag const> bag);
  void setProgress(float progress);

  std::size_t selectedIndex() const;

  void updateAllItemSlots();

  // Item states, keeping track of new items
  void updateItemState();
  void clearChangedSlots();
  bool slotsChanged();
  void indicateChangedSlots();

  void setHighlightEmpty(bool highlight);

  void setBackingImageAffinity(bool full, bool empty);
  void showDurability(bool show);

  virtual RectI getScissorRect() const override;

protected:
  void renderImpl() override;
  HashSet<ItemDescriptor> uniqueItemState();
  List<String> slotItemNames();

private:
  Vec2I locOfItemSlot(unsigned slot) const;

  std::shared_ptr<ItemBag const> m_bag;
  List<std::shared_ptr<ItemSlotWidget>> m_slots;
  unsigned m_bagOffset;
  Vec2I m_dimensions;
  Vec2I m_rowSpacing;
  Vec2I m_columnSpacing;

  List<String> m_itemNames;
  Set<std::size_t> m_changedSlots;

  RectI m_itemDraggableArea;

  String m_backingImage;
  bool m_drawBackingImageWhenFull;
  bool m_drawBackingImageWhenEmpty;
  bool m_showDurability;

  float m_progress;

  bool m_highlightEmpty;

  unsigned m_selectedIndex;
  WidgetCallbackFunc m_callback;
  WidgetCallbackFunc m_rightClickCallback;
  WidgetCallbackFunc m_middleClickCallback;
};

}

export module star.item_grid_widget;

export namespace Star {
  using ::Star::ItemGridWidget;
  using ::Star::ItemBag;
}

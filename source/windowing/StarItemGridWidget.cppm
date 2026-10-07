module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"
import star.application_controller;
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarVariant.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarRefPtr.hpp"
import star.renderer;
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
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

STAR_CLASS(ItemGridWidget);
STAR_CLASS(ItemBag);

class ItemGridWidget : public Widget {
public:
  ItemGridWidget(ItemBagConstPtr bag, Vec2I const& dimensions, Vec2I const& spacing, String const& backingImage, unsigned bagOffset);
  ItemGridWidget(ItemBagConstPtr bag, Vec2I const& dimensions, Vec2I const& rowSpacing, Vec2I const& columnSpacing, String const& backingImage, unsigned bagOffset);

  ItemBagConstPtr bag() const;

  ItemPtr itemAt(Vec2I const& position) const;
  ItemPtr itemAt(size_t index) const;
  ItemPtr selectedItem() const;

  ItemSlotWidgetPtr itemWidgetAt(Vec2I const& position) const;
  ItemSlotWidgetPtr itemWidgetAt(size_t index) const;

  // Returns the dimensions of the item grid
  Vec2I dimensions() const;

  // Returns the number of item slots in the grid (dimensions.x() * dimensions.y())
  size_t itemSlots() const;

  // Returns the size of the underlying bag.
  size_t bagSize() const;

  // Returns the min of bagSize() and itemSlots()
  size_t effectiveSize() const;

  size_t bagLocationAt(Vec2I const& position) const;
  Vec2I positionOfSlot(size_t slotNumber);

  bool sendEvent(InputEvent const& event) override;
  void setCallback(WidgetCallbackFunc callback);
  void setRightClickCallback(WidgetCallbackFunc callback);
  void setMiddleClickCallback(WidgetCallbackFunc callback);
  void setItemBag(ItemBagConstPtr bag);
  void setProgress(float progress);

  size_t selectedIndex() const;

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
  using ::Star::ItemGridWidgetPtr;
  using ::Star::ItemGridWidgetConstPtr;
  using ::Star::ItemGridWidgetWeakPtr;
  using ::Star::ItemGridWidgetConstWeakPtr;
  using ::Star::ItemGridWidgetUPtr;
  using ::Star::ItemGridWidgetConstUPtr;
  using ::Star::ItemBag;
  using ::Star::ItemBagPtr;
  using ::Star::ItemBagConstPtr;
  using ::Star::ItemBagWeakPtr;
  using ::Star::ItemBagConstWeakPtr;
  using ::Star::ItemBagUPtr;
  using ::Star::ItemBagConstUPtr;
}

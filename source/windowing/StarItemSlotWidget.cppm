module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
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

namespace Star {

STAR_CLASS(Item);
STAR_CLASS(ItemSlotWidget);

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

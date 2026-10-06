module;

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
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
import star.pane;
import star.game_timers;

namespace Star {
STAR_CLASS(Player);
}

namespace Star {

STAR_CLASS(WorldClient);
STAR_CLASS(ItemBag);
STAR_CLASS(ItemGridWidget);
STAR_CLASS(ListWidget);
STAR_CLASS(TextBoxWidget);
STAR_CLASS(ButtonWidget);
STAR_CLASS(LabelWidget);
STAR_CLASS(TabSetWidget);

STAR_CLASS(MerchantPane);

class MerchantPane : public Pane {
public:
  MerchantPane(WorldClientPtr worldClient, PlayerPtr player, Json const& settings, EntityId sourceEntityId = NullEntityId);

  void displayed() override;
  void dismissed() override;
  PanePtr createTooltip(Vec2I const& screenPosition) override;

  EntityId sourceEntityId() const;

  ItemPtr addItems(ItemPtr const& items);

protected:
  void update(float dt) override;

private:
  void swapSlot();

  void buildItemList();
  void setupWidget(WidgetPtr const& widget, Json const& itemConfig);
  void updateSelection();
  int itemPrice();
  void updateBuyTotal();
  void buy();

  void updateSellTotal();
  void sell();

  int maxBuyCount();
  void countChanged();
  void countTextChanged();

  WorldClientPtr m_worldClient;
  PlayerPtr m_player;
  EntityId m_sourceEntityId;
  Json m_settings;

  GameTimer m_refreshTimer;

  JsonArray m_itemList;
  size_t m_selectedIndex;
  ItemPtr m_selectedItem;

  float m_buyFactor;
  int m_buyTotal;
  float m_sellFactor;
  int m_sellTotal;

  TabSetWidgetPtr m_tabSet;
  ListWidgetPtr m_itemGuiList;
  TextBoxWidgetPtr m_countTextBox;
  LabelWidgetPtr m_buyTotalLabel;
  ButtonWidgetPtr m_buyButton;
  LabelWidgetPtr m_sellTotalLabel;
  ButtonWidgetPtr m_sellButton;

  ItemGridWidgetPtr m_itemGrid;
  ItemBagPtr m_itemBag;

  int m_buyCount;
  int m_maxBuyCount;
};

}

export module star.merchant_interface;

export namespace Star {
  using ::Star::WorldClient;
  using ::Star::WorldClientPtr;
  using ::Star::WorldClientConstPtr;
  using ::Star::WorldClientWeakPtr;
  using ::Star::WorldClientConstWeakPtr;
  using ::Star::WorldClientUPtr;
  using ::Star::WorldClientConstUPtr;
  using ::Star::ItemBag;
  using ::Star::ItemBagPtr;
  using ::Star::ItemBagConstPtr;
  using ::Star::ItemBagWeakPtr;
  using ::Star::ItemBagConstWeakPtr;
  using ::Star::ItemBagUPtr;
  using ::Star::ItemBagConstUPtr;
  using ::Star::ItemGridWidget;
  using ::Star::ItemGridWidgetPtr;
  using ::Star::ItemGridWidgetConstPtr;
  using ::Star::ItemGridWidgetWeakPtr;
  using ::Star::ItemGridWidgetConstWeakPtr;
  using ::Star::ItemGridWidgetUPtr;
  using ::Star::ItemGridWidgetConstUPtr;
  using ::Star::ListWidget;
  using ::Star::ListWidgetPtr;
  using ::Star::ListWidgetConstPtr;
  using ::Star::ListWidgetWeakPtr;
  using ::Star::ListWidgetConstWeakPtr;
  using ::Star::ListWidgetUPtr;
  using ::Star::ListWidgetConstUPtr;
  using ::Star::TextBoxWidget;
  using ::Star::TextBoxWidgetPtr;
  using ::Star::TextBoxWidgetConstPtr;
  using ::Star::TextBoxWidgetWeakPtr;
  using ::Star::TextBoxWidgetConstWeakPtr;
  using ::Star::TextBoxWidgetUPtr;
  using ::Star::TextBoxWidgetConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::TabSetWidget;
  using ::Star::TabSetWidgetPtr;
  using ::Star::TabSetWidgetConstPtr;
  using ::Star::TabSetWidgetWeakPtr;
  using ::Star::TabSetWidgetConstWeakPtr;
  using ::Star::TabSetWidgetUPtr;
  using ::Star::TabSetWidgetConstUPtr;
  using ::Star::MerchantPane;
  using ::Star::MerchantPanePtr;
  using ::Star::MerchantPaneConstPtr;
  using ::Star::MerchantPaneWeakPtr;
  using ::Star::MerchantPaneConstWeakPtr;
  using ::Star::MerchantPaneUPtr;
  using ::Star::MerchantPaneConstUPtr;
}

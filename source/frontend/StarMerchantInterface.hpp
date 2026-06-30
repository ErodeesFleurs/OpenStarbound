#pragma once

#include "StarWorldClient.hpp"
#include "StarPane.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"

namespace Star {

class WorldClient;
using WorldClientPtr = SharedPtr<WorldClient>;
class Player;
using PlayerPtr = SharedPtr<Player>;
class ItemBag;
using ItemBagPtr = SharedPtr<ItemBag>;
class ItemGridWidget;
using ItemGridWidgetPtr = SharedPtr<ItemGridWidget>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;
class TextBoxWidget;
using TextBoxWidgetPtr = SharedPtr<TextBoxWidget>;
class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;
class LabelWidget;
using LabelWidgetPtr = SharedPtr<LabelWidget>;
class TabSetWidget;
using TabSetWidgetPtr = SharedPtr<TabSetWidget>;

class MerchantPane;
using MerchantPanePtr = SharedPtr<MerchantPane>;
class ObjectDatabase;
using ObjectDatabaseConstPtr = SharedPtr<ObjectDatabase const>;
class StatusEffectDatabase;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;

struct MerchantPaneServices {
  AssetsConstPtr assets;
  ItemDatabaseConstPtr itemDatabase;
  ObjectDatabaseConstPtr objectDatabase;
  StatusEffectDatabaseConstPtr statusEffectDatabase;
};

class MerchantPane : public Pane {
public:
  MerchantPane(WorldClientPtr worldClient,
      PlayerPtr player,
      Json const& settings,
      EntityId sourceEntityId,
      MerchantPaneServices services);

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
  AssetsConstPtr m_assets;
  ItemDatabaseConstPtr m_itemDatabase;
  ObjectDatabaseConstPtr m_objectDatabase;
  StatusEffectDatabaseConstPtr m_statusEffectDatabase;
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

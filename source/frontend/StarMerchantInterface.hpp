#pragma once

#include "StarObserverPtr.hpp"
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
  GuiContext& guiContext;
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
  [[nodiscard]] UniquePtr<Pane> createTooltip(Vec2I const& screenPosition) override;

  [[nodiscard]] EntityId sourceEntityId() const;

  [[nodiscard]] ItemPtr addItems(ItemPtr const& items);

protected:
  void update(float dt) override;

private:
  void swapSlot();

  void buildItemList();
  void setupWidget(WidgetRef<Widget> const& widget, Json const& itemConfig);
  void updateSelection();
  [[nodiscard]] int itemPrice();
  void updateBuyTotal();
  void buy();

  void updateSellTotal();
  void sell();

  [[nodiscard]] int maxBuyCount();
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

  observer_ptr<TabSetWidget> m_tabSet;
  observer_ptr<ListWidget> m_itemGuiList;
  observer_ptr<TextBoxWidget> m_countTextBox;
  observer_ptr<LabelWidget> m_buyTotalLabel;
  observer_ptr<ButtonWidget> m_buyButton;
  observer_ptr<LabelWidget> m_sellTotalLabel;
  observer_ptr<ButtonWidget> m_sellButton;

  observer_ptr<ItemGridWidget> m_itemGrid;
  ItemBagPtr m_itemBag;

  int m_buyCount;
  int m_maxBuyCount;
};

}

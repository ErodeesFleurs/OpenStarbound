module;
import std;
import star.json;

import star.vector;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
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
import star.pane;
import star.game_timers;

namespace Star {
class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
}

namespace Star {

class WorldClient;
using WorldClientPtr = std::shared_ptr<WorldClient>;
using WorldClientConstPtr = std::shared_ptr<WorldClient const>;
using WorldClientWeakPtr = std::weak_ptr<WorldClient>;
using WorldClientConstWeakPtr = std::weak_ptr<WorldClient const>;
using WorldClientUPtr = std::unique_ptr<WorldClient>;
using WorldClientConstUPtr = std::unique_ptr<WorldClient const>;
class ItemBag;
using ItemBagPtr = std::shared_ptr<ItemBag>;
using ItemBagConstPtr = std::shared_ptr<ItemBag const>;
using ItemBagWeakPtr = std::weak_ptr<ItemBag>;
using ItemBagConstWeakPtr = std::weak_ptr<ItemBag const>;
using ItemBagUPtr = std::unique_ptr<ItemBag>;
using ItemBagConstUPtr = std::unique_ptr<ItemBag const>;
class ItemGridWidget;
using ItemGridWidgetPtr = std::shared_ptr<ItemGridWidget>;
using ItemGridWidgetConstPtr = std::shared_ptr<ItemGridWidget const>;
using ItemGridWidgetWeakPtr = std::weak_ptr<ItemGridWidget>;
using ItemGridWidgetConstWeakPtr = std::weak_ptr<ItemGridWidget const>;
using ItemGridWidgetUPtr = std::unique_ptr<ItemGridWidget>;
using ItemGridWidgetConstUPtr = std::unique_ptr<ItemGridWidget const>;
class ListWidget;
using ListWidgetPtr = std::shared_ptr<ListWidget>;
using ListWidgetConstPtr = std::shared_ptr<ListWidget const>;
using ListWidgetWeakPtr = std::weak_ptr<ListWidget>;
using ListWidgetConstWeakPtr = std::weak_ptr<ListWidget const>;
using ListWidgetUPtr = std::unique_ptr<ListWidget>;
using ListWidgetConstUPtr = std::unique_ptr<ListWidget const>;
class TextBoxWidget;
using TextBoxWidgetPtr = std::shared_ptr<TextBoxWidget>;
using TextBoxWidgetConstPtr = std::shared_ptr<TextBoxWidget const>;
using TextBoxWidgetWeakPtr = std::weak_ptr<TextBoxWidget>;
using TextBoxWidgetConstWeakPtr = std::weak_ptr<TextBoxWidget const>;
using TextBoxWidgetUPtr = std::unique_ptr<TextBoxWidget>;
using TextBoxWidgetConstUPtr = std::unique_ptr<TextBoxWidget const>;
class ButtonWidget;
using ButtonWidgetPtr = std::shared_ptr<ButtonWidget>;
using ButtonWidgetConstPtr = std::shared_ptr<ButtonWidget const>;
using ButtonWidgetWeakPtr = std::weak_ptr<ButtonWidget>;
using ButtonWidgetConstWeakPtr = std::weak_ptr<ButtonWidget const>;
using ButtonWidgetUPtr = std::unique_ptr<ButtonWidget>;
using ButtonWidgetConstUPtr = std::unique_ptr<ButtonWidget const>;
class LabelWidget;
using LabelWidgetPtr = std::shared_ptr<LabelWidget>;
using LabelWidgetConstPtr = std::shared_ptr<LabelWidget const>;
using LabelWidgetWeakPtr = std::weak_ptr<LabelWidget>;
using LabelWidgetConstWeakPtr = std::weak_ptr<LabelWidget const>;
using LabelWidgetUPtr = std::unique_ptr<LabelWidget>;
using LabelWidgetConstUPtr = std::unique_ptr<LabelWidget const>;
class TabSetWidget;
using TabSetWidgetPtr = std::shared_ptr<TabSetWidget>;
using TabSetWidgetConstPtr = std::shared_ptr<TabSetWidget const>;
using TabSetWidgetWeakPtr = std::weak_ptr<TabSetWidget>;
using TabSetWidgetConstWeakPtr = std::weak_ptr<TabSetWidget const>;
using TabSetWidgetUPtr = std::unique_ptr<TabSetWidget>;
using TabSetWidgetConstUPtr = std::unique_ptr<TabSetWidget const>;

class MerchantPane;
using MerchantPanePtr = std::shared_ptr<MerchantPane>;
using MerchantPaneConstPtr = std::shared_ptr<MerchantPane const>;
using MerchantPaneWeakPtr = std::weak_ptr<MerchantPane>;
using MerchantPaneConstWeakPtr = std::weak_ptr<MerchantPane const>;
using MerchantPaneUPtr = std::unique_ptr<MerchantPane>;
using MerchantPaneConstUPtr = std::unique_ptr<MerchantPane const>;

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
  std::size_t m_selectedIndex;
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

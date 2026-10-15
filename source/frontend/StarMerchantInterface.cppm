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
}

namespace Star {

class WorldClient;
class ItemBag;
class ItemGridWidget;
class ListWidget;
class TextBoxWidget;
class ButtonWidget;
class LabelWidget;
class TabSetWidget;

class MerchantPane : public Pane {
public:
  MerchantPane(std::shared_ptr<WorldClient> worldClient, std::shared_ptr<Player> player, Json const& settings, EntityId sourceEntityId = NullEntityId);

  void displayed() override;
  void dismissed() override;
  std::shared_ptr<Pane> createTooltip(Vec2I const& screenPosition) override;

  EntityId sourceEntityId() const;

  std::shared_ptr<Item> addItems(std::shared_ptr<Item> const& items);

protected:
  void update(float dt) override;

private:
  void swapSlot();

  void buildItemList();
  void setupWidget(std::shared_ptr<Widget> const& widget, Json const& itemConfig);
  void updateSelection();
  int itemPrice();
  void updateBuyTotal();
  void buy();

  void updateSellTotal();
  void sell();

  int maxBuyCount();
  void countChanged();
  void countTextChanged();

  std::shared_ptr<WorldClient> m_worldClient;
  std::shared_ptr<Player> m_player;
  EntityId m_sourceEntityId;
  Json m_settings;

  GameTimer m_refreshTimer;

  JsonArray m_itemList;
  std::size_t m_selectedIndex;
  std::shared_ptr<Item> m_selectedItem;

  float m_buyFactor;
  int m_buyTotal;
  float m_sellFactor;
  int m_sellTotal;

  std::shared_ptr<TabSetWidget> m_tabSet;
  std::shared_ptr<ListWidget> m_itemGuiList;
  std::shared_ptr<TextBoxWidget> m_countTextBox;
  std::shared_ptr<LabelWidget> m_buyTotalLabel;
  std::shared_ptr<ButtonWidget> m_buyButton;
  std::shared_ptr<LabelWidget> m_sellTotalLabel;
  std::shared_ptr<ButtonWidget> m_sellButton;

  std::shared_ptr<ItemGridWidget> m_itemGrid;
  std::shared_ptr<ItemBag> m_itemBag;

  int m_buyCount;
  int m_maxBuyCount;
};

}

export module star.merchant_interface;

export namespace Star {
  using ::Star::WorldClient;
  using ::Star::ItemBag;
  using ::Star::ItemGridWidget;
  using ::Star::ListWidget;
  using ::Star::TextBoxWidget;
  using ::Star::ButtonWidget;
  using ::Star::LabelWidget;
  using ::Star::TabSetWidget;
  using ::Star::MerchantPane;
}

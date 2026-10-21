import std;
import star.memory;
import star.json;
import star.xx_hash;
import star.json_extra;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.strong_typedef;
import star.rpc_promise;
import star.vector;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.lexical_cast;
import star.image;
import star.math_common;
import star.format;
import star.version;
import star.either;
import star.weighted_pool;
import star.net_compatibility;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.data_stream_devices;
import star.btree_database;
import star.ordered_set;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.periodic_function;

import star.lua_root;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;

import star.liquid_types;
import star.tile_sector_array;
import star.particle;
import star.weather_types;
import star.sky_types;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.warping;
import star.wiring;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.sky_parameters;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.entity_rendering;
import star.lua_components;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.application_controller;
import star.renderer;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
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


import star.merchant_interface;
import star.widget_parsing;
import star.gui_reader;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.ai_types;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.button_group;
import star.button_widget;
import star.label_widget;
import star.text_box_widget;
import star.image_widget;
import star.progress_widget;
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
import star.list_widget;
import star.tab_set;

import star.item_tooltip;


import star.player_inventory;
import star.item_bag;

import star.quest_manager;

import star.algorithm;
namespace Star {

MerchantPane::MerchantPane(GuiContext& guiContext,
    std::shared_ptr<WorldClient> worldClient, std::shared_ptr<Player> player, Json const& settings, EntityId sourceEntityId) : Pane(guiContext) {
  m_worldClient = std::move(worldClient);
  m_player = std::move(player);
  m_sourceEntityId = sourceEntityId;

  auto assets = context()->root().assets();
  auto baseConfig = settings.get("config", "/interface/windowconfig/merchant.config");
  m_settings = jsonMerge(assets->fetchJson(baseConfig), settings);

  m_refreshTimer = GameTimer(assets->json("/merchant.config:autoRefreshRate").toFloat());

  m_buyFactor = m_settings.getFloat("buyFactor", assets->json("/merchant.config:defaultBuyFactor").toFloat());
  m_sellFactor = m_settings.getFloat("sellFactor", assets->json("/merchant.config:defaultSellFactor").toFloat());

  m_itemBag = std::make_shared<ItemBag>(context()->root(), m_settings.getUInt("sellContainerSize"));

  m_maxBuyCount = m_settings.getUInt("maxSpinCount", assets->json("/interface/windowconfig/crafting.config:default").getUInt("maxSpinCount", 1000));

  GuiReader reader(*context());
  reader.registerCallback("spinCount.up", [this](Widget*) {
      if (m_selectedIndex != NPos) {
        if (m_buyCount < maxBuyCount())
          m_buyCount++;
        else
          m_buyCount = 1;
      } else {
        m_buyCount = 0;
      }
      countChanged();
    });

  reader.registerCallback("spinCount.down", [this](Widget*) {
      if (m_selectedIndex != NPos) {
        if (m_buyCount > 1)
          m_buyCount--;
        else
          m_buyCount = std::max(maxBuyCount(), 1);
      } else {
        m_buyCount = 0;
      }
      countChanged();
    });

  reader.registerCallback("countChanged", [this](Widget*) { countChanged(); });
  reader.registerCallback("parseCountText", [this](Widget*) { countTextChanged(); });

  reader.registerCallback("buy", [this](Widget*) { buy(); });

  reader.registerCallback("sell", [this](Widget*) { sell(); });

  reader.registerCallback("close", [this](Widget*) { dismiss(); });

  reader.registerCallback("itemGrid",
      [this](Widget*) {
        swapSlot();
        updateSellTotal();
      });

  Json paneLayout = m_settings.get("paneLayout");
  paneLayout = jsonMerge(paneLayout, m_settings.get("paneLayoutOverride", {}));
  reader.construct(paneLayout, this);

  m_tabSet = findChild<TabSetWidget>("buySellTabs");
  m_tabSet->setCallback([this](Widget*) {
    auto bgResult = getBG();
    if (m_tabSet->selectedTab() == 0)
      bgResult.body = m_settings.getString("buyBody");
    else
      bgResult.body = m_settings.getString("sellBody");
    setBG(bgResult);
  });
  m_itemGuiList = findChild<ListWidget>("itemList");
  m_countTextBox = findChild<TextBoxWidget>("tbCount");
  m_buyTotalLabel = findChild<LabelWidget>("lblBuyTotal");
  m_buyButton = findChild<ButtonWidget>("btnBuy");
  m_sellTotalLabel = findChild<LabelWidget>("lblSellTotal");
  m_sellButton = findChild<ButtonWidget>("btnSell");

  m_itemGrid = findChild<ItemGridWidget>("itemGrid");
  m_itemGrid->setItemBag(m_itemBag);

  buildItemList();

  updateSelection();

  updateSellTotal();
}

void MerchantPane::displayed() {
  Pane::displayed();
}

void MerchantPane::dismissed() {
  Pane::dismissed();

  for (auto unsold : m_itemBag->takeAll())
    m_player->giveItem(unsold);

  if (m_sourceEntityId != NullEntityId)
    m_worldClient->sendEntityMessage(m_sourceEntityId, "onMerchantClosed");
}

std::shared_ptr<Pane> MerchantPane::createTooltip(Vec2I const& screenPosition) {
  if (m_tabSet->selectedTab() == 0) {
    for (std::size_t i = 0; i < m_itemGuiList->numChildren(); ++i) {
      auto entry = m_itemGuiList->itemAt(i);
      if (entry->getChildAt(screenPosition)) {
        auto itemConfig = m_itemList.get(i);
        std::shared_ptr<Item> item = context()->root().itemDatabase()->itemShared(ItemDescriptor(itemConfig.get("item")));
        return ItemTooltipBuilder::buildItemTooltip(*context(), item, m_player);
      }
    }
  } else {
    if (auto item = m_itemGrid->itemAt(screenPosition))
      return ItemTooltipBuilder::buildItemTooltip(*context(), item, m_player);
  }
  return {};
}

void MerchantPane::update(float dt) {
  Pane::update(dt);

  if (m_sourceEntityId != NullEntityId && !m_worldClient->playerCanReachEntity(m_sourceEntityId))
    dismiss();

  if (m_refreshTimer.wrapTick(context()->root().timing().timestep)) {
    for (std::size_t i = 0; i < m_itemList.size(); ++i) {
      auto itemConfig = m_itemList.get(i);
      auto itemWidget = m_itemGuiList->itemAt(i);
      setupWidget(itemWidget, itemConfig);
    }
    updateBuyTotal();
  }

  updateSelection();

  m_itemGrid->updateAllItemSlots();
}

EntityId MerchantPane::sourceEntityId() const {
  return m_sourceEntityId;
}

std::shared_ptr<Item> MerchantPane::addItems(std::shared_ptr<Item> const& items) {
  if (m_tabSet->selectedTab() == 1) {
    auto remainder = m_itemBag->addItems(items);
    updateSellTotal();
    return remainder;
  } else {
    return items;
  }
}

void MerchantPane::swapSlot() {
  std::shared_ptr<Item> source = m_player->inventory()->swapSlotItem();
  auto inv = m_player->inventory();
  if (context()->shiftHeld()) {
    if (m_itemGrid->selectedItem()) {
      auto remainder = inv->addItems(m_itemBag->takeItems(m_itemGrid->selectedIndex()));
      if (remainder && !remainder->empty())
        m_itemBag->setItem(m_itemGrid->selectedIndex(), remainder);
    }
  } else {
    if (auto heldItem = m_player->inventory()->swapSlotItem())
      inv->setSwapSlotItem(m_itemBag->swapItems(m_itemGrid->selectedIndex(), heldItem));
    else
      inv->setSwapSlotItem(m_itemBag->takeItems(m_itemGrid->selectedIndex()));
  }
}

void MerchantPane::buildItemList() {
  m_itemGuiList->clear();
  m_itemList = m_settings.getArray("items");

  auto itemDatabase = context()->root().itemDatabase();
  filter(m_itemList, [&](Json const& itemConfig) {
      if (!itemDatabase->hasItem(ItemDescriptor(itemConfig.get("item")).name()))
        return false;

      if (auto prerequisite = itemConfig.optString("prerequisiteQuest")) {
        if (!m_player->questManager()->hasCompleted(*prerequisite))
          return false;
      }

      if (auto quests = itemConfig.optArray("exclusiveQuests")) {
        for (auto quest : *quests) {
          if (m_player->questManager()->hasQuest(quest.toString()))
            return false;
        }
      }

      if (auto prerequisite = itemConfig.optUInt("prerequisiteShipLevel")) {
        if (m_player->shipUpgrades().shipLevel < *prerequisite)
          return false;
      }

      if (auto maxLevel = itemConfig.optUInt("maxShipLevel")) {
        if (m_player->shipUpgrades().shipLevel > *maxLevel)
          return false;
      }

      return true;
    });

  for (auto itemConfig : m_itemList) {
    auto widget = m_itemGuiList->addItem();
    setupWidget(widget, itemConfig);
  }
}

void MerchantPane::setupWidget(std::shared_ptr<Widget> const& widget, Json const& itemConfig) {
  auto& root = context()->root();
  auto assets = root.assets();
  std::shared_ptr<Item> item = root.itemDatabase()->itemShared(ItemDescriptor(itemConfig.get("item")));

  String name = item->friendlyName();
  if (item->count() > 1)
    name = strf("{} (x{})", name, item->count());

  auto itemName = widget->fetchChild<LabelWidget>("itemName");
  itemName->setText(name);

  unsigned price = std::ceil(itemConfig.getInt("price", item->price()) * m_buyFactor);
  widget->setLabel("priceLabel", toString(price));
  widget->setData(price);

  bool unavailable = price > m_player->currency("money");
  auto unavailableoverlay = widget->fetchChild<ImageWidget>("unavailableoverlay");
  if (unavailable) {
    itemName->setColor(Color::Gray);
    unavailableoverlay->show();
  } else {
    itemName->setColor(Color::White);
    unavailableoverlay->hide();
  }

  widget->fetchChild<ItemSlotWidget>("itemIcon")->setItem(item);
  widget->show();
}

void MerchantPane::updateSelection() {
  if (m_selectedIndex != m_itemGuiList->selectedItem()) {
    m_selectedIndex = m_itemGuiList->selectedItem();

    if (m_selectedIndex != NPos) {
      auto itemConfig = m_itemList.get(m_selectedIndex);
      m_selectedItem = context()->root().itemDatabase()->itemShared(ItemDescriptor(itemConfig.get("item")));
      findChild<ButtonWidget>("spinCount.up")->enable();
      findChild<ButtonWidget>("spinCount.down")->enable();
      m_countTextBox->setColor(Color::White);
      m_buyCount = 1;
    } else {
      findChild<ButtonWidget>("spinCount.up")->disable();
      findChild<ButtonWidget>("spinCount.down")->disable();
      m_countTextBox->setColor(Color::Gray);
      m_buyCount = 0;
    }

    countChanged();
  }
}

void MerchantPane::updateBuyTotal() {
  if (auto selected = m_itemGuiList->selectedWidget())
    m_buyTotal = selected->data().toUInt() * m_buyCount;
  else
    m_buyTotal = 0;

  m_buyTotalLabel->setText(toString(m_buyTotal));

  if (m_selectedIndex != NPos && m_buyCount > 0)
    m_buyButton->enable();
  else
    m_buyButton->disable();

  if (m_buyTotal > (int)m_player->inventory()->currency("money")) {
    m_buyTotalLabel->setColor(Color::Red);
    m_buyButton->disable();
  } else {
    m_buyTotalLabel->setColor(Color::White);
  }
}

void MerchantPane::buy() {
  if (m_buyTotal > 0 && m_player->inventory()->consumeCurrency("money", m_buyTotal)) {
    auto countRemaining = m_buyCount;
    while (countRemaining > 0) {
      auto buyItem = m_selectedItem->clone();
      buyItem->setCount(m_selectedItem->count() * countRemaining);
      countRemaining -= buyItem->count();
      m_player->giveItem(buyItem);
    }

    auto reportItem = m_selectedItem->clone();
    reportItem->setCount(reportItem->count() * m_buyCount, true);
    auto buySummary = JsonObject{{"item", reportItem->descriptor().toJson()}, {"total", m_buyTotal}};
    if (m_sourceEntityId != NullEntityId)
      m_worldClient->sendEntityMessage(m_sourceEntityId, "onBuy", {buySummary});

    auto& guiContext = *context();
    guiContext.playAudio(context()->root().assets()->json("/merchant.config:buySound").toString());

    buildItemList();

    updateBuyTotal();
  }
}

void MerchantPane::updateSellTotal() {
  m_sellTotal = 0;
  for (auto item : m_itemBag->items()) {
    if (item)
      m_sellTotal += std::round(item->price() * m_sellFactor);
  }
  m_sellTotalLabel->setText(toString(m_sellTotal));
  if (m_sellTotal > 0)
    m_sellButton->enable();
  else
    m_sellButton->disable();
}

void MerchantPane::sell() {
  if (m_sellTotal > 0) {
    auto sellSummary = JsonObject{{"items", m_itemBag->toJson()}, {"total", m_sellTotal}};
    m_worldClient->sendEntityMessage(m_sourceEntityId, "onSell", {sellSummary});

    m_player->inventory()->addCurrency("money", m_sellTotal);
    m_itemBag->clearItems();
    updateSellTotal();

    auto& guiContext = *context();
    guiContext.playAudio(context()->root().assets()->json("/merchant.config:sellSound").toString());
  }
}

int MerchantPane::maxBuyCount() {
  if (auto selected = m_itemGuiList->selectedWidget()) {
    auto assets = context()->root().assets();
    auto unitPrice = selected->data().toUInt();
    if (unitPrice == 0)
      return m_maxBuyCount;
    return std::min(m_maxBuyCount, (int)std::floor(m_player->currency("money") / unitPrice));
  } else {
    return 0;
  }
}

void MerchantPane::countChanged() {
  m_countTextBox->setText(strf("x{}", m_buyCount));
  updateBuyTotal();
}

void MerchantPane::countTextChanged() {
  if (m_selectedIndex == NPos) {
    m_buyCount = 0;
    countChanged();
  } else {
    try {
      auto countString = m_countTextBox->getText().replace("x", "");
      if (countString.size()) {
        m_buyCount = clamp<int>(lexicalCast<int>(countString), 1, maxBuyCount());
        countChanged();
      }
    } catch (BadLexicalCast const&) {
      m_buyCount = 1;
      countChanged();
    }
  }
}

}

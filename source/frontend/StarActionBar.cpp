import std;
import star.xx_hash;
import star.json;
import star.json_extra;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.bi_map;
import star.strong_typedef;
import star.game_types;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.format;
import star.version;
import star.string;
import star.text;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.ordered_map;
import star.either;
import star.random;
import star.weighted_pool;
import star.ordered_set;
import star.byte_array;
import star.data_stream_devices;

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.id_map;
import star.math_common;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.btree_database;
import star.rpc_promise;
import star.image;
import star.a_star;
import star.periodic_function;


import star.lua_root;
import star.inventory_types;

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
import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;


import star.main_interface_types;


import star.action_bar;
import star.widget_parsing;
import star.gui_reader;
import star.host_address;
import star.celestial_coordinate;
import star.sky_types;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
import star.uuid;
import star.warping;
import star.item_descriptor;
import star.quest_descriptor;
import star.ai_types;
import star.socket;
import star.tcp;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.world_geometry;
import star.wiring;
import star.interactive_entity;
import star.tile_entity;
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
import star.net_packet_socket;
import star.universe_connection;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.entity_rendering;
import star.world;
import star.lua_components;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.item;
import star.progress_widget;
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
import star.button_group;
import star.button_widget;
import star.image_widget;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.actor_movement_controller;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;



import star.merchant_interface;

import star.item_tooltip;
import star.image_metadata_database;


import star.player_inventory;

import star.algorithm;
namespace Star {

ActionBar::ActionBar(GuiContext& guiContext, MainInterfacePaneManager& paneManager) : Pane(guiContext), m_paneManager(paneManager) {}

std::expected<std::shared_ptr<ActionBar>, StarException> ActionBar::create(GuiContext& guiContext, MainInterfacePaneManager& paneManager, std::shared_ptr<Player> player) {
  auto pane = std::shared_ptr<ActionBar>(new ActionBar(guiContext, paneManager));
  auto base = pane->initializePane();
  if (!base)
    return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize(std::move(player));
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> ActionBar::initialize(std::shared_ptr<Player> player) {
  m_player = std::move(player);

  auto assets = context().root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/interface/windowconfig/actionbar.config");
  if (!config)
    return std::unexpected(std::move(config.error()));
  m_config = std::move(*config);

  auto selectOffset = m_config.get("actionBarSelectOffset").and_then([](Json value) { return jsonToVec2I(value); });
  if (!selectOffset)
    return std::unexpected(std::move(selectOffset.error()));
  m_actionBarSelectOffset = *selectOffset;
  auto switchSounds =
    m_config.get("sounds").and_then([](Json sounds) { return sounds.get("switch"); }).and_then([](Json sounds) {
      return jsonToStringList(sounds);
    });
  if (!switchSounds)
    return std::unexpected(std::move(switchSounds.error()));
  m_switchSounds = std::move(*switchSounds);

  GuiReader reader(context());
  for (std::uint8_t i = 0; i < m_player->inventory()->customBarIndexes(); ++i) {
    reader.registerCallback(strf("customBar{}L", i + 1), [this, i](Widget&) { return customBarClick(i, true); });
    reader.registerCallback(strf("customBar{}R", i + 1), [this, i](Widget&) { return customBarClick(i, false); });
    reader.registerCallback(strf("customBar{}L.right", i + 1),
                            [this, i](Widget&) { return customBarClickRight(i, true); });
    reader.registerCallback(strf("customBar{}R.right", i + 1),
                            [this, i](Widget&) { return customBarClickRight(i, false); });
  }
  for (std::uint8_t i = 0; i < EssentialItemCount; ++i)
    reader.registerCallback(strf("essentialBar{}", i + 1), [this, i](Widget&) { return essentialBarClick(i); });

  reader.registerCallback("pickupToActionBar", [this](Widget& widget) -> std::expected<void, StarException> {
    auto button = rconvert<ButtonWidget>(widget);
    if (!button)
      return std::unexpected(std::move(button.error()));
    auto configuration = context().root().configuration();
    if (!configuration)
      return std::unexpected(std::move(configuration.error()));
    return (*configuration)->setPath("inventory.pickupToActionBar", button->get().isChecked());
  });
  reader.registerCallback("swapCustomBar", [this](Widget&) { return swapCustomBar(); });

  auto configuration = context().root().configuration();
  if (!configuration)
    return std::unexpected(std::move(configuration.error()));
  auto bottomBar =
    (*configuration)->getPath("inventory.bottomActionBar").and_then([](Json value) { return value.optBool(); });
  if (!bottomBar)
    return std::unexpected(std::move(bottomBar.error()));
  if (bottomBar->value_or(false)) {
    auto bottomConfig = (*assets)->json("/interface/windowconfig/actionbarbottom.config");
    if (!bottomConfig)
      return std::unexpected(std::move(bottomConfig.error()));
    m_config = jsonMerge(m_config, std::move(*bottomConfig));
  }

  auto layout = m_config.get("paneLayout");
  if (!layout)
    return std::unexpected(std::move(layout.error()));
  if (auto constructed = reader.construct(*layout, *this); !constructed)
    return std::unexpected(std::move(constructed.error()));
  if (bottomBar->value_or(false)) {
    setAnchor(PaneAnchor::CenterBottom);
    m_anchorOffset[1] *= -1;
    if (!m_bgBody.empty())
      m_bgBody += "?flipy";
    if (!m_bgHeader.empty())
      m_bgHeader += "?flipy";
    if (!m_bgFooter.empty())
      m_bgFooter += "?flipy";
    std::swap(m_bgHeader, m_bgFooter);
    for (auto& child : m_members) {
      auto position = child->relativePosition();
      child->setPosition({position[0], m_bodySize[1] - position[1] - child->size()[1] + 1});
    }
  }

  for (std::uint8_t i = 0; i < m_player->inventory()->customBarIndexes(); ++i) {
    auto left = fetchChild<ItemSlotWidget>(strf("customBar{}L", i + 1));
    if (!left)
      return std::unexpected(std::move(left.error()));
    auto right = fetchChild<ItemSlotWidget>(strf("customBar{}R", i + 1));
    if (!right)
      return std::unexpected(std::move(right.error()));
    auto leftOverlay = fetchChild<ImageWidget>(strf("customBar{}LOverlay", i + 1));
    if (!leftOverlay)
      return std::unexpected(std::move(leftOverlay.error()));
    auto rightOverlay = fetchChild<ImageWidget>(strf("customBar{}ROverlay", i + 1));
    if (!rightOverlay)
      return std::unexpected(std::move(rightOverlay.error()));
    auto countAnchor = m_config.get("countMidAnchor").and_then([](Json value) { return jsonToVec2F(value); });
    if (!countAnchor)
      return std::unexpected(std::move(countAnchor.error()));
    TextPositioning countPosition = {*countAnchor, HorizontalAnchor::HMidAnchor};
    (*left)->setCountPosition(countPosition);
    (*right)->setCountPosition(countPosition);
    m_customBarWidgets.append({*left, *right, *leftOverlay, *rightOverlay});
  }
  auto customSelection = fetchChild<ImageWidget>("customSelect");
  if (!customSelection)
    return std::unexpected(std::move(customSelection.error()));
  m_customSelectedWidget = std::move(*customSelection);

  for (std::uint8_t i = 0; i < EssentialItemCount; ++i) {
    auto slot = fetchChild<ItemSlotWidget>(strf("essentialBar{}", i + 1));
    if (!slot)
      return std::unexpected(std::move(slot.error()));
    m_essentialBarWidgets.append(std::move(*slot));
  }
  auto essentialSelection = fetchChild<ImageWidget>("essentialSelect");
  if (!essentialSelection)
    return std::unexpected(std::move(essentialSelection.error()));
  m_essentialSelectedWidget = std::move(*essentialSelection);
  return {};
}

std::expected<std::shared_ptr<Pane>, StarException> ActionBar::createTooltip(Vec2I const& screenPosition) {
  std::shared_ptr<Item> item;
  auto inspectSlot = [&](std::shared_ptr<ItemSlotWidget> const& widget) {
    if (widget->screenBoundRect().contains(screenPosition))
      item = widget->item();
  };
  for (auto const& widgets : m_customBarWidgets) {
    inspectSlot(widgets.left);
    inspectSlot(widgets.right);
  }
  for (auto const& widget : m_essentialBarWidgets)
    inspectSlot(widget);
  if (!item)
    return {};
  return ItemTooltipBuilder::buildItemTooltip(context(), item, m_player);
}

std::expected<bool, StarException> ActionBar::sendEvent(InputEvent const& event) {
  auto handled = Pane::sendEvent(event);
  if (!handled)
    return std::unexpected(std::move(handled.error()));
  if (*handled)
    return true;

  auto inventory = m_player->inventory();

  auto customBarIndexes = inventory->customBarIndexes();
  if (std::holds_alternative<MouseWheelEvent>(event)) {
    auto const& mouseWheel = std::get<MouseWheelEvent>(event);
    auto abl = inventory->selectedActionBarLocation();

    int index = 0;
    if (std::holds_alternative<std::monostate>(abl)) {
      if (mouseWheel.mouseWheel == MouseWheel::Down)
        index = 0;
      else
        index = customBarIndexes + EssentialItemCount - 1;
    } else {
      if (std::holds_alternative<CustomBarIndex>(abl)) {
        auto const& cbi = std::get<CustomBarIndex>(abl);
        if (cbi < customBarIndexes / 2)
          index = cbi;
        else
          index = cbi + EssentialItemCount;
      } else {
        index = customBarIndexes / 2 + (int)std::get<EssentialItem>(abl);
      }

      if (mouseWheel.mouseWheel == MouseWheel::Down)
        index = pmod(index + 1, customBarIndexes + EssentialItemCount);
      else
        index = pmod(index - 1, customBarIndexes + EssentialItemCount);
    }

    if (index < customBarIndexes / 2)
      abl = (CustomBarIndex)index;
    else if (index < customBarIndexes / 2 + EssentialItemCount)
      abl = (EssentialItem)(index - customBarIndexes / 2);
    else
      abl = (CustomBarIndex)(index - EssentialItemCount);

    inventory->selectActionBarLocation(abl);
    auto switchSound = RandomSource(context().root().random().randu64()).randFrom(m_switchSounds);
    if (!switchSound)
      return std::unexpected(std::move(switchSound.error()));
    if (auto played = context().playAudio(switchSound->get()); !played)
      return std::unexpected(std::move(played.error()));

    return true;
  }

  if (std::holds_alternative<MouseMoveEvent>(event)) {
    m_customBarHover.reset();
    Vec2I screenPosition = *context().mousePosition(event);
    for (std::uint8_t i = 0; i < customBarIndexes; ++i) {
      if (m_customBarWidgets[i].left->screenBoundRect().contains(screenPosition))
        m_customBarHover = std::make_pair((CustomBarIndex)i, false);
      else if (m_customBarWidgets[i].right->screenBoundRect().contains(screenPosition))
        m_customBarHover = std::make_pair((CustomBarIndex)i, true);
    }
  }

  for (auto action : context().actions(event)) {
    if (action >= InterfaceAction::InterfaceBar1 && action <= InterfaceAction::InterfaceBar10
      && ((int)action - (int)InterfaceAction::InterfaceBar1) < inventory->customBarIndexes())
      inventory->selectActionBarLocation((CustomBarIndex)((int)action - (int)InterfaceAction::InterfaceBar1));

    if (action >= InterfaceAction::EssentialBar1 && action <= InterfaceAction::EssentialBar4)
      inventory->selectActionBarLocation((EssentialItem)((int)action - (int)InterfaceAction::EssentialBar1));

    if (action == InterfaceAction::InterfaceDeselectHands) {
      if (auto previousSelectedLocation = inventory->selectedActionBarLocation(); !std::holds_alternative<std::monostate>(previousSelectedLocation)) {
        m_emptyHandsPreviousActionBarLocation = inventory->selectedActionBarLocation();
        inventory->selectActionBarLocation({});
      } else {
        inventory->selectActionBarLocation(std::exchange(m_emptyHandsPreviousActionBarLocation, {}));
      }
    }

    if (action == InterfaceAction::InterfaceChangeBarGroup)
      if (auto swapped = swapCustomBar(); !swapped)
        return std::unexpected(std::move(swapped.error()));
  }

  return false;
}

std::expected<void, StarException> ActionBar::update(float) {
  auto inventory = m_player->inventory();
  auto selection = inventory->selectedActionBarLocation();
  if (std::holds_alternative<CustomBarIndex>(selection)) {
    auto widgets = m_customBarWidgets.at(std::get<CustomBarIndex>(selection));
    if (!widgets)
      return std::unexpected(std::move(widgets.error()));
    m_customSelectedWidget->setPosition(widgets->get().left->position() + m_actionBarSelectOffset);
    if (auto shown = m_customSelectedWidget->show(); !shown)
      return std::unexpected(std::move(shown.error()));
    if (auto hidden = m_essentialSelectedWidget->hide(); !hidden)
      return std::unexpected(std::move(hidden.error()));
  } else if (std::holds_alternative<EssentialItem>(selection)) {
    auto widget = m_essentialBarWidgets.at((std::size_t)std::get<EssentialItem>(selection));
    if (!widget)
      return std::unexpected(std::move(widget.error()));
    m_essentialSelectedWidget->setPosition(widget->get()->position() + m_actionBarSelectOffset);
    if (auto shown = m_essentialSelectedWidget->show(); !shown)
      return std::unexpected(std::move(shown.error()));
    if (auto hidden = m_customSelectedWidget->hide(); !hidden)
      return std::unexpected(std::move(hidden.error()));
  } else {
    if (auto hidden = m_essentialSelectedWidget->hide(); !hidden)
      return std::unexpected(std::move(hidden.error()));
    if (auto hidden = m_customSelectedWidget->hide(); !hidden)
      return std::unexpected(std::move(hidden.error()));
  }

  for (std::uint8_t i = 0; i < EssentialItemCount; ++i)
    m_essentialBarWidgets[i]->setItem(inventory->essentialItem((EssentialItem)i));

  for (std::uint8_t i = 0; i < inventory->customBarIndexes(); ++i) {
    // Hovering a held item previews its link; two-handed items occupy both visible slots.
    std::shared_ptr<Item> primaryItem;
    auto primarySlot = inventory->customBarPrimarySlot(i);
    if (!primarySlot)
      return std::unexpected(std::move(primarySlot.error()));
    if (*primarySlot) {
      auto item = inventory->itemsAt(**primarySlot);
      if (!item)
        return std::unexpected(std::move(item.error()));
      primaryItem = std::move(*item);
    }
    std::shared_ptr<Item> secondaryItem;
    auto secondarySlot = inventory->customBarSecondarySlot(i);
    if (!secondarySlot)
      return std::unexpected(std::move(secondarySlot.error()));
    if (*secondarySlot) {
      auto item = inventory->itemsAt(**secondarySlot);
      if (!item)
        return std::unexpected(std::move(item.error()));
      secondaryItem = std::move(*item);
    }

    bool primaryPreview = false;
    bool secondaryPreview = false;
    auto swapSlotItem = inventory->swapSlotItem();
    if (swapSlotItem && m_customBarHover && m_customBarHover->first == i) {
      if (!m_customBarHover->second || swapSlotItem->twoHanded()) {
        if (!primaryItem && swapSlotItem == secondaryItem)
          secondaryItem = {};
        primaryItem = swapSlotItem;
        primaryPreview = true;
      } else {
        if (itemSafeTwoHanded(primaryItem))
          primaryItem = {};
        if (!secondaryItem && swapSlotItem == primaryItem)
          primaryItem = {};
        secondaryItem = swapSlotItem;
        secondaryPreview = true;
      }
    }

    auto& widgets = m_customBarWidgets[i];
    widgets.left->setItem(primaryItem);
    widgets.left->showDurability(!primaryPreview);
    widgets.left->showCount(!primaryPreview);
    auto leftOverlay = primaryPreview ? widgets.leftOverlay->show() : widgets.leftOverlay->hide();
    if (!leftOverlay)
      return std::unexpected(std::move(leftOverlay.error()));

    if (itemSafeTwoHanded(primaryItem)) {
      widgets.right->setItem(primaryItem);
      widgets.right->showSecondaryIcon(true);
      widgets.right->showDurability(false);
      widgets.right->showCount(false);
      auto rightOverlay =
        primaryItem->hasSecondaryDrawables() ? widgets.rightOverlay->hide() : widgets.rightOverlay->show();
      if (!rightOverlay)
        return std::unexpected(std::move(rightOverlay.error()));
    } else {
      widgets.right->showSecondaryIcon(false);
      widgets.right->setItem(secondaryItem);
      widgets.right->showDurability(!secondaryPreview);
      widgets.right->showCount(!secondaryPreview);
      auto rightOverlay = secondaryPreview ? widgets.rightOverlay->show() : widgets.rightOverlay->hide();
      if (!rightOverlay)
        return std::unexpected(std::move(rightOverlay.error()));
    }
    widgets.left->setHighlightEnabled(!widgets.left->item() && swapSlotItem);
    widgets.right->setHighlightEnabled(!widgets.right->item() && swapSlotItem);
  }

  auto pickupButton = fetchChild<ButtonWidget>("pickupToActionBar");
  if (!pickupButton)
    return std::unexpected(std::move(pickupButton.error()));
  auto configuration = context().root().configuration();
  if (!configuration)
    return std::unexpected(std::move(configuration.error()));
  auto pickupEnabled =
    (*configuration)->getPath("inventory.pickupToActionBar").and_then([](Json value) { return value.toBool(); });
  if (!pickupEnabled)
    return std::unexpected(std::move(pickupEnabled.error()));
  (*pickupButton)->setChecked(*pickupEnabled);
  auto swapButton = fetchChild<ButtonWidget>("swapCustomBar");
  if (!swapButton)
    return std::unexpected(std::move(swapButton.error()));
  (*swapButton)->setChecked(m_player->inventory()->customBarGroup() != 0);
  return {};
}

std::expected<std::optional<String>, StarException> ActionBar::cursorOverride(Vec2I const&) {
  if (m_customBarHover && m_player->inventory()->swapSlotItem()) {
    auto cursor = m_config.getString("linkCursor");
    if (!cursor)
      return std::unexpected(std::move(cursor.error()));
    return std::move(*cursor);
  }
  return {};
}

std::expected<void, StarException> ActionBar::customBarClick(std::uint8_t index, bool primary) {
  if (auto swapItem = m_player->inventory()->swapSlotItem()) {
    auto linked = primary || itemSafeTwoHanded(swapItem)
      ? m_player->inventory()->setCustomBarPrimarySlot(index, InventorySlot(SwapSlot()))
      : m_player->inventory()->setCustomBarSecondarySlot(index, InventorySlot(SwapSlot()));
    if (!linked)
      return std::unexpected(std::move(linked.error()));
    return m_player->inventory()->clearSwap().transform([](bool) {});
  }
  m_player->inventory()->selectActionBarLocation(index);
  return {};
}

std::expected<void, StarException> ActionBar::customBarClickRight(std::uint8_t index, bool primary) {
  if (!m_paneManager)
    return std::unexpected(StarException("Action bar is detached from its interface"));
  auto inventoryDisplayed = m_paneManager->get().registeredPaneIsDisplayed(MainInterfacePanes::Inventory);
  if (!inventoryDisplayed)
    return std::unexpected(std::move(inventoryDisplayed.error()));
  if (!*inventoryDisplayed)
    return {};

  auto inventory = m_player->inventory();
  auto primarySlot = inventory->customBarPrimarySlot(index);
  if (!primarySlot)
    return std::unexpected(std::move(primarySlot.error()));
  // Validate both links before changing either, including the unused secondary value.
  auto secondarySlot = inventory->customBarSecondarySlot(index);
  if (!secondarySlot)
    return std::unexpected(std::move(secondarySlot.error()));
  bool clearPrimary = primary;
  if (!clearPrimary && *primarySlot) {
    auto item = inventory->itemsAt(**primarySlot);
    if (!item)
      return std::unexpected(std::move(item.error()));
    clearPrimary = itemSafeTwoHanded(*item);
  }
  if (clearPrimary)
    return inventory->setCustomBarPrimarySlot(index, {});
  return inventory->setCustomBarSecondarySlot(index, {});
}

std::expected<void, StarException> ActionBar::essentialBarClick(std::uint8_t index) {
  m_player->inventory()->selectActionBarLocation((EssentialItem)index);
  return {};
}

std::expected<void, StarException> ActionBar::swapCustomBar() {
  m_player->inventory()->setCustomBarGroup((m_player->inventory()->customBarGroup() + 1) % m_player->inventory()->customBarGroups());
  return {};
}

}

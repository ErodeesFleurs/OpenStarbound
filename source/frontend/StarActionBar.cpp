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

ActionBar::ActionBar(GuiContext& guiContext) : Pane(guiContext) {}

std::expected<std::shared_ptr<ActionBar>, StarException> ActionBar::create(GuiContext& guiContext, MainInterfacePaneManager* paneManager, std::shared_ptr<Player> player) {
  auto pane = std::shared_ptr<ActionBar>(new ActionBar(guiContext));
  auto base = pane->initializePane();
  if (!base)
    return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize(paneManager, std::move(player));
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> ActionBar::initialize(MainInterfacePaneManager* paneManager, std::shared_ptr<Player> player) {
  m_paneManager = paneManager;
  m_player = std::move(player);

  auto assetsResult703 = context()->root().assets();
  if (!assetsResult703)
    return std::unexpected(std::move(assetsResult703.error()));
  auto assets = std::move(*assetsResult703);

  auto jsonResult704 = assets->json("/interface/windowconfig/actionbar.config");
  if (!jsonResult704)
    return std::unexpected(std::move(jsonResult704.error()));
  m_config = std::move(*jsonResult704);

  auto getResult705 = m_config.get("actionBarSelectOffset");
  if (!getResult705)
    return std::unexpected(std::move(getResult705.error()));
  auto jsonToVec2IResult706 = jsonToVec2I(std::move(*getResult705));
  if (!jsonToVec2IResult706)
    return std::unexpected(std::move(jsonToVec2IResult706.error()));
  m_actionBarSelectOffset = std::move(*jsonToVec2IResult706);
  auto getResult707 = m_config.get("sounds");
  if (!getResult707)
    return std::unexpected(std::move(getResult707.error()));
  auto getResult708 = std::move(*getResult707).get("switch");
  if (!getResult708)
    return std::unexpected(std::move(getResult708.error()));
  auto jsonToStringListResult709 = jsonToStringList(std::move(*getResult708));
  if (!jsonToStringListResult709)
    return std::unexpected(std::move(jsonToStringListResult709.error()));
  m_switchSounds = std::move(*jsonToStringListResult709);

  GuiReader reader(*context());

  for (std::uint8_t i = 0; i < m_player->inventory()->customBarIndexes(); ++i) {
    reader.registerCallback(strf("customBar{}L", i + 1), std::bind(&ActionBar::customBarClick, this, i, true));
    reader.registerCallback(strf("customBar{}R", i + 1), std::bind(&ActionBar::customBarClick, this, i, false));

    reader.registerCallback(strf("customBar{}L.right", i + 1), std::bind(&ActionBar::customBarClickRight, this, i, true));
    reader.registerCallback(strf("customBar{}R.right", i + 1), std::bind(&ActionBar::customBarClickRight, this, i, false));
  }

  for (std::uint8_t i = 0; i < EssentialItemCount; ++i)
    reader.registerCallback(strf("essentialBar{}", i + 1), std::bind(&ActionBar::essentialBarClick, this, i));

  reader.registerCallback("pickupToActionBar", [this](Widget* widget)  -> std::expected<void, StarException> {
      auto button = as<ButtonWidget>(widget);
      auto configurationResult710 = context()->root().configuration();
      if (!configurationResult710)
        return std::unexpected(std::move(configurationResult710.error()));
      auto setPathResult711 = std::move(*configurationResult710)->setPath("inventory.pickupToActionBar", button->isChecked());
      if (!setPathResult711)
        return std::unexpected(std::move(setPathResult711.error()));
    
  return {};
});

  reader.registerCallback("swapCustomBar", [this](Widget*)  -> std::expected<void, StarException> {
      auto swapCustomBarResult712 = swapCustomBar();
      if (!swapCustomBarResult712)
        return std::unexpected(std::move(swapCustomBarResult712.error()));
    
  return {};
});

  auto configurationResult713 = context()->root().configuration();
  if (!configurationResult713)
    return std::unexpected(std::move(configurationResult713.error()));
  auto configuration = std::move(*configurationResult713);
  auto getPathResult714 = configuration->getPath("inventory.bottomActionBar");
  if (!getPathResult714)
    return std::unexpected(std::move(getPathResult714.error()));
  auto optBoolResult715 = std::move(*getPathResult714).optBool();
  if (!optBoolResult715)
    return std::unexpected(std::move(optBoolResult715.error()));
  bool bottomBar = std::move(*optBoolResult715).value_or(false);
  if (bottomBar)
    {
      auto jsonResult716 = assets->json("/interface/windowconfig/actionbarbottom.config");
      if (!jsonResult716)
        return std::unexpected(std::move(jsonResult716.error()));
      m_config = jsonMerge(m_config, std::move(*jsonResult716));
    }

  auto getResult717 = m_config.get("paneLayout");
  if (!getResult717)
    return std::unexpected(std::move(getResult717.error()));
  auto constructResult718 = reader.construct(std::move(*getResult717), this);
  if (!constructResult718)
    return std::unexpected(std::move(constructResult718.error()));
  if (bottomBar) {
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
    auto fetchChildResult719 = fetchChild<ItemSlotWidget>(strf("customBar{}L", i + 1));
    if (!fetchChildResult719)
      return std::unexpected(std::move(fetchChildResult719.error()));
    auto customBarLeft = std::move(*fetchChildResult719);
    auto fetchChildResult720 = fetchChild<ItemSlotWidget>(strf("customBar{}R", i + 1));
    if (!fetchChildResult720)
      return std::unexpected(std::move(fetchChildResult720.error()));
    auto customBarRight = std::move(*fetchChildResult720);
    auto fetchChildResult721 = fetchChild<ImageWidget>(strf("customBar{}LOverlay", i + 1));
    if (!fetchChildResult721)
      return std::unexpected(std::move(fetchChildResult721.error()));
    auto customBarLeftOverlay = std::move(*fetchChildResult721);
    auto fetchChildResult722 = fetchChild<ImageWidget>(strf("customBar{}ROverlay", i + 1));
    if (!fetchChildResult722)
      return std::unexpected(std::move(fetchChildResult722.error()));
    auto customBarRightOverlay = std::move(*fetchChildResult722);

    auto getResult723 = m_config.get("countMidAnchor");
    if (!getResult723)
      return std::unexpected(std::move(getResult723.error()));
    auto jsonToVec2FResult724 = jsonToVec2F(std::move(*getResult723));
    if (!jsonToVec2FResult724)
      return std::unexpected(std::move(jsonToVec2FResult724.error()));
    TextPositioning countPosition = {std::move(*jsonToVec2FResult724), HorizontalAnchor::HMidAnchor};
    customBarLeft->setCountPosition(countPosition);
    customBarRight->setCountPosition(countPosition);

    m_customBarWidgets.append({customBarLeft, customBarRight, customBarLeftOverlay, customBarRightOverlay});
  }
  auto fetchChildResult725 = fetchChild<ImageWidget>("customSelect");
  if (!fetchChildResult725)
    return std::unexpected(std::move(fetchChildResult725.error()));
  m_customSelectedWidget = std::move(*fetchChildResult725);

  for (std::uint8_t i = 0; i < EssentialItemCount; ++i)
    {
      auto fetchChildResult726 = fetchChild<ItemSlotWidget>(strf("essentialBar{}", i + 1));
      if (!fetchChildResult726)
        return std::unexpected(std::move(fetchChildResult726.error()));
      m_essentialBarWidgets.append(std::move(*fetchChildResult726));
    }
  auto fetchChildResult727 = fetchChild<ImageWidget>("essentialSelect");
  if (!fetchChildResult727)
    return std::unexpected(std::move(fetchChildResult727.error()));
  m_essentialSelectedWidget = std::move(*fetchChildResult727);

return {};
}

std::expected<std::shared_ptr<Pane>, StarException> ActionBar::createTooltip(Vec2I const& screenPosition) {
  std::shared_ptr<Item> item;
  auto tryItemWidget = [&](std::shared_ptr<ItemSlotWidget> const& isw)  -> std::expected<void, StarException> {
    if (isw->screenBoundRect().contains(screenPosition))
      item = isw->item();
  
  return {};
};

  for (auto const& p : m_customBarWidgets) {
    auto tryItemWidgetResult728 = tryItemWidget(p.left);
    if (!tryItemWidgetResult728)
      return std::unexpected(std::move(tryItemWidgetResult728.error()));
    auto tryItemWidgetResult729 = tryItemWidget(p.right);
    if (!tryItemWidgetResult729)
      return std::unexpected(std::move(tryItemWidgetResult729.error()));
  }

  for (auto const& w : m_essentialBarWidgets)
    {
      auto tryItemWidgetResult730 = tryItemWidget(w);
      if (!tryItemWidgetResult730)
        return std::unexpected(std::move(tryItemWidgetResult730.error()));
      
    }

  if (!item)
    return {};

  auto buildItemTooltipResult731 = ItemTooltipBuilder::buildItemTooltip(*context(), item, m_player);
  if (!buildItemTooltipResult731)
    return std::unexpected(std::move(buildItemTooltipResult731.error()));
  return std::move(*buildItemTooltipResult731);
}

std::expected<bool, StarException> ActionBar::sendEvent(InputEvent const& event) {
  {
    auto sendEventResult732 = Pane::sendEvent(event);
    if (!sendEventResult732)
      return std::unexpected(std::move(sendEventResult732.error()));
    if (std::move(*sendEventResult732))
      return true;
  }

  auto inventory = m_player->inventory();

  auto customBarIndexes = inventory->customBarIndexes();
  if (auto mouseWheel = std::get_if<MouseWheelEvent>(&event)) {
    auto abl = inventory->selectedActionBarLocation();

    int index = 0;
    if (std::holds_alternative<std::monostate>(abl)) {
      if (mouseWheel->mouseWheel == MouseWheel::Down)
        index = 0;
      else
        index = customBarIndexes + EssentialItemCount - 1;
    } else {
      if (auto cbi = std::get_if<CustomBarIndex>(&abl)) {
        if (*cbi < customBarIndexes / 2)
          index = *cbi;
        else
          index = *cbi + EssentialItemCount;
      } else {
        index = customBarIndexes / 2 + (int)std::get<EssentialItem>(abl);
      }

      if (mouseWheel->mouseWheel == MouseWheel::Down)
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
    auto randFromResult733 = RandomSource(context()->root().random().randu64()).randFrom(m_switchSounds);
    if (!randFromResult733)
      return std::unexpected(std::move(randFromResult733.error()));
    auto playAudioResult734 = context()->playAudio((**randFromResult733));
    if (!playAudioResult734)
      return std::unexpected(std::move(playAudioResult734.error()));

    return true;
  }

  if (std::holds_alternative<MouseMoveEvent>(event)) {
    m_customBarHover.reset();
    Vec2I screenPosition = *context()->mousePosition(event);
    for (std::uint8_t i = 0; i < customBarIndexes; ++i) {
      if (m_customBarWidgets[i].left->screenBoundRect().contains(screenPosition))
        m_customBarHover = std::make_pair((CustomBarIndex)i, false);
      else if (m_customBarWidgets[i].right->screenBoundRect().contains(screenPosition))
        m_customBarHover = std::make_pair((CustomBarIndex)i, true);
    }
  }

  for (auto action : context()->actions(event)) {
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
      {
        auto swapCustomBarResult735 = swapCustomBar();
        if (!swapCustomBarResult735)
          return std::unexpected(std::move(swapCustomBarResult735.error()));
        
      }
  }

  return false;
}

std::expected<void, StarException> ActionBar::update(float) {
  auto inventory = m_player->inventory();
  auto abl = inventory->selectedActionBarLocation();
  if (std::holds_alternative<CustomBarIndex>(abl)) {
    auto atResult736 = m_customBarWidgets.at(std::get<CustomBarIndex>(abl));
    if (!atResult736)
      return std::unexpected(std::move(atResult736.error()));
    auto overlayLoc = (**atResult736).left->position();
    m_customSelectedWidget->setPosition(overlayLoc + m_actionBarSelectOffset);
    auto showResult737 = m_customSelectedWidget->show();
    if (!showResult737)
      return std::unexpected(std::move(showResult737.error()));
    auto hideResult738 = m_essentialSelectedWidget->hide();
    if (!hideResult738)
      return std::unexpected(std::move(hideResult738.error()));
  } else if (std::holds_alternative<EssentialItem>(abl)) {
    auto atResult739 = m_essentialBarWidgets.at((std::size_t)std::get<EssentialItem>(abl));
    if (!atResult739)
      return std::unexpected(std::move(atResult739.error()));
    auto overlayLoc = (**atResult739)->position();
    m_essentialSelectedWidget->setPosition(overlayLoc + m_actionBarSelectOffset);
    auto showResult740 = m_essentialSelectedWidget->show();
    if (!showResult740)
      return std::unexpected(std::move(showResult740.error()));
    auto hideResult741 = m_customSelectedWidget->hide();
    if (!hideResult741)
      return std::unexpected(std::move(hideResult741.error()));
  } else {
    auto hideResult742 = m_essentialSelectedWidget->hide();
    if (!hideResult742)
      return std::unexpected(std::move(hideResult742.error()));
    auto hideResult743 = m_customSelectedWidget->hide();
    if (!hideResult743)
      return std::unexpected(std::move(hideResult743.error()));
  }

  for (std::uint8_t i = 0; i < EssentialItemCount; ++i)
    m_essentialBarWidgets[i]->setItem(inventory->essentialItem((EssentialItem)i));

  for (std::uint8_t i = 0; i < inventory->customBarIndexes(); ++i) {
    // If there is no swap slot item being hovered over the custom bar, then
    // simply set the left and right item widgets in the custom bar to the
    // primary and secondary items.  If the primary item is two handed, the
    // secondary item will be null and the primary hand item is drawn dimmed in
    // the right hand slot.  If there IS a swap slot item being hovered over
    // this spot in the custom bar, things are more complex.  Instead of
    // showing what is currently in the custom bar, a preview of what WOULD
    // happen when linking is shown, except both the left and right item
    // widgets are always shown with no count and always dimmed to indicate
    // that it is just a preview.

    std::shared_ptr<Item> primaryItem;
    std::shared_ptr<Item> secondaryItem;

    {
      auto customBarPrimarySlotResult744 = inventory->customBarPrimarySlot(i);
      if (!customBarPrimarySlotResult744)
        return std::unexpected(std::move(customBarPrimarySlotResult744.error()));
      if (auto slot = std::move(*customBarPrimarySlotResult744))
        {
          auto itemsAtResult745 = inventory->itemsAt(*slot);
          if (!itemsAtResult745)
            return std::unexpected(std::move(itemsAtResult745.error()));
          primaryItem = std::move(*itemsAtResult745);
        }
    }

    {
      auto customBarSecondarySlotResult746 = inventory->customBarSecondarySlot(i);
      if (!customBarSecondarySlotResult746)
        return std::unexpected(std::move(customBarSecondarySlotResult746.error()));
      if (auto slot = std::move(*customBarSecondarySlotResult746))
        {
          auto itemsAtResult747 = inventory->itemsAt(*slot);
          if (!itemsAtResult747)
            return std::unexpected(std::move(itemsAtResult747.error()));
          secondaryItem = std::move(*itemsAtResult747);
        }
    }

    bool primaryPreview = false;
    bool secondaryPreview = false;

    std::shared_ptr<Item> swapSlotItem = inventory->swapSlotItem();
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
    if (primaryPreview) {
      widgets.left->showDurability(false);
      widgets.left->showCount(false);
      auto showResult748 = widgets.leftOverlay->show();
      if (!showResult748)
        return std::unexpected(std::move(showResult748.error()));
    } else {
      widgets.left->showDurability(true);
      widgets.left->showCount(true);
      auto hideResult749 = widgets.leftOverlay->hide();
      if (!hideResult749)
        return std::unexpected(std::move(hideResult749.error()));
    }

    if (itemSafeTwoHanded(primaryItem)) {
      widgets.right->setItem(primaryItem);
      widgets.right->showSecondaryIcon(true);
      widgets.right->showDurability(false);
      widgets.right->showCount(false);
      if (primaryItem->hasSecondaryDrawables())
        {
          auto hideResult750 = widgets.rightOverlay->hide();
          if (!hideResult750)
            return std::unexpected(std::move(hideResult750.error()));
          
        }
      else
        {
          auto showResult751 = widgets.rightOverlay->show();
          if (!showResult751)
            return std::unexpected(std::move(showResult751.error()));
          
        }
    } else {
      widgets.right->showSecondaryIcon(false);
      widgets.right->setItem(secondaryItem);
      if (secondaryPreview) {
        widgets.right->showDurability(false);
        widgets.right->showCount(false);
        auto showResult752 = widgets.rightOverlay->show();
        if (!showResult752)
          return std::unexpected(std::move(showResult752.error()));
      } else {
        widgets.right->showDurability(true);
        widgets.right->showCount(true);
        auto hideResult753 = widgets.rightOverlay->hide();
        if (!hideResult753)
          return std::unexpected(std::move(hideResult753.error()));
      }
    }

    widgets.left->setHighlightEnabled(!widgets.left->item() && swapSlotItem);
    widgets.right->setHighlightEnabled(!widgets.right->item() && swapSlotItem);
  }

  auto fetchChildResult754 = fetchChild<ButtonWidget>("pickupToActionBar");
  if (!fetchChildResult754)
    return std::unexpected(std::move(fetchChildResult754.error()));
  auto configurationResult755 = context()->root().configuration();
  if (!configurationResult755)
    return std::unexpected(std::move(configurationResult755.error()));
  auto getPathResult756 = std::move(*configurationResult755)->getPath("inventory.pickupToActionBar");
  if (!getPathResult756)
    return std::unexpected(std::move(getPathResult756.error()));
  auto toBoolResult757 = std::move(*getPathResult756).toBool();
  if (!toBoolResult757)
    return std::unexpected(std::move(toBoolResult757.error()));
  std::move(*fetchChildResult754)->setChecked(std::move(*toBoolResult757));
  auto fetchChildResult758 = fetchChild<ButtonWidget>("swapCustomBar");
  if (!fetchChildResult758)
    return std::unexpected(std::move(fetchChildResult758.error()));
  std::move(*fetchChildResult758)->setChecked(m_player->inventory()->customBarGroup() != 0);

return {};
}

std::expected<std::optional<String>, StarException> ActionBar::cursorOverride(Vec2I const&) {
  if (m_customBarHover && m_player->inventory()->swapSlotItem())
    {
      auto getStringResult759 = m_config.getString("linkCursor");
      if (!getStringResult759)
        return std::unexpected(std::move(getStringResult759.error()));
      return std::move(*getStringResult759);
    }
  return {};
}

std::expected<void, StarException> ActionBar::customBarClick(std::uint8_t index, bool primary) {
  if (auto swapItem = m_player->inventory()->swapSlotItem()) {
    if (primary || itemSafeTwoHanded(swapItem))
      {
        auto setCustomBarPrimarySlotResult760 = m_player->inventory()->setCustomBarPrimarySlot(index, InventorySlot(SwapSlot()));
        if (!setCustomBarPrimarySlotResult760)
          return std::unexpected(std::move(setCustomBarPrimarySlotResult760.error()));
        
      }
    else
      {
        auto setCustomBarSecondarySlotResult761 = m_player->inventory()->setCustomBarSecondarySlot(index, InventorySlot(SwapSlot()));
        if (!setCustomBarSecondarySlotResult761)
          return std::unexpected(std::move(setCustomBarSecondarySlotResult761.error()));
        
      }

    auto clearSwapResult762 = m_player->inventory()->clearSwap();
    if (!clearSwapResult762)
      return std::unexpected(std::move(clearSwapResult762.error()));

  } else {
    m_player->inventory()->selectActionBarLocation(index);
  }
  return {};
}

std::expected<void, StarException> ActionBar::customBarClickRight(std::uint8_t index, bool primary) {
  auto inventoryDisplayed = m_paneManager->registeredPaneIsDisplayed(MainInterfacePanes::Inventory);
  if (!inventoryDisplayed)
    return std::unexpected(std::move(inventoryDisplayed.error()));
  if (*inventoryDisplayed) {
    auto inventory = m_player->inventory();
    auto customBarPrimarySlotResult763 = inventory->customBarPrimarySlot(index);
    if (!customBarPrimarySlotResult763)
      return std::unexpected(std::move(customBarPrimarySlotResult763.error()));
    auto primarySlot = std::move(*customBarPrimarySlotResult763);
    auto customBarSecondarySlotResult764 = inventory->customBarSecondarySlot(index);
    if (!customBarSecondarySlotResult764)
      return std::unexpected(std::move(customBarSecondarySlotResult764.error()));
    auto secondarySlot = std::move(*customBarSecondarySlotResult764);

    {
      bool condition767 = static_cast<bool>(primary);
      if (!condition767) {
        bool condition766 = static_cast<bool>(primarySlot);
        if (condition766) {
          auto itemsAtResult765 = inventory->itemsAt(*primarySlot);
          if (!itemsAtResult765)
            return std::unexpected(std::move(itemsAtResult765.error()));
          condition766 = static_cast<bool>(itemSafeTwoHanded(std::move(*itemsAtResult765)));
        }
        condition767 = static_cast<bool>((condition766));
      }
      if (condition767)
        {
          auto setCustomBarPrimarySlotResult768 = inventory->setCustomBarPrimarySlot(index, {});
          if (!setCustomBarPrimarySlotResult768)
            return std::unexpected(std::move(setCustomBarPrimarySlotResult768.error()));
          
        }
      else
        {
          auto setCustomBarSecondarySlotResult769 = inventory->setCustomBarSecondarySlot(index, {});
          if (!setCustomBarSecondarySlotResult769)
            return std::unexpected(std::move(setCustomBarSecondarySlotResult769.error()));
          
        }
    }
  }
  return {};
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

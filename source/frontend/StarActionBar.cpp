#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarXXHash.hpp"
#include "StarAssetPath.hpp"
#include "StarBiMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarEither.hpp"
#include "StarVariant.hpp"
#include "StarWeightedPool.hpp"
#include "StarArray.hpp"
#include "StarOrderedSet.hpp"
#include "StarZSTDCompression.hpp"
#include "StarNetCompatibility.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
#include "StarIdMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarRpcPromise.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarAStar.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"


#include "StarLuaRoot.hpp"
import star.inventory_types;

#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
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
#include "StarP2PNetworkingService.hpp"
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
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
#include "StarLuaComponents.hpp"
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
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;



import star.merchant_interface;

import star.item_tooltip;
import star.image_metadata_database;


import star.player_inventory;

namespace Star {

ActionBar::ActionBar(MainInterfacePaneManager* paneManager, PlayerPtr player) {
  m_paneManager = paneManager;
  m_player = std::move(player);

  auto assets = Root::singleton().assets();

  m_config = assets->json("/interface/windowconfig/actionbar.config");

  m_actionBarSelectOffset = jsonToVec2I(m_config.get("actionBarSelectOffset"));
  m_switchSounds = jsonToStringList(m_config.get("sounds").get("switch"));

  GuiReader reader;

  for (uint8_t i = 0; i < m_player->inventory()->customBarIndexes(); ++i) {
    reader.registerCallback(strf("customBar{}L", i + 1), bind(&ActionBar::customBarClick, this, i, true));
    reader.registerCallback(strf("customBar{}R", i + 1), bind(&ActionBar::customBarClick, this, i, false));

    reader.registerCallback(strf("customBar{}L.right", i + 1), bind(&ActionBar::customBarClickRight, this, i, true));
    reader.registerCallback(strf("customBar{}R.right", i + 1), bind(&ActionBar::customBarClickRight, this, i, false));
  }

  for (uint8_t i = 0; i < EssentialItemCount; ++i)
    reader.registerCallback(strf("essentialBar{}", i + 1), bind(&ActionBar::essentialBarClick, this, i));

  reader.registerCallback("pickupToActionBar", [=](Widget* widget) {
      auto button = as<ButtonWidget>(widget);
      Root::singleton().configuration()->setPath("inventory.pickupToActionBar", button->isChecked());
    });

  reader.registerCallback("swapCustomBar", [this](Widget*) {
      swapCustomBar();
    });

  auto configuration = Root::singleton().configuration();
  bool bottomBar = configuration->getPath("inventory.bottomActionBar").optBool().value(false);
  if (bottomBar)
    m_config = jsonMerge(m_config, assets->json("/interface/windowconfig/actionbarbottom.config"));

  reader.construct(m_config.get("paneLayout"), this);
  if (bottomBar) {
    setAnchor(PaneAnchor::CenterBottom);
    m_anchorOffset[1] *= -1;

    if (!m_bgBody.empty())
      m_bgBody += "?flipy";
    if (!m_bgHeader.empty())
      m_bgHeader += "?flipy";
    if (!m_bgFooter.empty())
      m_bgFooter += "?flipy";
    swap(m_bgHeader, m_bgFooter);

    for (auto& child : m_members) {
      auto position = child->relativePosition();
      child->setPosition({position[0], m_bodySize[1] - position[1] - child->size()[1] + 1});
    }
  }

  for (uint8_t i = 0; i < m_player->inventory()->customBarIndexes(); ++i) {
    auto customBarLeft = fetchChild<ItemSlotWidget>(strf("customBar{}L", i + 1));
    auto customBarRight = fetchChild<ItemSlotWidget>(strf("customBar{}R", i + 1));
    auto customBarLeftOverlay = fetchChild<ImageWidget>(strf("customBar{}LOverlay", i + 1));
    auto customBarRightOverlay = fetchChild<ImageWidget>(strf("customBar{}ROverlay", i + 1));

    TextPositioning countPosition = {jsonToVec2F(m_config.get("countMidAnchor")), HorizontalAnchor::HMidAnchor};
    customBarLeft->setCountPosition(countPosition);
    customBarRight->setCountPosition(countPosition);

    m_customBarWidgets.append({customBarLeft, customBarRight, customBarLeftOverlay, customBarRightOverlay});
  }
  m_customSelectedWidget = fetchChild<ImageWidget>("customSelect");

  for (uint8_t i = 0; i < EssentialItemCount; ++i)
    m_essentialBarWidgets.append(fetchChild<ItemSlotWidget>(strf("essentialBar{}", i + 1)));
  m_essentialSelectedWidget = fetchChild<ImageWidget>("essentialSelect");
}

PanePtr ActionBar::createTooltip(Vec2I const& screenPosition) {
  ItemPtr item;
  auto tryItemWidget = [&](ItemSlotWidgetPtr const& isw) {
    if (isw->screenBoundRect().contains(screenPosition))
      item = isw->item();
  };

  for (auto const& p : m_customBarWidgets) {
    tryItemWidget(p.left);
    tryItemWidget(p.right);
  }

  for (auto const& w : m_essentialBarWidgets)
    tryItemWidget(w);

  if (!item)
    return {};

  return ItemTooltipBuilder::buildItemTooltip(item, m_player);
}

bool ActionBar::sendEvent(InputEvent const& event) {
  if (Pane::sendEvent(event))
    return true;

  auto inventory = m_player->inventory();

  auto customBarIndexes = inventory->customBarIndexes();
  if (auto mouseWheel = event.ptr<MouseWheelEvent>()) {
    auto abl = inventory->selectedActionBarLocation();

    int index = 0;
    if (!abl) {
      if (mouseWheel->mouseWheel == MouseWheel::Down)
        index = 0;
      else
        index = customBarIndexes + EssentialItemCount - 1;
    } else {
      if (auto cbi = abl.ptr<CustomBarIndex>()) {
        if (*cbi < customBarIndexes / 2)
          index = *cbi;
        else
          index = *cbi + EssentialItemCount;
      } else {
        index = customBarIndexes / 2 + (int)abl.get<EssentialItem>();
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
    context()->playAudio(RandomSource().randFrom(m_switchSounds));

    return true;
  }

  if (event.is<MouseMoveEvent>()) {
    m_customBarHover.reset();
    Vec2I screenPosition = *GuiContext::singleton().mousePosition(event);
    for (uint8_t i = 0; i < customBarIndexes; ++i) {
      if (m_customBarWidgets[i].left->screenBoundRect().contains(screenPosition))
        m_customBarHover = make_pair((CustomBarIndex)i, false);
      else if (m_customBarWidgets[i].right->screenBoundRect().contains(screenPosition))
        m_customBarHover = make_pair((CustomBarIndex)i, true);
    }
  }

  for (auto action : context()->actions(event)) {
    if (action >= InterfaceAction::InterfaceBar1 && action <= InterfaceAction::InterfaceBar10
      && ((int)action - (int)InterfaceAction::InterfaceBar1) < inventory->customBarIndexes())
      inventory->selectActionBarLocation((CustomBarIndex)((int)action - (int)InterfaceAction::InterfaceBar1));

    if (action >= InterfaceAction::EssentialBar1 && action <= InterfaceAction::EssentialBar4)
      inventory->selectActionBarLocation((EssentialItem)((int)action - (int)InterfaceAction::EssentialBar1));

    if (action == InterfaceAction::InterfaceDeselectHands) {
      if (auto previousSelectedLocation = inventory->selectedActionBarLocation()) {
        m_emptyHandsPreviousActionBarLocation = inventory->selectedActionBarLocation();
        inventory->selectActionBarLocation({});
      } else {
        inventory->selectActionBarLocation(take(m_emptyHandsPreviousActionBarLocation));
      }
    }

    if (action == InterfaceAction::InterfaceChangeBarGroup)
      swapCustomBar();
  }

  return false;
}

void ActionBar::update(float) {
  auto inventory = m_player->inventory();
  auto abl = inventory->selectedActionBarLocation();
  if (abl.is<CustomBarIndex>()) {
    auto overlayLoc = m_customBarWidgets.at(abl.get<CustomBarIndex>()).left->position();
    m_customSelectedWidget->setPosition(overlayLoc + m_actionBarSelectOffset);
    m_customSelectedWidget->show();
    m_essentialSelectedWidget->hide();
  } else if (abl.is<EssentialItem>()) {
    auto overlayLoc = m_essentialBarWidgets.at((size_t)abl.get<EssentialItem>())->position();
    m_essentialSelectedWidget->setPosition(overlayLoc + m_actionBarSelectOffset);
    m_essentialSelectedWidget->show();
    m_customSelectedWidget->hide();
  } else {
    m_essentialSelectedWidget->hide();
    m_customSelectedWidget->hide();
  }

  for (uint8_t i = 0; i < EssentialItemCount; ++i)
    m_essentialBarWidgets[i]->setItem(inventory->essentialItem((EssentialItem)i));

  for (uint8_t i = 0; i < inventory->customBarIndexes(); ++i) {
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

    ItemPtr primaryItem;
    ItemPtr secondaryItem;

    if (auto slot = inventory->customBarPrimarySlot(i))
      primaryItem = inventory->itemsAt(*slot);

    if (auto slot = inventory->customBarSecondarySlot(i))
      secondaryItem = inventory->itemsAt(*slot);

    bool primaryPreview = false;
    bool secondaryPreview = false;

    ItemPtr swapSlotItem = inventory->swapSlotItem();
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
      widgets.leftOverlay->show();
    } else {
      widgets.left->showDurability(true);
      widgets.left->showCount(true);
      widgets.leftOverlay->hide();
    }

    if (itemSafeTwoHanded(primaryItem)) {
      widgets.right->setItem(primaryItem);
      widgets.right->showSecondaryIcon(true);
      widgets.right->showDurability(false);
      widgets.right->showCount(false);
      if (primaryItem->hasSecondaryDrawables())
        widgets.rightOverlay->hide();
      else
        widgets.rightOverlay->show();
    } else {
      widgets.right->showSecondaryIcon(false);
      widgets.right->setItem(secondaryItem);
      if (secondaryPreview) {
        widgets.right->showDurability(false);
        widgets.right->showCount(false);
        widgets.rightOverlay->show();
      } else {
        widgets.right->showDurability(true);
        widgets.right->showCount(true);
        widgets.rightOverlay->hide();
      }
    }

    widgets.left->setHighlightEnabled(!widgets.left->item() && swapSlotItem);
    widgets.right->setHighlightEnabled(!widgets.right->item() && swapSlotItem);
  }

  fetchChild<ButtonWidget>("pickupToActionBar")->setChecked(Root::singleton().configuration()->getPath("inventory.pickupToActionBar").toBool());
  fetchChild<ButtonWidget>("swapCustomBar")->setChecked(m_player->inventory()->customBarGroup() != 0);
}

Maybe<String> ActionBar::cursorOverride(Vec2I const&) {
  if (m_customBarHover && m_player->inventory()->swapSlotItem())
    return m_config.getString("linkCursor");
  return {};
}

void ActionBar::customBarClick(uint8_t index, bool primary) {
  if (auto swapItem = m_player->inventory()->swapSlotItem()) {
    if (primary || itemSafeTwoHanded(swapItem))
      m_player->inventory()->setCustomBarPrimarySlot(index, InventorySlot(SwapSlot()));
    else
      m_player->inventory()->setCustomBarSecondarySlot(index, InventorySlot(SwapSlot()));

    m_player->inventory()->clearSwap();

  } else {
    m_player->inventory()->selectActionBarLocation(index);
  }
}

void ActionBar::customBarClickRight(uint8_t index, bool primary) {
  if (m_paneManager->registeredPaneIsDisplayed(MainInterfacePanes::Inventory)) {
    auto inventory = m_player->inventory();
    auto primarySlot = inventory->customBarPrimarySlot(index);
    auto secondarySlot = inventory->customBarSecondarySlot(index);

    if (primary || (primarySlot && itemSafeTwoHanded(inventory->itemsAt(*primarySlot))))
      inventory->setCustomBarPrimarySlot(index, {});
    else
      inventory->setCustomBarSecondarySlot(index, {});
  }
}

void ActionBar::essentialBarClick(uint8_t index) {
  m_player->inventory()->selectActionBarLocation((EssentialItem)index);
}

void ActionBar::swapCustomBar() {
  m_player->inventory()->setCustomBarGroup((m_player->inventory()->customBarGroup() + 1) % m_player->inventory()->customBarGroups());
}

}

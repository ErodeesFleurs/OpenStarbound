#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarTtlCache.hpp"
#include "StarCasting.hpp"
#include "StarGameTypes.hpp"
#include "StarMaybe.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarStrongTypedef.hpp"
#include "StarRpcPromise.hpp"
#include "StarVector.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarAStar.hpp"
#include "StarDataStream.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarAssetPath.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarLua.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarEither.hpp"
#include "StarWeightedPool.hpp"
#include "StarInterpolation.hpp"
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarPeriodic.hpp"
#include "StarHash.hpp"

#include "StarLuaRoot.hpp"

#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
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

import star.container_entity;


import star.container_interactor;
import star.widget_parsing;
import star.gui_reader;


import star.container_interface;
import star.liquid_types;
import star.worker_pool;
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
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.progress_widget;
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
import star.label_widget;
import star.image_widget;
import star.pane_manager;
import star.fuel_widget;
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
import star.radio_message_database;
import star.player;
import star.status_effect_entity;
import star.scripted_entity;
import star.wire_entity;
import star.object;
import star.widget_lua_bindings;
import star.augment_item;
import star.input;

import star.player_lua_bindings;
import star.item_tooltip;
import star.config_lua_bindings;
import star.status_controller_lua_bindings;


import star.player_inventory;
import star.item_bag;

namespace Star {

ContainerPane::ContainerPane(WorldClientPtr worldClient, PlayerPtr player, ContainerInteractorPtr containerInteractor) {
  m_worldClient = worldClient;
  m_player = player;
  m_containerInteractor = std::move(containerInteractor);

  auto container = m_containerInteractor->openContainer();

  if (!container)
    throw new StarException("Tried to instantiate a ContainerPane with no opened container!");

  auto guiConfig = container->containerGuiConfig();

  if (auto scripts = guiConfig.opt("scripts").apply(jsonToStringList)) {
    if (!m_script) {
      m_script.emplace();
      m_script->setScripts(*scripts);
    }
    m_script->addCallbacks("widget", LuaBindings::makeWidgetCallbacks(this));
    m_script->addCallbacks("config", LuaBindings::makeConfigCallbacks( [guiConfig](String const& name, Json const& def) {
        return guiConfig.query(name, def);
      }));
    m_script->addCallbacks("player", LuaBindings::makePlayerCallbacks(m_player.get()));
    m_script->addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(m_player->statusController()));

    LuaCallbacks containerPaneCallbacks;
    containerPaneCallbacks.registerCallback("containerEntityId", [this]() -> Maybe<EntityId> {
        return m_containerInteractor->openContainerId();
      });
    containerPaneCallbacks.registerCallback("playerEntityId", [this]() { return m_player->entityId(); });
    containerPaneCallbacks.registerCallback("dismiss", [this]() { dismiss(); });
    m_script->addCallbacks("pane", containerPaneCallbacks);

    m_script->setUpdateDelta(guiConfig.getUInt("scriptDelta", 1));
  }

  auto rightClickCallback = [this](size_t index) {
    if (m_expectingSwap != ExpectingSwap::None)
      return;

    auto callbackContainer = m_containerInteractor->openContainer();

    if (!callbackContainer) {
      Logger::warn("rightClickCallback failed in ContainerPane because we do not have an open container!");
      return;
    }

    if (ItemPtr slotItem = callbackContainer->itemBag()->at(index)) {
      auto swapItem = m_player->inventory()->swapSlotItem();
      if (!swapItem || swapItem->empty() || swapItem->couldStack(slotItem)) {
        size_t count = swapItem ? swapItem->couldStack(slotItem) : slotItem->maxStack();
        if (context()->shiftHeld())
          count = max<uint64_t>(1, min<uint64_t>(count, slotItem->count() / 2));
        else
          count = 1;

        m_containerInteractor->takeFromContainerSlot(index, count);
        m_expectingSwap = ExpectingSwap::SwapSlotStack;
      } else if (is<AugmentItem>(swapItem)) {
        m_containerInteractor->applyAugmentInContainer(index, swapItem);
        m_player->inventory()->setSwapSlotItem({});
        m_expectingSwap = ExpectingSwap::SwapSlot;
      }
    }
  };

  m_reader.registerCallback("close", [this](Widget*) { dismiss(); });
  m_reader.registerCallback("itemGrid", [this](Widget* paneObj) {
      if (auto itemGrid = as<ItemGridWidget>(paneObj))
        swapSlot(itemGrid);
      else
        throw GuiException("Invalid object type, expected ItemGridWidget.");
    });
  m_reader.registerCallback("itemGrid.right", [rightClickCallback](Widget* paneObj) {
      if (auto itemGrid = as<ItemGridWidget>(paneObj))
        rightClickCallback(itemGrid->selectedIndex());
      else
        throw GuiException("Invalid object type, expected ItemGridWidget.");
    });

  m_reader.registerCallback("itemGrid2", [this](Widget* paneObj) {
      if (auto itemGrid = as<ItemGridWidget>(paneObj))
        swapSlot(itemGrid);
      else
        throw GuiException("Invalid object type, expected ItemGridWidget.");
    });
  m_reader.registerCallback("itemGrid2.right", [rightClickCallback](Widget* paneObj) {
      if (auto itemGrid = as<ItemGridWidget>(paneObj))
        rightClickCallback(itemGrid->selectedIndex());
      else
        throw GuiException("Invalid object type, expected ItemGridWidget.");
    });

  m_reader.registerCallback("outputItemGrid", [this](Widget* paneObj) {
      if (auto itemGrid = as<ItemGridWidget>(paneObj))
        swapSlot(itemGrid);
      else
        throw GuiException("Invalid object type, expected ItemGridWidget.");
    });
  m_reader.registerCallback("outputItemGrid.right", [rightClickCallback](Widget* paneObj) {
      if (auto itemGrid = as<ItemGridWidget>(paneObj))
        rightClickCallback(itemGrid->selectedIndex());
      else
        throw GuiException("Invalid object type, expected ItemGridWidget.");
    });

  m_reader.registerCallback("toggleCrafting", [this](Widget*) { toggleCrafting(); });

  m_reader.registerCallback("clear", [this](Widget*) { clear(); });
  m_reader.registerCallback("burn", [this](Widget*) { burn(); });

  for (auto const& callbackName : jsonToStringList(guiConfig.get("scriptWidgetCallbacks", JsonArray{}))) {
    m_reader.registerCallback(callbackName, [this, callbackName](Widget* widget) {
      m_script->invoke(callbackName, widget->name(), widget->data());
    });
  }

  m_reader.construct(guiConfig.get("gui"), this);

  if (auto countWidget = fetchChild<LabelWidget>("count"))
    countWidget->setText(countWidget->text().replace("<slots>", toString(container->containerSize())));

  m_itemBag = make_shared<ItemBag>(container->containerSize());
  auto items = container->containerItems();

  fetchChild<ItemGridWidget>("itemGrid")->setItemBag(m_itemBag);
  if (containsChild("itemGrid2"))
    fetchChild<ItemGridWidget>("itemGrid2")->setItemBag(m_itemBag);
  if (containsChild("outputItemGrid"))
    fetchChild<ItemGridWidget>("outputItemGrid")->setItemBag(m_itemBag);

  if (container->iconItem()) {
    auto itemDatabase = Root::singleton().itemDatabase();
    auto iconItem = itemDatabase->itemShared(container->iconItem());
    auto icon = make_shared<ItemSlotWidget>(iconItem, "/interface/inventory/portrait.png");
    icon->showDurability(false);
    icon->showRarity(false);
    icon->setBackingImageAffinity(true, true);
    setTitle(icon, container->containerDescription(), container->containerSubTitle());
  }

  if (containsChild("objectImage"))
    if (auto containerObject = as<Object>(container))
      fetchChild<ImageWidget>("objectImage")->setDrawables(containerObject->cursorHintDrawables());

  m_expectingSwap = ExpectingSwap::None;
}

void ContainerPane::displayed() {
  Pane::displayed();

  m_expectingSwap = ExpectingSwap::None;

  if (m_script) {
    if (m_worldClient && m_worldClient->inWorld())
      m_script->init(m_worldClient.get());

    m_script->invoke("displayed");
  }
}

void ContainerPane::dismissed() {
  Pane::dismissed();

  if (m_script) {
    m_script->invoke("dismissed");
    m_script->uninit();
  }
}

bool ContainerPane::giveContainerResult(ContainerResult result) {
  if (m_expectingSwap == ExpectingSwap::None)
    return false;

  for (auto item : result) {
    auto inv = m_player->inventory();
    m_player->triggerPickupEvents(item);

    if (m_expectingSwap == ExpectingSwap::SwapSlot) {
      m_player->clearSwap();
      inv->setSwapSlotItem(item);
    } else if (m_expectingSwap == ExpectingSwap::SwapSlotStack) {
      auto swapItem = inv->swapSlotItem();
      if (swapItem && swapItem->stackWith(item)) {
        continue;
      } else {
        inv->clearSwap();
        inv->setSwapSlotItem(item);
      }
    } else {
      m_containerInteractor->addToContainer(inv->addItems(item));
    }
  }

  m_expectingSwap = ExpectingSwap::None;
  return true;
}

PanePtr ContainerPane::createTooltip(Vec2I const& screenPosition) {
  ItemPtr item;
  if (auto child = getChildAt(screenPosition)) {
    if (auto itemSlot = as<ItemSlotWidget>(child))
      item = itemSlot->item();
    if (auto itemGrid = as<ItemGridWidget>(child))
      item = itemGrid->itemAt(screenPosition);
  }
  if (item)
    return ItemTooltipBuilder::buildItemTooltip(item, m_player);
  return {};
}

void ContainerPane::swapSlot(ItemGridWidget* grid) {
  auto inv = m_player->inventory();
  if (context()->shiftHeld()) {
    auto containerItem = grid->selectedItem();
    if (containerItem && inv->itemsCanFit(containerItem) >= containerItem->count()) {
      m_containerInteractor->swapInContainer(grid->selectedIndex(), {});
      m_expectingSwap = ExpectingSwap::Inventory;
    }
  } else {
    m_containerInteractor->swapInContainer(grid->selectedIndex(), inv->swapSlotItem());
    inv->setSwapSlotItem({});
    m_expectingSwap = ExpectingSwap::SwapSlot;
  }
}

void ContainerPane::startCrafting() {
  m_containerInteractor->startCraftingInContainer();
}

void ContainerPane::stopCrafting() {
  m_containerInteractor->stopCraftingInContainer();
}

void ContainerPane::toggleCrafting() {
  if (auto container = m_containerInteractor->openContainer())
    if (container->isCrafting())
      stopCrafting();
    else
      startCrafting();
  else
    Logger::warn("ContainerPane::toggleCrafting failed because we don't have an open container!");
}

void ContainerPane::clear() {
  m_containerInteractor->clearContainer();
}

void ContainerPane::burn() {
  m_containerInteractor->burnContainer();
}

void ContainerPane::update(float dt) {
  Pane::update(dt);

  if (m_script)
    m_script->update(m_script->updateDt(dt));

  m_itemBag->clearItems();
  Input& input = Input::singleton();

  if (!m_containerInteractor->containerOpen())
    return dismiss();
  
  auto container = m_containerInteractor->openContainer();

  for (size_t i = 0; i < m_itemBag->size(); ++i)
    m_itemBag->putItems(i, container->containerItems()[i]);

  if (container->isInteractive()) {
    if (auto itemGrid = fetchChild<ItemGridWidget>("itemGrid")) {
      itemGrid->setProgress(container->craftingProgress());
      itemGrid->updateAllItemSlots();
    }
    if (auto itemGrid = fetchChild<ItemGridWidget>("itemGrid2")) {
      itemGrid->setProgress(container->craftingProgress());
      itemGrid->updateAllItemSlots();
    }

    if (auto fuelGauge = fetchChild<FuelWidget>("fuelGauge")) {
      fuelGauge->setCurrentFuelLevel(m_worldClient->getProperty("ship.fuel", 0).toUInt());
      fuelGauge->setMaxFuelLevel(m_worldClient->getProperty("ship.maxFuel", 0).toUInt());
      float totalFuelAmount = 0;
      for (auto& item : container->containerItems()) {
        if (item)
          totalFuelAmount += item->instanceValue("fuelAmount", 0).toUInt() * item->count();
      }
      fuelGauge->setPotentialFuelAmount(totalFuelAmount);
      fuelGauge->setRequestedFuelAmount(0);
    }
  
    if (input.bindDown("opensb", "takeAll")) {
      m_containerInteractor->clearContainer();
    }
  }
}

}

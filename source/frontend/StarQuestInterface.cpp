#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarTtlCache.hpp"
#include "StarCasting.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarJsonRpc.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarAssetPath.hpp"
#include "StarVector.hpp"
#include "StarThread.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarInputEvent.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarTime.hpp"
#include "StarOrderedMap.hpp"
#include "StarRandom.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarEither.hpp"
#include "StarVariant.hpp"
#include "StarWeightedPool.hpp"
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
#include "StarLuaActorMovementComponent.hpp"
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
#include "StarLuaComponents.hpp"
import star.uuid;
import star.warping;
import star.quests;
import star.quest_manager;

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
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;


import star.quest_interface;
import star.cinematic;
import star.widget_parsing;
import star.gui_reader;
import star.host_address;
import star.sky_types;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
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
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.game_timers;
import star.pane_manager;
import star.list_widget;
import star.progress_widget;
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
import star.button_group;
import star.button_widget;
import star.label_widget;
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
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.actor_movement_controller;
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.layout;
import star.vertical_layout;

import star.item_tooltip;
import star.quest_template_database;


import star.item_bag;

namespace Star {

QuestLogInterface::QuestLogInterface(QuestManagerPtr manager, PlayerPtr player, CinematicPtr cinematic, UniverseClientPtr client) {
  m_manager = manager;
  m_player = player;
  m_cinematic = cinematic;
  m_client = client;

  auto assets = Root::singleton().assets();
  auto config = assets->json("/interface/windowconfig/questlog.config");

  m_trackLabel = config.getString("trackLabel");
  m_untrackLabel = config.getString("untrackLabel");

  GuiReader reader;

  reader.registerCallback("close", [this](Widget*) { dismiss(); });
  reader.registerCallback("btnToggleTracking",
      [this](Widget*) {
        toggleTracking();
      });
  reader.registerCallback("btnAbandon",
      [this](Widget*) {
        abandon();
      });
  reader.registerCallback("filter",
      [this](Widget*) {
        fetchData();
      });

  reader.construct(config.get("paneLayout"), this);

  auto mainQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.mainQuestList");
  auto sideQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.sideQuestList");
  mainQuestList->setCallback([sideQuestList = sideQuestList.get()](Widget* widget) {
      auto listWidget = as<ListWidget>(widget);
      if (listWidget->selectedItem() != NPos)
        sideQuestList->clearSelected();
    });
  sideQuestList->setCallback([mainQuestList = mainQuestList.get()](Widget* widget) {
      auto listWidget = as<ListWidget>(widget);
      if (listWidget->selectedItem() != NPos)
        mainQuestList->clearSelected();
    });
  mainQuestList->disableScissoring();
  sideQuestList->disableScissoring();

  m_rewardItems = make_shared<ItemBag>(5);
  fetchChild<ItemGridWidget>("rewardItems")->setItemBag(m_rewardItems);

  m_refreshRate = 30;
  m_refreshTimer = 0;
}

void QuestLogInterface::pollDialog(PaneManager* paneManager) {
  if (paneManager->topPane({PaneLayer::ModalWindow}))
    return;

  if (auto failableQuest = m_manager->getFirstFailableQuest()) {
    auto qfi = make_shared<QuestFailedInterface>(failableQuest.value(), m_player);
    (*failableQuest)->setDialogShown();
    paneManager->displayPane(PaneLayer::ModalWindow, qfi);
  } else if (auto completableQuest = m_manager->getFirstCompletableQuest()) {
    auto qci = make_shared<QuestCompleteInterface>(completableQuest.value(), m_player, m_cinematic);
    (*completableQuest)->setDialogShown();
    paneManager->displayPane(PaneLayer::ModalWindow, qci);
  } else if (auto newQuest = m_manager->getFirstNewQuest()) {
    auto nqd = make_shared<NewQuestInterface>(m_manager, newQuest.value(), m_player);
    paneManager->displayPane(PaneLayer::ModalWindow, nqd);
  }
}

void QuestLogInterface::displayed() {
  Pane::displayed();
  tick(0);
  fetchData();
}

void QuestLogInterface::tick(float dt) {
  Pane::tick(dt);
  auto selected = getSelected();
  if (selected && m_manager->hasQuest(selected->data().toString())) {
    auto quest = m_manager->getQuest(selected->data().toString());

    m_manager->markAsRead(quest->questId());

    if (m_manager->isActive(quest->questId())) {
      fetchChild<ButtonWidget>("btnToggleTracking")->enable();
      fetchChild<ButtonWidget>("btnToggleTracking")->setText(m_manager->isCurrent(quest->questId()) ? m_untrackLabel : m_trackLabel);
      if (quest->canBeAbandoned())
        fetchChild<ButtonWidget>("btnAbandon")->enable();
      else
        fetchChild<ButtonWidget>("btnAbandon")->disable();
    } else {
      fetchChild<ButtonWidget>("btnToggleTracking")->disable();
      fetchChild<ButtonWidget>("btnToggleTracking")->setText(m_trackLabel);
      fetchChild<ButtonWidget>("btnAbandon")->disable();
    }
    fetchChild<LabelWidget>("lblQuestTitle")->setText(quest->title());
    fetchChild<LabelWidget>("lblQuestBody")->setText(quest->text());

    auto portraitName = "Objective";
    auto imagePortrait = quest->portrait(portraitName);
    if (imagePortrait) {
      auto portraitTitleLabel = fetchChild<LabelWidget>("lblPortraitTitle");
      String portraitTitle = quest->portraitTitle(portraitName).value("");
      Maybe<unsigned> charLimit = portraitTitleLabel->getTextCharLimit();
      portraitTitleLabel->setText(portraitTitle);

      Drawable::scaleAll(*imagePortrait, Vec2F(-1, 1));
      fetchChild<ImageWidget>("imgPortrait")->setDrawables(*imagePortrait);
      fetchChild<ImageWidget>("imgPolaroid")->setVisibility(true);
      fetchChild<ImageWidget>("imgPolaroidBack")->setVisibility(true);

    } else {
      fetchChild<LabelWidget>("lblPortraitTitle")->setText("");
      fetchChild<ImageWidget>("imgPortrait")->setDrawables({});
      fetchChild<ImageWidget>("imgPolaroid")->setVisibility(false);
      fetchChild<ImageWidget>("imgPolaroidBack")->setVisibility(false);
    }

    m_rewardItems->clearItems();
    if (quest->rewards().size() > 0) {
      fetchChild<LabelWidget>("lblRewards")->setVisibility(true);
      fetchChild<ItemGridWidget>("rewardItems")->setVisibility(true);
      for (auto const& reward : quest->rewards())
        m_rewardItems->addItems(reward->clone());
    } else {
      fetchChild<LabelWidget>("lblRewards")->setVisibility(false);
      fetchChild<ItemGridWidget>("rewardItems")->setVisibility(false);
    }
  } else {
    fetchChild<ButtonWidget>("btnToggleTracking")->disable();
    fetchChild<ButtonWidget>("btnToggleTracking")->setText(m_trackLabel);
    fetchChild<ButtonWidget>("btnAbandon")->disable();
    fetchChild<LabelWidget>("lblQuestTitle")->setText("");
    fetchChild<LabelWidget>("lblQuestBody")->setText("");
    fetchChild<LabelWidget>("lblPortraitTitle")->setText("");
    fetchChild<ImageWidget>("imgPortrait")->setDrawables({});
    fetchChild<LabelWidget>("lblRewards")->setVisibility(false);
    fetchChild<ItemGridWidget>("rewardItems")->setVisibility(false);
    fetchChild<ImageWidget>("imgPolaroid")->setVisibility(false);
    fetchChild<ImageWidget>("imgPolaroidBack")->setVisibility(false);
    m_rewardItems->clearItems();
  }

  m_refreshTimer--;
  if (m_refreshTimer < 0) {
    fetchData();
    m_refreshTimer = m_refreshRate;
  }
}

PanePtr QuestLogInterface::createTooltip(Vec2I const& screenPosition) {
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

WidgetPtr QuestLogInterface::getSelected() {
  auto mainQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.mainQuestList");
  if (auto selected = mainQuestList->selectedWidget())
    return selected;

  auto sideQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.sideQuestList");
  if (auto selected = sideQuestList->selectedWidget())
    return selected;

  return {};
}

void QuestLogInterface::setSelected(WidgetPtr selected) {
  auto mainQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.mainQuestList");
  auto mainQuestListPos = mainQuestList->itemPosition(selected);
  if (mainQuestListPos != NPos) {
    mainQuestList->setSelected(mainQuestListPos);
    return;
  }

  auto sideQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.sideQuestList");
  auto sideQuestListPos = sideQuestList->itemPosition(selected);
  if (sideQuestListPos != NPos) {
    sideQuestList->setSelected(sideQuestListPos);
    return;
  }
}

void QuestLogInterface::toggleTracking() {
  if (auto selected = getSelected()) {
    String questId = selected->data().toString();
    if (!m_manager->isCurrent(questId))
      m_manager->setAsTracked(questId);
    else
      m_manager->setAsTracked({});
  }
}

void QuestLogInterface::abandon() {
  if (auto selected = getSelected()) {
    m_manager->getQuest(selected->data().toString())->abandon();
  }
}

void QuestLogInterface::fetchData() {
  auto filter = fetchChild<ButtonGroupWidget>("filter")->checkedButton()->data().toString();
  if (filter.equalsIgnoreCase("inProgress"))
    showQuests(m_manager->listActiveQuests());
  else if (filter.equalsIgnoreCase("completed"))
    showQuests(m_manager->listCompletedQuests());
  else
    throw StarException(strf("Unknown quest filter '{}'", filter));
}

void QuestLogInterface::showQuests(List<QuestPtr> quests) {
  auto mainQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.mainQuestList");
  auto sideQuestList = fetchChild<ListWidget>("scrollArea.verticalLayout.sideQuestList");

  auto mainQuestHeader = fetchChild<Widget>("scrollArea.verticalLayout.mainQuestHeader");
  auto sideQuestHeader = fetchChild<Widget>("scrollArea.verticalLayout.sideQuestHeader");

  String selectedQuest;
  if (auto selected = getSelected())
    selectedQuest = selected->data().toString();
  else if (m_manager->currentQuest())
    selectedQuest = (*m_manager->currentQuest())->questId();

  mainQuestList->clear();
  mainQuestHeader->hide();
  sideQuestList->clear();
  sideQuestHeader->hide();
  for (auto const& quest : quests) {
    WidgetPtr entry;
    if (quest->mainQuest()) {
      entry = mainQuestList->addItem();
      mainQuestHeader->show();
    } else {
      entry = sideQuestList->addItem();
      sideQuestHeader->show();
    }

    entry->setData(quest->questId());
    entry->fetchChild<LabelWidget>("lblQuestEntry")->setText(quest->title());
    entry->fetchChild<ImageWidget>("imgNew")->setVisibility(quest->unread());
    entry->fetchChild<ImageWidget>("imgTracked")->setVisibility(m_manager->isCurrent(quest->questId()));

    bool currentWorld = false;
    if (auto questWorld = quest->worldId()) {
      currentWorld = m_client->playerWorld() == *questWorld;
    }
    entry->fetchChild<ImageWidget>("imgCurrent")->setVisibility(currentWorld);

    entry->fetchChild<ImageWidget>("imgPortrait")->setDrawables(quest->portrait("QuestStarted").value({}));
    entry->show();
    if (quest->questId() == selectedQuest)
      setSelected(entry);
  }

  auto verticalLayout = fetchChild<VerticalLayout>("scrollArea.verticalLayout");
  verticalLayout->update(0);
}

QuestPane::QuestPane(QuestPtr const& quest, PlayerPtr player) : Pane(), m_quest(quest), m_player(std::move(player)) {}

void QuestPane::commonSetup(Json config, String bodyText, String const& portraitName) {
  GuiReader reader;

  reader.registerCallback("close", [this](Widget*) { close(); });
  reader.registerCallback("btnDecline", [this](Widget*) { decline(); });
  reader.registerCallback("btnAccept", [this](Widget*) { accept(); });
  reader.construct(config.get("paneLayout"), this);

  if (auto titleLabel = fetchChild<LabelWidget>("lblQuestTitle"))
    titleLabel->setText(m_quest->title());
  if (auto bodyLabel = fetchChild<LabelWidget>("lblQuestBody"))
    bodyLabel->setText(bodyText);

  if (auto portraitImage = fetchChild<ImageWidget>("portraitImage")) {
    Maybe<List<Drawable>> portrait = m_quest->portrait(portraitName);
    portraitImage->setDrawables(portrait.value({}));
  }

  if (auto portraitTitleLabel = fetchChild<LabelWidget>("portraitTitle")) {
    Maybe<String> portraitTitle = m_quest->portraitTitle(portraitName);
    String text = m_quest->portraitTitle(portraitName).value("");
    Maybe<unsigned> charLimit = portraitTitleLabel->getTextCharLimit();
    portraitTitleLabel->setText(text);
  }

  if (auto rewardItemsWidget = fetchChild<ItemGridWidget>("rewardItems")) {
    auto rewardItems = make_shared<ItemBag>(5);
    for (auto const& reward : m_quest->rewards())
      rewardItems->addItems(reward->clone());
    rewardItemsWidget->setItemBag(rewardItems);
  }

  auto sound = Random::randValueFrom(config.get("onShowSound").toArray(), "").toString();
  if (!sound.empty())
    context()->playAudio(sound);
}

void QuestPane::close() {
  dismiss();
}

void QuestPane::decline() {
  close();
}

void QuestPane::accept() {
  close();
}

PanePtr QuestPane::createTooltip(Vec2I const& screenPosition) {
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

NewQuestInterface::NewQuestInterface(QuestManagerPtr const& manager, QuestPtr const& quest, PlayerPtr player)
  : QuestPane(quest, std::move(player)), m_manager(manager), m_decision(QuestDecision::Cancelled) {
  auto assets = Root::singleton().assets();

  List<Drawable> objectivePortrait = m_quest->portrait("Objective").value({});
  bool shortDialog = objectivePortrait.size() == 0;

  String configFile;
  if (shortDialog)
    configFile = m_quest->getTemplate()->newQuestGuiConfig.value(assets->json("/quests/quests.config:defaultGuiConfigs.newQuest").toString());
  else
    configFile = m_quest->getTemplate()->newQuestGuiConfig.value(assets->json("/quests/quests.config:defaultGuiConfigs.newQuestPortrait").toString());

  Json config = assets->json(configFile);

  commonSetup(config, m_quest->text(), "QuestStarted");

  if (!m_quest->canBeAbandoned()) {
    if (auto declineButton = fetchChild<ButtonWidget>("btnDecline"))
      declineButton->disable();
  }

  if (!shortDialog) {
    if (auto objectivePortraitImage = fetchChild<ImageWidget>("objectivePortraitImage")) {
      Drawable::scaleAll(objectivePortrait, Vec2F(-1, 1));
      objectivePortraitImage->setDrawables(objectivePortrait);

      String objectivePortraitTitle = m_quest->portraitTitle("Objective").value("");
      auto portraitLabel = fetchChild<LabelWidget>("objectivePortraitTitle");
      portraitLabel->setText(objectivePortraitTitle);
      portraitLabel->setVisibility(objectivePortrait.size() > 0);

      fetchChild<ImageWidget>("imgPolaroid")->setVisibility(objectivePortrait.size() > 0);
      fetchChild<ImageWidget>("imgPolaroidBack")->setVisibility(objectivePortrait.size() > 0);
    }
  }

  if (auto rewardItemsWidget = fetchChild<ItemGridWidget>("rewardItems"))
    rewardItemsWidget->setVisibility(m_quest->rewards().size() > 0);
  if (auto rewardsLabel = fetchChild<LabelWidget>("lblRewards"))
    rewardsLabel->setVisibility(m_quest->rewards().size() > 0);
}

void NewQuestInterface::close() {
  m_decision = QuestDecision::Cancelled;
  dismiss();
}

void NewQuestInterface::decline() {
  m_decision = QuestDecision::Declined;
  dismiss();
}

void NewQuestInterface::accept() {
  m_decision = QuestDecision::Accepted;
  dismiss();
}

void NewQuestInterface::dismissed() {
  QuestPane::dismissed();
  if (m_decision == QuestDecision::Declined && m_quest->canBeAbandoned()) {
    m_manager->getQuest(m_quest->questId())->declineOffer();
  } else if (m_decision == QuestDecision::Accepted) {
    m_manager->getQuest(m_quest->questId())->start();
  } else {
    m_manager->getQuest(m_quest->questId())->cancelOffer();
  }
}

QuestCompleteInterface::QuestCompleteInterface(QuestPtr const& quest, PlayerPtr player, CinematicPtr cinematic)
  : QuestPane(quest, player) {
  auto assets = Root::singleton().assets();
  String configFile = m_quest->getTemplate()->questCompleteGuiConfig.value(assets->json("/quests/quests.config:defaultGuiConfigs.questComplete").toString());
  Json config = assets->json(configFile);

  m_player = player;
  m_cinematic = cinematic;

  commonSetup(config, m_quest->completionText(), "QuestComplete");

  if (auto moneyLabel = fetchChild<LabelWidget>("lblMoneyAmount"))
    moneyLabel->setText(toString(m_quest->money()));
  disableScissoring();
}

void QuestCompleteInterface::close() {
  auto assets = Root::singleton().assets();
  if (m_quest->completionCinema() && m_cinematic) {
    String cinema = m_quest->completionCinema()->replaceTags(
        StringMap<String>{{"species", m_player->species()}, {"gender", GenderNames.getRight(m_player->gender())}});
    m_cinematic->load(assets->fetchJson(cinema));
  }
  dismiss();
}

QuestFailedInterface::QuestFailedInterface(QuestPtr const& quest, PlayerPtr player) : QuestPane(quest, std::move(player)) {
  auto assets = Root::singleton().assets();
  String configFile = m_quest->getTemplate()->questFailedGuiConfig.value(assets->json("/quests/quests.config:defaultGuiConfigs.questFailed").toString());
  Json config = assets->json(configFile);
  commonSetup(config, m_quest->failureText(), "QuestFailed");
  disableScissoring();
}

}

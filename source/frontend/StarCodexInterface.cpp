#include "StarJson.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarPoly.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarIdMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarStrongTypedef.hpp"
#include "StarAStar.hpp"
#include "StarOrderedSet.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarEither.hpp"


// Keep Uuid's textual definition ahead of the global codex interface on GCC.
#include "StarLuaRoot.hpp"
import star.uuid;
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
#include "StarImage.hpp"
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
import star.player_codexes;


import star.codex_interface;
import star.codex;
import star.widget_parsing;
import star.gui_reader;
import star.item_descriptor;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.light_source;
import star.networked_animator;
import star.humanoid;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.interaction_types;
import star.tile_damage;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.game_timers;
import star.actor_movement_controller;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.label_widget;
import star.list_widget;
import star.stack_widget;
import star.image_widget;
import star.button_group;
import star.button_widget;

namespace Star {

CodexInterface::CodexInterface(PlayerPtr player) {
  m_player = player;

  auto assets = Root::singleton().assets();

  GuiReader reader;

  reader.registerCallback("close", [this](Widget*) { dismiss(); });
  reader.registerCallback("prevButton", [this](Widget*) { backwardPage(); });
  reader.registerCallback("nextButton", [this](Widget*) { forwardPage(); });
  reader.registerCallback("selectCodex", [this](Widget*) { showSelectedContents(); });
  reader.registerCallback("updateSpecies", [this](Widget*) { updateSpecies(); });

  reader.construct(assets->json("/interface/windowconfig/codex.config:paneLayout"), this);

  m_speciesTabs = fetchChild<ButtonGroupWidget>("speciesTabs");
  m_selectLabel = fetchChild<LabelWidget>("selectLabel");
  m_titleLabel = fetchChild<LabelWidget>("titleLabel");
  m_bookList = fetchChild<ListWidget>("scrollArea.bookList");
  m_pageContent = fetchChild<LabelWidget>("pageText");
  m_pageLabelWidget = fetchChild<LabelWidget>("pageLabel");
  m_pageNumberWidget = fetchChild<LabelWidget>("pageNum");
  m_prevPageButton = fetchChild<ButtonWidget>("prevButton");
  m_nextPageButton = fetchChild<ButtonWidget>("nextButton");

  m_selectText = assets->json("/interface/windowconfig/codex.config:selectText").toString();

  m_currentPage = 0;
  updateSpecies();
  setupPageText();
}

void CodexInterface::show() {
  Pane::show();
  updateCodexList();
}

void CodexInterface::tick(float) {
  updateCodexList();
}

void CodexInterface::showSelectedContents() {
  if (m_bookList->selectedItem() == NPos || m_bookList->selectedItem() >= m_codexList.size())
    return;

  showContents(m_codexList[m_bookList->selectedItem()].first);
}

void CodexInterface::showContents(String const& codexId) {
  CodexConstPtr result;
  for (auto entry : m_codexList)
    if (entry.first->id() == codexId) {
      result = entry.first;
      break;
    }
  if (result)
    showContents(result);
}

void CodexInterface::showContents(CodexConstPtr codex) {
  if (m_player->codexes()->markCodexRead(codex->id()))
    updateCodexList();
  m_currentCodex = codex;
  m_currentPage = 0;
  setupPageText();
}

void CodexInterface::forwardPage() {
  if (m_currentCodex && m_currentPage < m_currentCodex->pageCount() - 1) {
    ++m_currentPage;
    setupPageText();
  }
}

void CodexInterface::backwardPage() {
  if (m_currentCodex && m_currentPage > 0) {
    --m_currentPage;
    setupPageText();
  }
}

bool CodexInterface::showNewCodex() {
  if (auto newCodex = m_player->codexes()->firstNewCodex()) {
    for (auto button : m_speciesTabs->buttons()) {
      if (button->data().getString("species") == newCodex->species()) {
        m_speciesTabs->select(m_speciesTabs->id(button));
        break;
      }
    }
    showContents(newCodex);
    return true;
  }

  return false;
}

void CodexInterface::updateSpecies() {
  String newSpecies = "other";
  if (auto speciesButton = m_speciesTabs->checkedButton())
    newSpecies = speciesButton->data().getString("species");
  if (newSpecies != m_currentSpecies) {
    m_currentCodex = {};
    m_currentSpecies = newSpecies;
    m_bookList->clearSelected();
    setupPageText();
  }
  m_selectLabel->setText(m_selectText.replaceTags(StringMap<String>{{"species", m_currentSpecies}}).titleCase());
}

void CodexInterface::setupPageText() {
  if (m_currentCodex) {
    m_pageContent->setText(m_currentCodex->page(m_currentPage));
    m_pageLabelWidget->show();
    m_pageNumberWidget->setText(strf("{} of {}", m_currentPage + 1, m_currentCodex->pageCount()));
    m_titleLabel->setText(m_currentCodex->title());
    m_nextPageButton->setEnabled(m_currentPage < m_currentCodex->pageCount() - 1);
    m_prevPageButton->setEnabled(m_currentPage > 0);
  } else {
    m_pageContent->setText("");
    m_pageLabelWidget->hide();
    m_pageNumberWidget->setText("");
    m_titleLabel->setText("");
    m_nextPageButton->disable();
    m_prevPageButton->disable();
  }
}

void CodexInterface::updateCodexList() {
  auto newCodexList = m_player->codexes()->codexes();
  filter(newCodexList, [&](auto const& p) {
      return p.first->species() == m_currentSpecies;
    });
  if (m_codexList != newCodexList) {
    m_bookList->removeAllChildren();
    m_codexList = newCodexList;
    for (auto entry : m_codexList) {
      auto newEntry = m_bookList->addItem();
      newEntry->fetchChild<LabelWidget>("bookName")->setText(entry.first->title());
      newEntry->fetchChild<ImageWidget>("bookIcon")->setImage(entry.first->icon());
    }
  }
}

}

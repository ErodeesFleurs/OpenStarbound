import std;
import star.memory;
import star.json;
import star.data_stream;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.format;
import star.version;
import star.string;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.id_map;
import star.rpc_promise;
import star.strong_typedef;
import star.a_star;
import star.ordered_set;
import star.random;
import star.periodic_function;
import star.either;


// Keep Uuid's textual definition ahead of the global codex interface on GCC.
import star.lua_root;
import star.uuid;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
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
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.label_widget;
import star.list_widget;
import star.stack_widget;
import star.image_widget;
import star.button_group;
import star.button_widget;

import star.algorithm;
namespace Star {

CodexInterface::CodexInterface(GuiContext& guiContext, std::shared_ptr<Player> player)
  : Pane(guiContext), m_currentPage(0), m_player(std::move(player)) {}

std::expected<std::shared_ptr<CodexInterface>, StarException> CodexInterface::create(GuiContext& guiContext, std::shared_ptr<Player> player) {
  auto pane = std::shared_ptr<CodexInterface>(new CodexInterface(guiContext, std::move(player)));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize();
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> CodexInterface::initialize() {
  auto assets = context()->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  GuiReader reader(*context());
  reader.registerCallback("close", [this](Widget*) -> std::expected<void, StarException> { dismiss(); return {}; });
  reader.registerCallback("prevButton", [this](Widget*) { return backwardPage(); });
  reader.registerCallback("nextButton", [this](Widget*) { return forwardPage(); });
  reader.registerCallback("selectCodex", [this](Widget*) { return showSelectedContents(); });
  reader.registerCallback("updateSpecies", [this](Widget*) { return updateSpecies(); });
  auto layout = (*assets)->json("/interface/windowconfig/codex.config:paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto constructed = reader.construct(*layout, this);
  if (!constructed) return std::unexpected(std::move(constructed.error()));
  auto speciesTabs = fetchChild<ButtonGroupWidget>("speciesTabs");
  if (!speciesTabs) return std::unexpected(std::move(speciesTabs.error()));
  m_speciesTabs = std::move(*speciesTabs);
  auto selectLabel = fetchChild<LabelWidget>("selectLabel");
  if (!selectLabel) return std::unexpected(std::move(selectLabel.error()));
  m_selectLabel = std::move(*selectLabel);
  auto titleLabel = fetchChild<LabelWidget>("titleLabel");
  if (!titleLabel) return std::unexpected(std::move(titleLabel.error()));
  m_titleLabel = std::move(*titleLabel);
  auto bookList = fetchChild<ListWidget>("scrollArea.bookList");
  if (!bookList) return std::unexpected(std::move(bookList.error()));
  m_bookList = std::move(*bookList);
  auto pageContent = fetchChild<LabelWidget>("pageText");
  if (!pageContent) return std::unexpected(std::move(pageContent.error()));
  m_pageContent = std::move(*pageContent);
  auto pageLabel = fetchChild<LabelWidget>("pageLabel");
  if (!pageLabel) return std::unexpected(std::move(pageLabel.error()));
  m_pageLabelWidget = std::move(*pageLabel);
  auto pageNumber = fetchChild<LabelWidget>("pageNum");
  if (!pageNumber) return std::unexpected(std::move(pageNumber.error()));
  m_pageNumberWidget = std::move(*pageNumber);
  auto prevPage = fetchChild<ButtonWidget>("prevButton");
  if (!prevPage) return std::unexpected(std::move(prevPage.error()));
  m_prevPageButton = std::move(*prevPage);
  auto nextPage = fetchChild<ButtonWidget>("nextButton");
  if (!nextPage) return std::unexpected(std::move(nextPage.error()));
  m_nextPageButton = std::move(*nextPage);
  auto selectConfig = (*assets)->json("/interface/windowconfig/codex.config:selectText");
  if (!selectConfig) return std::unexpected(std::move(selectConfig.error()));
  auto selectText = selectConfig->toString();
  if (!selectText) return std::unexpected(std::move(selectText.error()));
  m_selectText = std::move(*selectText);
  auto species = updateSpecies();
  if (!species) return std::unexpected(std::move(species.error()));
  return setupPageText();
}

std::expected<void, StarException> CodexInterface::show() {
  auto shown = Pane::show();
  if (!shown) return std::unexpected(std::move(shown.error()));
  return updateCodexList();
}

std::expected<void, StarException> CodexInterface::tick(float) {
  return updateCodexList();
}

std::expected<void, StarException> CodexInterface::showSelectedContents() {
  if (m_bookList->selectedItem() == NPos || m_bookList->selectedItem() >= m_codexList.size()) return {};
  return showContents(m_codexList[m_bookList->selectedItem()].first);
}

std::expected<void, StarException> CodexInterface::showContents(String const& codexId) {
  for (auto const& entry : m_codexList)
    if (entry.first->id() == codexId) return showContents(entry.first);
  return {};
}

std::expected<void, StarException> CodexInterface::showContents(std::shared_ptr<Codex const> codex) {
  if (m_player->codexes()->markCodexRead(codex->id())) {
    auto updated = updateCodexList();
    if (!updated) return std::unexpected(std::move(updated.error()));
  }
  m_currentCodex = std::move(codex);
  m_currentPage = 0;
  return setupPageText();
}

std::expected<void, StarException> CodexInterface::forwardPage() {
  if (m_currentCodex && m_currentPage < m_currentCodex->pageCount() - 1) {
    ++m_currentPage;
    return setupPageText();
  }
  return {};
}

std::expected<void, StarException> CodexInterface::backwardPage() {
  if (m_currentCodex && m_currentPage > 0) {
    --m_currentPage;
    return setupPageText();
  }
  return {};
}

std::expected<bool, StarException> CodexInterface::showNewCodex() {
  if (auto newCodex = m_player->codexes()->firstNewCodex()) {
    for (auto const& button : m_speciesTabs->buttons()) {
      auto species = button->data().getString("species");
      if (!species) return std::unexpected(std::move(species.error()));
      if (*species == newCodex->species()) {
        auto selected = m_speciesTabs->select(m_speciesTabs->id(button));
        if (!selected) return std::unexpected(std::move(selected.error()));
        break;
      }
    }
    auto shown = showContents(newCodex);
    if (!shown) return std::unexpected(std::move(shown.error()));
    return true;
  }
  return false;
}

std::expected<void, StarException> CodexInterface::updateSpecies() {
  String newSpecies = "other";
  if (auto speciesButton = m_speciesTabs->checkedButton()) {
    auto species = speciesButton->data().getString("species");
    if (!species) return std::unexpected(std::move(species.error()));
    newSpecies = std::move(*species);
  }
  if (newSpecies != m_currentSpecies) {
    m_currentCodex = {};
    m_currentSpecies = std::move(newSpecies);
    auto cleared = m_bookList->clearSelected();
    if (!cleared) return std::unexpected(std::move(cleared.error()));
    auto page = setupPageText();
    if (!page) return std::unexpected(std::move(page.error()));
  }
  auto text = m_selectText.replaceTags(StringMap<String>{{"species", m_currentSpecies}}).titleCase();
  if (!text) return std::unexpected(std::move(text.error()));
  return m_selectLabel->setText(*text);
}

std::expected<void, StarException> CodexInterface::setupPageText() {
  String content;
  if (m_currentCodex)
    content = m_currentCodex->page(m_currentPage);
  auto text = m_pageContent->setText(content);
  if (!text) return std::unexpected(std::move(text.error()));
  auto label = m_pageLabelWidget->setVisibility(bool(m_currentCodex));
  if (!label) return std::unexpected(std::move(label.error()));
  auto number = m_pageNumberWidget->setText(m_currentCodex ? strf("{} of {}", m_currentPage + 1, m_currentCodex->pageCount()) : "");
  if (!number) return std::unexpected(std::move(number.error()));
  auto title = m_titleLabel->setText(m_currentCodex ? m_currentCodex->title() : "");
  if (!title) return std::unexpected(std::move(title.error()));
  m_nextPageButton->setEnabled(m_currentCodex && m_currentPage < m_currentCodex->pageCount() - 1);
  m_prevPageButton->setEnabled(m_currentCodex && m_currentPage > 0);
  return {};
}

std::expected<void, StarException> CodexInterface::updateCodexList() {
  auto newCodexList = m_player->codexes()->codexes();
  filter(newCodexList, [&](auto const& p) { return p.first->species() == m_currentSpecies; });
  if (m_codexList != newCodexList) {
    m_bookList->removeAllChildren();
    m_codexList = std::move(newCodexList);
    for (auto const& entry : m_codexList) {
      auto newEntry = m_bookList->addItem();
      if (!newEntry) return std::unexpected(std::move(newEntry.error()));
      auto bookName = (*newEntry)->fetchChild<LabelWidget>("bookName");
      if (!bookName) return std::unexpected(std::move(bookName.error()));
      auto text = (*bookName)->setText(entry.first->title());
      if (!text) return std::unexpected(std::move(text.error()));
      auto bookIcon = (*newEntry)->fetchChild<ImageWidget>("bookIcon");
      if (!bookIcon) return std::unexpected(std::move(bookIcon.error()));
      auto image = (*bookIcon)->setImage(entry.first->icon());
      if (!image) return std::unexpected(std::move(image.error()));
    }
  }
  return {};
}

}

import std;
import star.format;
import star.memory;
import star.json;
import star.xx_hash;
import star.json_extra;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.data_stream;
import star.bi_map;
import star.game_types;
import star.set;
import star.json_rpc;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.vector;
import star.strong_typedef;
import star.input_event;
import star.font;
import star.text;
import star.listener;
import star.audio;
import star.map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.format;
import star.version;
import star.either;
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
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.btree_database;
import star.rpc_promise;
import star.image;
import star.a_star;
import star.periodic_function;

import star.lua_root;
import star.lua_actor_movement_component;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.lua_components;
import star.uuid;
import star.warping;
import star.quests;
import star.quest_manager;

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
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
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
import star.lua_animation_component;
import star.radio_message_database;
import star.player;

import star.item_tooltip;
import star.quest_template_database;


import star.item_bag;

namespace Star {

namespace {
template <typename T, typename Callback>
[[nodiscard]] std::expected<void, StarException> questChildAction(Widget& parent, String const& name, Callback&& callback, bool optional = false) {
  auto child = parent.fetchChild<T>(name);
  if (!child) return std::unexpected(std::move(child.error()));
  if (!*child) {
    if (optional) return {};
    return std::unexpected(StarException(strf("Missing quest interface widget '{}'", name)));
  }
  return std::forward<Callback>(callback)(**child);
}

[[nodiscard]] std::expected<void, StarException> questLabel(Widget& parent, String const& name, String const& text, bool optional = false) {
  return questChildAction<LabelWidget>(parent, name, [&](LabelWidget& label) { return label.setText(text); }, optional);
}

[[nodiscard]] std::expected<void, StarException> questVisibility(Widget& parent, String const& name, bool visible, bool optional = false) {
  return questChildAction<Widget>(parent, name, [&](Widget& child) { return child.setVisibility(visible); }, optional);
}

[[nodiscard]] std::expected<void, StarException> questPortrait(Widget& parent, String const& name, List<Drawable> drawables, bool optional = false) {
  return questChildAction<ImageWidget>(parent, name, [&](ImageWidget& child) { return child.setDrawables(std::move(drawables)); }, optional);
}
}

std::expected<void, StarException> pollQuestDialog(GuiContext& context, PaneManager& paneManager,
    std::shared_ptr<QuestManager> const& manager, std::shared_ptr<Player> const& player,
    std::shared_ptr<Cinematic> const& cinematic) {
  if (paneManager.topPane({PaneLayer::ModalWindow})) return {};
  if (auto failableQuest = manager->getFirstFailableQuest()) {
    auto pane = QuestFailedInterface::create(context, *failableQuest, player);
    if (!pane) return std::unexpected(std::move(pane.error()));
    (*failableQuest)->setDialogShown();
    return paneManager.displayPane(PaneLayer::ModalWindow, *pane);
  } else if (auto completableQuest = manager->getFirstCompletableQuest()) {
    auto pane = QuestCompleteInterface::create(context, *completableQuest, player, cinematic);
    if (!pane) return std::unexpected(std::move(pane.error()));
    (*completableQuest)->setDialogShown();
    return paneManager.displayPane(PaneLayer::ModalWindow, *pane);
  } else if (auto newQuest = manager->getFirstNewQuest()) {
    auto pane = NewQuestInterface::create(context, manager, *newQuest, player);
    if (!pane) return std::unexpected(std::move(pane.error()));
    return paneManager.displayPane(PaneLayer::ModalWindow, *pane);
  }
  return {};
}

QuestPane::QuestPane(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player) : Pane(guiContext), m_quest(quest), m_player(std::move(player)) {}

std::expected<void, StarException> QuestPane::commonSetup(Json config, String bodyText, String const& portraitName) {
  GuiReader reader(context());
  reader.registerCallback("close", [this](Widget&) { return close(); });
  reader.registerCallback("btnDecline", [this](Widget&) { return decline(); });
  reader.registerCallback("btnAccept", [this](Widget&) { return accept(); });
  auto layout = config.get("paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto constructed = reader.construct(*layout, *this);
  if (!constructed) return std::unexpected(std::move(constructed.error()));
  auto title = questLabel(*this, "lblQuestTitle", m_quest->title(), true);
  if (!title) return std::unexpected(std::move(title.error()));
  auto body = questLabel(*this, "lblQuestBody", bodyText, true);
  if (!body) return std::unexpected(std::move(body.error()));
  auto portrait = questPortrait(*this, "portraitImage", m_quest->portrait(portraitName).value_or(List<Drawable>{}), true);
  if (!portrait) return std::unexpected(std::move(portrait.error()));
  auto portraitTitle = questLabel(*this, "portraitTitle", m_quest->portraitTitle(portraitName).value_or(""), true);
  if (!portraitTitle) return std::unexpected(std::move(portraitTitle.error()));
  auto rewardItemsWidget = fetchChild<ItemGridWidget>("rewardItems");
  if (!rewardItemsWidget) return std::unexpected(std::move(rewardItemsWidget.error()));
  if (*rewardItemsWidget) {
    auto rewardItems = std::make_shared<ItemBag>(context().root(), 5);
    for (auto const& reward : m_quest->rewards()) {
      auto clone = reward->clone();
      if (!clone) return std::unexpected(std::move(clone.error()));
      auto added = rewardItems->addItems(std::move(*clone));
      if (!added) return std::unexpected(std::move(added.error()));
    }
    auto assigned = (*rewardItemsWidget)->setItemBag(std::move(rewardItems));
    if (!assigned) return std::unexpected(std::move(assigned.error()));
  }
  auto soundJson = config.get("onShowSound");
  if (!soundJson) return std::unexpected(std::move(soundJson.error()));
  auto sounds = soundJson->toArray();
  if (!sounds) return std::unexpected(std::move(sounds.error()));
  auto sound = context().root().random().randValueFrom(*sounds, "").toString();
  if (!sound) return std::unexpected(std::move(sound.error()));
  if (!sound->empty()) {
    auto played = context().playAudio(*sound);
    if (!played) return std::unexpected(std::move(played.error()));
  }
  return {};
}

std::expected<void, StarException> QuestPane::close() {
  dismiss();
  return {};
}

std::expected<void, StarException> QuestPane::decline() {
  return close();
}

std::expected<void, StarException> QuestPane::accept() {
  return close();
}

std::expected<std::shared_ptr<Pane>, StarException> QuestPane::createTooltip(Vec2I const& screenPosition) {
  std::shared_ptr<Item> item;
  if (auto child = getChildAt(screenPosition)) {
    if (auto itemSlot = as<ItemSlotWidget>(child))
      item = itemSlot->item();
    if (auto itemGrid = as<ItemGridWidget>(child)) {
      auto found = itemGrid->itemAt(screenPosition);
      if (!found) return std::unexpected(std::move(found.error()));
      item = std::move(*found);
    }
  }
  if (item)
    return ItemTooltipBuilder::buildItemTooltip(context(), item, m_player);
  return std::shared_ptr<Pane>{};
}

NewQuestInterface::NewQuestInterface(GuiContext& guiContext, std::shared_ptr<QuestManager> const& manager, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player)
  : QuestPane(guiContext, quest, std::move(player)), m_manager(manager), m_decision(QuestDecision::Cancelled) {}

std::expected<std::shared_ptr<NewQuestInterface>, StarException> NewQuestInterface::create(GuiContext& guiContext, std::shared_ptr<QuestManager> const& manager, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player) {
  auto pane = std::shared_ptr<NewQuestInterface>(new NewQuestInterface(guiContext, manager, quest, std::move(player)));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize();
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> NewQuestInterface::initialize() {
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  List<Drawable> objectivePortrait = m_quest->portrait("Objective").value_or(List<Drawable>{});
  bool shortDialog = objectivePortrait.empty();
  auto questTemplate = m_quest->getTemplate();
  if (!questTemplate) return std::unexpected(std::move(questTemplate.error()));
  auto defaultConfig = (*assets)->json(shortDialog ? String("/quests/quests.config:defaultGuiConfigs.newQuest") : String("/quests/quests.config:defaultGuiConfigs.newQuestPortrait"));
  if (!defaultConfig) return std::unexpected(std::move(defaultConfig.error()));
  auto defaultPath = defaultConfig->toString();
  if (!defaultPath) return std::unexpected(std::move(defaultPath.error()));
  auto config = (*assets)->json((*questTemplate)->newQuestGuiConfig.value_or(*defaultPath));
  if (!config) return std::unexpected(std::move(config.error()));
  auto setup = commonSetup(*config, m_quest->text(), "QuestStarted");
  if (!setup) return std::unexpected(std::move(setup.error()));
  auto abandonable = m_quest->canBeAbandoned();
  if (!abandonable) return std::unexpected(std::move(abandonable.error()));
  if (!*abandonable) {
    auto declined = questChildAction<ButtonWidget>(*this, "btnDecline", [](ButtonWidget& button) -> std::expected<void, StarException> { button.disable(); return {}; }, true);
    if (!declined) return std::unexpected(std::move(declined.error()));
  }
  if (!shortDialog) {
    auto objectivePortraitImage = fetchChild<ImageWidget>("objectivePortraitImage");
    if (!objectivePortraitImage) return std::unexpected(std::move(objectivePortraitImage.error()));
    if (*objectivePortraitImage) {
      Drawable::scaleAll(objectivePortrait, Vec2F(-1, 1));
      bool visible = !objectivePortrait.empty();
      auto portrait = (*objectivePortraitImage)->setDrawables(std::move(objectivePortrait));
      if (!portrait) return std::unexpected(std::move(portrait.error()));
      auto title = questLabel(*this, "objectivePortraitTitle", m_quest->portraitTitle("Objective").value_or(""));
      if (!title) return std::unexpected(std::move(title.error()));
      for (auto const& name : {String("objectivePortraitTitle"), String("imgPolaroid"), String("imgPolaroidBack")}) {
        auto visibility = questVisibility(*this, name, visible);
        if (!visibility) return std::unexpected(std::move(visibility.error()));
      }
    }
  }
  for (auto const& name : {String("rewardItems"), String("lblRewards")}) {
    auto visibility = questVisibility(*this, name, !m_quest->rewards().empty(), true);
    if (!visibility) return std::unexpected(std::move(visibility.error()));
  }
  return {};
}

std::expected<void, StarException> NewQuestInterface::close() {
  m_decision = QuestDecision::Cancelled;
  dismiss();
  return {};
}

std::expected<void, StarException> NewQuestInterface::decline() {
  m_decision = QuestDecision::Declined;
  dismiss();
  return {};
}

std::expected<void, StarException> NewQuestInterface::accept() {
  m_decision = QuestDecision::Accepted;
  dismiss();
  return {};
}

std::expected<void, StarException> NewQuestInterface::dismissed() {
  auto dismissed = QuestPane::dismissed();
  if (!dismissed) return std::unexpected(std::move(dismissed.error()));
  auto quest = m_manager->getQuest(m_quest->questId());
  if (!quest) return std::unexpected(std::move(quest.error()));
  if (m_decision == QuestDecision::Declined) {
    auto abandonable = m_quest->canBeAbandoned();
    if (!abandonable) return std::unexpected(std::move(abandonable.error()));
    if (*abandonable) return (*quest)->declineOffer();
  } else if (m_decision == QuestDecision::Accepted) {
    return (*quest)->start();
  }
  return (*quest)->cancelOffer();
}

QuestCompleteInterface::QuestCompleteInterface(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic)
  : QuestPane(guiContext, quest, player), m_player(std::move(player)), m_cinematic(std::move(cinematic)) {}

std::expected<std::shared_ptr<QuestCompleteInterface>, StarException> QuestCompleteInterface::create(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player, std::shared_ptr<Cinematic> cinematic) {
  auto pane = std::shared_ptr<QuestCompleteInterface>(new QuestCompleteInterface(guiContext, quest, std::move(player), std::move(cinematic)));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto initialized = pane->initialize();
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return pane;
}

std::expected<void, StarException> QuestCompleteInterface::initialize() {
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto questTemplate = m_quest->getTemplate();
  if (!questTemplate) return std::unexpected(std::move(questTemplate.error()));
  auto defaultConfig = (*assets)->json("/quests/quests.config:defaultGuiConfigs.questComplete");
  if (!defaultConfig) return std::unexpected(std::move(defaultConfig.error()));
  auto defaultPath = defaultConfig->toString();
  if (!defaultPath) return std::unexpected(std::move(defaultPath.error()));
  auto config = (*assets)->json((*questTemplate)->questCompleteGuiConfig.value_or(*defaultPath));
  if (!config) return std::unexpected(std::move(config.error()));
  auto setup = commonSetup(*config, m_quest->completionText(), "QuestComplete");
  if (!setup) return std::unexpected(std::move(setup.error()));
  auto money = questLabel(*this, "lblMoneyAmount", toString(m_quest->money()), true);
  if (!money) return std::unexpected(std::move(money.error()));
  disableScissoring();
  return {};
}

std::expected<void, StarException> QuestCompleteInterface::close() {
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto completionCinema = m_quest->completionCinema();
  if (!completionCinema) return std::unexpected(std::move(completionCinema.error()));
  if (*completionCinema && m_cinematic) {
    if (!GenderNames) return std::unexpected(GenderNames.error());
    auto gender = GenderNames->getRight(m_player->gender());
    if (!gender) return std::unexpected(std::move(gender.error()));
    auto cinema = (**completionCinema).replaceTags(StringMap<String>{{"species", m_player->species()}, {"gender", gender->get()}});
    auto config = (*assets)->fetchJson(cinema);
    if (!config) return std::unexpected(std::move(config.error()));
    auto loaded = m_cinematic->load(*config);
    if (!loaded) return std::unexpected(std::move(loaded.error()));
  }
  dismiss();
  return {};
}

QuestFailedInterface::QuestFailedInterface(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player)
  : QuestPane(guiContext, quest, std::move(player)) {}

std::expected<std::shared_ptr<QuestFailedInterface>, StarException> QuestFailedInterface::create(GuiContext& guiContext, std::shared_ptr<Quest> const& quest, std::shared_ptr<Player> player) {
  auto pane = std::shared_ptr<QuestFailedInterface>(new QuestFailedInterface(guiContext, quest, std::move(player)));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto assets = guiContext.root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto questTemplate = quest->getTemplate();
  if (!questTemplate) return std::unexpected(std::move(questTemplate.error()));
  auto defaultConfig = (*assets)->json("/quests/quests.config:defaultGuiConfigs.questFailed");
  if (!defaultConfig) return std::unexpected(std::move(defaultConfig.error()));
  auto defaultPath = defaultConfig->toString();
  if (!defaultPath) return std::unexpected(std::move(defaultPath.error()));
  auto config = (*assets)->json((*questTemplate)->questFailedGuiConfig.value_or(*defaultPath));
  if (!config) return std::unexpected(std::move(config.error()));
  auto setup = pane->commonSetup(*config, quest->failureText(), "QuestFailed");
  if (!setup) return std::unexpected(std::move(setup.error()));
  pane->disableScissoring();
  return pane;
}

}

import star.json_extra;
import std;
import star.optional;
import star.json;
import star.xx_hash;
import star.algorithm;
import star.id_map;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.ordered_map;
import star.either;
import star.vector;
import star.poly;
import star.game_types;
import star.color;
import star.bi_map;
import star.random;
import star.weighted_pool;
import star.logging;
import star.output_proxy;
import star.lexical_cast;
import star.casting;
import star.input_event;
import star.font;
import star.string;
import star.text;
import star.listener;
import star.set;
import star.audio;
import star.map;
import star.interpolation;
import star.image;
import star.math_common;
import star.lru_cache;
import star.time;
import star.ttl_cache;
import star.perlin;
import star.random_point;
import star.rpc_promise;
import star.byte_array;
import star.encode;
import star.bytes;
import star.format;
import star.version;
import star.strong_typedef;
import star.ordered_set;
import star.io_device;
import star.ref_ptr;
import star.data_stream_devices;

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;
import star.net_compatibility;
import star.worker_pool;

import star.sector_array_2d;
import star.btree;
import star.btree_database;
import star.a_star;
import star.periodic_function;
import star.lua;

import star.lua_root;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
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
import star.root;
import star.host_address;
import star.socket;
import star.tcp;
import star.tile_sector_array;
import star.world_layout;
import star.tile_modification;
import star.inspectable_entity;
import star.plant;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.system_world;
import star.damage_manager;
import star.net_packets;
import star.net_packet_socket;
import star.universe_connection;
import star.world_structure;
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
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.container_entity;
import star.inventory;
import star.interface_cursor;
import star.main_interface;

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



import star.container_interactor;
import star.widget_parsing;
import star.gui_reader;


import star.container_interface;

import star.liquid_types;
import star.collision_generator;
import star.world_tiles;
import star.entity_rendering_types;
import star.sky_parameters;
import star.sky_render_data;
import star.plant_database;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.material_render_profile;
import star.tile_drawer;
import star.tile_painter;
import star.environment_painter;
import star.world_camera;
import star.world_painter;
import star.item_descriptor;
import star.item_recipe;


import star.crafting_interface;



import star.merchant_interface;

import star.uuid;
import star.player_codexes;


import star.codex_interface;



import star.songbook_interface;



import star.quest_interface;



import star.popup_interface;



import star.confirmation_dialog;



import star.join_request_dialog;
import star.game_timers;
import star.pane_manager;
import star.shell_parser;
import star.cinematic;
import star.client_command_processor;




import star.base_script_pane;
import star.chat_types;


import star.chat;

import star.configuration;

import star.registered_pane_manager;


import star.main_interface_types;


import star.options_menu;

import star.inventory_types;





import star.action_bar;

import star.world_geometry;
import star.wiring;


import star.wire_interface;




import star.progress_widget;
import star.label_widget;


import star.team_bar;






import star.status_pane;
import star.canvas_widget;
import star.item_slot_widget;
import star.button_group;
import star.button_widget;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.actor_movement_controller;
import star.player;
import star.player_log;
import star.aggressive_entity;
import star.scripted_entity;
import star.monster_database;
import star.effect_emitter;
import star.monster;
import star.item_drop;
import star.item;

import star.quest_descriptor;
import star.ai_types;
import star.warping;



import star.tech_database;



import star.ai_interface;
import star.fireable_item;
import star.tool_user_entity;


import star.player_universe_map;


import star.bookmark_interface;


import star.teleport_dialog;




import star.chat_bubble_separation;



import star.nameplate_painter;




import star.quest_indicator_painter;






import star.script_pane;
import star.warp_target_entity;
import star.world_template;


import star.radio_message_database;



import star.radio_message_popup;



import star.quest_tracker;




import star.chat_action;






import star.chat_bubble_manager;
import star.npc_database;
import star.npc;



import star.char_selection;

import star.inspection_tool;
import star.active_item;
import star.http_trust_dialog;
import star.image_metadata_database;
import star.celestial_database;


import star.player_inventory;
import star.client_context;
import star.player_storage;

import star.quest_manager;

namespace Star {

GuiMessage::GuiMessage() : message(), cooldown(), springState() {}

GuiMessage::GuiMessage(String const& message, float cooldown, float spring)
  : message(message), cooldown(cooldown), springState(spring) {}

MainInterface::MainInterface(GuiContext& guiContext, std::shared_ptr<UniverseClient> client, std::shared_ptr<WorldPainter> painter, std::shared_ptr<Cinematic> cinematicOverlay)
  : m_guiContext(&guiContext)
  , m_client(std::move(client))
  , m_worldPainter(std::move(painter))
  , m_cinematicOverlay(std::move(cinematicOverlay))
  , m_paneManager(guiContext)
  , m_containerInteractor(std::make_shared<ContainerInteractor>(guiContext.root())) {}

std::expected<std::shared_ptr<MainInterface>, StarException> MainInterface::create(GuiContext& guiContext, std::shared_ptr<UniverseClient> client, std::shared_ptr<WorldPainter> painter, std::shared_ptr<Cinematic> cinematicOverlay) {
  auto interface = std::shared_ptr<MainInterface>(new MainInterface(guiContext, std::move(client), std::move(painter), std::move(cinematicOverlay)));
  auto initialized = interface->initialize();
  if (!initialized)
    return std::unexpected(std::move(initialized.error()));
  return interface;
}

std::expected<void, StarException> MainInterface::initialize() {
  auto config = MainInterfaceConfig::loadFromAssets(*m_guiContext);
  if (!config)
    return std::unexpected(std::move(config.error()));
  m_config = std::move(*config);
  auto cursor = InterfaceCursor::create(*m_guiContext);
  if (!cursor)
    return std::unexpected(std::move(cursor.error()));
  m_cursor = std::move(*cursor);
  if (auto initialized = m_paneManager.initialize(); !initialized)
    return std::unexpected(std::move(initialized.error()));

  auto install = [this]<typename T>(MainInterfacePanes id, PaneLayer layer, std::expected<std::shared_ptr<T>, StarException> pane, std::shared_ptr<T>& destination, PaneManager::DismissCallback dismissed = {}) -> std::expected<void, StarException> {
    if (!pane)
      return std::unexpected(std::move(pane.error()));
    auto registered = m_paneManager.registerPane(id, layer, *pane, std::move(dismissed));
    if (!registered)
      return std::unexpected(std::move(registered.error()));
    destination = std::move(*pane);
    return {};
  };

  GuiReader itemSlotReader(*m_guiContext);
  auto slot = itemSlotReader.makeSingle("cursorItemSlot", m_config->cursorItemSlot);
  if (!slot)
    return std::unexpected(std::move(slot.error()));
  auto cursorItem = convert<ItemSlotWidget>(*slot);
  if (!cursorItem)
    return std::unexpected(std::move(cursorItem.error()));
  m_cursorItem = std::move(*cursorItem);

  m_planetNameTimer = GameTimer(m_config->planetNameTime);
  m_debugSpatialClearTimer = GameTimer(m_config->debugSpatialClearTime);
  m_debugMapClearTimer = GameTimer(m_config->debugMapClearTime);
  m_stickyTargetingTimer = GameTimer(m_config->monsterHealthBarTime);

  auto inventoryDismissed = [this](std::shared_ptr<Pane> const&) -> std::expected<void, StarException> {
    if (auto player = m_client->mainPlayer()) {
      if (auto cleared = player->clearSwap(); !cleared)
        return std::unexpected(std::move(cleared.error()));
    }
    if (m_containerPane) {
      m_containerPane->dismiss();
      m_containerPane.reset();
      if (auto closed = m_containerInteractor->closeContainer(); !closed)
        return std::unexpected(std::move(closed.error()));
    }
    for (EntityId id : m_interactionScriptPanes.keyView()) {
      auto pane = as<ScriptPane>(m_interactionScriptPanes[id]);
      if (m_paneManager.isDisplayed(pane)) {
        auto withInventory = pane->openWithInventory();
        if (!withInventory)
          return std::unexpected(std::move(withInventory.error()));
        if (*withInventory)
          pane->dismiss();
      }
    }
    return {};
  };
  if (auto result = install(MainInterfacePanes::Inventory, PaneLayer::Window, InventoryPane::create(*m_guiContext, this, m_client->mainPlayer(), m_containerInteractor), m_inventoryWindow, inventoryDismissed); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::CraftingPlain, PaneLayer::Window, CraftingPane::create(*m_guiContext, m_client->worldClient(), m_client->mainPlayer(), JsonObject{{"filter", JsonArray{"plain"}}}, m_client->mainPlayer()->entityId()), m_plainCraftingWindow); !result)
    return std::unexpected(std::move(result.error()));
  auto escape = createEscapeDialog();
  if (!escape)
    return std::unexpected(std::move(escape.error()));
  if (auto result = m_paneManager.registerPane(MainInterfacePanes::EscapeDialog, PaneLayer::ModalWindow, *escape); !result)
    return std::unexpected(std::move(result.error()));
  auto songbook = SongbookInterface::create(*m_guiContext, m_client->mainPlayer());
  if (!songbook)
    return std::unexpected(std::move(songbook.error()));
  if (auto result = m_paneManager.registerPane(MainInterfacePanes::Songbook, PaneLayer::Window, *songbook); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::QuestLog, PaneLayer::Window, QuestLogInterface::create(*m_guiContext, m_client->questManager(), m_client->mainPlayer(), m_cinematicOverlay, m_client), m_questLogInterface); !result)
    return std::unexpected(std::move(result.error()));
  auto ai = AiInterface::create(*m_guiContext, m_client, m_cinematicOverlay, &m_paneManager);
  if (!ai)
    return std::unexpected(std::move(ai.error()));
  if (auto result = m_paneManager.registerPane(MainInterfacePanes::Ai, PaneLayer::Window, *ai); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::Codex, PaneLayer::Window, CodexInterface::create(*m_guiContext, m_client->mainPlayer()), m_codexInterface); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::Options, PaneLayer::ModalWindow, OptionsMenu::create(*m_guiContext, &m_paneManager, m_client), m_optionsMenu); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::Popup, PaneLayer::Window, PopupInterface::create(*m_guiContext), m_popupInterface); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::Confirmation, PaneLayer::ModalWindow, ConfirmationDialog::create(*m_guiContext), m_confirmationDialog); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = initHttpTrustDialog(); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::JoinRequest, PaneLayer::ModalWindow, JoinRequestDialog::create(*m_guiContext), m_joinRequestDialog); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::ActionBar, PaneLayer::Hud, ActionBar::create(*m_guiContext, &m_paneManager, m_client->mainPlayer()), m_actionBar); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::QuestTracker, PaneLayer::Hud, QuestTrackerPane::create(*m_guiContext), m_questTracker); !result)
    return std::unexpected(std::move(result.error()));

  auto assets = m_guiContext->root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto mmUpgrade = (*assets)->json("/interface.config:mainBar.mmUpgrade");
  if (!mmUpgrade)
    return std::unexpected(std::move(mmUpgrade.error()));
  auto mmUpgradeConfig = mmUpgrade->getString("scriptPane", "/interface/scripted/mmupgrade/mmupgradegui.config");
  if (!mmUpgradeConfig)
    return std::unexpected(std::move(mmUpgradeConfig.error()));
  if (auto result = install(MainInterfacePanes::MmUpgrade, PaneLayer::Window, ScriptPane::create(*m_guiContext, m_client, *mmUpgradeConfig), m_mmUpgrade); !result)
    return std::unexpected(std::move(result.error()));
  auto collections = (*assets)->json("/interface.config:mainBar.collections");
  if (!collections)
    return std::unexpected(std::move(collections.error()));
  auto collectionConfig = collections->getString("scriptPane", "/interface/scripted/collections/collectionsgui.config");
  if (!collectionConfig)
    return std::unexpected(std::move(collectionConfig.error()));
  if (auto result = install(MainInterfacePanes::Collections, PaneLayer::Window, ScriptPane::create(*m_guiContext, m_client, *collectionConfig), m_collections); !result)
    return std::unexpected(std::move(result.error()));
  auto chatConfig = (*assets)->json("/interface/chat/chat.config");
  if (!chatConfig)
    return std::unexpected(std::move(chatConfig.error()));
  if (auto result = install(MainInterfacePanes::Chat, PaneLayer::Hud, Chat::create(*m_guiContext, m_client, *chatConfig), m_chat); !result)
    return std::unexpected(std::move(result.error()));
  m_clientCommandProcessor = std::make_shared<ClientCommandProcessor>(*m_guiContext, m_client, m_cinematicOverlay, &m_paneManager, m_config->macroCommands);
  if (auto result = install(MainInterfacePanes::RadioMessagePopup, PaneLayer::Hud, RadioMessagePopup::create(*m_guiContext), m_radioMessagePopup); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = install(MainInterfacePanes::WireInterface, PaneLayer::World, WirePane::create(*m_guiContext, m_client->worldClient(), m_client->mainPlayer(), m_worldPainter), m_wireInterface); !result)
    return std::unexpected(std::move(result.error()));
  m_client->mainPlayer()->setWireConnector(m_wireInterface.get());

  auto team = TeamBar::create(*m_guiContext, this, m_client);
  if (!team)
    return std::unexpected(std::move(team.error()));
  if (auto result = m_paneManager.registerPane(MainInterfacePanes::TeamBar, PaneLayer::Hud, *team); !result)
    return std::unexpected(std::move(result.error()));
  auto status = StatusPane::create(*m_guiContext, &m_paneManager, m_client);
  if (!status)
    return std::unexpected(std::move(status.error()));
  if (auto result = m_paneManager.registerPane(MainInterfacePanes::StatusPane, PaneLayer::Hud, *status); !result)
    return std::unexpected(std::move(result.error()));

  auto planetName = Pane::create(*m_guiContext);
  if (!planetName)
    return std::unexpected(std::move(planetName.error()));
  auto planetText = LabelWidget::create(*m_guiContext);
  if (!planetText)
    return std::unexpected(std::move(planetText.error()));
  m_planetText = std::move(*planetText);
  if (auto result = m_planetText->setTextStyle(m_config->planetNameTextStyle); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = m_planetText->setAnchor(HorizontalAnchor::HMidAnchor, VerticalAnchor::VMidAnchor); !result)
    return std::unexpected(std::move(result.error()));
  (*planetName)->disableScissoring();
  (*planetName)->setPosition(m_config->planetNameOffset);
  (*planetName)->setAnchor(PaneAnchor::Center);
  if (auto result = (*planetName)->addChild("planetText", m_planetText); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = m_paneManager.registerPane(MainInterfacePanes::PlanetText, PaneLayer::Hud, *planetName); !result)
    return std::unexpected(std::move(result.error()));

  auto selection = CharSelectionPane::create(*m_guiContext, m_client->playerStorage(), []() -> std::expected<void, StarException> { return {}; },
    [this](std::shared_ptr<Player> mainPlayer) -> std::expected<void, StarException> {
      auto playerUuid = mainPlayer->uuid();
      if (!playerUuid)
        return std::unexpected(std::move(playerUuid.error()));
      if (auto result = m_client->switchPlayer(*playerUuid); !result)
        return std::unexpected(std::move(result.error()));
      auto configuration = m_guiContext->root().configuration();
      if (!configuration)
        return std::unexpected(std::move(configuration.error()));
      auto moves = (*configuration)->get("characterSwapMovesToFront", false);
      if (!moves)
        return std::unexpected(std::move(moves.error()));
      auto move = moves->toBool();
      if (!move)
        return std::unexpected(std::move(move.error()));
      if (*move) {
        if (auto result = m_client->playerStorage()->moveToFront(*playerUuid); !result)
          return std::unexpected(std::move(result.error()));
      }
      auto dismissConfig = (*configuration)->get("characterSwapDismisses", false);
      if (!dismissConfig)
        return std::unexpected(std::move(dismissConfig.error()));
      auto dismiss = dismissConfig->toBool();
      if (!dismiss)
        return std::unexpected(std::move(dismiss.error()));
      if (*dismiss) {
        if (auto result = m_paneManager.dismissRegisteredPane(MainInterfacePanes::CharacterSwap); !result)
          return std::unexpected(std::move(result.error()));
      }
      return {};
    }, [](Uuid) -> std::expected<void, StarException> { return {}; });
  if (!selection)
    return std::unexpected(std::move(selection.error()));
  auto charSelectionMenu = std::move(*selection);
  if (auto result = charSelectionMenu->setReadOnly(true); !result)
    return std::unexpected(std::move(result.error()));
  charSelectionMenu->setAnchor(PaneAnchor::Center);
  charSelectionMenu->unlockPosition();
  auto backgrounds = charSelectionMenu->getBG();
  backgrounds.header = std::move(backgrounds.body);
  if (auto result = charSelectionMenu->setBG(backgrounds); !result)
    return std::unexpected(std::move(result.error()));
  auto dismissLabel = charSelectionMenu->requireChild("toggleDismissLabel");
  if (!dismissLabel)
    return std::unexpected(std::move(dismissLabel.error()));
  if (auto result = (*dismissLabel)->setVisibility(true); !result)
    return std::unexpected(std::move(result.error()));
  auto toggleDismiss = charSelectionMenu->requireChild<ButtonWidget>("toggleDismissCheckbox");
  if (!toggleDismiss)
    return std::unexpected(std::move(toggleDismiss.error()));
  auto configuration = m_guiContext->root().configuration();
  if (!configuration)
    return std::unexpected(std::move(configuration.error()));
  auto dismissConfig = (*configuration)->get("characterSwapDismisses", false);
  if (!dismissConfig)
    return std::unexpected(std::move(dismissConfig.error()));
  auto dismiss = dismissConfig->toBool();
  if (!dismiss)
    return std::unexpected(std::move(dismiss.error()));
  (*toggleDismiss)->setChecked(*dismiss);
  if (auto result = (*toggleDismiss)->setVisibility(true); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = m_paneManager.registerPane(MainInterfacePanes::CharacterSwap, PaneLayer::Window, charSelectionMenu); !result)
    return std::unexpected(std::move(result.error()));
  auto nameplates = NameplatePainter::create(*m_guiContext);
  if (!nameplates)
    return std::unexpected(std::move(nameplates.error()));
  m_nameplatePainter = std::move(*nameplates);
  m_questIndicatorPainter = std::make_shared<QuestIndicatorPainter>(*m_guiContext, m_client);
  auto bubbles = ChatBubbleManager::create(*m_guiContext);
  if (!bubbles)
    return std::unexpected(std::move(bubbles.error()));
  m_chatBubbleManager = std::move(*bubbles);
  return {};
}

MainInterface::~MainInterface() {
  auto dismissed = m_paneManager.dismissAllPanes();
  if (!dismissed)
    EmergencyLog::error("MainInterface: failed to dismiss panes during teardown: {}", dismissed.error().what());
}

std::expected<void, StarException> MainInterface::close() {
  return m_paneManager.dismissAllPanes();
}

MainInterface::RunningState MainInterface::currentState() const {
  return m_state;
}

MainInterfacePaneManager* MainInterface::paneManager() {
  return &m_paneManager;
}

GuiContext& MainInterface::context() const {
  return *m_guiContext;
}

std::expected<bool, StarException> MainInterface::escapeDialogOpen() const {
  auto escape = m_paneManager.registeredPaneIsDisplayed(MainInterfacePanes::EscapeDialog);
  if (!escape)
    return std::unexpected(std::move(escape.error()));
  if (*escape)
    return true;
  return m_paneManager.registeredPaneIsDisplayed(MainInterfacePanes::Options);
}

std::expected<void, StarException> MainInterface::openCraftingWindow(Json const& config, EntityId sourceEntityId) {
  if (m_craftingWindow && m_paneManager.isDisplayed(m_craftingWindow)) {
    if (auto dismissed = m_paneManager.dismissPane(m_craftingWindow); !dismissed)
      return std::unexpected(std::move(dismissed.error()));
    if (sourceEntityId != NullEntityId && m_craftingWindow->sourceEntityId() == sourceEntityId) {
      m_craftingWindow.reset();
      return {};
    }
  }
  auto window = CraftingPane::create(*m_guiContext, m_client->worldClient(), m_client->mainPlayer(), config, sourceEntityId);
  if (!window)
    return std::unexpected(std::move(window.error()));
  m_craftingWindow = std::move(*window);
  auto displayed = m_paneManager.displayPane(PaneLayer::Window, m_craftingWindow, [this](std::shared_ptr<Pane> const&) -> std::expected<void, StarException> {
    if (auto player = m_client->mainPlayer())
      return player->clearSwap();
    return {};
  });
  if (!displayed)
    return std::unexpected(std::move(displayed.error()));
  return {};
}

std::expected<void, StarException> MainInterface::openMerchantWindow(Json const& config, EntityId sourceEntityId) {
  if (m_merchantWindow && m_paneManager.isDisplayed(m_merchantWindow)) {
    if (auto dismissed = m_paneManager.dismissPane(m_merchantWindow); !dismissed)
      return std::unexpected(std::move(dismissed.error()));
    if (sourceEntityId != NullEntityId && m_merchantWindow->sourceEntityId() == sourceEntityId) {
      m_merchantWindow.reset();
      return {};
    }
  }
  auto openWithInventory = config.getBool("openWithInventory", true);
  if (!openWithInventory)
    return std::unexpected(std::move(openWithInventory.error()));
  auto inventoryDisplayed = m_paneManager.registeredPaneIsDisplayed(MainInterfacePanes::Inventory);
  if (!inventoryDisplayed)
    return std::unexpected(std::move(inventoryDisplayed.error()));
  auto closeWithInventory = config.getBool("closeWithInventory", !*inventoryDisplayed);
  if (!closeWithInventory)
    return std::unexpected(std::move(closeWithInventory.error()));
  auto window = MerchantPane::create(*m_guiContext, m_client->worldClient(), m_client->mainPlayer(), config, sourceEntityId);
  if (!window)
    return std::unexpected(std::move(window.error()));
  m_merchantWindow = std::move(*window);
  auto displayed = m_paneManager.displayPane(PaneLayer::Window, m_merchantWindow, [this, close = *closeWithInventory](std::shared_ptr<Pane> const&) -> std::expected<void, StarException> {
    if (close) {
      auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::Inventory);
      if (!dismissed)
        return std::unexpected(std::move(dismissed.error()));
    }
    return {};
  });
  if (!displayed)
    return std::unexpected(std::move(displayed.error()));
  if (*openWithInventory) {
    auto opened = m_paneManager.displayRegisteredPane(MainInterfacePanes::Inventory);
    if (!opened)
      return std::unexpected(std::move(opened.error()));
  }
  auto inventory = m_paneManager.registeredPane(MainInterfacePanes::Inventory);
  if (!inventory)
    return std::unexpected(std::move(inventory.error()));
  auto assets = m_guiContext->root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto gapConfig = (*assets)->json("/interface.config:bringAdjacentWindowGap");
  if (!gapConfig)
    return std::unexpected(std::move(gapConfig.error()));
  auto gap = gapConfig->toFloat();
  if (!gap)
    return std::unexpected(std::move(gap.error()));
  return m_paneManager.bringPaneAdjacent(*inventory, m_merchantWindow, *gap);
}

std::expected<void, StarException> MainInterface::togglePlainCraftingWindow() {
  auto toggled = m_paneManager.toggleRegisteredPane(MainInterfacePanes::CraftingPlain);
  if (!toggled)
    return std::unexpected(std::move(toggled.error()));
  if (m_craftingWindow && m_craftingWindow->isDisplayed() && m_craftingWindow != m_plainCraftingWindow) {
    auto dismissed = m_paneManager.dismissPane(m_craftingWindow);
    if (!dismissed)
      return std::unexpected(std::move(dismissed.error()));
  }
  m_craftingWindow = m_plainCraftingWindow;
  return {};
}

bool MainInterface::windowsOpen() const {
  return (bool)m_paneManager.topPane({PaneLayer::Window});
}

std::shared_ptr<MerchantPane> MainInterface::activeMerchantPane() const {
  if (m_paneManager.isDisplayed(m_merchantWindow))
    return m_merchantWindow;
  else
    return {};
}

std::expected<bool, StarException> MainInterface::handleInputEvent(InputEvent const& event) {
  auto player = m_client->mainPlayer();
  auto& root = m_guiContext->root();
  if (auto mouseMove = std::get_if<MouseMoveEvent>(&event))
    m_cursorScreenIPos = Vec2I::round(m_cursorScreenPos = mouseMove->mousePosition);

  auto paneConsumed = m_paneManager.sendInputEvent(event);
  if (!paneConsumed)
    return std::unexpected(std::move(paneConsumed.error()));
  if (*paneConsumed && !std::holds_alternative<MouseButtonUpEvent>(event) && !std::holds_alternative<KeyUpEvent>(event))
    return true;

  if (std::holds_alternative<KeyDownEvent>(event)) {
    auto chatFocus = m_chat->hasFocus();
    if (!chatFocus)
      return std::unexpected(std::move(chatFocus.error()));
    if (*chatFocus) {
      if (m_guiContext->actions(event).contains(InterfaceAction::ChatSendLine)) {
        auto chat = m_chat->currentChat();
        if (!chat)
          return std::unexpected(std::move(chat.error()));
        if (auto sent = doChat(*chat, true); !sent)
          return std::unexpected(std::move(sent.error()));
        if (auto cleared = m_chat->clearCurrentChat(); !cleared)
          return std::unexpected(std::move(cleared.error()));
        if (auto stopped = m_chat->stopChat(); !stopped)
          return std::unexpected(std::move(stopped.error()));
        return true;
      }
    } else {
      auto capturedWidget = m_paneManager.keyboardCapturedWidget();
      if (!capturedWidget)
        return std::unexpected(std::move(capturedWidget.error()));
      if (!*capturedWidget) {
      for (auto action : m_guiContext->actions(event)) {
        auto handled = [&, action]() -> std::expected<void, StarException> {
          switch (action) {
            case InterfaceAction::GuiShifting:
              m_guiContext->setShiftHeld(true);
              return {};
            case InterfaceAction::ChatBegin:
              return m_chat->startChat();
            case InterfaceAction::InterfaceHideHud:
              m_disableHud = !m_disableHud;
              return {};
            case InterfaceAction::InterfaceRepeatCommand:
              if (!m_lastCommand.empty())
                return doChat(m_lastCommand, false);
              return {};
            case InterfaceAction::InterfaceToggleFullscreen:
              return m_optionsMenu->toggleFullscreen();
            case InterfaceAction::InterfaceReload:
              if (auto reloaded = root.reload(); !reloaded)
                return std::unexpected(std::move(reloaded.error()));
              return root.fullyLoad();
            case InterfaceAction::ChatBeginCommand:
              return m_chat->startCommand();
            case InterfaceAction::InterfaceCrafting:
              return togglePlainCraftingWindow();
            case InterfaceAction::InterfaceEscapeMenu:
            case InterfaceAction::InterfaceInventory:
            case InterfaceAction::InterfaceCodex:
            case InterfaceAction::InterfaceQuest: {
              auto pane = action == InterfaceAction::InterfaceEscapeMenu ? MainInterfacePanes::EscapeDialog
                : action == InterfaceAction::InterfaceInventory ? MainInterfacePanes::Inventory
                : action == InterfaceAction::InterfaceCodex ? MainInterfacePanes::Codex : MainInterfacePanes::QuestLog;
              auto toggled = m_paneManager.toggleRegisteredPane(pane);
              if (!toggled)
                return std::unexpected(std::move(toggled.error()));
              return {};
            }
            default:
              return {};
          }
        }();
        if (!handled)
          return std::unexpected(std::move(handled.error()));
      }
      }
    }
    return false;
  } else if (auto keyUp = std::get_if<KeyUpEvent>(&event)) {
    if (m_guiContext->actionsForKey(keyUp->key).contains(InterfaceAction::GuiShifting))
      m_guiContext->setShiftHeld(false);
    return false;
  } else if (auto mouseDown = std::get_if<MouseButtonDownEvent>(&event)) {
    auto button = mouseDown->mouseButton;
    if (button >= MouseButton::Left && button <= MouseButton::Right) {
      auto overlay = overlayClick(mouseDown->mousePosition, button);
      if (!overlay)
        return std::unexpected(std::move(overlay.error()));
      if (*overlay)
        return true;
    }
    if (button == MouseButton::Left)
      player->beginPrimaryFire();
    if (button == MouseButton::Right)
      player->beginAltFire();
    if (button == MouseButton::Middle)
      player->beginTrigger();
  } else if (auto mouseUp = std::get_if<MouseButtonUpEvent>(&event)) {
    if (mouseUp->mouseButton == MouseButton::Left)
      player->endPrimaryFire();
    if (mouseUp->mouseButton == MouseButton::Right)
      player->endAltFire();
    if (mouseUp->mouseButton == MouseButton::Middle)
      player->endTrigger();
  }
  bool captured = false;
  for (auto& pair : m_canvases) {
    auto consumed = pair.second->sendEvent(event);
    if (!consumed)
      return std::unexpected(std::move(consumed.error()));
    captured |= *consumed;
  }
  return captured;
}

std::expected<bool, StarException> MainInterface::inputFocus() const {
  auto widget = m_paneManager.keyboardCapturedWidget();
  if (!widget)
    return std::unexpected(std::move(widget.error()));
  return static_cast<bool>(*widget);
}

std::expected<bool, StarException> MainInterface::textInputActive() const {
  return m_paneManager.keyboardCapturedForTextInput();
}

std::expected<void, StarException> MainInterface::handleInteractAction(InteractAction interactAction) {
  auto assets = m_guiContext->root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto world = m_client->worldClient();

  if (interactAction.type == InteractActionType::OpenContainer) {
    if (m_containerPane && m_containerInteractor->openContainerId() == interactAction.entityId)
      return m_paneManager.dismissPane(m_containerPane);
    if (m_containerPane) {
      if (auto dismissed = m_paneManager.dismissPane(m_containerPane); !dismissed)
        return std::unexpected(std::move(dismissed.error()));
    }
    auto entity = world->get<ContainerEntity>(interactAction.entityId);
    if (!entity)
      return {};
    auto inventoryDisplayed = m_paneManager.registeredPaneIsDisplayed(MainInterfacePanes::Inventory);
    if (!inventoryDisplayed)
      return std::unexpected(std::move(inventoryDisplayed.error()));
    auto opened = m_containerInteractor->openContainer(entity);
    if (!opened)
      return std::unexpected(std::move(opened.error()));
    auto containerPane = ContainerPane::create(*m_guiContext, world, m_client->mainPlayer(), m_containerInteractor);
    if (!containerPane) {
      auto closed = m_containerInteractor->closeContainer();
      if (!closed)
        return std::unexpected(containerPane.error().withContext(strf("Closing container after pane construction failure also failed: {}", outputException(ExceptionDiagnostic(closed.error()), true))));
      return std::unexpected(std::move(containerPane.error()));
    }
    auto inventory = m_paneManager.displayRegisteredPane(MainInterfacePanes::Inventory);
    if (!inventory) {
      auto closed = m_containerInteractor->closeContainer();
      if (!closed)
        return std::unexpected(inventory.error().withContext(strf("Closing container after inventory display failure also failed: {}", outputException(ExceptionDiagnostic(closed.error()), true))));
      return std::unexpected(std::move(inventory.error()));
    }
    m_containerPane = std::move(*containerPane);
    auto displayed = m_paneManager.displayPane(PaneLayer::Window, m_containerPane, [this, closeWithInventory = !*inventoryDisplayed](std::shared_ptr<Pane> const&) -> std::expected<void, StarException> {
      if (closeWithInventory) {
        auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::Inventory);
        if (!dismissed)
          return std::unexpected(std::move(dismissed.error()));
      } else {
        auto closed = m_containerInteractor->closeContainer();
        m_containerPane.reset();
        if (!closed)
          return std::unexpected(std::move(closed.error()));
      }
      return {};
    });
    if (!displayed) {
      auto closed = m_containerInteractor->closeContainer();
      m_containerPane.reset();
      if (!closed)
        return std::unexpected(displayed.error().withContext(strf("Closing container after pane display failure also failed: {}", outputException(ExceptionDiagnostic(closed.error()), true))));
      return std::unexpected(std::move(displayed.error()));
    }
    auto inventoryPane = m_paneManager.registeredPane(MainInterfacePanes::Inventory);
    if (!inventoryPane)
      return std::unexpected(std::move(inventoryPane.error()));
    auto gapConfig = (*assets)->json("/interface.config:bringAdjacentWindowGap");
    if (!gapConfig)
      return std::unexpected(std::move(gapConfig.error()));
    auto gap = gapConfig->toFloat();
    if (!gap)
      return std::unexpected(std::move(gap.error()));
    return m_paneManager.bringPaneAdjacent(*inventoryPane, m_containerPane, *gap);
  } else if (interactAction.type == InteractActionType::SitDown) {
    auto index = interactAction.data.toUInt();
    if (!index)
      return std::unexpected(std::move(index.error()));
    auto lounged = m_client->mainPlayer()->lounge(interactAction.entityId, *index);
    if (!lounged)
      return std::unexpected(std::move(lounged.error()));
  } else if (interactAction.type == InteractActionType::OpenCraftingInterface || interactAction.type == InteractActionType::OpenNpcCraftingInterface) {
    if (interactAction.entityId != NullEntityId && !world->entity(interactAction.entityId))
      return {};
    return openCraftingWindow(interactAction.data, interactAction.entityId);
  } else if (interactAction.type == InteractActionType::OpenSongbookInterface) {
    auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Songbook);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
  } else if (interactAction.type == InteractActionType::OpenMerchantInterface) {
    if (interactAction.entityId != NullEntityId && !world->entity(interactAction.entityId))
      return {};
    return openMerchantWindow(interactAction.data, interactAction.entityId);
  } else if (interactAction.type == InteractActionType::OpenAiInterface) {
    auto pane = m_paneManager.registeredPane<AiInterface>(MainInterfacePanes::Ai);
    if (!pane)
      return std::unexpected(std::move(pane.error()));
    (*pane)->setSourceEntityId(interactAction.entityId);
    auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Ai);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
  } else if (interactAction.type == InteractActionType::OpenTeleportDialog) {
    if (m_teleportDialog)
      m_teleportDialog->dismiss();
    auto teleportAllowed = m_client->canTeleport();
    if (!teleportAllowed)
      return std::unexpected(std::move(teleportAllowed.error()));
    if (!*teleportAllowed)
      return {};
    TeleportBookmark currentLocation;
    auto config = (*assets)->fetchJson(interactAction.data);
    if (!config)
      return std::unexpected(std::move(config.error()));
    auto canBookmark = config->getBool("canBookmark", false);
    if (!canBookmark)
      return std::unexpected(std::move(canBookmark.error()));
    auto canTeleport = config->getBool("canTeleport", true);
    if (!canTeleport)
      return std::unexpected(std::move(canTeleport.error()));
    if (*canBookmark) {
      if (auto entity = world->entity(interactAction.entityId)) {
        if (auto uniqueEntityId = entity->uniqueId()) {
          auto worldTemplate = world->currentTemplate();
          String icon, planetName;
          auto playerWorld = m_client->playerWorld();
          if (std::holds_alternative<ClientShipWorldId>(playerWorld)) {
            icon = "ship";
            planetName = "Player Ship";
          } else if (std::holds_alternative<CelestialWorldId>(playerWorld)
              || std::holds_alternative<InstanceWorldId>(playerWorld)
              || std::holds_alternative<CustomWorldId>(playerWorld)
              || std::holds_alternative<ClientCustomWorldId>(playerWorld)) {
            icon = worldTemplate->worldParameters()->typeName;
            planetName = worldTemplate->worldName();
          } else {
            icon = "default";
            planetName = "???";
          }
          auto name = config->getString("bookmarkName", "");
          if (!name)
            return std::unexpected(std::move(name.error()));
          currentLocation = TeleportBookmark{{playerWorld, SpawnTargetUniqueEntity(*uniqueEntityId)}, planetName, *name, icon};
          auto bookmarks = m_client->mainPlayer()->universeMap()->teleportBookmarks();
          if (!bookmarks)
            return std::unexpected(std::move(bookmarks.error()));
          if (!bookmarks->contains(currentLocation) || !*canTeleport) {
            auto dialog = EditBookmarkDialog::create(*m_guiContext, m_client->mainPlayer()->universeMap());
            if (!dialog)
              return std::unexpected(std::move(dialog.error()));
            if (auto set = (*dialog)->setBookmark(currentLocation); !set)
              return std::unexpected(std::move(set.error()));
            return m_paneManager.displayPane(PaneLayer::ModalWindow, *dialog);
          }
        }
      }
    }
    if (*canTeleport) {
      auto dialog = TeleportDialog::create(*m_guiContext, m_client, &m_paneManager, interactAction.data, interactAction.entityId, currentLocation);
      if (!dialog)
        return std::unexpected(std::move(dialog.error()));
      m_teleportDialog = std::move(*dialog);
      auto displayed = m_paneManager.displayPane(PaneLayer::ModalWindow, m_teleportDialog);
      if (!displayed)
        return std::unexpected(std::move(displayed.error()));
    }
  } else if (interactAction.type == InteractActionType::ShowPopup) {
    auto message = interactAction.data.getString("message");
    if (!message)
      return std::unexpected(std::move(message.error()));
    auto title = interactAction.data.getString("title", "");
    if (!title)
      return std::unexpected(std::move(title.error()));
    auto subtitle = interactAction.data.getString("subtitle", "");
    if (!subtitle)
      return std::unexpected(std::move(subtitle.error()));
    auto sound = interactAction.data.optString("sound");
    if (!sound)
      return std::unexpected(std::move(sound.error()));
    auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Popup);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
    return m_popupInterface->displayMessage(*message, *title, *subtitle, *sound);
  } else if (interactAction.type == InteractActionType::ScriptPane) {
    auto sourceEntity = interactAction.entityId;
    if (sourceEntity != NullEntityId && m_interactionScriptPanes.contains(sourceEntity) && m_paneManager.isDisplayed(m_interactionScriptPanes[sourceEntity])) {
      if (auto dismissed = m_paneManager.dismissPane(m_interactionScriptPanes[sourceEntity]); !dismissed)
        return std::unexpected(std::move(dismissed.error()));
    }
    auto pane = ScriptPane::create(*m_guiContext, m_client, interactAction.data, sourceEntity);
    if (!pane)
      return std::unexpected(std::move(pane.error()));
    return displayScriptPane(*pane, sourceEntity);
  } else if (interactAction.type == InteractActionType::Message) {
    auto type = interactAction.data.getString("messageType");
    if (!type)
      return std::unexpected(std::move(type.error()));
    auto arguments = interactAction.data.getArray("messageArgs");
    if (!arguments)
      return std::unexpected(std::move(arguments.error()));
    auto received = m_client->mainPlayer()->receiveMessage(connectionForEntity(interactAction.entityId), *type, *arguments);
    if (!received)
      return std::unexpected(std::move(received.error()));
  }
  return {};
}

std::expected<void, StarException> MainInterface::preUpdate(float) {
  auto player = m_client->mainPlayer();
  if (!m_client->paused())
    player->aim(cursorWorldPosition());
  if (m_paneManager.topPane({PaneLayer::Window, PaneLayer::ModalWindow}))
    return player->setBusyState(PlayerBusyState::Menu);
  auto chatFocus = m_chat->hasFocus();
  if (!chatFocus)
    return std::unexpected(std::move(chatFocus.error()));
  return player->setBusyState(*chatFocus ? PlayerBusyState::Chatting : PlayerBusyState::None);
}

std::expected<void, StarException> MainInterface::update(float dt) {
  auto panesUpdated = m_paneManager.update(dt);
  if (!panesUpdated)
    return std::unexpected(std::move(panesUpdated.error()));
  auto cursorUpdated = m_cursor->update(dt);
  if (!cursorUpdated)
    return std::unexpected(std::move(cursorUpdated.error()));

  auto dialogPolled = m_questLogInterface->pollDialog(&m_paneManager);
  if (!dialogPolled)
    return std::unexpected(std::move(dialogPolled.error()));

  if (!m_paneManager.topPane({PaneLayer::ModalWindow})) {
    auto newCodex = m_codexInterface->showNewCodex();
    if (!newCodex)
      return std::unexpected(std::move(newCodex.error()));
    if (*newCodex) {
      auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Codex);
      if (!displayed)
        return std::unexpected(std::move(displayed.error()));
    }
  }

  auto player = m_client->mainPlayer();
  auto cursorWorldPos = cursorWorldPosition();
  if (player->wireToolInUse()) {
    auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::WireInterface);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
    player->setWireConnector(m_wireInterface.get());
  } else {
    auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::WireInterface);
    if (!dismissed)
      return std::unexpected(std::move(dismissed.error()));
  }

  // update inventory pane items, to know if item slots changed
  auto inventoryUpdated = m_inventoryWindow->updateItems();
  if (!inventoryUpdated)
    return std::unexpected(std::move(inventoryUpdated.error()));

  // update mouseover target
  EntityId newMouseOverTarget = NullEntityId;
  m_stickyTargetingTimer.tick(dt);
  auto mouseoverEntities = m_client->worldClient()->query<DamageBarEntity>(RectF::withCenter(cursorWorldPos, Vec2F(1, 1)), [=, this](std::shared_ptr<DamageBarEntity> const& entity) -> std::expected<bool, StarException> {
      if (entity == player
          || entity->damageBar() != DamageBarType::Default
          || (entity->getTeam().type != TeamType::Enemy && entity->getTeam().type != TeamType::PVP))
        return false;
      auto light = m_client->worldClient()->lightLevel(entity->position());
      if (!light)
        return std::unexpected(std::move(light.error()));
      return *light > 0;
    });
  if (!mouseoverEntities)
    return std::unexpected(std::move(mouseoverEntities.error()));
  sortByComputedValue(*mouseoverEntities, [&](std::shared_ptr<DamageBarEntity> const& a) {
      return m_client->worldClient()->geometry().diff(a->position(), cursorWorldPos).magnitude();
    });
  if (!mouseoverEntities->empty()) {
    newMouseOverTarget = (*mouseoverEntities)[0]->entityId();
  } else if (m_lastMouseoverTarget == NullEntityId && player->lastDamagedTarget() != NullEntityId && player->timeSinceLastGaveDamage() < m_stickyTargetingTimer.time / 2) {
    if (auto targetEntity = as<DamageBarEntity>(m_client->worldClient()->entity(player->lastDamagedTarget()))) {
      if (targetEntity->damageBar() == DamageBarType::Default && (targetEntity->getTeam().type == TeamType::Enemy || targetEntity->getTeam().type == TeamType::PVP)) {
        newMouseOverTarget = targetEntity->entityId();
      }
    }
  }
  if (newMouseOverTarget != NullEntityId && newMouseOverTarget != m_lastMouseoverTarget) {
    m_lastMouseoverTarget = newMouseOverTarget;
    m_portraitScale = 0;
    m_stickyTargetingTimer.reset();
  }

  if (m_stickyTargetingTimer.ready())
    m_lastMouseoverTarget = NullEntityId;

  // special damage bar entities
  auto assets = m_guiContext->root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto barConfig = (*assets)->json("/interface.config:specialDamageBar");
  if (!barConfig)
    return std::unexpected(std::move(barConfig.error()));
  auto maxBars = barConfig->getUInt("maxCount", 10);
  if (!maxBars)
    return std::unexpected(std::move(maxBars.error()));

  for (auto it = m_specialDamageBars.begin(); it != m_specialDamageBars.end();) {
    auto& bar = *it;
    auto damageBarEntity = as<DamageBarEntity>(m_client->worldClient()->entity(bar.first));
    if (damageBarEntity && damageBarEntity->damageBar() == DamageBarType::Special) {
      auto maxHealth = damageBarEntity->maxHealth();
      if (!maxHealth)
        return std::unexpected(std::move(maxHealth.error()));
      float targetHealth = damageBarEntity->health() / *maxHealth;
      auto fillTime = barConfig->getFloat("fillTime");
      if (!fillTime)
        return std::unexpected(std::move(fillTime.error()));
      float fillSpeed = 1.0f / *fillTime;
      if (std::abs(targetHealth - bar.second) < fillSpeed * dt)
        bar.second = targetHealth;
      else
        bar.second += std::copysign(1.0f, targetHealth - bar.second) * fillSpeed * dt;

      it++;
    } else {
      it = m_specialDamageBars.erase(it);
    }
  }

  if (m_specialDamageBars.size() < *maxBars && m_client->mainPlayer()->inWorld()) {
    List<std::shared_ptr<DamageBarEntity>> specialDamageTargets;
    auto collected = m_client->worldClient()->forAllEntities([&specialDamageTargets](std::shared_ptr<Entity> const& entity) -> std::expected<void, StarException> {
        if (auto damageBarEntity = as<DamageBarEntity>(entity))
          if (damageBarEntity->damageBar() == DamageBarType::Special)
            specialDamageTargets.append(damageBarEntity);
        return {};
      });
    if (!collected)
      return std::unexpected(std::move(collected.error()));
    sortByComputedValue(specialDamageTargets, [&](std::shared_ptr<DamageBarEntity> entity) {
        return m_client->worldClient()->geometry().diff(entity->position(), m_client->mainPlayer()->position());
      });

    for (std::size_t i = 0; i < specialDamageTargets.size(); i++) {
      auto id = specialDamageTargets[i]->entityId();
      if (!m_specialDamageBars.contains(id)) {
        auto added = m_specialDamageBars.add(id, 0);
        if (!added)
          return std::unexpected(std::move(added.error()));
        if (m_specialDamageBars.size() >= *maxBars)
          break;
      }
    }
  }

  for (auto const& message : m_client->mainPlayer()->pullQueuedMessages()) {
    auto queued = queueMessage(message);
    if (!queued)
      return std::unexpected(std::move(queued.error()));
  }

  int chatHeight = 0;
  if (m_chat->active()) {
    auto visible = m_chat->visible();
    if (!visible)
      return std::unexpected(std::move(visible.error()));
    if (*visible > 0.1)
      chatHeight = m_chat->size()[1];
  }
  m_radioMessagePopup->setChatHeight(chatHeight);
  if (!m_cinematicOverlay || m_cinematicOverlay->completed()) {
    if (m_client->mainPlayer()->interruptRadioMessage()) {
      auto interrupted = m_radioMessagePopup->interrupt();
      if (!interrupted)
        return std::unexpected(std::move(interrupted.error()));
    }
    if (!m_radioMessagePopup->messageActive()) {
      auto pendingRadioMessage = m_client->mainPlayer()->pullPendingRadioMessage();
      if (!pendingRadioMessage)
        return std::unexpected(std::move(pendingRadioMessage.error()));
      if (auto& radioMessage = *pendingRadioMessage) {
        auto shown = m_radioMessagePopup->setMessage(*radioMessage);
        if (!shown)
          return std::unexpected(std::move(shown.error()));
        auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::RadioMessagePopup);
        if (!displayed)
          return std::unexpected(std::move(displayed.error()));
        auto portrait = radioMessage->portraitImage.replace("<frame>", "0");
        if (!portrait)
          return std::unexpected(std::move(portrait.error()));
        ChatReceivedMessage message = {
          {MessageContext::RadioMessage},
          ServerConnectionId,
          radioMessage->senderName,
          radioMessage->text,
          Text::stripEscapeCodes(*portrait)
        };
        auto added = m_chat->addMessages({message}, false);
        if (!added)
          return std::unexpected(std::move(added.error()));
      } else {
        auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::RadioMessagePopup);
        if (!dismissed)
          return std::unexpected(std::move(dismissed.error()));
      }
    }

    auto cinematicState = m_client->mainPlayer()->setInCinematic(false);
    if (!cinematicState)
      return std::unexpected(std::move(cinematicState.error()));
  } else {
    auto cinematicState = m_client->mainPlayer()->setInCinematic(true);
    if (!cinematicState)
      return std::unexpected(std::move(cinematicState.error()));
  }

  for (auto const& drop : m_client->mainPlayer()->pullQueuedItemDrops()) {
    auto queued = queueItemPickupText(drop);
    if (!queued)
      return std::unexpected(std::move(queued.error()));
  }

  auto chatAdded = m_chat->addMessages(m_client->pullChatMessages());
  if (!chatAdded)
    return std::unexpected(std::move(chatAdded.error()));
  auto commandResults = m_clientCommandProcessor->updatePromises();
  if (!commandResults)
    return std::unexpected(std::move(commandResults.error()));
  for (auto const& result : *commandResults) {
    auto added = m_chat->addLine(result);
    if (!added)
      return std::unexpected(std::move(added.error()));
  }

  if (auto worldClient = m_client->worldClient()) {
    if (worldClient->inWorld()) {
      auto pendingCinematic = m_client->mainPlayer()->pullPendingCinematic();
      if (!pendingCinematic)
        return std::unexpected(std::move(pendingCinematic.error()));
      if (auto& cinematic = *pendingCinematic) {
        if (*cinematic) {
          Json definition;
          auto fetched = (*assets)->fetchJson(std::move(*cinematic));
          if (!fetched) {
            auto logged = m_guiContext->root().logger().warn("MainInterface: failed to fetch cinematic: {}", fetched.error().what());
            if (!logged)
              return std::unexpected(std::move(logged.error()));
          } else {
            definition = std::move(*fetched);
          }
          if (definition.isType(Json::Type::Object)) {
            auto loaded = m_cinematicOverlay->load(definition);
            if (!loaded) {
              auto logged = m_guiContext->root().logger().warn("MainInterface: rejected malformed cinematic ({}), stopping overlay", loaded.error().what());
              if (!logged)
                return std::unexpected(std::move(logged.error()));
              m_cinematicOverlay->stop();
            }
          } else {
            auto logged = m_guiContext->root().logger().warn("MainInterface: ignoring non-object cinematic of type {}", (int)definition.type());
            if (!logged)
              return std::unexpected(std::move(logged.error()));
            m_cinematicOverlay->stop();
          }
        } else {
          m_cinematicOverlay->stop();
        }
      }
    }
  }

  if (!m_confirmationDialog->isDisplayed()) {
    auto pendingConfirmation = m_client->mainPlayer()->pullPendingConfirmation();
    if (!pendingConfirmation)
      return std::unexpected(std::move(pendingConfirmation.error()));
    if (auto& confirmation = *pendingConfirmation) {
      auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Confirmation);
      if (!displayed)
        return std::unexpected(std::move(displayed.error()));
      auto confirmed = m_confirmationDialog->displayConfirmation(confirmation->first, std::move(confirmation->second));
      if (!confirmed)
        return std::unexpected(std::move(confirmed.error()));
    }
  } else {
    auto confirmationSource = m_confirmationDialog->sourceEntityId();
    if (confirmationSource) {
      auto reachable = m_client->worldClient()->playerCanReachEntity(*confirmationSource);
      if (!reachable)
        return std::unexpected(std::move(reachable.error()));
      if (!*reachable)
        m_confirmationDialog->dismiss();
    }
  }

  if (!m_joinRequestDialog->isDisplayed()) {
    if (auto req = m_queuedJoinRequests.maybeTakeLast()) {
      auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::JoinRequest);
      if (!displayed)
        return std::unexpected(std::move(displayed.error()));
      auto userName = std::move(req->first);
      auto requested = m_joinRequestDialog->displayRequest(userName, [req = std::move(*req)](P2PJoinRequestReply reply) mutable -> std::expected<void, StarException> {
          return req.second.fulfill(reply);
        });
      if (!requested)
        return std::unexpected(std::move(requested.error()));
    }
  }

  for (EntityId id : m_interactionScriptPanes.keys()) {
    if (!m_paneManager.isDisplayed(m_interactionScriptPanes[id]))
      m_interactionScriptPanes.remove(id);
  }

  if (!m_messages.contains(m_overflowMessage))
    m_messageOverflow = 0;
  unsigned maxMessages = m_messageOverflow == 0 ? m_config->maxMessageCount : m_config->maxMessageCount + 1; // exclude overflow message
  if (m_messages.size() > maxMessages) {
    if (m_messageOverflow == 0) {
      m_messages.prepend(m_overflowMessage);
    }

    m_messageOverflow++;
    auto overflowText = m_config->overflowMessageText.replace("<count>", toString(m_messageOverflow));
    if (!overflowText)
      return std::unexpected(std::move(overflowText.error()));
    m_overflowMessage->message = std::move(*overflowText);
    m_overflowMessage->cooldown = m_config->messageTime;
    if (auto oldest = std::ranges::min_element(m_messages, {}, &GuiMessage::cooldown); oldest != m_messages.end())
      m_overflowMessage->cooldown = (*oldest)->cooldown;

    if (auto bottom = std::ranges::find_if(m_messages, [this](auto const& m) { return m != m_overflowMessage; }); bottom != m_messages.end())
      (*bottom)->cooldown = 0;
  }

  for (auto it = m_messages.begin(); it != m_messages.end();) {
    auto& message = *it;
    message->cooldown -= dt;
    if (message->cooldown < 0)
      it = m_messages.erase(it);
    else
      it++;
  }

  for (auto it = m_itemDropMessages.begin(); it != m_itemDropMessages.end();) {
    auto& message = *it;
    if (message.second.second->cooldown < 0)
      it = m_itemDropMessages.erase(it);
    else
      it++;
  }

  bool playerInWorld = m_client->mainPlayer()->inWorld();
  if (m_cinematicOverlay->completed()) {
    if (m_planetNameTimer.tick(dt)) {
      auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::PlanetText);
      if (!dismissed)
        return std::unexpected(std::move(dismissed.error()));
    } else {
      if (playerInWorld) {
        String worldName;
        if (auto worldTemplate = m_client->worldClient()->currentTemplate())
          worldName = worldTemplate->worldName();

        if (!worldName.empty()) {
          m_planetText->parent()->setPosition(m_config->planetNameOffset * (2.f / interfaceScale()));
          std::array<std::pair<std::string_view, std::string_view>, 1> tags{{{"worldName", worldName.utf8()}}};
          auto planetName = formatTags(m_config->planetNameFormatString.utf8(), tags);
          if (!planetName)
            return std::unexpected(std::move(planetName.error()));
          auto textSet = m_planetText->setText(String(std::move(*planetName)));
          if (!textSet)
            return std::unexpected(std::move(textSet.error()));
          auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::PlanetText);
          if (!displayed)
            return std::unexpected(std::move(displayed.error()));
        }
      }

      Color textColor = Color::White; // probably need to make this jsonable
      float fadeTimer = m_planetNameTimer.timer;
      if (fadeTimer < m_config->planetNameFadeTime)
        textColor.setAlphaF(fadeTimer / m_config->planetNameFadeTime);

      m_planetText->setColor(textColor);
    }
  } else if (!playerInWorld) {
    m_planetNameTimer.reset();
    auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::PlanetText);
    if (!dismissed)
      return std::unexpected(std::move(dismissed.error()));
  }

  auto containerResults = m_containerInteractor->pullContainerResults();
  if (!containerResults)
    return std::unexpected(std::move(containerResults.error()));
  for (auto& containerResult : *containerResults) {
    bool handled = false;
    if (m_containerPane) {
      auto given = m_containerPane->giveContainerResult(containerResult);
      if (!given)
        return std::unexpected(std::move(given.error()));
      handled = *given;
    }
    if (!handled) {
      auto given = m_inventoryWindow->giveContainerResult(containerResult);
      if (!given)
        return std::unexpected(std::move(given.error()));
      if (!*given) {
        for (auto const& item : containerResult) {
          if (m_containerInteractor->containerOpen()) {
            auto remaining = m_client->mainPlayer()->pickupItems(item);
            if (!remaining)
              return std::unexpected(std::move(remaining.error()));
            auto returned = m_containerInteractor->addToContainer(*remaining);
            if (!returned)
              return std::unexpected(std::move(returned.error()));
          } else {
            auto givenToPlayer = m_client->mainPlayer()->giveItem(item);
            if (!givenToPlayer)
              return std::unexpected(std::move(givenToPlayer.error()));
          }
        }
      }
    }
  }

  if (auto currentQuest = m_client->questManager()->currentQuest()) {
    auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::QuestTracker);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
    m_questTracker->setQuest(*currentQuest);
  } else {
    auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::QuestTracker);
    if (!dismissed)
      return std::unexpected(std::move(dismissed.error()));
  }

  auto cursorState = updateCursor();
  if (!cursorState)
    return std::unexpected(std::move(cursorState.error()));

  auto nameplatesUpdated = m_nameplatePainter->update(dt, m_client->worldClient(), m_worldPainter->camera(), m_client->worldClient()->interactiveHighlightMode());
  if (!nameplatesUpdated)
    return std::unexpected(std::move(nameplatesUpdated.error()));
  auto indicatorsUpdated = m_questIndicatorPainter->update(dt, m_client->worldClient(), m_worldPainter->camera());
  if (!indicatorsUpdated)
    return std::unexpected(std::move(indicatorsUpdated.error()));

  auto cameraSet = m_chatBubbleManager->setCamera(m_worldPainter->camera());
  if (!cameraSet)
    return std::unexpected(std::move(cameraSet.error()));
  if (auto worldClient = m_client->worldClient()) {
    auto chatActions = worldClient->pullPendingChatActions();
    if (!chatActions)
      return std::unexpected(std::move(chatActions.error()));

    for (auto& action : *chatActions) {
      if (std::holds_alternative<PortraitChatAction>(action)) {
        PortraitChatAction& portraitAction = std::get<PortraitChatAction>(action);

        String name;
        if (auto npc = as<Npc>(worldClient->entity(portraitAction.entity))) {
          auto npcName = npc->name();
          if (!npcName)
            return std::unexpected(std::move(npcName.error()));
          name = std::move(*npcName);
        }
        auto portrait = portraitAction.portrait.replace("<frame>", "0");
        if (!portrait)
          return std::unexpected(std::move(portrait.error()));

        ChatReceivedMessage message = {
          {MessageContext::World},
          ServerConnectionId,
          Text::stripEscapeCodes(name),
          Text::stripEscapeCodes(portraitAction.text),
          Text::stripEscapeCodes(*portrait)};
        auto added = m_chat->addMessages({message}, false);
        if (!added)
          return std::unexpected(std::move(added.error()));
      } else if (std::holds_alternative<SayChatAction>(action)) {
        SayChatAction& sayAction = std::get<SayChatAction>(action);

        if (sayAction.config) {
          auto message = sayAction.config.opt("message");
          if (!message)
            return std::unexpected(std::move(message.error()));
          if (*message) {
            auto received = ChatReceivedMessage::create(**message);
            if (!received)
              return std::unexpected(std::move(received.error()));
            auto showPane = sayAction.config.getBool("showPane", false);
            if (!showPane)
              return std::unexpected(std::move(showPane.error()));
            auto added = m_chat->addMessages({std::move(*received)}, *showPane);
            if (!added)
              return std::unexpected(std::move(added.error()));
          }
        }
      }
    }

    auto actionsAdded = m_chatBubbleManager->addChatActions(std::move(*chatActions));
    if (!actionsAdded)
      return std::unexpected(std::move(actionsAdded.error()));
    auto bubblesUpdated = m_chatBubbleManager->update(dt, worldClient);
    if (!bubblesUpdated)
      return std::unexpected(std::move(bubblesUpdated.error()));
  }

  if (auto container = m_client->worldClient()->get<ContainerEntity>(m_containerInteractor->openContainerId())) {
    auto reachable = m_client->worldClient()->playerCanReachEntity(container->entityId());
    if (!reachable)
      return std::unexpected(std::move(reachable.error()));
    if (!*reachable || !container->isInteractive()) {
      auto closed = m_containerInteractor->closeContainer();
      if (!closed)
        return std::unexpected(std::move(closed.error()));
    }
  }

  for (auto& pair : m_canvases) {
    pair.second->setPosition(Vec2I());
    auto size = pair.second->ignoreInterfaceScale() ? m_guiContext->windowSize() : m_guiContext->windowInterfaceSize();
    if (!size)
      return std::unexpected(std::move(size.error()));
    pair.second->setSize(Vec2I(*size));
    auto updated = pair.second->update(dt);
    if (!updated)
      return std::unexpected(std::move(updated.error()));
  }
  return {};
}

std::expected<void, StarException> MainInterface::renderInWorldElements() {
  if (m_disableHud)
    return {};
  if (auto cleared = m_guiContext->clearTextStyle(); !cleared)
    return std::unexpected(std::move(cleared.error()));
  if (auto rendered = m_questIndicatorPainter->render(); !rendered)
    return std::unexpected(std::move(rendered.error()));
  if (auto rendered = m_nameplatePainter->render(); !rendered)
    return std::unexpected(std::move(rendered.error()));
  return m_chatBubbleManager->render();
}

std::expected<void, StarException> MainInterface::render() {
  if (m_disableHud)
    return {};
  if (auto cleared = m_guiContext->clearTextStyle(); !cleared)
    return std::unexpected(std::move(cleared.error()));
  if (auto result = renderBreath(); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = renderMessages(); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = renderMonsterHealthBar(); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = renderSpecialDamageBar(); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = renderMainBar(); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = renderDebug(); !result)
    return std::unexpected(std::move(result.error()));
  auto size = m_guiContext->windowSize();
  if (!size)
    return std::unexpected(std::move(size.error()));
  RectI screenRect = RectI::withSize(Vec2I(), Vec2I(*size));
  for (auto& pair : m_canvases) {
    auto rendered = pair.second->render(screenRect);
    if (!rendered)
      return std::unexpected(std::move(rendered.error()));
  }
  if (auto rendered = renderWindows(); !rendered)
    return std::unexpected(std::move(rendered.error()));
  return renderCursor();
}

Vec2F MainInterface::cursorWorldPosition() const {
  return m_worldPainter->camera().screenToWorld(m_cursorScreenPos);
}

bool MainInterface::isDebugDisplayed() {
  return m_clientCommandProcessor->debugDisplayEnabled();
}

std::expected<void, StarException> MainInterface::doChat(String const& chat, bool addToHistory) {
  if (chat.empty())
    return {};
  auto command = chat.beginsWith("/");
  if (!command)
    return std::unexpected(std::move(command.error()));
  if (*command) {
    m_lastCommand = chat;
    auto results = m_clientCommandProcessor->handleCommand(chat, true);
    if (!results)
      return std::unexpected(std::move(results.error()));
    for (auto const& result : *results) {
      if (auto added = m_chat->addLine(result); !added)
        return std::unexpected(std::move(added.error()));
    }
  } else {
    auto mode = m_chat->sendMode();
    if (!mode)
      return std::unexpected(std::move(mode.error()));
    if (auto sent = m_client->sendChat(chat, *mode); !sent)
      return std::unexpected(std::move(sent.error()));
  }
  if (addToHistory)
    return m_chat->addHistory(chat);
  return {};
}

std::expected<void, StarException> MainInterface::queueMessage(String const& message, std::optional<float> cooldown, float spring) {
  m_messages.append(std::make_shared<GuiMessage>(message, cooldown.value_or(m_config->messageTime), spring));
  return {};
}

std::expected<void, StarException> MainInterface::queueMessage(String const& message) {
  return queueMessage(message, m_config->messageTime, 0.0f);
}

std::expected<void, StarException> MainInterface::queueJoinRequest(std::pair<String, RpcPromiseKeeper<P2PJoinRequestReply>> request) {
  m_queuedJoinRequests.push_back(std::move(request));
  return {};
}

std::expected<void, StarException> MainInterface::queueItemPickupText(std::shared_ptr<Item> const& item) {
  auto descriptor = item->descriptor();
  if (auto existing = m_itemDropMessages.ptr(descriptor.singular())) {
    auto newCount = item->count() + existing->first;
    auto message = existing->second;
    message->message = strf("{} - {}", item->friendlyName(), newCount);
    message->cooldown = m_config->messageTime;
    *existing = {newCount, std::move(message)};
  } else {
    auto message = std::make_shared<GuiMessage>(strf("{} - {}", item->friendlyName(), item->count()), m_config->messageTime, 0);
    m_messages.append(message);
    m_itemDropMessages[descriptor.singular()] = {item->count(), std::move(message)};
  }
  return {};
}

bool MainInterface::fixedCamera() const {
  return m_clientCommandProcessor->fixedCameraEnabled();
}

bool MainInterface::hudVisible() const {
  return !m_disableHud;
}

void MainInterface::setHudVisible(bool visible) {
  m_disableHud = !visible;
}

std::expected<void, StarException> MainInterface::warpToOrbitedWorld(bool deploy) {
  auto allowed = m_client->canBeamDown(deploy);
  if (!allowed)
    return std::unexpected(std::move(allowed.error()));
  if (*allowed)
    return deploy ? m_client->warpPlayer(WarpAlias::OrbitedWorld, true, "deploy", true)
                  : m_client->warpPlayer(WarpAlias::OrbitedWorld, true, "beam");
  auto played = m_guiContext->playAudio("/sfx/interface/clickon_error.ogg");
  if (!played)
    return std::unexpected(std::move(played.error()));
  return {};
}

std::expected<void, StarException> MainInterface::warpToOwnShip() {
  auto allowed = m_client->canBeamUp();
  if (!allowed)
    return std::unexpected(std::move(allowed.error()));
  if (*allowed)
    return warpTo(WarpAlias::OwnShip);
  auto played = m_guiContext->playAudio("/sfx/interface/clickon_error.ogg");
  if (!played)
    return std::unexpected(std::move(played.error()));
  return {};
}

std::expected<void, StarException> MainInterface::warpTo(WarpAction const& warpAction) {
  if (m_client->beamUpRule() == BeamUpRule::AnywhereWithWarning) {
    if (m_confirmationDialog->isDisplayed())
      m_confirmationDialog->dismiss();
    auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Confirmation);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
    return m_confirmationDialog->displayConfirmation("/interface/windowconfig/beamupconfirmation.config",
      [this, warpAction](Widget*) -> std::expected<void, StarException> { return m_client->warpPlayer(warpAction, true, "beam"); },
      [](Widget*) -> std::expected<void, StarException> { return {}; });
  }
  return m_client->warpPlayer(warpAction, true, "beam");
}

std::expected<std::shared_ptr<CanvasWidget>, StarException> MainInterface::fetchCanvas(String const& canvasName, bool ignoreInterfaceScale) {
  if (auto existing = m_canvases.ptr(canvasName)) {
    (*existing)->setIgnoreInterfaceScale(ignoreInterfaceScale);
    return *existing;
  }
  auto created = CanvasWidget::create(*m_guiContext);
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto size = ignoreInterfaceScale ? m_guiContext->windowSize() : m_guiContext->windowInterfaceSize();
  if (!size)
    return std::unexpected(std::move(size.error()));
  auto canvas = std::move(*created);
  canvas->setPosition(Vec2I());
  canvas->setSize(Vec2I(*size));
  canvas->setIgnoreInterfaceScale(ignoreInterfaceScale);
  m_canvases.emplace(canvasName, canvas);
  return canvas;
}

std::shared_ptr<ClientCommandProcessor> MainInterface::commandProcessor() const {
  return m_clientCommandProcessor;
}

// For when the player swaps characters. We need to completely reload ScriptPanes,
// because a lot of ScriptPanes do not expect the character to suddenly change and may break or spill data over.
std::expected<void, StarException> MainInterface::takeScriptPanes(List<ScriptPaneInfo>& out) {
  return m_paneManager.dismissWhere([&](std::shared_ptr<Pane> const& pane) -> std::expected<bool, StarException> {
    if (auto scriptPane = as<ScriptPane>(pane)) {
      if (scriptPane->isDismissed())
        return false;
      auto sourceEntityId = scriptPane->sourceEntityId();
      m_interactionScriptPanes.remove(sourceEntityId);
      auto& info = out.emplaceAppend();
      info.scriptPane = scriptPane;
      info.config = scriptPane->rawConfig();
      info.sourceEntityId = sourceEntityId;
      info.visible = scriptPane->visibility();
      info.position = scriptPane->relativePosition();
      return true;
    }
    return false;
  });
}

std::expected<void, StarException> MainInterface::reviveScriptPanes(List<ScriptPaneInfo>& panes) {
  for (auto& info : panes) {
    auto created = ScriptPane::create(*m_guiContext, m_client, info.config, info.sourceEntityId);
    if (!created)
      return std::unexpected(std::move(created.error()));
    auto pane = std::move(*created);
    if (auto visible = pane->setVisibility(info.visible); !visible)
      return std::unexpected(std::move(visible.error()));
    if (info.scriptPane == m_mmUpgrade || info.scriptPane == m_collections) {
      auto id = info.scriptPane == m_mmUpgrade ? MainInterfacePanes::MmUpgrade : MainInterfacePanes::Collections;
      auto removed = m_paneManager.deregisterPane(id);
      if (!removed)
        return std::unexpected(std::move(removed.error()));
      if (auto registered = m_paneManager.registerPane(id, PaneLayer::Window, pane); !registered)
        return std::unexpected(std::move(registered.error()));
      if (id == MainInterfacePanes::MmUpgrade)
        m_mmUpgrade = pane;
      else
        m_collections = pane;
    }
    if (auto displayed = displayScriptPane(pane, info.sourceEntityId); !displayed)
      return std::unexpected(std::move(displayed.error()));
    pane->setPosition(info.position);
    info.scriptPane = std::move(pane);
  }
  return {};
}

std::expected<void, StarException> MainInterface::displayDefaultPanes() {
  for (auto pane : {MainInterfacePanes::ActionBar, MainInterfacePanes::Chat, MainInterfacePanes::TeamBar, MainInterfacePanes::StatusPane}) {
    auto displayed = m_paneManager.displayRegisteredPane(pane);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
  }
  return {};
}

std::expected<std::shared_ptr<Pane>, StarException> MainInterface::createEscapeDialog() {
  auto assets = m_guiContext->root().assets();
  if (!assets)
    return std::unexpected(std::move(assets.error()));
  auto created = Pane::create(*m_guiContext);
  if (!created)
    return std::unexpected(std::move(created.error()));
  auto escapeDialog = std::move(*created);
  auto escapeDialogPtr = escapeDialog.get();
  GuiReader reader(*m_guiContext);
  reader.registerCallback("returnToGame", [escapeDialogPtr](Widget*) -> std::expected<void, StarException> {
    escapeDialogPtr->dismiss();
    return {};
  });
  reader.registerCallback("showOptions", [escapeDialogPtr, this](Widget*) -> std::expected<void, StarException> {
    escapeDialogPtr->dismiss();
    auto displayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Options);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
    return {};
  });
  reader.registerCallback("saveAndQuit", [escapeDialogPtr, this](Widget*) -> std::expected<void, StarException> {
    m_state = ReturnToTitle;
    escapeDialogPtr->dismiss();
    return {};
  });
  auto config = (*assets)->json("/interface.config:escapeDialog");
  if (!config)
    return std::unexpected(std::move(config.error()));
  if (auto constructed = reader.construct(*config, escapeDialogPtr); !constructed)
    return std::unexpected(std::move(constructed.error()));
  auto label = escapeDialog->requireChild<LabelWidget>("lblversion");
  if (!label)
    return std::unexpected(std::move(label.error()));
  if (auto text = (*label)->setText(strf("OpenStarbound - {} ({})", StarVersionString, StarArchitectureString)); !text)
    return std::unexpected(std::move(text.error()));
  return escapeDialog;
}

float MainInterface::interfaceScale() const {
  return m_guiContext->interfaceScale();
}

std::expected<unsigned, StarException> MainInterface::windowHeight() const {
  return m_guiContext->windowHeight();
}

std::expected<unsigned, StarException> MainInterface::windowWidth() const {
  return m_guiContext->windowWidth();
}

std::expected<Vec2F, StarException> MainInterface::mainBarPosition() const {
  auto width = windowWidth();
  if (!width)
    return std::unexpected(std::move(width.error()));
  auto height = windowHeight();
  if (!height)
    return std::unexpected(std::move(height.error()));
  return Vec2F(*width, *height) - Vec2F(m_config->mainBarSize) * interfaceScale();
}

std::expected<void, StarException> MainInterface::renderBreath() {
  auto assets = m_guiContext->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto imgMetadata = m_guiContext->root().imageMetadataDatabase();
  if (!imgMetadata) return std::unexpected(std::move(imgMetadata.error()));
  auto size = m_guiContext->textureSize("/interface/breath/empty.png");
  if (!size) return std::unexpected(std::move(size.error()));
  Vec2I breathBarSize = Vec2I(*size) * interfaceScale();
  auto offsetJson = (*assets)->json("/interface.config:breathPos");
  if (!offsetJson) return std::unexpected(std::move(offsetJson.error()));
  auto breathOffset = jsonToVec2I(*offsetJson);
  if (!breathOffset) return std::unexpected(std::move(breathOffset.error()));
  auto width = windowWidth();
  if (!width) return std::unexpected(std::move(width.error()));
  auto height = windowHeight();
  if (!height) return std::unexpected(std::move(height.error()));
  Vec2F breathBackgroundCenterPos(*width * 0.5f + (*breathOffset)[0] * interfaceScale(), *height - (*breathOffset)[1] * interfaceScale());
  auto barPosJson = (*assets)->json("/interface.config:breathBarPos");
  if (!barPosJson) return std::unexpected(std::move(barPosJson.error()));
  auto barOffset = jsonToVec2I(*barPosJson);
  if (!barOffset) return std::unexpected(std::move(barOffset.error()));
  Vec2F breathBarPos = breathBackgroundCenterPos + Vec2F(*barOffset * interfaceScale());
  float breath = m_client->mainPlayer()->breath();
  auto breathMax = m_client->mainPlayer()->maxBreath();
  if (!breathMax) return std::unexpected(std::move(breathMax.error()));
  std::size_t blocks = std::round((10 * breath) / *breathMax);
  if (blocks < 10) {
    String breathPath = "/interface/breath/breath.png";
    auto backgroundSize = (*imgMetadata)->imageSize(breathPath);
    if (!backgroundSize) return std::unexpected(std::move(backgroundSize.error()));
    if (auto r = m_guiContext->drawQuad(breathPath, RectF::withCenter(breathBackgroundCenterPos, Vec2F(*backgroundSize) * interfaceScale())); !r)
      return std::unexpected(std::move(r.error()));
    for (std::size_t i = 0; i < 10; i++) {
      String path;
      if (i >= blocks)
        path = blocks == 0 && Time::monotonicMilliseconds() % 500 > 250 ? "/interface/breath/warning.png" : "/interface/breath/empty.png";
      else
        path = "/interface/breath/breathbar.png";
      if (auto r = m_guiContext->drawQuad(path, breathBarPos + Vec2F(breathBarSize[0] * i, 0), interfaceScale()); !r)
        return std::unexpected(std::move(r.error()));
    }
  }
  return {};
}

std::expected<void, StarException> MainInterface::renderMessages() {
  if (m_messages.empty())
    return {};

  Vec2F totalOffset = {};
  auto imgMetadata = m_guiContext->root().imageMetadataDatabase();
  if (!imgMetadata) return std::unexpected(std::move(imgMetadata.error()));
  auto configuration = m_guiContext->root().configuration();
  if (!configuration) return std::unexpected(std::move(configuration.error()));
  auto bottomJson = (*configuration)->getPath("inventory.bottomActionBar");
  if (!bottomJson) return std::unexpected(std::move(bottomJson.error()));
  auto bottom = bottomJson->optBool();
  if (!bottom) return std::unexpected(std::move(bottom.error()));
  unsigned bottomOffset = bottom->value_or(false) ? 32 : 0;
  auto width = windowWidth();
  if (!width) return std::unexpected(std::move(width.error()));
  for (auto& message : m_messages) {
    Vec2F hiddenOffset = Vec2F(m_config->messageHiddenOffset);
    Vec2F activeOffset = Vec2F(m_config->messageActiveOffset);
    if (bottomOffset) {
      activeOffset[1] += bottomOffset;
      bottomOffset = 0;
    }
    Vec2F messageOffset = lerp(message->springState, Vec2F(), activeOffset - hiddenOffset);
    totalOffset += messageOffset;
    messageOffset = totalOffset + hiddenOffset;

    Vec2F backgroundCenterPos = Vec2F(*width * 0.5f + messageOffset[0] * interfaceScale(), messageOffset[1] * interfaceScale());

    Vec2F backgroundTextCenterPos = backgroundCenterPos + Vec2F(m_config->messageTextContainerOffset) * interfaceScale();
    Vec2F messageTextOffset = backgroundTextCenterPos + Vec2F(m_config->messageTextOffset) * interfaceScale();

    if (message->cooldown > m_config->messageHideTime)
      message->springState = (message->springState * m_config->messageWindowSpring + 1.0f) / (m_config->messageWindowSpring + 1.0f);
    else
      message->springState = (message->springState * m_config->messageWindowSpring) / (m_config->messageWindowSpring + 1.0f);

    auto size = (*imgMetadata)->imageSize(m_config->messageTextContainer);
    if (!size) return std::unexpected(std::move(size.error()));
    if (auto r = m_guiContext->drawQuad(m_config->messageTextContainer,
        RectF::withCenter(backgroundTextCenterPos, Vec2F(*size) * interfaceScale())); !r)
      return std::unexpected(std::move(r.error()));
    if (auto r = m_guiContext->setTextStyle(m_config->textStyle); !r) return std::unexpected(std::move(r.error()));
    if (auto r = m_guiContext->renderText(message->message, {messageTextOffset, HorizontalAnchor::HMidAnchor, VerticalAnchor::VMidAnchor}); !r)
      return std::unexpected(std::move(r.error()));
  }
  return {};
}

std::expected<void, StarException> MainInterface::renderMonsterHealthBar() {
  auto assets = m_guiContext->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto imgMetadata = m_guiContext->root().imageMetadataDatabase();
  if (!imgMetadata) return std::unexpected(std::move(imgMetadata.error()));
  if (m_lastMouseoverTarget != NullEntityId && !m_stickyTargetingTimer.ready()) {
    auto showDamageEntity = as<DamageBarEntity>(m_client->worldClient()->entity(m_lastMouseoverTarget));
    if (!showDamageEntity) {
      m_lastMouseoverTarget = NullEntityId;
      return {};
    }
    auto width = windowWidth();
    if (!width) return std::unexpected(std::move(width.error()));
    auto height = windowHeight();
    if (!height) return std::unexpected(std::move(height.error()));
    Vec2F backgroundCenterPos(*width / 2.0f, *height);
    auto vectorAt = [&](char const* path) -> std::expected<Vec2F, StarException> {
      auto json = (*assets)->json(path);
      if (!json) return std::unexpected(std::move(json.error()));
      auto vector = jsonToVec2F(*json);
      if (!vector) return std::unexpected(std::move(vector.error()));
      return *vector * interfaceScale();
    };
    auto stringAt = [&](char const* path) -> std::expected<String, StarException> {
      auto json = (*assets)->json(path);
      if (!json) return std::unexpected(std::move(json.error()));
      return json->toString();
    };
    auto container = stringAt("/interface.config:monsterHealth.container");
    if (!container) return std::unexpected(std::move(container.error()));
    auto offset = vectorAt("/interface.config:monsterHealth.offset");
    if (!offset) return std::unexpected(std::move(offset.error()));
    auto containerSize = (*imgMetadata)->imageSize(*container);
    if (!containerSize) return std::unexpected(std::move(containerSize.error()));
    if (auto r = m_guiContext->drawQuad(*container, RectF::withCenter(backgroundCenterPos + *offset, Vec2F(*containerSize * interfaceScale()))); !r)
      return std::unexpected(std::move(r.error()));
    auto nameOffset = vectorAt("/interface.config:monsterHealth.nameTextOffset");
    if (!nameOffset) return std::unexpected(std::move(nameOffset.error()));
    if (auto r = m_guiContext->setTextStyle(m_config->textStyle); !r) return std::unexpected(std::move(r.error()));
    auto name = showDamageEntity->name();
    if (!name) return std::unexpected(std::move(name.error()));
    if (auto r = m_guiContext->renderText(*name, backgroundCenterPos + *nameOffset); !r)
      return std::unexpected(std::move(r.error()));
    auto empty = stringAt("/interface.config:monsterHealth.progressEmpty");
    if (!empty) return std::unexpected(std::move(empty.error()));
    auto filled = stringAt("/interface.config:monsterHealth.progressFilled");
    if (!filled) return std::unexpected(std::move(filled.error()));
    auto progressOffset = vectorAt("/interface.config:monsterHealth.progressBarOffset");
    if (!progressOffset) return std::unexpected(std::move(progressOffset.error()));
    auto chunksJson = (*assets)->json("/interface.config:monsterHealth.progressChunks");
    if (!chunksJson) return std::unexpected(std::move(chunksJson.error()));
    auto chunks = chunksJson->toInt();
    if (!chunks) return std::unexpected(std::move(chunks.error()));
    auto maxHealth = showDamageEntity->maxHealth();
    if (!maxHealth) return std::unexpected(std::move(maxHealth.error()));
    int blocks = std::round(showDamageEntity->health() / *maxHealth * *chunks);
    Vec2F barPos = backgroundCenterPos + *progressOffset;
    auto filledSize = (*imgMetadata)->imageSize(*filled);
    if (!filledSize) return std::unexpected(std::move(filledSize.error()));
    Vec2F barItemOffset = Vec2F(*filledSize) * interfaceScale();
    barItemOffset[1] = 0;
    auto emptySize = (*imgMetadata)->imageSize(*empty);
    if (!emptySize) return std::unexpected(std::move(emptySize.error()));
    if (auto r = m_guiContext->drawQuad(*empty, RectF::withSize(backgroundCenterPos + barPos, Vec2F(*emptySize) * interfaceScale())); !r)
      return std::unexpected(std::move(r.error()));
    for (int i = 0; i < blocks; ++i) {
      if (auto r = m_guiContext->drawQuad(*filled, barPos + barItemOffset * i, interfaceScale()); !r)
        return std::unexpected(std::move(r.error()));
    }
    auto portraitOffset = vectorAt("/interface.config:monsterHealth.portraitOffset");
    if (!portraitOffset) return std::unexpected(std::move(portraitOffset.error()));
    auto scaleJson = (*assets)->json("/interface.config:monsterHealth.portraitScale");
    if (!scaleJson) return std::unexpected(std::move(scaleJson.error()));
    auto scale = scaleJson->toFloat();
    if (!scale) return std::unexpected(std::move(scale.error()));
    float portraitScale = *scale * interfaceScale();
    auto scissorJson = (*assets)->json("/interface.config:monsterHealth.portraitScissorRect");
    if (!scissorJson) return std::unexpected(std::move(scissorJson.error()));
    auto scissor = jsonToRectF(*scissorJson);
    if (!scissor) return std::unexpected(std::move(scissor.error()));
    auto rect = scissor->scaled(interfaceScale()).translated(backgroundCenterPos + *portraitOffset);
    auto drawPortrait = [&]() -> std::expected<void, StarException> {
      if (auto r = m_guiContext->setInterfaceScissorRect(RectI(rect.scaled(1.0f / interfaceScale()))); !r)
        return std::unexpected(std::move(r.error()));
      auto maxSizeJson = (*assets)->json("/interface.config:monsterHealth.portraitMaxSize");
      if (!maxSizeJson) return std::unexpected(std::move(maxSizeJson.error()));
      auto maxSize = jsonToVec2I(*maxSizeJson);
      if (!maxSize) return std::unexpected(std::move(maxSize.error()));
      auto portrait = showDamageEntity->portrait(PortraitMode::Full);
      if (!portrait) return std::unexpected(std::move(portrait.error()));
      auto bounds = Drawable::boundBoxAll(**imgMetadata, *portrait, true);
      if (!bounds) return std::unexpected(std::move(bounds.error()));
      if (m_portraitScale == 0)
        m_portraitScale = std::max<int>(1, std::ceil(std::max(bounds->size().x() / maxSize->x(), bounds->size().y() / maxSize->y())));
      Drawable::translateAll(*portrait, {-bounds->xMin() - bounds->width() * 0.5f, -bounds->yMin()});
      Drawable::scaleAll(*portrait, 1.0f / m_portraitScale);
      for (auto drawable : *portrait) {
        if (auto r = m_guiContext->drawDrawable(std::move(drawable), backgroundCenterPos + *portraitOffset, portraitScale); !r)
          return std::unexpected(std::move(r.error()));
      }
      return {};
    };
    auto drawn = drawPortrait();
    auto reset = m_guiContext->resetInterfaceScissorRect();
    if (!drawn && !reset)
      return std::unexpected(drawn.error().withContext(strf("Portrait scissor cleanup failed: {}", printException(ExceptionDiagnostic(reset.error()), true))));
    if (!drawn) return std::unexpected(std::move(drawn.error()));
    if (!reset) return std::unexpected(std::move(reset.error()));
  }
  return {};
}

std::expected<void, StarException> MainInterface::renderSpecialDamageBar() {
  // Count every entry to preserve spacing until removed entities leave the list.
  std::size_t num = m_specialDamageBars.size();
  if (num == 0) return {};
  auto assets = m_guiContext->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto imgMetadata = m_guiContext->root().imageMetadataDatabase();
  if (!imgMetadata) return std::unexpected(std::move(imgMetadata.error()));
  auto barConfig = (*assets)->json("/interface.config:specialDamageBar");
  if (!barConfig) return std::unexpected(std::move(barConfig.error()));
  auto vectorAt = [&](char const* key) -> std::expected<Vec2F, StarException> {
    auto json = barConfig->get(key);
    if (!json) return std::unexpected(std::move(json.error()));
    return jsonToVec2F(*json);
  };
  auto backgroundOffsetValue = vectorAt("backgroundOffset");
  if (!backgroundOffsetValue) return std::unexpected(std::move(backgroundOffsetValue.error()));
  auto spacing = barConfig->getFloat("multibarSpacing", backgroundOffsetValue->x() * -2.0f);
  if (!spacing) return std::unexpected(std::move(spacing.error()));
  float hSpacing = *spacing * interfaceScale();
  auto width = windowWidth();
  if (!width) return std::unexpected(std::move(width.error()));
  auto edgeOffset = barConfig->getFloat("multibarScreenEdgeOffset", 20.0f);
  if (!edgeOffset) return std::unexpected(std::move(edgeOffset.error()));
  float totalWidth = hSpacing * num;
  float maxWidth = std::min(totalWidth, *width - *edgeOffset * interfaceScale());
  float hScale = maxWidth / totalWidth;
  float allOffset = -hSpacing * 0.5f * (num - 1);
  float center = *width / 2.0f;
  auto background = barConfig->getString("background");
  if (!background) return std::unexpected(std::move(background.error()));
  Vec2F backgroundOffset = *backgroundOffsetValue * interfaceScale();
  backgroundOffset.setX(backgroundOffset.x() * hScale);
  auto backgroundSize = (*imgMetadata)->imageSize(*background);
  if (!backgroundSize) return std::unexpected(std::move(backgroundSize.error()));
  Vec2F backgroundImageSize = Vec2F(*backgroundSize) * interfaceScale();
  backgroundImageSize.setX(backgroundImageSize.x() * hScale);
  auto fill = barConfig->getString("fill");
  if (!fill) return std::unexpected(std::move(fill.error()));
  auto fillOffsetValue = vectorAt("fillOffset");
  if (!fillOffsetValue) return std::unexpected(std::move(fillOffsetValue.error()));
  Vec2F fillOffset = *fillOffsetValue * interfaceScale();
  fillOffset.setX(fillOffset.x() * hScale);
  auto nameOffsetValue = vectorAt("nameOffset");
  if (!nameOffsetValue) return std::unexpected(std::move(nameOffsetValue.error()));
  Vec2F nameOffset = *nameOffsetValue * interfaceScale();
  nameOffset.setX(nameOffset.x() * hScale);
  std::size_t i = 0;
  for (auto& bar : m_specialDamageBars) {
    if (auto target = as<DamageBarEntity>(m_client->worldClient()->entity(bar.first))) {
      Vec2F bottomCenter(center + (allOffset + hSpacing * i) * hScale, 0);
      if (auto r = m_guiContext->drawQuad(*background, RectF::withSize(bottomCenter + backgroundOffset, backgroundImageSize)); !r)
        return std::unexpected(std::move(r.error()));
      auto fillWidth = barConfig->getInt("fillWidth");
      if (!fillWidth) return std::unexpected(std::move(fillWidth.error()));
      auto fillSize = (*imgMetadata)->imageSize(*fill);
      if (!fillSize) return std::unexpected(std::move(fillSize.error()));
      Vec2F size(*fillWidth * bar.second, fillSize->y());
      size.setX(size.x() * hScale);
      if (auto r = m_guiContext->drawQuad(*fill, RectF::withSize(bottomCenter + fillOffset, size * interfaceScale())); !r)
        return std::unexpected(std::move(r.error()));
      auto drawName = [&]() -> std::expected<void, StarException> {
        auto colorJson = barConfig->get("nameColor");
        if (!colorJson) return std::unexpected(std::move(colorJson.error()));
        auto color = jsonToColor(*colorJson);
        if (!color) return std::unexpected(std::move(color.error()));
        if (auto r = m_guiContext->setFontColor(color->toRgba()); !r) return std::unexpected(std::move(r.error()));
        auto nameSize = barConfig->getUInt("nameSize");
        if (!nameSize) return std::unexpected(std::move(nameSize.error()));
        if (auto r = m_guiContext->setFontSize(*nameSize); !r) return std::unexpected(std::move(r.error()));
        auto styleJson = barConfig->get("nameStyle");
        if (!styleJson) return std::unexpected(std::move(styleJson.error()));
        if (*styleJson) {
          auto style = TextStyle::create(*styleJson);
          if (!style) return std::unexpected(std::move(style.error()));
          if (auto r = m_guiContext->setTextStyle(*style); !r) return std::unexpected(std::move(r.error()));
        }
        auto name = target->name();
        if (!name) return std::unexpected(std::move(name.error()));
        auto text = m_guiContext->renderText(*name, TextPositioning(bottomCenter + nameOffset, HorizontalAnchor::HMidAnchor, VerticalAnchor::BottomAnchor));
        if (!text) return std::unexpected(std::move(text.error()));
        return {};
      };
      auto drawn = drawName();
      auto cleared = m_guiContext->clearTextStyle();
      if (!drawn && !cleared)
        return std::unexpected(drawn.error().withContext(strf("Damage bar text cleanup failed: {}", printException(ExceptionDiagnostic(cleared.error()), true))));
      if (!drawn) return std::unexpected(std::move(drawn.error()));
      if (!cleared) return std::unexpected(std::move(cleared.error()));
    }
    ++i;
  }
  return {};
}

std::expected<void, StarException> MainInterface::renderMainBar() {
  auto barPosition = mainBarPosition();
  if (!barPosition) return std::unexpected(std::move(barPosition.error()));
  Vec2F barPos = *barPosition;
  m_cursorTooltip = {};
  auto assets = m_guiContext->root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto tooltipAt = [&](char const* path) -> std::expected<String, StarException> {
    auto json = (*assets)->json(path);
    if (!json) return std::unexpected(std::move(json.error()));
    return json->toString();
  };
  Vec2F inventoryButtonPos = barPos + Vec2F(m_config->mainBarInventoryButtonOffset) * interfaceScale();
  auto inventoryDisplayed = m_paneManager.registeredPaneIsDisplayed(MainInterfacePanes::Inventory);
  if (!inventoryDisplayed) return std::unexpected(std::move(inventoryDisplayed.error()));
  auto inventoryHovered = overButton(m_config->mainBarInventoryButtonPoly, m_cursorScreenPos);
  if (!inventoryHovered) return std::unexpected(std::move(inventoryHovered.error()));
  String const* inventoryImage;
  if (*inventoryDisplayed)
    inventoryImage = *inventoryHovered ? &m_config->inventoryImageOpenHover : &m_config->inventoryImageOpen;
  else if (*inventoryHovered)
    inventoryImage = m_inventoryWindow->containsNewItems() ? &m_config->inventoryImageGlowHover : &m_config->inventoryImageHover;
  else
    inventoryImage = m_inventoryWindow->containsNewItems() ? &m_config->inventoryImageGlow : &m_config->inventoryImage;
  if (auto r = m_guiContext->drawQuad(*inventoryImage, inventoryButtonPos, interfaceScale()); !r)
    return std::unexpected(std::move(r.error()));
  if (*inventoryHovered) {
    auto tooltip = tooltipAt("/interface.config:cursorTooltip.inventoryText");
    if (!tooltip) return std::unexpected(std::move(tooltip.error()));
    m_cursorTooltip = std::move(*tooltip);
  }
  auto drawStateButton = [this, &tooltipAt](MainInterfacePanes paneType, Vec2F pos, PolyI const& poly,
      String const& image, String const& hoverImage, String const& openImage, String const& hoverOpenImage,
      char const* tooltipPath) -> std::expected<void, StarException> {
    auto tooltip = tooltipAt(tooltipPath);
    if (!tooltip) return std::unexpected(std::move(tooltip.error()));
    auto displayed = m_paneManager.registeredPaneIsDisplayed(paneType);
    if (!displayed) return std::unexpected(std::move(displayed.error()));
    auto hovered = overButton(poly, m_cursorScreenPos);
    if (!hovered) return std::unexpected(std::move(hovered.error()));
    auto const& selected = *displayed ? (*hovered ? hoverOpenImage : openImage) : (*hovered ? hoverImage : image);
    if (auto r = m_guiContext->drawQuad(selected, pos, interfaceScale()); !r) return std::unexpected(std::move(r.error()));
    if (*hovered) m_cursorTooltip = std::move(*tooltip);
    return {};
  };

  Vec2F craftButtonPos = barPos + Vec2F(m_config->mainBarCraftButtonOffset) * interfaceScale();
  if (auto r = drawStateButton(MainInterfacePanes::CraftingPlain,
      craftButtonPos, m_config->mainBarCraftButtonPoly, m_config->craftImage,
      m_config->craftImageHover, m_config->craftImageOpen, m_config->craftImageOpenHover,
      "/interface.config:cursorTooltip.craftingText"); !r) return std::unexpected(std::move(r.error()));

  Vec2F codexButtonPos = barPos + Vec2F(m_config->mainBarCodexButtonOffset) * interfaceScale();
  if (auto r = drawStateButton(MainInterfacePanes::Codex,
      codexButtonPos, m_config->mainBarCodexButtonPoly, m_config->codexImage,
      m_config->codexImageHover, m_config->codexImageOpen, m_config->codexImageHoverOpen,
      "/interface.config:cursorTooltip.codexText"); !r) return std::unexpected(std::move(r.error()));

  Vec2F mmUpgradeButtonPos = barPos + Vec2F(m_config->mainBarMmUpgradeButtonOffset) * interfaceScale();
  if (m_client->mainPlayer()->inventory()->essentialItem(EssentialItem::BeamAxe)) {
    if (auto r = drawStateButton(MainInterfacePanes::MmUpgrade,
        mmUpgradeButtonPos, m_config->mainBarMmUpgradeButtonPoly, m_config->mmUpgradeImage,
        m_config->mmUpgradeImageHover, m_config->mmUpgradeImageOpen, m_config->mmUpgradeImageHoverOpen,
        "/interface.config:cursorTooltip.mmUpgradeText"); !r) return std::unexpected(std::move(r.error()));
  } else {
    if (auto r = drawStateButton(MainInterfacePanes::MmUpgrade,
        mmUpgradeButtonPos, m_config->mainBarMmUpgradeButtonPoly, m_config->mmUpgradeImageDisabled,
        m_config->mmUpgradeImageDisabled, m_config->mmUpgradeImageDisabled, m_config->mmUpgradeImageDisabled,
        "/interface.config:cursorTooltip.disabledText"); !r) return std::unexpected(std::move(r.error()));
  }

  Vec2F collectionsButtonPos = barPos + Vec2F(m_config->mainBarCollectionsButtonOffset) * interfaceScale();
  if (auto r = drawStateButton(MainInterfacePanes::Collections,
      collectionsButtonPos, m_config->mainBarCollectionsButtonPoly, m_config->collectionsImage,
      m_config->collectionsImageHover, m_config->collectionsImageOpen, m_config->collectionsImageHoverOpen,
      "/interface.config:cursorTooltip.collectionsText"); !r) return std::unexpected(std::move(r.error()));

  // when the player can't deploy or beam, show the deploy button disabled
  // when the player can beam up they can't deploy down, show beaming up button in deploy button's place
  // when the player can only deploy, only show deploy button
  // when the player can deploy or beam down, show both buttons

  Vec2F deployButtonPos(Vec2F(barPos) + Vec2F(m_config->mainBarDeployButtonOffset) * interfaceScale());
  auto beamUpResult = m_client->canBeamUp();
  if (!beamUpResult) return std::unexpected(std::move(beamUpResult.error()));
  bool beamUp = *beamUpResult;
  bool canDeploy = false;
  if (!beamUp) {
    auto deploy = m_client->canBeamDown(true);
    if (!deploy) return std::unexpected(std::move(deploy.error()));
    canDeploy = *deploy;
  }
  if (beamUp || canDeploy) {
    auto hovered = overButton(m_config->mainBarDeployButtonPoly, m_cursorScreenPos);
    if (!hovered) return std::unexpected(std::move(hovered.error()));
    auto const& image = beamUp ? (*hovered ? m_config->beamUpImageHover : m_config->beamUpImage)
        : (*hovered ? m_config->deployImageHover : m_config->deployImage);
    if (auto r = m_guiContext->drawQuad(image, deployButtonPos, interfaceScale()); !r)
      return std::unexpected(std::move(r.error()));
    if (*hovered) {
      auto tooltip = tooltipAt(beamUp ? "/interface.config:cursorTooltip.beamUpText" : "/interface.config:cursorTooltip.deployText");
      if (!tooltip) return std::unexpected(std::move(tooltip.error()));
      m_cursorTooltip = std::move(*tooltip);
    }
  } else {
    if (auto r = m_guiContext->drawQuad(m_config->deployImageDisabled, deployButtonPos, interfaceScale()); !r)
      return std::unexpected(std::move(r.error()));
  }
  Vec2F beamButtonPos(barPos + Vec2F(m_config->mainBarBeamButtonOffset) * interfaceScale());
  auto beamDown = m_client->canBeamDown();
  if (!beamDown) return std::unexpected(std::move(beamDown.error()));
  if (*beamDown) {
    auto hovered = overButton(m_config->mainBarBeamButtonPoly, m_cursorScreenPos);
    if (!hovered) return std::unexpected(std::move(hovered.error()));
    if (auto r = m_guiContext->drawQuad(*hovered ? m_config->beamDownImageHover : m_config->beamDownImage, beamButtonPos, interfaceScale()); !r)
      return std::unexpected(std::move(r.error()));
    if (*hovered) {
      auto tooltip = tooltipAt("/interface.config:cursorTooltip.beamDownText");
      if (!tooltip) return std::unexpected(std::move(tooltip.error()));
      m_cursorTooltip = std::move(*tooltip);
    }
  }

  Vec2F questLogButtonPos = barPos + Vec2F(m_config->mainBarQuestLogButtonOffset) * interfaceScale();
  if (auto r = drawStateButton(MainInterfacePanes::QuestLog,
      questLogButtonPos, m_config->mainBarQuestLogButtonPoly, m_config->questLogImage,
      m_config->questLogImageHover, m_config->questLogImageOpen, m_config->questLogImageHoverOpen,
      "/interface.config:cursorTooltip.questsText"); !r) return std::unexpected(std::move(r.error()));
  return {};
}

std::expected<void, StarException> MainInterface::renderWindows() {
  return m_paneManager.render();
}

std::expected<void, StarException> MainInterface::renderDebug() {
  if (!isDebugDisplayed()) {
    m_guiContext->root().spatialLogger().clear();
    m_debugTextRect = RectF::null();
    m_guiContext->root().logMap().clear();
    m_guiContext->root().spatialLogger().setObserved(false);
    return {};
  }
  m_guiContext->root().spatialLogger().setObserved(true);
  auto drawDebug = [&]() -> std::expected<void, StarException> {
    if (m_clientCommandProcessor->debugHudEnabled()) {
      if (auto r = m_guiContext->setTextStyle(m_config->debugTextStyle); !r) return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->setLineSpacing(0.5f); !r) return std::unexpected(std::move(r.error()));
      bool clearMap = m_debugMapClearTimer.wrapTick(m_guiContext->root().timing().timestep);
      auto logMapValues = m_guiContext->root().logMap().getValues();
      if (clearMap) m_guiContext->root().logMap().clear();
      auto height = windowHeight();
      if (!height) return std::unexpected(std::move(height.error()));
      List<String> formatted;
      formatted.reserve(logMapValues.size());
      int counter = 0;
      for (auto const& pair : logMapValues) {
        TextPositioning positioning = {Vec2F(m_config->debugOffset[0], *height - m_config->debugOffset[1] - m_config->textStyle.fontSize * interfaceScale() * counter++)};
        String& text = formatted.emplace_back(strf("{}^lightgray;:^green,set; {}", pair.first, pair.second));
        auto size = m_guiContext->determineTextSize(text, positioning);
        if (!size) return std::unexpected(std::move(size.error()));
        m_debugTextRect.combine(size->padded(m_config->debugBackgroundPad));
      }
      if (!m_debugTextRect.isNull()) {
        auto renderer = m_guiContext->renderer();
        if (!renderer) return std::unexpected(std::move(renderer.error()));
        RenderQuad& quad = std::get<RenderQuad>((*renderer)->immediatePrimitives()
            .emplace_back(std::in_place_type_t<RenderQuad>(), m_debugTextRect, m_config->debugBackgroundColor.toRgba(), 0.0f));
        quad.b.color[3] = quad.c.color[3] = 0;
      }
      m_debugTextRect = RectF::null();
      for (std::size_t index = 0; index != formatted.size(); ++index) {
        TextPositioning positioning = {Vec2F(m_config->debugOffset[0], *height - m_config->debugOffset[1] - m_config->textStyle.fontSize * interfaceScale() * index)};
        if (auto r = m_guiContext->renderText(formatted[index], positioning); !r) return std::unexpected(std::move(r.error()));
      }
    }
    auto const& camera = m_worldPainter->camera();
    bool clearSpatial = m_debugSpatialClearTimer.wrapTick(m_guiContext->root().timing().timestep);
    for (auto const& line : m_guiContext->root().spatialLogger().getLines("world", clearSpatial)) {
      Vec2F begin = camera.worldToScreen(line.begin);
      Vec2F end = camera.worldGeometry().diff(line.end, line.begin) * camera.pixelRatio() * TilePixels + begin;
      if (auto r = m_guiContext->drawLine(begin, end, line.color, 1); !r) return std::unexpected(std::move(r.error()));
    }
    for (auto const& line : m_guiContext->root().spatialLogger().getLines("screen", clearSpatial)) {
      if (auto r = m_guiContext->drawLine(Vec2F(line.begin), Vec2F(line.end), line.color, 1); !r) return std::unexpected(std::move(r.error()));
    }
    auto drawPoint = [&](Vec2F position, Vec4B const& color) -> std::expected<void, StarException> {
      if (auto r = m_guiContext->drawLine(position + Vec2F(-2, -2), position + Vec2F(-2, 2), color, 1); !r) return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->drawLine(position + Vec2F(-2, 2), position + Vec2F(2, 2), color, 1); !r) return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->drawLine(position + Vec2F(2, 2), position + Vec2F(2, -2), color, 1); !r) return std::unexpected(std::move(r.error()));
      return m_guiContext->drawLine(position + Vec2F(2, -2), position + Vec2F(-2, -2), color, 1);
    };
    for (auto const& point : m_guiContext->root().spatialLogger().getPoints("world", clearSpatial)) {
      if (auto r = drawPoint(camera.worldToScreen(point.position), point.color); !r) return std::unexpected(std::move(r.error()));
    }
    for (auto const& point : m_guiContext->root().spatialLogger().getPoints("screen", clearSpatial)) {
      if (auto r = drawPoint(point.position, point.color); !r) return std::unexpected(std::move(r.error()));
    }
    if (auto r = m_guiContext->setTextStyle(m_config->debugTextStyle); !r) return std::unexpected(std::move(r.error()));
    for (auto const& logText : m_guiContext->root().spatialLogger().getText("world", clearSpatial)) {
      if (auto r = m_guiContext->setFontColor(logText.color); !r) return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->renderText(logText.text.utf8Ptr(), camera.worldToScreen(logText.position)); !r)
        return std::unexpected(std::move(r.error()));
    }
    for (auto const& logText : m_guiContext->root().spatialLogger().getText("screen", clearSpatial)) {
      if (auto r = m_guiContext->setFontColor(logText.color); !r) return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->renderText(logText.text.utf8Ptr(), logText.position); !r)
        return std::unexpected(std::move(r.error()));
    }
    return {};
  };
  auto drawn = drawDebug();
  auto cleared = m_guiContext->clearTextStyle();
  if (!drawn && !cleared)
    return std::unexpected(drawn.error().withContext(strf("Debug text cleanup failed: {}", printException(ExceptionDiagnostic(cleared.error()), true))));
  if (!drawn) return std::unexpected(std::move(drawn.error()));
  return cleared;
}

std::expected<void, StarException> MainInterface::updateCursor() {
  auto actionOverride = m_actionBar->cursorOverride(m_cursorScreenIPos);
  if (!actionOverride) return std::unexpected(std::move(actionOverride.error()));
  std::optional<String> cursorOverride = std::move(*actionOverride);

  if (!cursorOverride) {
    if (auto pane = m_paneManager.getPaneAt(m_cursorScreenIPos / interfaceScale())) {
      auto override = pane->cursorOverride(m_cursorScreenIPos / interfaceScale());
      if (!override) return std::unexpected(std::move(override.error()));
      cursorOverride = std::move(*override);
    } else {
      auto player = m_client->mainPlayer();
      if (auto anchorState = m_client->mainPlayer()->loungingIn()) {
        if (auto loungeable = m_client->worldClient()->get<LoungeableEntity>(anchorState->entityId)) {
          auto loungeAnchor = loungeable->loungeAnchor(anchorState->positionIndex);
          if (!loungeAnchor) return std::unexpected(std::move(loungeAnchor.error()));
          if (*loungeAnchor) cursorOverride = (*loungeAnchor)->cursorOverride;
        }
      }
      if (!cursorOverride) {
        for (auto item : {player->primaryHandItem(), player->altHandItem()}) {
          if (auto activeItem = as<ActiveItem>(item)) {
            if (auto cursor = activeItem->cursor()) {
              cursorOverride = cursor;
              break;
            }
          } else if (auto inspectionTool = as<InspectionTool>(item)) {
            cursorOverride = String("/cursors/inspect.cursor");
            break;
          }
        }
      }
    }
  }

  if (cursorOverride)
    return m_cursor->setCursor(*cursorOverride);
  return m_cursor->resetCursor();
}

std::expected<void, StarException> MainInterface::renderCursor() {
  // A cinematic hides both hardware and drawn cursors.
  if (m_cinematicOverlay && !m_cinematicOverlay->completed()) {
    m_guiContext->applicationController()->setCursorVisible(false);
    return {};
  }
  Vec2I cursorPos = m_cursorScreenIPos;
  Vec2I cursorSize = m_cursor->size();
  Vec2I cursorOffset = m_cursor->offset();
  float cursorScale = m_cursor->scale(interfaceScale());
  auto cursorDrawable = m_cursor->drawable();
  if (!cursorDrawable) return std::unexpected(std::move(cursorDrawable.error()));
  cursorPos[0] -= cursorOffset[0] * cursorScale;
  cursorPos[1] -= (cursorSize[1] - cursorOffset[1]) * cursorScale;
  auto hardwareCursor = m_guiContext->trySetCursor(*cursorDrawable, cursorOffset, cursorScale);
  if (!hardwareCursor) return std::unexpected(std::move(hardwareCursor.error()));
  if (!*hardwareCursor) {
    if (auto r = m_guiContext->drawDrawable(*cursorDrawable, Vec2F(cursorPos), cursorScale); !r)
      return std::unexpected(std::move(r.error()));
  }
  if (m_cursorTooltip) {
    auto drawTooltip = [&]() -> std::expected<void, StarException> {
      auto assets = m_guiContext->root().assets();
      if (!assets) return std::unexpected(std::move(assets.error()));
      auto imgDb = m_guiContext->root().imageMetadataDatabase();
      if (!imgDb) return std::unexpected(std::move(imgDb.error()));
      auto config = (*assets)->json("/interface.config:cursorTooltip");
      if (!config) return std::unexpected(std::move(config.error()));
      auto backgroundImage = config->getString("background");
      if (!backgroundImage) return std::unexpected(std::move(backgroundImage.error()));
      auto offsetJson = config->get("offset");
      if (!offsetJson) return std::unexpected(std::move(offsetJson.error()));
      auto rawOffset = jsonToVec2I(*offsetJson);
      if (!rawOffset) return std::unexpected(std::move(rawOffset.error()));
      auto backgroundSize = (*imgDb)->imageSize(*backgroundImage);
      if (!backgroundSize) return std::unexpected(std::move(backgroundSize.error()));
      Vec2I tooltipSize = Vec2I(*backgroundSize) * interfaceScale();
      Vec2I offset = (Vec2I{0, -cursorSize.y()} + *rawOffset) * cursorScale;
      Vec2I tooltipOffset = m_cursorScreenIPos + offset;
      auto styleJson = config->get("textStyle");
      if (!styleJson) return std::unexpected(std::move(styleJson.error()));
      auto textStyle = TextStyle::create(*styleJson);
      if (!textStyle) return std::unexpected(std::move(textStyle.error()));
      auto fontSize = config->getUInt("fontSize");
      if (!fontSize) return std::unexpected(std::move(fontSize.error()));
      auto colorJson = config->get("color");
      if (!colorJson) return std::unexpected(std::move(colorJson.error()));
      auto color = jsonToColor(*colorJson);
      if (!color) return std::unexpected(std::move(color.error()));
      if (auto r = m_guiContext->drawQuad(*backgroundImage, Vec2F(tooltipOffset) + Vec2F(-tooltipSize.x(), 0), interfaceScale()); !r)
        return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->setTextStyle(*textStyle); !r) return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->setFontSize(*fontSize); !r) return std::unexpected(std::move(r.error()));
      if (auto r = m_guiContext->setFontColor(color->toRgba()); !r) return std::unexpected(std::move(r.error()));
      auto text = m_guiContext->renderText(*m_cursorTooltip,
          TextPositioning(Vec2F(tooltipOffset) + Vec2F(-tooltipSize.x(), tooltipSize.y()) / 2,
              HorizontalAnchor::HMidAnchor, VerticalAnchor::VMidAnchor));
      if (!text) return std::unexpected(std::move(text.error()));
      return {};
    };
    auto drawn = drawTooltip();
    auto cleared = m_guiContext->clearTextStyle();
    if (!drawn && !cleared)
      return std::unexpected(drawn.error().withContext(strf("Cursor text cleanup failed: {}", printException(ExceptionDiagnostic(cleared.error()), true))));
    if (!drawn) return std::unexpected(std::move(drawn.error()));
    if (!cleared) return std::unexpected(std::move(cleared.error()));
  }
  m_cursorItem->setPosition(Vec2I::round(m_cursorScreenPos / interfaceScale() + Vec2F(m_config->inventoryItemMouseOffset)));
  m_cursorItem->setItem(m_client->mainPlayer()->inventory()->swapSlotItem());
  auto drawItem = [&]() -> std::expected<void, StarException> {
    auto width = windowWidth();
    if (!width) return std::unexpected(std::move(width.error()));
    auto height = windowHeight();
    if (!height) return std::unexpected(std::move(height.error()));
    return m_cursorItem->render(RectI::withSize({}, {(int)*width, (int)*height}));
  };
  auto drawn = drawItem();
  auto reset = m_guiContext->resetInterfaceScissorRect();
  if (!drawn && !reset)
    return std::unexpected(drawn.error().withContext(strf("Cursor item scissor cleanup failed: {}", printException(ExceptionDiagnostic(reset.error()), true))));
  if (!drawn) return std::unexpected(std::move(drawn.error()));
  return reset;
}

std::expected<bool, StarException> MainInterface::overButton(PolyI const& buttonPoly, Vec2F const& mousePos) const {
  auto barPos = mainBarPosition();
  if (!barPos) return std::unexpected(std::move(barPos.error()));
  PolyF poly(buttonPoly);
  poly.translate(*barPos);
  poly.scale(interfaceScale(), *barPos);
  return poly.contains(mousePos);
}

std::expected<bool, StarException> MainInterface::overlayClick(Vec2F const& mousePos, MouseButton) {
  auto inventory = overButton(m_config->mainBarInventoryButtonPoly, mousePos);
  if (!inventory) return std::unexpected(std::move(inventory.error()));
  if (*inventory) {
    if (auto r = m_paneManager.toggleRegisteredPane(MainInterfacePanes::Inventory); !r) return std::unexpected(std::move(r.error()));
    return true;
  }
  auto crafting = overButton(m_config->mainBarCraftButtonPoly, mousePos);
  if (!crafting) return std::unexpected(std::move(crafting.error()));
  if (*crafting) {
    if (auto r = togglePlainCraftingWindow(); !r) return std::unexpected(std::move(r.error()));
    return true;
  }
  auto codex = overButton(m_config->mainBarCodexButtonPoly, mousePos);
  if (!codex) return std::unexpected(std::move(codex.error()));
  if (*codex) {
    if (auto r = m_paneManager.toggleRegisteredPane(MainInterfacePanes::Codex); !r) return std::unexpected(std::move(r.error()));
    return true;
  }
  auto deploy = overButton(m_config->mainBarDeployButtonPoly, mousePos);
  if (!deploy) return std::unexpected(std::move(deploy.error()));
  if (*deploy) {
    auto canDeploy = m_client->canBeamDown(true);
    if (!canDeploy) return std::unexpected(std::move(canDeploy.error()));
    if (*canDeploy) {
      if (auto r = warpToOrbitedWorld(true); !r) return std::unexpected(std::move(r.error()));
    } else {
      auto canBeamUp = m_client->canBeamUp();
      if (!canBeamUp) return std::unexpected(std::move(canBeamUp.error()));
      if (*canBeamUp) {
        if (auto r = warpToOwnShip(); !r) return std::unexpected(std::move(r.error()));
      }
    }
    return true;
  }
  auto beam = overButton(m_config->mainBarBeamButtonPoly, mousePos);
  if (!beam) return std::unexpected(std::move(beam.error()));
  if (*beam) {
    auto canBeamDown = m_client->canBeamDown();
    if (!canBeamDown) return std::unexpected(std::move(canBeamDown.error()));
    if (*canBeamDown) {
      if (auto r = warpToOrbitedWorld(false); !r) return std::unexpected(std::move(r.error()));
    }
    return true;
  }
  auto questLog = overButton(m_config->mainBarQuestLogButtonPoly, mousePos);
  if (!questLog) return std::unexpected(std::move(questLog.error()));
  if (*questLog) {
    if (auto r = m_paneManager.toggleRegisteredPane(MainInterfacePanes::QuestLog); !r) return std::unexpected(std::move(r.error()));
    return true;
  }
  auto mmUpgrade = overButton(m_config->mainBarMmUpgradeButtonPoly, mousePos);
  if (!mmUpgrade) return std::unexpected(std::move(mmUpgrade.error()));
  if (*mmUpgrade) {
    if (m_client->mainPlayer()->inventory()->essentialItem(EssentialItem::BeamAxe)) {
      if (auto r = m_paneManager.toggleRegisteredPane(MainInterfacePanes::MmUpgrade); !r) return std::unexpected(std::move(r.error()));
    }
    return true;
  }
  auto collections = overButton(m_config->mainBarCollectionsButtonPoly, mousePos);
  if (!collections) return std::unexpected(std::move(collections.error()));
  if (*collections) {
    if (auto r = m_paneManager.toggleRegisteredPane(MainInterfacePanes::Collections); !r) return std::unexpected(std::move(r.error()));
    return true;
  }
  return false;
}

std::expected<void, StarException> MainInterface::initHttpTrustDialog() {
  auto dialog = HttpTrustDialog::create(*m_guiContext);
  if (!dialog)
    return std::unexpected(std::move(dialog.error()));
  return m_paneManager.registerPane(MainInterfacePanes::HttpTrustDialog, PaneLayer::ModalWindow, *dialog);
}

std::expected<void, StarException> MainInterface::displayScriptPane(std::shared_ptr<ScriptPane>& scriptPane, EntityId sourceEntity) {
  if (sourceEntity != NullEntityId)
    m_interactionScriptPanes[sourceEntity] = scriptPane;
  PaneLayer layer = PaneLayer::Window;
  auto layerName = scriptPane->config().optString("paneLayer");
  if (!layerName)
    return std::unexpected(std::move(layerName.error()));
  if (*layerName) {
    if (!PaneLayerNames)
      return std::unexpected(PaneLayerNames.error());
    auto parsed = PaneLayerNames->getLeft(**layerName);
    if (!parsed)
      return std::unexpected(std::move(parsed.error()));
    layer = **parsed;
  }
  auto withInventory = scriptPane->openWithInventory();
  if (!withInventory)
    return std::unexpected(std::move(withInventory.error()));
  if (*withInventory) {
    auto close = scriptPane->closeWithInventory();
    if (!close)
      return std::unexpected(std::move(close.error()));
    auto inventoryPane = m_paneManager.registeredPane(MainInterfacePanes::Inventory);
    if (!inventoryPane)
      return std::unexpected(std::move(inventoryPane.error()));
    auto assets = m_guiContext->root().assets();
    if (!assets)
      return std::unexpected(std::move(assets.error()));
    auto gapConfig = (*assets)->json("/interface.config:bringAdjacentWindowGap");
    if (!gapConfig)
      return std::unexpected(std::move(gapConfig.error()));
    auto gap = gapConfig->toFloat();
    if (!gap)
      return std::unexpected(std::move(gap.error()));
    auto displayed = m_paneManager.displayPane(layer, scriptPane, [this, close = *close](std::shared_ptr<Pane> const&) -> std::expected<void, StarException> {
      if (close) {
        auto dismissed = m_paneManager.dismissRegisteredPane(MainInterfacePanes::Inventory);
        if (!dismissed)
          return std::unexpected(std::move(dismissed.error()));
      }
      return {};
    });
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
    auto inventoryDisplayed = m_paneManager.displayRegisteredPane(MainInterfacePanes::Inventory);
    if (!inventoryDisplayed)
      return std::unexpected(std::move(inventoryDisplayed.error()));
    if (auto positioned = m_paneManager.bringPaneAdjacent(*inventoryPane, scriptPane, *gap); !positioned)
      return std::unexpected(std::move(positioned.error()));
  } else {
    auto displayed = m_paneManager.displayPane(layer, scriptPane);
    if (!displayed)
      return std::unexpected(std::move(displayed.error()));
  }
  return {};
}

}

import std;
import star.xx_hash;
import star.json;
import star.json_extra;
import star.id_map;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.format;
import star.text;
import star.asset_path;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;
import star.ordered_map;
import star.strong_typedef;
import star.rpc_promise;
import star.math_common;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.time;
import star.version;
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
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.btree_database;
import star.image;
import star.a_star;
import star.periodic_function;


import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.application_controller;
import star.renderer;
import star.lua_root;
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
import star.uuid;

import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;


import star.main_interface_types;
import star.progress_widget;
import star.label_widget;


import star.team_bar;
import star.inventory_types;
import star.item_descriptor;
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
import star.container_entity;
import star.container_interactor;
import star.inventory;
import star.animation;
import star.interface_cursor;
import star.warping;
import star.main_interface;
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
import star.liquid_types;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.celestial_types;
import star.tile_modification;
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
import star.widget_parsing;
import star.gui_reader;
import star.button_group;
import star.button_widget;
import star.image_widget;
import star.text_box_widget;
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
import star.portrait_widget;

import star.client_context;
import star.team_client;

namespace Star {

TeamBar::TeamBar(GuiContext& guiContext, MainInterface& mainInterface, std::shared_ptr<UniverseClient> client)
  : Pane(guiContext), m_mainInterface(mainInterface), m_client(std::move(client)) {}

std::expected<std::shared_ptr<TeamBar>, StarException> TeamBar::create(GuiContext& guiContext, MainInterface& mainInterface, std::shared_ptr<UniverseClient> client) {
  auto pane = std::shared_ptr<TeamBar>(new TeamBar(guiContext, mainInterface, std::move(client)));
  auto result = pane->initializePane();
  if (!result) return std::unexpected(std::move(result.error()));
  result = pane->initialize();
  if (!result) return std::unexpected(std::move(result.error()));
  return pane;
}

std::expected<void, StarException> TeamBar::initialize() {
  auto invite = TeamInvite::create(context(), *this);
  if (!invite) return std::unexpected(std::move(invite.error()));
  m_teamInvite = std::move(*invite);
  auto invitation = TeamInvitation::create(context(), *this);
  if (!invitation) return std::unexpected(std::move(invitation.error()));
  m_teamInvitation = std::move(*invitation);
  auto menu = TeamMemberMenu::create(context(), *this);
  if (!menu) return std::unexpected(std::move(menu.error()));
  m_teamMemberMenu = std::move(*menu);
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/interface/windowconfig/teambar.config");
  if (!config) return std::unexpected(std::move(config.error()));
  GuiReader reader(context());
  reader.registerCallback("inviteButton", [this](Widget&) { return inviteButton(); });
  reader.registerCallback("showSelfMenu", [this](Widget&) -> std::expected<void, StarException> {
    if (!m_client->teamClient()->isMemberOfTeam())
      return {};
    auto assets = context().root().assets();
    if (!assets) return std::unexpected(std::move(assets.error()));
    auto offset = (*assets)->json("/interface/windowconfig/teambar.config:selfMenuOffset");
    if (!offset) return std::unexpected(std::move(offset.error()));
    auto position = jsonToVec2I(*offset);
    if (!position) return std::unexpected(std::move(position.error()));
    auto height = windowHeight();
    if (!height) return std::unexpected(std::move(height.error()));
    (*position)[1] += *height / context().interfaceScale();
    return showMemberMenu(m_client->mainPlayer()->clientContext()->playerUuid(), *position);
  });
  auto layout = config->get("paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto result = reader.construct(*layout, *this);
  if (!result) return std::unexpected(std::move(result.error()));
  auto health = fetchChild<ProgressWidget>("healthBar");
  if (!health) return std::unexpected(std::move(health.error()));
  m_healthBar = std::move(*health);
  auto energy = fetchChild<ProgressWidget>("energyBar");
  if (!energy) return std::unexpected(std::move(energy.error()));
  m_energyBar = std::move(*energy);
  auto food = fetchChild<ProgressWidget>("foodBar");
  if (!food) return std::unexpected(std::move(food.error()));
  m_foodBar = std::move(*food);
  auto name = fetchChild<LabelWidget>("name");
  if (!name) return std::unexpected(std::move(name.error()));
  m_nameLabel = std::move(*name);
  auto colorConfig = config->get("energyBarColor");
  if (!colorConfig) return std::unexpected(std::move(colorConfig.error()));
  auto color = jsonToColor(*colorConfig);
  if (!color) return std::unexpected(std::move(color.error()));
  m_energyBarColor = *color;
  colorConfig = config->get("energyBarRegenMixColor");
  if (!colorConfig) return std::unexpected(std::move(colorConfig.error()));
  color = jsonToColor(*colorConfig);
  if (!color) return std::unexpected(std::move(color.error()));
  m_energyBarRegenMixColor = *color;
  colorConfig = config->get("energyBarUnusableColor");
  if (!colorConfig) return std::unexpected(std::move(colorConfig.error()));
  color = jsonToColor(*colorConfig);
  if (!color) return std::unexpected(std::move(color.error()));
  m_energyBarUnusableColor = *color;
  m_energyBar->setColor(m_energyBarColor);
  auto portrait = fetchChild<PortraitWidget>("portrait");
  if (!portrait) return std::unexpected(std::move(portrait.error()));
  auto entitySet = (*portrait)->setEntity(as<PortraitEntity>(m_client->mainPlayer()));
  if (!entitySet) return std::unexpected(std::move(entitySet.error()));
  result = updatePlayerResources();
  if (!result) return std::unexpected(std::move(result.error()));
  disableScissoring();
  return {};
}

std::expected<bool, StarException> TeamBar::sendEvent(InputEvent const& event) {
  if (std::holds_alternative<MouseButtonDownEvent>(event) &&
      (std::get<MouseButtonDownEvent>(event).mouseButton == MouseButton::Left || std::get<MouseButtonDownEvent>(event).mouseButton == MouseButton::Right)) {
    auto position = context().mousePosition(event);
    if (position && m_teamMemberMenu->isDisplayed() && !m_teamMemberMenu->inMember(*position))
      m_teamMemberMenu->dismiss();
  }
  return Pane::sendEvent(event);
}

std::expected<void, StarException> TeamBar::invitePlayer(String const& playerName) {
  return m_client->teamClient()->invitePlayer(playerName);
}

std::expected<void, StarException> TeamBar::acceptInvitation(Uuid const& inviterUuid) {
  return m_client->teamClient()->acceptInvitation(inviterUuid);
}

std::expected<void, StarException> TeamBar::update(float dt) {
  auto result = Pane::update(dt);
  if (!result) return std::unexpected(std::move(result.error()));
  result = updatePlayerResources();
  if (!result) return std::unexpected(std::move(result.error()));
  auto teamClient = m_client->teamClient();
  if (!m_teamInvitation->active() && teamClient->hasInvitationPending()) {
    auto invitation = teamClient->pullInvitation();
    result = m_teamInvitation->open(invitation.first, invitation.second);
    if (!result) return std::unexpected(std::move(result.error()));
    if (!m_teamInvitation->isDisplayed()) {
      if (!m_mainInterface) return std::unexpected(StarException("Team bar is detached from its interface"));
      result = m_mainInterface->get().paneManager().displayPane(PaneLayer::Window, m_teamInvitation);
      if (!result) return std::unexpected(std::move(result.error()));
    }
  }
  if (teamClient->currentTeam() && !teamClient->isTeamLeader())
    m_teamInvite->dismiss();
  auto leader = fetchChild<ImageWidget>("leader");
  if (!leader) return std::unexpected(std::move(leader.error()));
  result = (*leader)->setVisibility(teamClient->isTeamLeader());
  if (!result) return std::unexpected(std::move(result.error()));
  return buildTeamBar();
}

std::expected<void, StarException> TeamBar::updatePlayerResources() {
  auto player = m_client->mainPlayer();
  auto health = player->healthPercentage();
  if (!health) return std::unexpected(std::move(health.error()));
  m_healthBar->setCurrentProgressLevel(*health);
  auto energy = player->energyPercentage();
  if (!energy) return std::unexpected(std::move(energy.error()));
  m_energyBar->setCurrentProgressLevel(*energy);
  std::expected<void, StarException> result;
  if (player->modeConfig().hunger) {
    auto food = player->foodPercentage();
    if (!food) return std::unexpected(std::move(food.error()));
    m_foodBar->setCurrentProgressLevel(*food);
    auto assets = context().root().assets();
    if (!assets) return std::unexpected(std::move(assets.error()));
    auto thresholdConfig = (*assets)->json("/player.config:foodLowThreshold");
    if (!thresholdConfig) return std::unexpected(std::move(thresholdConfig.error()));
    auto threshold = thresholdConfig->toFloat();
    if (!threshold) return std::unexpected(std::move(threshold.error()));
    if (*food <= *threshold) {
      auto flashConfig = (*assets)->json("/interface/windowconfig/teambar.config:foodBarFlashTime");
      if (!flashConfig) return std::unexpected(std::move(flashConfig.error()));
      auto flashTime = flashConfig->toFloat();
      if (!flashTime) return std::unexpected(std::move(flashTime.error()));
      if (std::fmod(Time::monotonicTime(), *flashTime * 2) < *flashTime) {
        auto overlayConfig = (*assets)->json("/interface/windowconfig/teambar.config:foodBarFlashOverlay");
        if (!overlayConfig) return std::unexpected(std::move(overlayConfig.error()));
        auto overlay = overlayConfig->toString();
        if (!overlay) return std::unexpected(std::move(overlay.error()));
        m_foodBar->setOverlay(*overlay);
      } else {
        m_foodBar->setOverlay("");
      }
    } else {
      m_foodBar->setOverlay("");
    }
    result = m_foodBar->show();
  } else {
    result = m_foodBar->hide();
  }
  if (!result) return std::unexpected(std::move(result.error()));
  auto locked = player->energyLocked();
  if (!locked) return std::unexpected(std::move(locked.error()));
  if (*locked) {
    m_energyBar->setColor(m_energyBarUnusableColor);
  } else {
    auto regen = player->energyRegenBlockPercent();
    if (!regen) return std::unexpected(std::move(regen.error()));
    m_energyBar->setColor(m_energyBarColor.mix(m_energyBarRegenMixColor, *regen));
  }
  auto name = player->name();
  if (!name) return std::unexpected(std::move(name.error()));
  return m_nameLabel->setText(*name);
}

std::expected<void, StarException> TeamBar::inviteButton() {
  if (!m_teamInvite->isDisplayed()) {
    if (!m_mainInterface) return std::unexpected(StarException("Team bar is detached from its interface"));
    return m_mainInterface->get().paneManager().displayPane(PaneLayer::Window, m_teamInvite);
  }
  return {};
}

std::expected<void, StarException> TeamBar::buildTeamBar() {
  auto teamClient = m_client->teamClient();
  auto player = m_client->mainPlayer();
  auto list = fetchChild("list");
  if (!list) return std::unexpected(std::move(list.error()));
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/interface/windowconfig/teambar.config");
  if (!config) return std::unexpected(std::move(config.error()));
  auto portraitScale = config->getFloat("memberPortraitScale");
  if (!portraitScale) return std::unexpected(std::move(portraitScale.error()));
  auto memberSize = config->getInt("memberSize");
  if (!memberSize) return std::unexpected(std::move(memberSize.error()));
  auto memberSpacing = config->getInt("memberSpacing");
  if (!memberSpacing) return std::unexpected(std::move(memberSpacing.error()));
  Vec2I offset;
  std::size_t controlIndex = 0;
  Uuid myUuid = player->clientContext()->playerUuid();
  for (auto const& member : teamClient->members()) {
    if (member.uuid == myUuid)
      continue;
    String cellName = toString(controlIndex);
    auto cellResult = (*list)->fetchChild(cellName);
    if (!cellResult) return std::unexpected(std::move(cellResult.error()));
    auto cell = std::move(*cellResult);
    if (!cell) {
      GuiReader reader(context());
      cell = std::make_shared<Widget>(context());
      cell->disableScissoring();
      cell->markAsContainer();
      reader.registerCallback("showMemberMenu", [this](Widget& widget) -> std::expected<void, StarException> {
        auto assets = context().root().assets();
        if (!assets) return std::unexpected(std::move(assets.error()));
        auto config = (*assets)->json("/interface/windowconfig/teambar.config:memberMenuOffset");
        if (!config) return std::unexpected(std::move(config.error()));
        auto offset = jsonToVec2I(*config);
        if (!offset) return std::unexpected(std::move(offset.error()));
        auto memberId = widget.parent()->get().data().toString();
        if (!memberId) return std::unexpected(std::move(memberId.error()));
        auto uuid = Uuid::create(*memberId);
        if (!uuid) return std::unexpected(std::move(uuid.error()));
        return showMemberMenu(*uuid, widget.screenPosition() + *offset);
      });
      auto entry = config->get("entry");
      if (!entry) return std::unexpected(std::move(entry.error()));
      auto result = reader.construct(*entry, *cell);
      if (!result) return std::unexpected(std::move(result.error()));
      result = (*list)->addChild(cellName, cell);
      if (!result) return std::unexpected(std::move(result.error()));
    }
    offset[1] -= *memberSize;
    cell->setPosition(offset);
    cell->setData(member.uuid.hex());
    auto result = cell->show();
    if (!result) return std::unexpected(std::move(result.error()));
    auto leader = cell->fetchChild<ImageWidget>("leader");
    if (!leader) return std::unexpected(std::move(leader.error()));
    result = (*leader)->setVisibility(teamClient->isTeamLeader(member.uuid));
    if (!result) return std::unexpected(std::move(result.error()));
    auto portrait = cell->fetchChild<ImageWidget>("portrait");
    if (!portrait) return std::unexpected(std::move(portrait.error()));
    auto drawables = member.portrait;
    Drawable::scaleAll(drawables, *portraitScale);
    result = (*portrait)->setDrawables(std::move(drawables));
    if (!result) return std::unexpected(std::move(result.error()));
    auto compass = cell->fetchChild<ImageWidget>("compass");
    if (!compass) return std::unexpected(std::move(compass.error()));
    auto offworld = cell->fetchChild<ImageWidget>("compassoffworld");
    if (!offworld) return std::unexpected(std::move(offworld.error()));
    if (member.world == m_client->playerWorld() && m_client->worldClient()) {
      auto position = member.position;
      if (auto entity = m_client->worldClient()->entity(member.entity))
        position = entity->position();
      auto direction = m_client->worldClient()->geometry().diff(position, player->position());
      result = (*compass)->setRotation(direction.angle() - std::numbers::pi / 2.0f);
      if (!result) return std::unexpected(std::move(result.error()));
      result = (*compass)->show();
      if (!result) return std::unexpected(std::move(result.error()));
      result = (*offworld)->hide();
    } else {
      result = (*compass)->hide();
      if (!result) return std::unexpected(std::move(result.error()));
      result = (*offworld)->show();
    }
    if (!result) return std::unexpected(std::move(result.error()));
    auto health = cell->fetchChild<ProgressWidget>("healthBar");
    if (!health) return std::unexpected(std::move(health.error()));
    (*health)->setCurrentProgressLevel(member.healthPercentage);
    auto energy = cell->fetchChild<ProgressWidget>("energyBar");
    if (!energy) return std::unexpected(std::move(energy.error()));
    (*energy)->setCurrentProgressLevel(member.energyPercentage);
    offset[1] -= *memberSpacing;
    ++controlIndex;
  }
  auto invite = fetchChild<ButtonWidget>("inviteButton");
  if (!invite) return std::unexpected(std::move(invite.error()));
  auto noInvite = fetchChild<ImageWidget>("noInviteImage");
  if (!noInvite) return std::unexpected(std::move(noInvite.error()));
  Vec2I inviteOffset = (*list)->position() + offset;
  (*invite)->setPosition(inviteOffset - Vec2I{0, (*invite)->size()[1]});
  (*noInvite)->setPosition(inviteOffset - Vec2I{0, (*noInvite)->size()[1]});
  bool couldInvite = !teamClient->currentTeam() || teamClient->isTeamLeader();
  if (couldInvite) {
    auto configuration = context().root().configuration();
    if (!configuration) return std::unexpected(std::move(configuration.error()));
    auto maxTeamConfig = (*configuration)->get("maxTeamSize");
    if (!maxTeamConfig) return std::unexpected(std::move(maxTeamConfig.error()));
    auto maxTeamSize = maxTeamConfig->toUInt();
    if (!maxTeamSize) return std::unexpected(std::move(maxTeamSize.error()));
    couldInvite = teamClient->members().size() < *maxTeamSize;
  }
  auto result = (*invite)->setVisibility(couldInvite);
  if (!result) return std::unexpected(std::move(result.error()));
  (*invite)->setEnabled(!m_teamInvitation->active());
  result = (*noInvite)->setVisibility(!couldInvite);
  if (!result) return std::unexpected(std::move(result.error()));
  while (true) {
    auto cell = (*list)->fetchChild(toString(controlIndex));
    if (!cell) return std::unexpected(std::move(cell.error()));
    if (!*cell)
      break;
    result = (*cell)->hide();
    if (!result) return std::unexpected(std::move(result.error()));
    ++controlIndex;
  }
  return {};
}

std::expected<void, StarException> TeamBar::showMemberMenu(Uuid memberUuid, Vec2I position) {
  auto result = m_teamMemberMenu->open(memberUuid, position);
  if (!result) return std::unexpected(std::move(result.error()));
  if (!m_teamMemberMenu->isDisplayed()) {
    if (!m_mainInterface) return std::unexpected(StarException("Team bar is detached from its interface"));
    return m_mainInterface->get().paneManager().displayPane(PaneLayer::Window, m_teamMemberMenu);
  }
  return {};
}

TeamInvite::TeamInvite(GuiContext& guiContext, TeamBar& owner) : Pane(guiContext), m_owner(std::static_pointer_cast<TeamBar>(owner.shared_from_this())) {}

std::expected<std::shared_ptr<TeamInvite>, StarException> TeamInvite::create(GuiContext& guiContext, TeamBar& owner) {
  auto pane = std::shared_ptr<TeamInvite>(new TeamInvite(guiContext, owner));
  auto result = pane->initializePane();
  if (!result) return std::unexpected(std::move(result.error()));
  result = pane->initialize();
  if (!result) return std::unexpected(std::move(result.error()));
  return pane;
}

std::expected<void, StarException> TeamInvite::initialize() {
  GuiReader reader(context());
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  reader.registerCallback("ok", [this](Widget&) { return ok(); });
  reader.registerCallback("close", [this](Widget&) -> std::expected<void, StarException> { close(); return {}; });
  reader.registerCallback("name", [](Widget&) -> std::expected<void, StarException> { return {}; });
  auto layout = (*assets)->json("/interface/windowconfig/teaminvite.config:paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto result = reader.construct(*layout, *this);
  if (!result) return std::unexpected(std::move(result.error()));
  dismiss();
  return {};
}

std::expected<void, StarException> TeamInvite::show() {
  auto result = Pane::show();
  if (!result) return std::unexpected(std::move(result.error()));
  auto name = fetchChild<TextBoxWidget>("name");
  if (!name) return std::unexpected(std::move(name.error()));
  auto textSet = (*name)->setText("", false);
  if (!textSet) return std::unexpected(std::move(textSet.error()));
  return (*name)->focus();
}

std::expected<void, StarException> TeamInvite::ok() {
  auto name = fetchChild<TextBoxWidget>("name");
  if (!name) return std::unexpected(std::move(name.error()));
  auto owner = m_owner.lock();
  if (!owner) return std::unexpected(StarException("Team bar is no longer available"));
  auto result = owner->invitePlayer((*name)->getText());
  if (!result) return std::unexpected(std::move(result.error()));
  dismiss();
  return {};
}

void TeamInvite::close() {
  dismiss();
}

TeamInvitation::TeamInvitation(GuiContext& guiContext, TeamBar& owner)
  : Pane(guiContext), m_owner(std::static_pointer_cast<TeamBar>(owner.shared_from_this())), m_inviterUuid(guiContext.root().random()) {}

std::expected<std::shared_ptr<TeamInvitation>, StarException> TeamInvitation::create(GuiContext& guiContext, TeamBar& owner) {
  auto pane = std::shared_ptr<TeamInvitation>(new TeamInvitation(guiContext, owner));
  auto result = pane->initializePane();
  if (!result) return std::unexpected(std::move(result.error()));
  result = pane->initialize();
  if (!result) return std::unexpected(std::move(result.error()));
  return pane;
}

std::expected<void, StarException> TeamInvitation::initialize() {
  GuiReader reader(context());
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  reader.registerCallback("ok", [this](Widget&) { return ok(); });
  reader.registerCallback("close", [this](Widget&) -> std::expected<void, StarException> { close(); return {}; });
  auto layout = (*assets)->json("/interface/windowconfig/teaminvitation.config:paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  auto result = reader.construct(*layout, *this);
  if (!result) return std::unexpected(std::move(result.error()));
  dismiss();
  return {};
}

std::expected<void, StarException> TeamInvitation::open(Uuid const& inviterUuid, String const& inviterName) {
  if (active())
    return {};
  auto name = fetchChild<LabelWidget>("inviterName");
  if (!name) return std::unexpected(std::move(name.error()));
  auto result = (*name)->setText(inviterName);
  if (!result) return std::unexpected(std::move(result.error()));
  m_inviterUuid = inviterUuid;
  return Pane::show();
}

std::expected<void, StarException> TeamInvitation::ok() {
  auto owner = m_owner.lock();
  if (!owner) return std::unexpected(StarException("Team bar is no longer available"));
  auto result = owner->acceptInvitation(m_inviterUuid);
  if (!result) return std::unexpected(std::move(result.error()));
  dismiss();
  return {};
}

void TeamInvitation::close() {
  dismiss();
}

TeamMemberMenu::TeamMemberMenu(GuiContext& guiContext, TeamBar& owner)
  : Pane(guiContext), m_owner(std::static_pointer_cast<TeamBar>(owner.shared_from_this())), m_memberUuid(guiContext.root().random()), m_canBeam(false) {}

std::expected<std::shared_ptr<TeamMemberMenu>, StarException> TeamMemberMenu::create(GuiContext& guiContext, TeamBar& owner) {
  auto pane = std::shared_ptr<TeamMemberMenu>(new TeamMemberMenu(guiContext, owner));
  auto result = pane->initializePane();
  if (!result) return std::unexpected(std::move(result.error()));
  result = pane->initialize();
  if (!result) return std::unexpected(std::move(result.error()));
  return pane;
}

std::expected<void, StarException> TeamMemberMenu::initialize() {
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  GuiReader reader(context());
  reader.registerCallback("beamToShip", [this](Widget&) { return beamToShip(); });
  reader.registerCallback("close", [this](Widget&) -> std::expected<void, StarException> { close(); return {}; });
  reader.registerCallback("makeLeader", [this](Widget&) { return makeLeader(); });
  reader.registerCallback("removeFromTeam", [this](Widget&) { return removeFromTeam(); });
  auto layout = (*assets)->json("/interface/windowconfig/teammembermenu.config:paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  return reader.construct(*layout, *this);
}

std::expected<void, StarException> TeamMemberMenu::open(Uuid memberUuid, Vec2I position) {
  if (active())
    return {};
  setPosition(position);
  m_memberUuid = memberUuid;
  auto owner = m_owner.lock();
  if (!owner) return std::unexpected(StarException("Team bar is no longer available"));
  for (auto const& member : owner->m_client->teamClient()->members()) {
    if (member.uuid == m_memberUuid) {
      auto name = fetchChild<LabelWidget>("name");
      if (!name) return std::unexpected(std::move(name.error()));
      auto result = (*name)->setText(member.name);
      if (!result) return std::unexpected(std::move(result.error()));
      break;
    }
  }
  auto result = updateWidgets();
  if (!result) return std::unexpected(std::move(result.error()));
  return Pane::show();
}

std::expected<void, StarException> TeamMemberMenu::update(float dt) {
  bool stillValid = false;
  auto owner = m_owner.lock();
  if (!owner) {
    close();
    return {};
  }
  for (auto const& member : owner->m_client->teamClient()->members()) {
    if (member.uuid == m_memberUuid) {
      stillValid = true;
      m_canBeam = false;
      if (member.warpMode != WarpMode::None) {
        auto canBeam = owner->m_client->canBeamToTeamShip();
        if (!canBeam) return std::unexpected(std::move(canBeam.error()));
        m_canBeam = *canBeam;
      }
    }
  }
  if (!stillValid) {
    close();
    return {};
  }
  auto result = updateWidgets();
  if (!result) return std::unexpected(std::move(result.error()));
  return Pane::update(dt);
}

std::expected<void, StarException> TeamMemberMenu::updateWidgets() {
  auto owner = m_owner.lock();
  if (!owner) return std::unexpected(StarException("Team bar is no longer available"));
  bool isLeader = owner->m_client->teamClient()->isTeamLeader();
  bool isSelf = owner->m_client->mainPlayer()->clientContext()->playerUuid() == m_memberUuid;
  auto beam = fetchChild<ButtonWidget>("beamToShip");
  if (!beam) return std::unexpected(std::move(beam.error()));
  (*beam)->setEnabled(m_canBeam);
  auto leader = fetchChild<ButtonWidget>("makeLeader");
  if (!leader) return std::unexpected(std::move(leader.error()));
  (*leader)->setEnabled(isLeader && !isSelf);
  auto remove = fetchChild<ButtonWidget>("removeFromTeam");
  if (!remove) return std::unexpected(std::move(remove.error()));
  (*remove)->setEnabled(isLeader || isSelf);
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto textConfig = (*assets)->json(isSelf ? String("/interface/windowconfig/teammembermenu.config:removeSelfText") : String("/interface/windowconfig/teammembermenu.config:removeOtherText"));
  if (!textConfig) return std::unexpected(std::move(textConfig.error()));
  auto text = textConfig->toString();
  if (!text) return std::unexpected(std::move(text.error()));
  (*remove)->setText(*text);
  return {};
}

std::expected<void, StarException> TeamMemberMenu::beamToShip() {
  if (m_canBeam) {
    auto owner = m_owner.lock();
    if (!owner) return std::unexpected(StarException("Team bar is no longer available"));
    if (!owner->m_mainInterface) return std::unexpected(StarException("Team bar is detached from its interface"));
    auto result = owner->m_mainInterface->get().warpTo(WarpToWorld{ClientShipWorldId(m_memberUuid), {}});
    if (!result) return std::unexpected(std::move(result.error()));
  }
  dismiss();
  return {};
}

void TeamMemberMenu::close() {
  dismiss();
}

std::expected<void, StarException> TeamMemberMenu::makeLeader() {
  auto owner = m_owner.lock();
  if (!owner) return std::unexpected(StarException("Team bar is no longer available"));
  auto result = owner->m_client->teamClient()->makeLeader(m_memberUuid);
  if (!result) return std::unexpected(std::move(result.error()));
  dismiss();
  return {};
}

std::expected<void, StarException> TeamMemberMenu::removeFromTeam() {
  auto owner = m_owner.lock();
  if (!owner) return std::unexpected(StarException("Team bar is no longer available"));
  auto result = owner->m_client->teamClient()->removeFromTeam(m_memberUuid);
  if (!result) return std::unexpected(std::move(result.error()));
  dismiss();
  return {};
}

}

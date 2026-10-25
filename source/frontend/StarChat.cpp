import star.world_lua_bindings;
import std;
import star.json;
import star.xx_hash;
import star.json_extra;
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
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.time;
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
import star.either;
import star.random;
import star.weighted_pool;
import star.strong_typedef;
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
import star.logging;
import star.a_star;
import star.periodic_function;



import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.application_controller;
import star.renderer;
import star.lua_components;
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
import star.widget_parsing;
import star.gui_reader;


import star.base_script_pane;
import star.chat_types;


import star.chat;
import star.host_address;
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
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
import star.lua_root;
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
import star.game_timers;
import star.world_client;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.button_group;
import star.button_widget;
import star.text_box_widget;
import star.label_widget;
import star.image_stretch_widget;
import star.canvas_widget;

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
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.lua_game_converters;

import star.player_lua_bindings;
import star.config_lua_bindings;
import star.celestial_lua_bindings;
import star.status_controller_lua_bindings;

import star.player_storage;
import star.team_client;

import star.algorithm;
namespace Star {
Chat::Chat(GuiContext& guiContext, std::shared_ptr<UniverseClient> client) : BaseScriptPane(guiContext), m_client(std::move(client)) {}

std::expected<std::shared_ptr<Chat>, StarException> Chat::create(GuiContext& guiContext, std::shared_ptr<UniverseClient> client, Json const& baseConfig) {
  auto chat = std::shared_ptr<Chat>(new Chat(guiContext, std::move(client)));
  auto initialized = chat->initialize(baseConfig);
  if (!initialized) return std::unexpected(std::move(initialized.error()));
  return chat;
}

std::expected<void, StarException> Chat::initialize(Json const& baseConfig) {
  auto base = BaseScriptPane::init(baseConfig, false);
  if (!base) return std::unexpected(std::move(base.error()));
  auto scripts = baseConfig.get("scripts", Json());
  if (!scripts) return std::unexpected(std::move(scripts.error()));
  m_scripted = scripts->isType(Json::Type::Array);
  auto luaRoot = m_script.setLuaRoot(m_client->luaRoot());
  if (!luaRoot) return std::unexpected(std::move(luaRoot.error()));
  auto worldCallbacks = LuaBindings::makeWorldCallbacks(context().root(), *m_client->worldClient());
  if (!worldCallbacks) return std::unexpected(std::move(worldCallbacks.error()));
  auto addedWorld = m_script.addCallbacks("world", std::move(*worldCallbacks));
  if (!addedWorld) return std::unexpected(std::move(addedWorld.error()));
  m_chatPrevIndex = 0;
  m_historyOffset = 0;
  auto config = baseConfig.get("config");
  if (!config) return std::unexpected(std::move(config.error()));
  m_timeChatLastActive = Time::monotonicMilliseconds();
  auto textJson = config->get("textStyle");
  if (!textJson) return std::unexpected(std::move(textJson.error()));
  auto textStyle = TextStyle::create(*textJson);
  if (!textStyle) return std::unexpected(std::move(textStyle.error()));
  m_chatTextStyle = std::move(*textStyle);
  {
    auto value = config->getFloat("lineHeight");
    if (!value) return std::unexpected(std::move(value.error()));
    m_chatTextStyle.lineSpacing = std::move(*value);
  }
  {
    auto value = config->getFloat("visTime");
    if (!value) return std::unexpected(std::move(value.error()));
    m_chatVisTime = std::move(*value);
  }
  {
    auto value = config->getDouble("fadeRate");
    if (!value) return std::unexpected(std::move(value.error()));
    m_fadeRate = std::move(*value);
  }
  {
    auto value = config->getInt("chatHistoryLimit");
    if (!value) return std::unexpected(std::move(value.error()));
    m_chatHistoryLimit = std::move(*value);
  }
  {
    auto value = config->getString("chatFormatString");
    if (!value) return std::unexpected(std::move(value.error()));
    m_chatFormatString = std::move(*value);
  }
  {
    auto value = config->getFloat("portraitScale");
    if (!value) return std::unexpected(std::move(value.error()));
    m_portraitScale = std::move(*value);
  }
  {
    auto value = config->getFloat("portraitVerticalMargin");
    if (!value) return std::unexpected(std::move(value.error()));
    m_portraitVerticalMargin = std::move(*value);
  }
  {
    auto value = config->getString("portraitBackground");
    if (!value) return std::unexpected(std::move(value.error()));
    m_portraitBackground = std::move(*value);
  }
  {
    auto value = config->getInt("bodyHeight");
    if (!value) return std::unexpected(std::move(value.error()));
    m_bodyHeight = std::move(*value);
  }
  {
    auto value = config->getInt("expandedBodyHeight");
    if (!value) return std::unexpected(std::move(value.error()));
    m_expandedBodyHeight = std::move(*value);
  }
  {
    auto json = config->get("portraitTextOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    m_portraitTextOffset = *value;
  }
  {
    auto json = config->get("portraitImageOffset");
    if (!json) return std::unexpected(std::move(json.error()));
    auto value = jsonToVec2I(*json);
    if (!value) return std::unexpected(std::move(value.error()));
    m_portraitImageOffset = *value;
  }
  {
    auto value = config->queryString("colors.local");
    if (!value) return std::unexpected(std::move(value.error()));
    m_colorCodes[MessageContext::Local] = std::move(*value);
  }
  {
    auto value = config->queryString("colors.party");
    if (!value) return std::unexpected(std::move(value.error()));
    m_colorCodes[MessageContext::Party] = std::move(*value);
  }
  {
    auto value = config->queryString("colors.broadcast");
    if (!value) return std::unexpected(std::move(value.error()));
    m_colorCodes[MessageContext::Broadcast] = std::move(*value);
  }
  {
    auto value = config->queryString("colors.whisper");
    if (!value) return std::unexpected(std::move(value.error()));
    m_colorCodes[MessageContext::Whisper] = std::move(*value);
  }
  {
    auto value = config->queryString("colors.commandResult");
    if (!value) return std::unexpected(std::move(value.error()));
    m_colorCodes[MessageContext::CommandResult] = std::move(*value);
  }
  {
    auto value = config->queryString("colors.radioMessage");
    if (!value) return std::unexpected(std::move(value.error()));
    m_colorCodes[MessageContext::RadioMessage] = std::move(*value);
  }
  {
    auto value = config->queryString("colors.world");
    if (!value) return std::unexpected(std::move(value.error()));
    m_colorCodes[MessageContext::World] = std::move(*value);
  }
  if (!m_scripted) {
    m_reader->registerCallback("textBox", [this](Widget&) { return startChat(); });
    m_reader->registerCallback("upButton", [this](Widget&) { return scrollUp(); });
    m_reader->registerCallback("downButton", [this](Widget&) { return scrollDown(); });
    m_reader->registerCallback("bottomButton", [this](Widget&) { return scrollBottom(); });
    m_reader->registerCallback("filterGroup", [this](Widget& widget) -> std::expected<void, StarException> {
      Json data = static_cast<ButtonWidget&>(widget).data();
      auto filter = data.getArray("filter", {});
      if (!filter) return std::unexpected(std::move(filter.error()));
      m_modeFilter.clear();
      if (!MessageContextModeNames) return std::unexpected(MessageContextModeNames.error());
      for (auto const& mode : *filter) {
        auto name = mode.toString();
        if (!name) return std::unexpected(std::move(name.error()));
        auto value = MessageContextModeNames->getLeft(*name);
        if (!value) return std::unexpected(std::move(value.error()));
        m_modeFilter.insert(value->get());
      }
      auto mode = data.getString("sendMode", "Broadcast");
      if (!mode) return std::unexpected(std::move(mode.error()));
      if (!ChatSendModeNames) return std::unexpected(ChatSendModeNames.error());
      auto value = ChatSendModeNames->getLeft(*mode);
      if (!value) return std::unexpected(std::move(value.error()));
      m_sendMode = value->get();
      m_historyOffset = 0;
      return {};
    });
  }
  auto gui = baseConfig.get("gui");
  if (!gui) return std::unexpected(std::move(gui.error()));
  auto constructed = construct(*gui);
  if (!constructed) return std::unexpected(std::move(constructed.error()));
  m_sendMode = ChatSendMode::Broadcast;
  {
    auto child = fetchChild<CanvasWidget>("chatLog");
    if (!child) return std::unexpected(std::move(child.error()));
    m_chatLog = std::move(*child);
  }
  {
    auto child = fetchChild<ButtonWidget>("bottomButton");
    if (!child) return std::unexpected(std::move(child.error()));
    m_bottomButton = std::move(*child);
  }
  {
    auto child = fetchChild<ButtonWidget>("upButton");
    if (!child) return std::unexpected(std::move(child.error()));
    m_upButton = std::move(*child);
  }
  {
    auto child = fetchChild<TextBoxWidget>("textBox");
    if (!child) return std::unexpected(std::move(child.error()));
    m_textBox = std::move(*child);
  }
  {
    auto child = fetchChild<LabelWidget>("say");
    if (!child) return std::unexpected(std::move(child.error()));
    m_say = std::move(*child);
  }
  {
    auto child = fetchChild<ImageStretchWidget>("background");
    if (!child) return std::unexpected(std::move(child.error()));
    m_background = std::move(*child);
  }
  if (!m_scripted) {
    auto padding = config->optQuery("padding");
    if (!padding) return std::unexpected(std::move(padding.error()));
    if (*padding) {
      auto value = jsonToVec2I(**padding);
      if (!value) return std::unexpected(std::move(value.error()));
      m_chatLogPadding = *value;
      m_chatLog->setSize(m_chatLog->size() + m_chatLogPadding * 2);
      m_chatLog->setPosition(m_chatLog->position() - m_chatLogPadding);
    } else m_chatLogPadding = Vec2I();
    auto history = m_client->playerStorage()->getMetadata("chatHistory");
    if (!history) return std::unexpected(std::move(history.error()));
    if (auto value = history->opt()) {
      auto lines = jsonToStringList(*value);
      if (!lines) return std::unexpected(std::move(lines.error()));
      m_chatHistory.appendAll(*lines);
    }
  } else {
    {
      auto callbacks = LuaBindings::makePlayerCallbacks(context().root(), *m_client->mainPlayer());
      if (!callbacks) return std::unexpected(std::move(callbacks.error()));
      auto added = m_script.addCallbacks("player", std::move(*callbacks));
      if (!added) return std::unexpected(std::move(added.error()));
    }
    {
      auto callbacks = LuaBindings::makeStatusControllerCallbacks(m_client->mainPlayer()->statusController());
      if (!callbacks) return std::unexpected(std::move(callbacks.error()));
      auto added = m_script.addCallbacks("status", std::move(*callbacks));
      if (!added) return std::unexpected(std::move(added.error()));
    }
    {
      auto callbacks = LuaBindings::makeCelestialCallbacks(context().root(), *m_client);
      if (!callbacks) return std::unexpected(std::move(callbacks.error()));
      auto added = m_script.addCallbacks("celestial", std::move(*callbacks));
      if (!added) return std::unexpected(std::move(added.error()));
    }
  }
  auto shown = show();
  if (!shown) return std::unexpected(std::move(shown.error()));
  m_defaultHeight = m_background->size()[1];
  m_expanded = false;
  return updateSize();
}

std::expected<void, StarException> Chat::update(float dt) {
  auto updated = Pane::update(dt);
  if (!updated) return std::unexpected(std::move(updated.error()));
  if (m_scripted) return {};
  auto team = m_client->teamClient()->currentTeam();
  auto group = fetchChild<ButtonGroup>("filterGroup");
  if (!group) return std::unexpected(std::move(group.error()));
  if (!ChatSendModeNames) return std::unexpected(ChatSendModeNames.error());
  for (auto const& button : (*group)->buttons()) {
    auto name = button.get().data().getString("sendMode", "Broadcast");
    if (!name) return std::unexpected(std::move(name.error()));
    auto mode = ChatSendModeNames->getLeft(*name);
    if (!mode) return std::unexpected(std::move(mode.error()));
    if (!team && m_sendMode == ChatSendMode::Party && mode->get() == ChatSendMode::Broadcast) {
      auto selected = button.get().check();
      if (!selected) return std::unexpected(std::move(selected.error()));
    }
    if (mode->get() == ChatSendMode::Party) button.get().setEnabled(team.has_value());
  }
  return {};
}

std::expected<void, StarException> Chat::startChat() {
  if (m_scripted) {
    auto invoked = m_script.invoke("startChat");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
  } else {
    auto shown = show();
    if (!shown) return std::unexpected(std::move(shown.error()));
    return m_textBox->focus();
  }
  return {};
}

std::expected<void, StarException> Chat::startCommand() {
  if (m_scripted) {
    auto invoked = m_script.invoke("startCommand");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
  } else {
    auto shown = show();
    if (!shown) return std::unexpected(std::move(shown.error()));
    auto text = m_textBox->setText("/");
    if (!text) return std::unexpected(std::move(text.error()));
    return m_textBox->focus();
  }
  return {};
}

std::expected<void, StarException> Chat::stopChat() {
  if (m_scripted) {
    auto invoked = m_script.invoke("stopChat");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
  } else {
    auto text = m_textBox->setText("");
    if (!text) return std::unexpected(std::move(text.error()));
    auto blurred = m_textBox->blur();
    if (!blurred) return std::unexpected(std::move(blurred.error()));
    m_timeChatLastActive = Time::monotonicMilliseconds();
  }
  return {};
}

std::expected<void, StarException> Chat::clearCurrentChat() {
  if (m_scripted) {
    auto invoked = m_script.invoke("clearCurrentChat");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
  } else {
    auto text = m_textBox->setText("");
    if (!text) return std::unexpected(std::move(text.error()));
    m_chatPrevIndex = 0;
  }
  return {};
}

std::expected<bool, StarException> Chat::hasFocus() const {
  if (m_scripted) {
    auto invoked = m_script.invoke<bool>("hasFocus");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
    return invoked->value_or(false);
  }
  return m_textBox->hasFocus();
}

std::expected<String, StarException> Chat::currentChat() const {
  if (m_scripted) {
    auto invoked = m_script.invoke<String>("currentChat");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
    return invoked->value_or(String{});
  }
  return m_textBox->getText();
}

std::expected<bool, StarException> Chat::setCurrentChat(String const& chat, bool moveCursor) {
  if (m_scripted) {
    auto invoked = m_script.invoke<bool>("setCurrentChat");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
    return invoked->value_or(false);
  }
  return m_textBox->setText(chat, true, moveCursor);
}

std::expected<ChatSendMode, StarException> Chat::sendMode() const {
  if (!m_scripted) return m_sendMode;
  auto invoked = m_script.invoke<String>("sendMode");
  if (!invoked) return std::unexpected(std::move(invoked.error()));
  if (!ChatSendModeNames) return std::unexpected(ChatSendModeNames.error());
  auto mode = ChatSendModeNames->getLeft(invoked->value_or(String{}));
  if (!mode) return std::unexpected(std::move(mode.error()));
  return mode->get();
}

std::expected<void, StarException> Chat::incrementIndex() {
  if (!m_chatHistory.empty()) {
    m_chatPrevIndex = std::min(m_chatPrevIndex + 1, (unsigned)m_chatHistory.size());
    auto text = m_textBox->setText(m_chatHistory[m_chatPrevIndex - 1]);
    if (!text) return std::unexpected(std::move(text.error()));
  }
  return {};
}

std::expected<void, StarException> Chat::decrementIndex() {
  if (m_chatPrevIndex > 1 && !m_chatHistory.empty()) {
    --m_chatPrevIndex;
    auto text = m_textBox->setText(m_chatHistory[m_chatPrevIndex - 1]);
    if (!text) return std::unexpected(std::move(text.error()));
  } else {
    m_chatPrevIndex = 0;
    auto text = m_textBox->setText("");
    if (!text) return std::unexpected(std::move(text.error()));
  }
  return {};
}

std::expected<void, StarException> Chat::addLine(String const& text, bool showPane) {
  ChatReceivedMessage message = {{MessageContext::CommandResult}, ServerConnectionId, "", text};
  return addMessages({message}, showPane);
}

std::expected<void, StarException> Chat::addMessages(List<ChatReceivedMessage> const& messages, bool showPane) {
  if (messages.empty()) return {};
  if (m_scripted) {
    JsonArray jsonMessages;
    for (auto const& message : messages) {
      auto json = message.toJson();
      if (!json) return std::unexpected(std::move(json.error()));
      jsonMessages.append(std::move(*json));
    }
    auto invoked = m_script.invoke("addMessages", jsonMessages, showPane);
    if (!invoked) return std::unexpected(std::move(invoked.error()));
    return {};
  }
  auto& guiContext = context();
  for (auto const& message : messages) {
    std::optional<unsigned> wrapWidth;
    if (message.portrait.empty()) wrapWidth = m_chatLog->size()[0] - m_chatLogPadding[0] * 2;
    auto styled = guiContext.setTextStyle(m_chatTextStyle);
    if (!styled) return std::unexpected(std::move(styled.error()));
    String text = message.text;
    if (!message.fromNick.empty() && message.portrait.empty()) {
      std::array<std::pair<std::string_view, std::string_view>, 2> tags{{{"fromNick", message.fromNick.utf8()}, {"text", message.text.utf8()}}};
      auto formatted = formatTags(m_chatFormatString.utf8(), tags);
      if (!formatted) return std::unexpected(std::move(formatted.error()));
      text = String(std::move(*formatted));
    }
    auto lines = guiContext.wrapInterfaceText(text, wrapWidth);
    if (!lines) return std::unexpected(std::move(lines.error()));
    for (auto& line : *lines) m_receivedMessages.prepend({message.context.mode, message.portrait, std::move(line)});
    if (!message.fromNick.empty()) {
      auto logged = context().root().logger().info("Chat: <{}> {}", message.fromNick, message.text);
      if (!logged) return std::unexpected(std::move(logged.error()));
    } else {
      auto logged = context().root().logger().info("Chat: {}", message.text);
      if (!logged) return std::unexpected(std::move(logged.error()));
    }
  }
  if (showPane) {
    m_timeChatLastActive = Time::monotonicMilliseconds();
    auto shown = show();
    if (!shown) return std::unexpected(std::move(shown.error()));
  }
  if ((unsigned)m_receivedMessages.size() > m_chatHistoryLimit) m_receivedMessages.resize(m_chatHistoryLimit);
  return {};
}

std::expected<void, StarException> Chat::addHistory(String const& chat) {
  if (m_scripted) {
    auto invoked = m_script.invoke("addHistory", chat);
    if (!invoked) return std::unexpected(std::move(invoked.error()));
    return {};
  }
  if (!m_chatHistory.empty() && m_chatHistory[0] == chat) return {};
  m_chatHistory.prepend(chat);
  m_chatHistory.resize(std::min((unsigned)m_chatHistory.size(), m_chatHistoryLimit));
  m_timeChatLastActive = Time::monotonicMilliseconds();
  return m_client->playerStorage()->setMetadata("chatHistory", JsonArray::from(m_chatHistory));
}

std::expected<void, StarException> Chat::clear(std::size_t count) {
  if (m_scripted) {
    auto invoked = m_script.invoke("clear", count);
    if (!invoked) return std::unexpected(std::move(invoked.error()));
  } else if (count > m_receivedMessages.size()) m_receivedMessages.clear();
  else m_receivedMessages.erase(m_receivedMessages.begin(), m_receivedMessages.begin() + count);
  return {};
}

std::expected<void, StarException> Chat::renderImpl() {
  auto rendered = Pane::renderImpl();
  if (!rendered) return std::unexpected(std::move(rendered.error()));
  if (m_scripted)
    return {};
  auto focused = m_textBox->hasFocus();
  if (!focused) return std::unexpected(std::move(focused.error()));
  if (*focused)
    m_timeChatLastActive = Time::monotonicMilliseconds();
  Vec4B fade = {255, 255, 255, 255};
  auto visibility = visible();
  if (!visibility) return std::unexpected(std::move(visibility.error()));
  fade[3] = (std::uint8_t)(*visibility * 255);
  if (!*visibility) {
    auto hidden = hide();
    if (!hidden) return std::unexpected(std::move(hidden.error()));
    return {};
  }

  Color fadeGreen = Color::Green;
  fadeGreen.setAlpha(fade[3]);
  m_say->setColor(fadeGreen);

  m_chatLog->clear();
  Vec2I chatMin = m_chatLogPadding;
  int messageIndex = -m_historyOffset;

  GuiContext& guiContext = context();
  float lineHeight = m_chatTextStyle.lineSpacing;
  float fontSize = m_chatTextStyle.fontSize;
  auto styled = guiContext.setTextStyle(m_chatTextStyle);
  if (!styled) return std::unexpected(std::move(styled.error()));
  for (auto& message : m_receivedMessages) {
    if (!m_modeFilter.empty() && !m_modeFilter.contains(message.mode))
      continue;

    messageIndex++;
    if (messageIndex <= 0)
      continue;
    if (chatMin[1] > m_chatLog->size()[1])
      break;

    String channelColorCode = "^reset";
    if (m_colorCodes.contains(message.mode))
      channelColorCode = m_colorCodes[message.mode];
    channelColorCode += "^set;";

    String messageString = channelColorCode + message.text;

    float messageHeight = 0;
    float lineHeightMargin = ((lineHeight * fontSize) - fontSize);
    unsigned wrapWidth = m_chatLog->size()[0] - m_chatLogPadding[0] * 2;

    if (message.portrait != "") {
      TextPositioning tp = {Vec2F(chatMin +  m_portraitTextOffset), HorizontalAnchor::LeftAnchor, VerticalAnchor::VMidAnchor, (wrapWidth - m_portraitTextOffset[0])};
      auto textBounds = guiContext.determineInterfaceTextSize(messageString, tp);
      if (!textBounds) return std::unexpected(std::move(textBounds.error()));
      auto portraitBounds = guiContext.textureSize(m_portraitBackground);
      if (!portraitBounds) return std::unexpected(std::move(portraitBounds.error()));
      Vec2F textSize = textBounds->size().floor();
      Vec2F portraitSize = Vec2F(*portraitBounds);
      messageHeight = std::max(portraitSize[1] + m_portraitVerticalMargin, textSize[1] + lineHeightMargin);

      // Draw both image and text anchored left and centered vertically
      auto imagePosition = chatMin + Vec2I(0, std::floor(messageHeight / 2)) - Vec2I(0, std::floor(portraitSize[1] / 2));
      m_chatLog->drawImage(m_portraitBackground, Vec2F(imagePosition), 1.0f, fade);
      m_chatLog->drawImage(message.portrait, Vec2F(imagePosition + m_portraitImageOffset), m_portraitScale, fade);
      tp.pos += Vec2F(0, std::floor(messageHeight / 2));
      m_chatTextStyle.color = fade;
      m_chatLog->drawText(messageString, tp, m_chatTextStyle);

    } else {
      TextPositioning tp = {Vec2F(chatMin), HorizontalAnchor::LeftAnchor, VerticalAnchor::BottomAnchor, wrapWidth};
      auto bounds = guiContext.determineInterfaceTextSize(messageString, tp);
      if (!bounds) return std::unexpected(std::move(bounds.error()));
      messageHeight = bounds->size()[1] + lineHeightMargin;
      m_chatTextStyle.color = fade;
      m_chatLog->drawText(messageString, tp, m_chatTextStyle);
    }

    chatMin[1] += std::ceil(messageHeight);
  }

  auto spacing = guiContext.setDefaultLineSpacing();
  if (!spacing) return std::unexpected(std::move(spacing.error()));
  return guiContext.setDefaultFont();
}
std::expected<void, StarException> Chat::hide() {
  auto stopped = stopChat();
  if (!stopped) return std::unexpected(std::move(stopped.error()));
  return Pane::hide();
}

std::expected<float, StarException> Chat::visible() const {
  if (m_scripted) {
    auto invoked = m_script.invoke<float>("visible");
    if (!invoked) return std::unexpected(std::move(invoked.error()));
    return invoked->value_or(1.0f);
  }
  double difference = (Time::monotonicMilliseconds() - m_timeChatLastActive) / 1000.0;
  if (difference < m_chatVisTime) return 1.0f;
  return clamp<float>(1 - (difference - m_chatVisTime) / m_fadeRate, 0, 1);
}

std::expected<bool, StarException> Chat::sendEvent(InputEvent const& event) {
  if (!m_scripted && active()) {
    auto focused = hasFocus();
    if (!focused) return std::unexpected(std::move(focused.error()));
    if (*focused && std::holds_alternative<KeyDownEvent>(event)) {
      auto actions = context().actions(event);
      if (actions.contains(InterfaceAction::ChatStop)) {
        auto handled = stopChat();
        if (!handled) return std::unexpected(std::move(handled.error()));
        return true;
      }
      if (actions.contains(InterfaceAction::ChatPreviousLine)) {
        auto handled = incrementIndex();
        if (!handled) return std::unexpected(std::move(handled.error()));
        return true;
      }
      if (actions.contains(InterfaceAction::ChatNextLine)) {
        auto handled = decrementIndex();
        if (!handled) return std::unexpected(std::move(handled.error()));
        return true;
      }
      if (actions.contains(InterfaceAction::ChatPageDown)) {
        auto handled = scrollDown();
        if (!handled) return std::unexpected(std::move(handled.error()));
        return true;
      }
      if (actions.contains(InterfaceAction::ChatPageUp)) {
        auto handled = scrollUp();
        if (!handled) return std::unexpected(std::move(handled.error()));
        return true;
      }
    }
    if (std::holds_alternative<MouseWheelEvent>(event)) {
      if (inMember(*context().mousePosition(event))) {
        auto scrolled = std::get<MouseWheelEvent>(event).mouseWheel == MouseWheel::Down ? scrollDown() : scrollUp();
        if (!scrolled) return std::unexpected(std::move(scrolled.error()));
        return true;
      }
    }
    if (std::holds_alternative<MouseMoveEvent>(event) && inMember(*context().mousePosition(event))) m_timeChatLastActive = Time::monotonicMilliseconds();
    if (std::holds_alternative<MouseButtonDownEvent>(event) && m_chatLog->inMember(*context().mousePosition(event))) {
      m_expanded = !m_expanded;
      auto resized = updateSize();
      if (!resized) return std::unexpected(std::move(resized.error()));
      return true;
    }
  }
  return Pane::sendEvent(event);
}

std::expected<void, StarException> Chat::scrollUp() {
  auto shown = std::ranges::count_if(m_receivedMessages, [this](LogMessage const& msg) { return m_modeFilter.empty() || m_modeFilter.contains(msg.mode); });
  m_historyOffset = std::max(0, std::min((int)shown - 1, m_historyOffset + 1));
  m_timeChatLastActive = Time::monotonicMilliseconds();
  return updateBottomButton();
}

std::expected<void, StarException> Chat::scrollDown() {
  m_historyOffset = std::max(0, m_historyOffset - 1);
  m_timeChatLastActive = Time::monotonicMilliseconds();
  return updateBottomButton();
}

std::expected<void, StarException> Chat::scrollBottom() {
  m_historyOffset = 0;
  m_timeChatLastActive = Time::monotonicMilliseconds();
  return updateBottomButton();
}

std::expected<void, StarException> Chat::updateSize() {
  auto height = m_expanded ? m_expandedBodyHeight : m_bodyHeight;
  m_background->setSize(Vec2I(m_background->size()[0], m_defaultHeight + height));
  m_chatLog->setSize(Vec2I(m_chatLog->size()[0], height));
  m_upButton->setPosition(Vec2I(m_upButton->position()[0], m_chatLog->position()[1] + m_chatLog->size()[1] - m_upButton->size()[1]));
  determineSizeFromChildren();
  return {};
}

std::expected<void, StarException> Chat::updateBottomButton() {
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto config = (*assets)->json("/interface/chat/chat.config:bottom");
  if (!config) return std::unexpected(std::move(config.error()));
  auto images = config->get(m_historyOffset == 0 ? String("atbottom") : String("scrolling"));
  if (!images) return std::unexpected(std::move(images.error()));
  auto base = images->getString("base");
  if (!base) return std::unexpected(std::move(base.error()));
  auto hover = images->getString("hover");
  if (!hover) return std::unexpected(std::move(hover.error()));
  return m_bottomButton->setImages(*base, *hover);
}

}

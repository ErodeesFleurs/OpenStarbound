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

Chat::Chat(GuiContext& guiContext, std::shared_ptr<UniverseClient> client, Json const& baseConfig) : BaseScriptPane(guiContext, baseConfig, false) {
  m_client = client;
  m_scripted = baseConfig.get("scripts", Json()).isType(Json::Type::Array);
  m_script.setLuaRoot(m_client->luaRoot());
  m_script.addCallbacks("world", LuaBindings::makeWorldCallbacks(context()->root(), (World*)m_client->worldClient().get()));
  m_chatPrevIndex = 0;
  m_historyOffset = 0;
  
  auto assets = context()->root().assets();
  auto config = baseConfig.get("config");
  m_timeChatLastActive = Time::monotonicMilliseconds();
  m_chatTextStyle = config.get("textStyle");
  m_chatTextStyle.lineSpacing = config.get("lineHeight").toFloat();
  m_chatVisTime = config.get("visTime").toFloat();
  m_fadeRate = config.get("fadeRate").toDouble();
  m_chatHistoryLimit = config.get("chatHistoryLimit").toInt();
  m_chatFormatString = config.getString("chatFormatString");

  m_portraitTextOffset = jsonToVec2I(config.get("portraitTextOffset"));
  m_portraitImageOffset = jsonToVec2I(config.get("portraitImageOffset"));
  m_portraitScale = config.get("portraitScale").toFloat();
  m_portraitVerticalMargin = config.get("portraitVerticalMargin").toFloat();
  m_portraitBackground = config.get("portraitBackground").toString();

  m_bodyHeight = config.get("bodyHeight").toInt();
  m_expandedBodyHeight = config.get("expandedBodyHeight").toInt();

  m_colorCodes[MessageContext::Local] = config.query("colors.local").toString();
  m_colorCodes[MessageContext::Party] = config.query("colors.party").toString();
  m_colorCodes[MessageContext::Broadcast] = config.query("colors.broadcast").toString();
  m_colorCodes[MessageContext::Whisper] = config.query("colors.whisper").toString();
  m_colorCodes[MessageContext::CommandResult] = config.query("colors.commandResult").toString();
  m_colorCodes[MessageContext::RadioMessage] = config.query("colors.radioMessage").toString();
  m_colorCodes[MessageContext::World] = config.query("colors.world").toString();
  if (!m_scripted) {
    m_reader->registerCallback("textBox", [this](Widget*) { startChat(); });
    m_reader->registerCallback("upButton", [this](Widget*) { scrollUp(); });
    m_reader->registerCallback("downButton", [this](Widget*) { scrollDown(); });
    m_reader->registerCallback("bottomButton", [this](Widget*) { scrollBottom(); });

    m_reader->registerCallback("filterGroup", [this](Widget* widget) {
      Json data = as<ButtonWidget>(widget)->data();
      auto filter = data.getArray("filter", {});
      m_modeFilter.clear();
      for (auto mode : filter)
        m_modeFilter.insert(MessageContextModeNames.getLeft(mode.toString()));
      m_sendMode = ChatSendModeNames.getLeft(data.getString("sendMode", "Broadcast"));
      m_historyOffset = 0;
    });
  }

  construct(baseConfig.get("gui"));

  m_sendMode = ChatSendMode::Broadcast;
  m_chatLog = fetchChild<CanvasWidget>("chatLog");
  m_bottomButton = fetchChild<ButtonWidget>("bottomButton");
  m_upButton = fetchChild<ButtonWidget>("upButton");
  m_textBox = fetchChild<TextBoxWidget>("textBox");
  m_say = fetchChild<LabelWidget>("say");
  if (!m_scripted) {
    if (auto logPadding = config.optQuery("padding")) {
      m_chatLogPadding = jsonToVec2I(logPadding.value());
      m_chatLog->setSize(m_chatLog->size() + m_chatLogPadding * 2);
      m_chatLog->setPosition(m_chatLog->position() - m_chatLogPadding);
    } else
      m_chatLogPadding = Vec2I();

    m_chatHistory.appendAll(m_client->playerStorage()->getMetadata("chatHistory").opt().transform(jsonToStringList).value_or(StringList{}));
  } else {
    m_script.addCallbacks("player", LuaBindings::makePlayerCallbacks(context()->root(), m_client->mainPlayer().get()));
    m_script.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(m_client->mainPlayer()->statusController()));
    m_script.addCallbacks("celestial", LuaBindings::makeCelestialCallbacks(context()->root(), m_client.get()));
  }

  show();

  //if (!m_scripted) {
    //updateBottomButton();

    m_background = fetchChild<ImageStretchWidget>("background");
    m_defaultHeight = m_background->size()[1];
    m_expanded = false;
    updateSize();
  //}
}

void Chat::update(float dt) {
  Pane::update(dt);

  if (m_scripted)
    return;

  auto team = m_client->teamClient()->currentTeam();
  for (auto button : fetchChild<ButtonGroup>("filterGroup")->buttons()) {
    auto mode = ChatSendModeNames.getLeft(button->data().getString("sendMode", "Broadcast"));
    if (!team.has_value() && m_sendMode == ChatSendMode::Party && mode == ChatSendMode::Broadcast)
      button->check();
    if (mode == ChatSendMode::Party)
      button->setEnabled(team.has_value());
  }
}

void Chat::startChat() {
  if (m_scripted)
    m_script.invoke("startChat");
  else {
    show();
    m_textBox->focus();
  }
}

void Chat::startCommand() {
  if (m_scripted)
    m_script.invoke("startCommand");
  else {
    show();
    m_textBox->setText("/");
    m_textBox->focus();
  }
}

bool Chat::hasFocus() const {
  if (m_scripted)
    return m_script.invoke<bool>("hasFocus").value_or(false);
  return m_textBox->hasFocus();
}

void Chat::stopChat() {
  if (m_scripted)
    m_script.invoke("stopChat");
  else {
    m_textBox->setText("");
    m_textBox->blur();
    m_timeChatLastActive = Time::monotonicMilliseconds();
  }
}

String Chat::currentChat() const {
  if (m_scripted)
    return m_script.invoke<String>("currentChat").value_or(String{});
  return m_textBox->getText();
}

bool Chat::setCurrentChat(String const& chat, bool moveCursor) {
  if (m_scripted)
    return m_script.invoke<bool>("setCurrentChat").value_or(false);
  return m_textBox->setText(chat, true, moveCursor);
}

void Chat::clearCurrentChat() {
  if (m_scripted)
    m_script.invoke("clearCurrentChat");
  else {
    m_textBox->setText("");
    m_chatPrevIndex = 0;
  }
}

ChatSendMode Chat::sendMode() const {
  if (m_scripted)
    return ChatSendModeNames.getLeft(m_script.invoke<String>("sendMode").value_or(String{}));
  return m_sendMode;
}

void Chat::incrementIndex() {
  if (!m_chatHistory.empty()) {
    m_chatPrevIndex = std::min(m_chatPrevIndex + 1, (unsigned)m_chatHistory.size());
    m_textBox->setText(m_chatHistory.at(m_chatPrevIndex - 1));
  }
}

void Chat::decrementIndex() {
  if (m_chatPrevIndex > 1 && !m_chatHistory.empty()) {
    --m_chatPrevIndex;
    m_textBox->setText(m_chatHistory.at(m_chatPrevIndex - 1));
  } else {
    m_chatPrevIndex = 0;
    m_textBox->setText("");
  }
}

void Chat::addLine(String const& text, bool showPane) {
  ChatReceivedMessage message = {{MessageContext::CommandResult}, ServerConnectionId, "", text};
  addMessages({message}, showPane);
}

void Chat::addMessages(List<ChatReceivedMessage> const& messages, bool showPane) {
  if (messages.empty())
    return;

  if (m_scripted) {
    m_script.invoke("addMessages", messages.transformed([](ChatReceivedMessage const& message) {
      return message.toJson();
    }), showPane);
    return;
  }

  GuiContext& guiContext = *context();

  for (auto const& message : messages) {
    std::optional<unsigned> wrapWidth;
    if (message.portrait.empty())
      wrapWidth = m_chatLog->size()[0] - m_chatLogPadding[0] * 2;

    guiContext.setTextStyle(m_chatTextStyle);
    StringList lines;
    if (message.fromNick != "" && message.portrait == "")
      lines = guiContext.wrapInterfaceText(strf(m_chatFormatString.utf8Ptr(), message.fromNick, message.text), wrapWidth);
    else
      lines = guiContext.wrapInterfaceText(message.text, wrapWidth);

    for (std::size_t i = 0; i < lines.size(); ++i) {
      m_receivedMessages.prepend({
          message.context.mode,
          message.portrait,
          std::move(lines[i])
        });
    }

    if (message.fromNick != "")
      context()->root().logger().info("Chat: <{}> {}", message.fromNick, message.text);
    else
      context()->root().logger().info("Chat: {}", message.text);
  }

  if (showPane) {
    m_timeChatLastActive = Time::monotonicMilliseconds();
    show();
  }

  if ((unsigned)m_receivedMessages.size() > m_chatHistoryLimit)
    m_receivedMessages.resize(m_chatHistoryLimit);
}

void Chat::addHistory(String const& chat) {
  if (m_scripted)
    m_script.invoke("addHistory", chat);
  else if (m_chatHistory.size() > 0 && m_chatHistory.get(0).equals(chat))
    return;
  else {
    m_chatHistory.prepend(chat);
    m_chatHistory.resize(std::min((unsigned)m_chatHistory.size(), m_chatHistoryLimit));
    m_timeChatLastActive = Time::monotonicMilliseconds();
    m_client->playerStorage()->setMetadata("chatHistory", JsonArray::from(m_chatHistory));
  }
}

void Chat::clear(std::size_t count) {
  if (m_scripted)
    m_script.invoke("clear", count);
  else if (count > m_receivedMessages.size())
    m_receivedMessages.clear();
  else
    m_receivedMessages.erase(m_receivedMessages.begin(), m_receivedMessages.begin() + count);
}

void Chat::renderImpl() {
  Pane::renderImpl();
  if (m_scripted)
    return;
  if (m_textBox->hasFocus())
    m_timeChatLastActive = Time::monotonicMilliseconds();
  Vec4B fade = {255, 255, 255, 255};
  fade[3] = (std::uint8_t)(visible() * 255);
  if (!visible()) {
    hide();
    return;
  }

  Color fadeGreen = Color::Green;
  fadeGreen.setAlpha(fade[3]);
  m_say->setColor(fadeGreen);

  m_chatLog->clear();
  Vec2I chatMin = m_chatLogPadding;
  int messageIndex = -m_historyOffset;

  GuiContext& guiContext = *context();
  float lineHeight = m_chatTextStyle.lineSpacing;
  float fontSize = m_chatTextStyle.fontSize;
  guiContext.setTextStyle(m_chatTextStyle);
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
      Vec2F textSize = guiContext.determineInterfaceTextSize(messageString, tp).size().floor();
      Vec2F portraitSize = Vec2F(guiContext.textureSize(m_portraitBackground));
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
      messageHeight = guiContext.determineInterfaceTextSize(messageString, tp).size()[1] + lineHeightMargin;
      m_chatTextStyle.color = fade;
      m_chatLog->drawText(messageString, tp, m_chatTextStyle);
    }

    chatMin[1] += std::ceil(messageHeight);
  }

  guiContext.setDefaultLineSpacing();
  guiContext.setDefaultFont();
}

void Chat::hide() {
  stopChat();
  Pane::hide();
}

float Chat::visible() const {
  if (m_scripted)
    return m_script.invoke<float>("visible").value_or(1.0f);

  double difference = (Time::monotonicMilliseconds() - m_timeChatLastActive) / 1000.0;
  if (difference < m_chatVisTime)
    return 1;
  return clamp<float>(1 - (difference - m_chatVisTime) / m_fadeRate, 0, 1);
}

bool Chat::sendEvent(InputEvent const& event) {
  if (!m_scripted && active()) {
    if (hasFocus()) {
      if (std::holds_alternative<KeyDownEvent>(event)) {
        auto actions = context()->actions(event);
        if (actions.contains(InterfaceAction::ChatStop)) {
          stopChat();
          return true;
        } else if (actions.contains(InterfaceAction::ChatPreviousLine)) {
          incrementIndex();
          return true;
        } else if (actions.contains(InterfaceAction::ChatNextLine)) {
          decrementIndex();
          return true;
        } else if (actions.contains(InterfaceAction::ChatPageDown)) {
          scrollDown();
          return true;
        } else if (actions.contains(InterfaceAction::ChatPageUp)) {
          scrollUp();
          return true;
        }
      }
    }

    if (auto mouseWheel = std::get_if<MouseWheelEvent>(&event)) {
      if (inMember(*context()->mousePosition(event))) {
        if (mouseWheel->mouseWheel == MouseWheel::Down)
          scrollDown();
        else
          scrollUp();
        return true;
      }
    }

    if (std::holds_alternative<MouseMoveEvent>(event) && inMember(*context()->mousePosition(event)))
      m_timeChatLastActive = Time::monotonicMilliseconds();

    if (std::holds_alternative<MouseButtonDownEvent>(event)) {
      if (m_chatLog->inMember(*context()->mousePosition(event))) {
        m_expanded = !m_expanded;
        updateSize();
        return true;
      }
    }
  }

  return Pane::sendEvent(event);
}

void Chat::scrollUp() {
  auto shownMessageCount = std::ranges::count_if(m_receivedMessages, [this](LogMessage const& msg) {
      return (m_modeFilter.empty() || m_modeFilter.contains(msg.mode));
    });

  m_historyOffset = std::max(0, std::min((int)shownMessageCount - 1, m_historyOffset + 1));
  m_timeChatLastActive = Time::monotonicMilliseconds();
  updateBottomButton();
}

void Chat::scrollDown() {
  m_historyOffset = std::max(0, m_historyOffset - 1);
  m_timeChatLastActive = Time::monotonicMilliseconds();
  updateBottomButton();
}

void Chat::scrollBottom() {
  m_historyOffset = 0;
  m_timeChatLastActive = Time::monotonicMilliseconds();
  updateBottomButton();
}

void Chat::updateSize() {
  auto height = m_expanded ? m_expandedBodyHeight : m_bodyHeight;
  m_background->setSize(Vec2I(m_background->size()[0], m_defaultHeight + height));
  m_chatLog->setSize(Vec2I(m_chatLog->size()[0], height));
  m_upButton->setPosition(Vec2I(m_upButton->position()[0], m_chatLog->position()[1] + m_chatLog->size()[1] - m_upButton->size()[1]));
  determineSizeFromChildren();
}

void Chat::updateBottomButton() {
  auto assets = context()->root().assets();
  auto bottomConfig = assets->json("/interface/chat/chat.config:bottom");
  if (m_historyOffset == 0)
    m_bottomButton->setImages(bottomConfig.get("atbottom").getString("base"), bottomConfig.get("atbottom").getString("hover"));
  else
    m_bottomButton->setImages(bottomConfig.get("scrolling").getString("base"), bottomConfig.get("scrolling").getString("hover"));
}

}

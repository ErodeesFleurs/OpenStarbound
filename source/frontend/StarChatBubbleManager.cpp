import std;
import star.xx_hash;
import star.json;
import star.json_extra;
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
import star.game_types;
import star.color;
import star.font;
import star.bi_map;
import star.string;
import star.vector;
import star.text;
import star.format;
import star.version;
import star.casting;
import star.strong_typedef;
import star.maybe;
import star.listener;
import star.input_event;
import star.set;
import star.audio;
import star.map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.image;
import star.math_common;
import star.either;
import star.random;
import star.weighted_pool;
import star.variant;
import star.rpc_promise;
import star.net_compatibility;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.btree;
import star.lru_cache;
import star.data_stream_devices;
import star.btree_database;
import star.ordered_set;

import star.world_geometry;

import star.chat_action;
import star.renderer;
import star.font_texture_group;
import star.anchor_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.root;
import star.text_painter;



import star.chat_bubble_separation;

import star.world_camera;


import star.chat_bubble_manager;
import star.configuration;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
import star.animation;
import star.particle;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
import star.celestial_types;
import star.chat_types;
import star.uuid;
import star.tile_modification;
import star.interaction_types;
import star.warping;
import star.wiring;
import star.quest_descriptor;
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
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.entity_rendering;
import star.world;
import star.lua_components;
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.chatty_entity;
import star.asset_texture_group;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.application_controller;
import star.drawable;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;

import star.stored_functions;

import star.image_metadata_database;

import star.algorithm;
namespace Star {

ChatBubbleManager::ChatBubbleManager(GuiContext& context)
  : m_guiContext(&context), m_textTemplate(Vec2F()), m_portraitTextTemplate(Vec2F()) {
  auto assets = context.root().assets();

  m_cachedInterfaceScale = m_guiContext->interfaceScale();

  auto jsonData = assets->json("/interface/windowconfig/chatbubbles.config");

  m_textStyle.color = jsonToColor(jsonData.get("textColor")).toRgba();
  m_textStyle.loadJson(jsonData.get("textStyle"));
  m_textPadding = jsonToVec2F(jsonData.get("textPadding"));

  m_zoom = jsonData.getInt("textZoom");
  m_bubbleOffset = jsonToVec2F(jsonData.get("bubbleOffset"));
  m_maxAge = jsonData.getFloat("maxAge");
  m_portraitMaxAge = jsonData.getFloat("portraitMaxAge");

  unsigned textWrapWidth = jsonData.getUInt("textWrapWidth");
  m_textTemplate = TextPositioning{Vec2F(), HorizontalAnchor::HMidAnchor, VerticalAnchor::TopAnchor, textWrapWidth * m_zoom};

  m_interBubbleMargin = jsonData.getFloat("interBubbleMargin");

  m_maxMessagePerEntity = jsonData.getInt("maxMessagePerEntity");

  m_bubbles.setTweenFactor(jsonData.getFloat("tweenFactor"));
  m_bubbles.setMovementThreshold(jsonData.getFloat("movementThreshold"));

  m_portraitBackgroundImage = jsonData.getString("portraitBackgroundImage");
  m_portraitMoreImage = jsonData.getString("portraitMoreImage");
  m_portraitMorePosition = jsonToVec2I(jsonData.get("portraitMorePosition"));
  m_portraitBackgroundSize = jsonToVec2I(jsonData.get("portraitBackgroundSize"));
  m_portraitPosition = jsonToVec2I(jsonData.get("portraitPosition"));
  m_portraitSize = jsonToVec2I(jsonData.get("portraitSize"));
  m_portraitTextPosition = jsonToVec2I(jsonData.get("portraitTextPosition"));
  m_portraitTextWidth = jsonData.getUInt("portraitTextWidth");
  m_portraitChatterFramerate = jsonData.getFloat("portraitChatterFramerate");
  m_portraitChatterDuration = jsonData.getFloat("portraitChatterDuration");

  m_portraitTextTemplate = TextPositioning{Vec2F(m_portraitTextPosition), HorizontalAnchor::LeftAnchor, VerticalAnchor::TopAnchor, m_portraitTextWidth * m_zoom};

  // This is a factor(0.0 - 1.0) based on the window size.
  // 0.0 is directly over the player, 1.0 is the edge of the window
  m_furthestVisibleTextDistance = jsonData.getFloat("furthestTextDistance");

  String textFadeFunctionName = jsonData.getString("textFadeFunction");
  m_textFadeFunction = context.root().functionDatabase()->function(textFadeFunctionName);

  String bubbleFadeFunctionName = jsonData.getString("bubbleFadeFunction");
  m_bubbleFadeFunction = context.root().functionDatabase()->function(bubbleFadeFunctionName);
}

void ChatBubbleManager::setCamera(WorldCamera const& camera) {
  float oldPixelRatio = m_camera.pixelRatio();
  float oldInterfaceScale = m_cachedInterfaceScale;
  m_camera = camera;
  m_cachedInterfaceScale = m_guiContext->interfaceScale();
  if (m_camera.pixelRatio() != oldPixelRatio || m_cachedInterfaceScale != oldInterfaceScale) {
    List<ChatAction> actions;
    m_bubbles.forEach([&actions](BubbleState<Bubble> const& state, Bubble& bubble) {
        actions.append(SayChatAction{bubble.entity, bubble.text, state.idealDestination, bubble.config});
      });
    m_bubbles.clear();
    for (auto& portraitBubble : m_portraitBubbles)
      actions.append(PortraitChatAction{
          portraitBubble.entity,
          portraitBubble.portrait,
          portraitBubble.text,
          portraitBubble.position,
          portraitBubble.config
        });
    m_portraitBubbles.clear();
    addChatActions(actions, true);
  }
}

void ChatBubbleManager::update(float dt, std::shared_ptr<WorldClient> world) {
  m_bubbles.forEach([this, dt, &world](BubbleState<Bubble>& bubbleState, Bubble& bubble) {
    bubble.age += dt;
    if (auto entity = world->get<ChattyEntity>(bubble.entity)) {
      bubble.onscreen = m_camera.worldGeometry().rectIntersectsRect(
          m_camera.worldScreenRect(), entity->metaBoundBox().translated(entity->position()));
      bubbleState.idealDestination = m_camera.worldToScreen(entity->mouthPosition() + m_bubbleOffset);
    }
  });

  for (auto& portraitBubble : m_portraitBubbles) {
    portraitBubble.age += dt;
    if (auto entity = world->entity(portraitBubble.entity)) {
      portraitBubble.onscreen = m_camera.worldGeometry().rectIntersectsRect(m_camera.worldScreenRect(), entity->metaBoundBox().translated(entity->position()));
      if (auto chatter = as<ChattyEntity>(entity))
        portraitBubble.position = chatter->mouthPosition();
      else
        portraitBubble.position = entity->position();
    }
  }

  auto& count = m_entityBubbleCounts;
  count.clear();
  filter(m_portraitBubbles, [&](PortraitBubble const& portraitBubble) -> bool {
      count[portraitBubble.entity] += m_maxMessagePerEntity;
      if (count[portraitBubble.entity] > m_maxMessagePerEntity)
        return false;
      if (world->get<ChattyEntity>(portraitBubble.entity))
        return portraitBubble.age < m_portraitMaxAge;
      return false;
    });

  m_bubbles.filter([&](BubbleState<Bubble> const&, Bubble const& bubble) -> bool {
      if (++count[bubble.entity] > m_maxMessagePerEntity)
        return false;
      if (world->get<ChattyEntity>(bubble.entity))
        return bubble.age < m_maxAge;
      return false;
    });

  m_bubbles.update(dt);
}

std::uint8_t ChatBubbleManager::calcDistanceFadeAlpha(Vec2F bubbleScreenPosition, std::shared_ptr<StoredFunction> fadeFunction) const {
  // first calculate bubble position as a factor, distance from center to edge
  // of screen (0.0-1.0)
  float halfScreenwidth = m_camera.screenSize()[0] * 0.5f;
  float distanceFactor = (std::fabsf(bubbleScreenPosition[0] - halfScreenwidth)) / halfScreenwidth;

  // that distance factor is divided by the max allowable distance
  // to re-space the distance as a 0 - 1 over the max allowable distance
  distanceFactor = clamp(distanceFactor / m_furthestVisibleTextDistance, 0.0f, 1.0f);

  int alpha = fadeFunction->evaluate(distanceFactor);
  return clamp(alpha, 0, 255);
}

void ChatBubbleManager::render() {
  if (m_bubbles.empty() && m_portraitBubbles.empty())
    return;
  if (!m_guiContext->root().configuration()->get("speechBubbles").toBool())
    return;

  float pixelRatio = m_guiContext->interfaceScale();

  m_bubbles.forEach([this, pixelRatio](BubbleState<Bubble> const& state, Bubble& bubble) {
      if (bubble.onscreen) {
        int alpha = calcDistanceFadeAlpha(state.currentPosition, m_bubbleFadeFunction);
        if (alpha) {
          for (auto const& bubbleImage : bubble.backgroundImages)
            drawBubbleImage(state.currentPosition, bubbleImage, pixelRatio, alpha);
          for (auto const& bubbleText : bubble.bubbleText)
            drawBubbleText(state.currentPosition, bubbleText, pixelRatio, alpha, false);
        }
      }
    });

  for (auto portraitBubble : m_portraitBubbles) {
    if (portraitBubble.onscreen) {
      Vec2F screenPos = m_camera.worldToScreen(portraitBubble.position + m_bubbleOffset);
      int frame = 0;
      if (portraitBubble.age <= m_portraitChatterDuration)
        frame = int((portraitBubble.age / m_portraitChatterFramerate) * 2) % 2;
      // 255 here because portrait bubbles are always full opacity
      for (auto const& bubbleImage : portraitBubble.backgroundImages)
        drawBubbleImage(screenPos, std::make_tuple(std::get<0>(bubbleImage).replace("<frame>", toString(frame)), std::get<1>(bubbleImage)), pixelRatio, 255);
      // 255 here because portrait bubbles are always full opacity
      for (auto const& bubbleText : portraitBubble.bubbleText)
        drawBubbleText(screenPos, bubbleText, pixelRatio, 255, true);
    }
  }
}

void ChatBubbleManager::addChatActions(List<ChatAction> chatActions, bool silent) {
  auto assets = m_guiContext->root().assets();
  auto config = assets->json("/interface/windowconfig/chatbubbles.config");

  float partSize = config.getFloat("partSize");
  float pixelRatio = m_guiContext->interfaceScale();

  for (auto action : chatActions) {
    Json config = JsonObject{};
    Vec2F position;

    if (action.is<SayChatAction>()) {
      auto sayAction = action.get<SayChatAction>();
      config = sayAction.config.optObject().value(JsonObject{});
      position = sayAction.position;

      // TODO: Get rid of this stupid fucking bullshit, this is the ugliest
      // fragilest pointlessest horseshit code in the codebase.  It wouldn't
      // bother me so bad if it weren't so fucking easy to do right.

      // yea I agree
      m_guiContext->setTextStyle(m_textStyle, pixelRatio);
      TextPositioning tp = m_textTemplate;
      tp.wrapWidth = tp.wrapWidth.apply([&](unsigned w) { return (unsigned)(w * pixelRatio / m_zoom); });
      auto result = m_guiContext->determineTextSize(sayAction.text, tp);
      float textWidth = result.width() / pixelRatio + m_textPadding[0];
      float textHeight = result.height() / pixelRatio + m_textPadding[1];

      Vec2I innerTiles = Vec2I::ceil(Vec2F((textWidth + 4) / partSize, (textHeight + 4) / partSize));
      if (innerTiles[0] % 2 == 0)
        innerTiles[0] += 1;
      if (innerTiles[0] < 3)
        innerTiles[0] = 3;
      int middleIdx = (innerTiles[0] - 1) / 2;

      List<BubbleImage> backgroundImages;
      if (config.getBool("drawBorder", true)) {
        for (int y = 0; y < innerTiles[1]; y++) {
          for (int x = 0; x < innerTiles[0]; x++) {
            auto partPosition = [partSize](int x, int y) {
              return Vec2F(x * partSize, y * partSize);
            };
            if (y == 0) {
              if (x == 0) {
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/cornerBottomLeft.png", partPosition(x, y)));
              } else if (x == innerTiles[0] - 1) {
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/cornerBottomRight.png", partPosition(x, y)));
              } else {
                if (middleIdx == x)
                  backgroundImages.append(std::make_tuple("/interface/chatbubbles/point.png", partPosition(x, y - 1)));
                else
                  backgroundImages.append(std::make_tuple("/interface/chatbubbles/sideDown.png", partPosition(x, y)));
              }
            } else if (y == innerTiles[1] - 1) {
              if (x == 0)
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/cornerTopLeft.png", partPosition(x, y)));
              else if (x == innerTiles[0] - 1)
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/cornerTopRight.png", partPosition(x, y)));
              else
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/sideUp.png", partPosition(x, y)));
            } else {
              if (x == 0)
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/sideLeft.png", partPosition(x, y)));
              else if (x == innerTiles[0] - 1)
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/sideRight.png", partPosition(x, y)));
              else
                backgroundImages.append(std::make_tuple("/interface/chatbubbles/center.png", partPosition(x, y)));
            }
          }
        }
      }

      float textMultiLineShift = textHeight;
      float horizontalCenter = partSize * innerTiles[0] * 0.5f;
      float verticalShift = (partSize * innerTiles[1] - textMultiLineShift) * 0.5f + textMultiLineShift - 1.0f;
      Vec2F position = Vec2F(horizontalCenter, verticalShift);
      List<BubbleText> bubbleTexts;
      TextStyle textStyle = m_textStyle;
      textStyle.fontSize = config.getUInt("fontSize", textStyle.fontSize);
      if (auto jColor = config.opt("color"))
        textStyle.color = jsonToColor(*jColor).toRgba();
      textStyle.loadJson(config.get("style", Json()));
      bubbleTexts.append(std::make_tuple(sayAction.text, textStyle, true, position));

      for (auto& backgroundImage : backgroundImages)
        std::get<1>(backgroundImage) += Vec2F(-horizontalCenter, partSize);
      for (auto& bubbleText : bubbleTexts)
        std::get<3>(bubbleText) += Vec2F(-horizontalCenter, partSize);

      auto pos = m_camera.worldToScreen(sayAction.position + m_bubbleOffset);
      RectF boundBox = std::ranges::fold_left(backgroundImages, RectF::null(), [pos, pixelRatio, this](RectF const& boundBox, BubbleImage const& bubbleImage) {
          return boundBox.combined(bubbleImageRect(pos, bubbleImage, pixelRatio));
        });
      Bubble bubble = {sayAction.entity, sayAction.text, sayAction.config, 0, std::move(backgroundImages), std::move(bubbleTexts), false};
      List<BubbleState<Bubble>> oldBubbles = m_bubbles.filtered([&sayAction](BubbleState<Bubble> const&, Bubble const& bubble) {
          return bubble.entity == sayAction.entity;
        });
      m_bubbles.filter([&sayAction](BubbleState<Bubble> const&, Bubble const& bubble) { return bubble.entity != sayAction.entity; });
      m_bubbles.addBubble(pos, boundBox, std::move(bubble), m_interBubbleMargin * pixelRatio);
      oldBubbles.sort([](BubbleState<Bubble> const& a, BubbleState<Bubble> const& b) { return a.contents.age < b.contents.age; });
      for (auto& bubble : oldBubbles.slice(0, m_maxMessagePerEntity - 1))
        m_bubbles.addBubble(bubble.idealDestination, bubble.boundBox, bubble.contents, 0);

    } else if (action.is<PortraitChatAction>()) {
      auto portraitAction = action.get<PortraitChatAction>();
      config = portraitAction.config.optObject().value(JsonObject{});
      position = portraitAction.position;

      List<BubbleImage> backgroundImages;
      backgroundImages.append(std::make_tuple(m_portraitBackgroundImage, Vec2F()));
      if (config.getBool("drawMoreIndicator", false))
        backgroundImages.append(std::make_tuple(m_portraitMoreImage, Vec2F(m_portraitMorePosition)));
      backgroundImages.append(std::make_tuple(portraitAction.portrait, Vec2F(m_portraitPosition)));
      List<BubbleText> bubbleTexts;
      bubbleTexts.append(std::make_tuple(portraitAction.text, m_textStyle, false, Vec2F(m_portraitTextPosition)));

      for (auto& backgroundImage : backgroundImages)
        std::get<1>(backgroundImage) += Vec2F(-m_portraitBackgroundSize[0] / 2, 0);
      for (auto& bubbleText : bubbleTexts)
        std::get<3>(bubbleText) += Vec2F(-m_portraitBackgroundSize[0] / 2, 0);

      m_portraitBubbles.prepend({
          portraitAction.entity,
          portraitAction.portrait,
          portraitAction.text,
          portraitAction.position,
          portraitAction.config,
          0,
          std::move(backgroundImages),
          std::move(bubbleTexts),
          false
        });
    }

    if (!silent) {
      if (auto sound = config.optString("sound")) {
        auto assets = m_guiContext->root().assets();
        std::shared_ptr<AudioInstance> audioInstance = std::make_shared<AudioInstance>(*assets->audio(*sound));
        audioInstance->setPosition(position);
        audioInstance->setVolume(config.getFloat("volume", 1.0f));
        audioInstance->setPitchMultiplier(config.getFloat("pitch", 1.0f));
        m_guiContext->playAudio(audioInstance);
      }
    }
  }
}

RectF ChatBubbleManager::bubbleImageRect(Vec2F screenPos, BubbleImage const& bubbleImage, float pixelRatio) {
  auto imgMetadata = m_guiContext->root().imageMetadataDatabase();
  auto& image = std::get<0>(bubbleImage);
  return RectF::withSize(screenPos + std::get<1>(bubbleImage) * pixelRatio, Vec2F(imgMetadata->imageSize(image)) * pixelRatio);
}

void ChatBubbleManager::drawBubbleImage(Vec2F screenPos, BubbleImage const& bubbleImage, float pixelRatio, int alpha) {
  auto& image = std::get<0>(bubbleImage);
  auto offset = std::get<1>(bubbleImage) * pixelRatio;
  m_guiContext->drawQuad(image, screenPos + offset, pixelRatio, {255, 255, 255, alpha});
}

void ChatBubbleManager::drawBubbleText(Vec2F screenPos, BubbleText const& bubbleText, float pixelRatio, int alpha, bool isPortrait) {
  TextStyle style = std::get<1>(bubbleText);
  style.color[3] *= ((float)alpha / 255.f);
  m_guiContext->setTextStyle(style, pixelRatio);
  auto offset = std::get<3>(bubbleText) * pixelRatio;
  TextPositioning tp = isPortrait ? m_portraitTextTemplate : m_textTemplate;
  tp.pos = screenPos + offset;
  tp.wrapWidth = tp.wrapWidth.apply([&](unsigned w) { return (unsigned)(w * pixelRatio / m_zoom); });
  m_guiContext->renderText(std::get<0>(bubbleText), tp);
}

}

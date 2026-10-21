import std;
import star.json;
import star.json_extra;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.vector;
import star.casting;
import star.input_event;
import star.color;
import star.font;
import star.bi_map;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.version;
import star.string;
import star.text;
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
import star.either;
import star.random;
import star.periodic_function;

import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.image;
import star.application_controller;
import star.renderer;
import star.lua_root;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.uuid;
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
import star.portrait_widget;


import star.image_metadata_database;

namespace Star {

PortraitWidget::PortraitWidget(GuiContext& context, std::shared_ptr<PortraitEntity> entity, PortraitMode mode) : Widget(context), m_entity(entity), m_portraitMode(mode) {
  m_scale = 1;
  m_renderHumanoid = false;
  m_iconMode = false;

  init();
}

PortraitWidget::PortraitWidget(GuiContext& context) : Widget(context) {
  m_entity = {};
  m_portraitMode = PortraitMode::Full;
  m_scale = 1;
  m_renderHumanoid = false;
  m_iconMode = false;

  init();
}

RectI PortraitWidget::getScissorRect() const {
  return noScissor();
}

void PortraitWidget::renderImpl() {
  auto imgMetadata = this->context()->root().imageMetadataDatabase();

  Vec2I offset = Vec2I();
  if (m_iconMode) {
    auto imgSize = Vec2F(imgMetadata->imageSize(m_iconImage));
    offset = Vec2I(m_scale * imgSize / 2);
    offset += m_iconOffset;
    context()->drawInterfaceQuad(m_iconImage, Vec2F(screenPosition()), m_scale);
  }
  if (m_entity) {
    std::shared_ptr<Humanoid> humanoid = nullptr;
    if (m_renderHumanoid) {
      if (auto player = as<Player>(m_entity))
        if (!player->isPermaDead())
          humanoid = player->humanoid();
    }

    List<Drawable> portrait = humanoid ? humanoid->render(false, false) : m_entity->portrait(m_portraitMode);
    for (auto& i : portrait) {
      i.scale(humanoid ? m_scale * 8.0f : m_scale);
      context()->drawInterfaceDrawable(i, Vec2F(screenPosition() + offset));
    }
  } else {
    if (m_portraitMode == PortraitMode::Bust || m_portraitMode == PortraitMode::Head) {
      Vec2I pos = offset;
      auto imgSize = Vec2F(imgMetadata->imageSize(m_noEntityImagePart));
      pos -= Vec2I(m_scale * imgSize * 0.5);
      context()->drawInterfaceQuad(m_noEntityImagePart, Vec2F(screenPosition() + pos), m_scale);
    } else {
      Vec2I pos = offset;
      auto imgSize = Vec2F(imgMetadata->imageSize(m_noEntityImageFull));
      pos -= Vec2I(m_scale * imgSize * 0.5);
      context()->drawInterfaceQuad(m_noEntityImageFull, Vec2F(screenPosition() + pos), m_scale);
    }
  }
}

void PortraitWidget::init() {
  auto assets = this->context()->root().assets();

  m_noEntityImageFull = assets->json("/interface.config:portraitNullPlayerImageFull").toString();
  m_noEntityImagePart = assets->json("/interface.config:portraitNullPlayerImagePart").toString();
  m_iconImage = assets->json("/interface.config:portraitIconImage").toString();
  m_iconOffset = jsonToVec2I(assets->json("/interface.config:portraitIconOffset"));

  updateSize();
}

void PortraitWidget::setEntity(std::shared_ptr<PortraitEntity> entity) {
  m_entity = entity;
  updateSize();
}

void PortraitWidget::setMode(PortraitMode mode) {
  m_portraitMode = mode;
  updateSize();
}

void PortraitWidget::setScale(float scale) {
  m_scale = scale;
  updateSize();
}

void PortraitWidget::setIconMode() {
  m_iconMode = true;
  updateSize();
}

void PortraitWidget::setRenderHumanoid(bool renderHumanoid) {
  m_renderHumanoid = renderHumanoid;
}

bool PortraitWidget::sendEvent(InputEvent const&) {
  return false;
}

void PortraitWidget::updateSize() {
  auto imgMetadata = this->context()->root().imageMetadataDatabase();

  if (m_iconMode) {
    setSize(Vec2I(imgMetadata->imageSize(m_iconImage) * m_scale));
  } else {
    if (m_entity) {
      setSize(Vec2I(
          (Drawable::boundBoxAll(*this->context()->root().imageMetadataDatabase(), m_entity->portrait(m_portraitMode), false)
                  .size()
              * TilePixels
              * m_scale)
              .ceil()));
    } else {
      if (m_portraitMode == PortraitMode::Bust || m_portraitMode == PortraitMode::Head)
        setSize(Vec2I(imgMetadata->imageSize(m_iconImage) * m_scale));
      else
        setSize(Vec2I(imgMetadata->imageSize(m_noEntityImageFull) * m_scale));
    }
  }
}

}

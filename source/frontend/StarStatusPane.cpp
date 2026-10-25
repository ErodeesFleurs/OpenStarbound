import std;
import star.format;
import star.xx_hash;
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
import star.ordered_map;
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

import star.game_timers;
import star.pane_manager;
import star.registered_pane_manager;


import star.main_interface_types;


import star.status_pane;
import star.host_address;
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.chat_types;
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
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;

import star.simple_tooltip;
import star.status_effect_database;
import star.image_metadata_database;

namespace Star {

StatusPane::StatusPane(GuiContext& guiContext, std::shared_ptr<UniverseClient> client)
  : Pane(guiContext), m_client(std::move(client)), m_player(m_client->mainPlayer()) {}

std::expected<std::shared_ptr<StatusPane>, StarException> StatusPane::create(GuiContext& guiContext, std::shared_ptr<UniverseClient> client) {
  auto pane = std::shared_ptr<StatusPane>(new StatusPane(guiContext, std::move(client)));
  auto base = pane->initializePane();
  if (!base) return std::unexpected(std::move(base.error()));
  auto assets = guiContext.root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto layout = (*assets)->json("/interface/windowconfig/statuspane.config:paneLayout");
  if (!layout) return std::unexpected(std::move(layout.error()));
  GuiReader reader(guiContext);
  auto constructed = reader.construct(*layout, *pane);
  if (!constructed) return std::unexpected(std::move(constructed.error()));
  pane->disableScissoring();
  return pane;
}

std::expected<std::shared_ptr<Pane>, StarException> StatusPane::createTooltip(Vec2I const& screenPosition) {
  auto interfaceScale = context().interfaceScale();
  for (auto const& indicator : m_statusIndicators) {
    if (indicator.screenRect.contains(Vec2F(screenPosition * interfaceScale))) {
      if (!indicator.label.empty())
        return SimpleTooltipBuilder::buildTooltip(context(), indicator.label);
    }
  }
  return std::shared_ptr<Pane>{};
}

std::expected<void, StarException> StatusPane::renderImpl() {
  auto rendered = Pane::renderImpl();
  if (!rendered) return std::unexpected(std::move(rendered.error()));
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto interfaceScale = context().interfaceScale();
  auto imageMetadataDatabase = context().root().imageMetadataDatabase();
  if (!imageMetadataDatabase) return std::unexpected(std::move(imageMetadataDatabase.error()));
  auto darkenConfig = (*assets)->json("/interface.config:statusIconDarkenImage");
  if (!darkenConfig) return std::unexpected(std::move(darkenConfig.error()));
  auto statusIconDarkenImage = darkenConfig->toString();
  if (!statusIconDarkenImage) return std::unexpected(std::move(statusIconDarkenImage.error()));
  for (auto const& entry : m_statusIndicators) {
    String image = entry.icon;
    if (entry.durationPercentage) {
      auto imageSize = (*imageMetadataDatabase)->imageSize(image);
      if (!imageSize) return std::unexpected(std::move(imageSize.error()));
      int imageHeight = (*imageSize)[1];
      int yOffset = -(int)(*entry.durationPercentage * imageHeight);
      image += "?" + imageOperationToString(BlendImageOperation{
                         BlendImageOperation::Multiply, {*statusIconDarkenImage}, Vec2I(0, yOffset)});
    }
    auto drawn = context().drawQuad(image, entry.screenRect.min(), interfaceScale);
    if (!drawn) return std::unexpected(std::move(drawn.error()));
  }
  return {};
}

std::expected<void, StarException> StatusPane::update(float dt) {
  auto updated = Pane::update(dt);
  if (!updated) return std::unexpected(std::move(updated.error()));
  auto assets = context().root().assets();
  if (!assets) return std::unexpected(std::move(assets.error()));
  auto interfaceScale = context().interfaceScale();
  auto height = windowHeight();
  if (!height) return std::unexpected(std::move(height.error()));
  int roundWindowHeight = std::ceil(*height / interfaceScale) * interfaceScale;
  auto imageMetadataDatabase = context().root().imageMetadataDatabase();
  if (!imageMetadataDatabase) return std::unexpected(std::move(imageMetadataDatabase.error()));
  auto statusEffectDatabase = context().root().statusEffectDatabase();
  if (!statusEffectDatabase) return std::unexpected(std::move(statusEffectDatabase.error()));
  auto offsetConfig = (*assets)->json("/interface.config:statusIconPos");
  if (!offsetConfig) return std::unexpected(std::move(offsetConfig.error()));
  auto statusIconOffset = jsonToVec2I(*offsetConfig);
  if (!statusIconOffset) return std::unexpected(std::move(statusIconOffset.error()));
  Vec2I statusIconPos = Vec2I((*statusIconOffset)[0] * interfaceScale, roundWindowHeight - (*statusIconOffset)[1] * interfaceScale);
  auto shiftConfig = (*assets)->json("/interface.config:statusIconShift");
  if (!shiftConfig) return std::unexpected(std::move(shiftConfig.error()));
  auto shift = jsonToVec2I(*shiftConfig);
  if (!shift) return std::unexpected(std::move(shift.error()));
  Vec2I statusIconShift = *shift * interfaceScale;
  RectF boundRect = RectF::null();
  auto summary = m_player->activeUniqueStatusEffectSummary();
  if (!summary) return std::unexpected(std::move(summary.error()));
  m_statusIndicators.clear();
  for (auto const& pair : *summary) {
    auto effectConfig = (*statusEffectDatabase)->uniqueEffectConfig(pair.first);
    if (!effectConfig) return std::unexpected(std::move(effectConfig.error()));
    if (effectConfig->icon) {
      auto imageSize = (*imageMetadataDatabase)->imageSize(*effectConfig->icon);
      if (!imageSize) return std::unexpected(std::move(imageSize.error()));
      RectF rect = RectF::withSize(Vec2F(statusIconPos), Vec2F(*imageSize) * interfaceScale);
      boundRect.combine(rect);
      m_statusIndicators.append(StatusEffectIndicator{*effectConfig->icon, pair.second, effectConfig->label, rect});
      statusIconPos += statusIconShift;
    }
  }
  setPosition(Vec2I::round(boundRect.min() / interfaceScale));
  setSize(Vec2I::round(boundRect.size() / interfaceScale));
  return {};
}

}

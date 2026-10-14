import star.xx_hash;
import star.json;
import star.game_types;
import star.id_map;
import star.image;
#include "StarConfig.hpp"
import star.version;
import star.either;
import star.random;
import star.weighted_pool;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
import star.variant;
import star.rpc_promise;
import star.ordered_map;
import star.net_compatibility;
import std;
import star.set;
import star.vector;
import star.thread;
import star.worker_pool;

import star.sector_array_2d;
import star.bi_map;
import star.perlin;
#include "StarBTree.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
import star.ordered_set;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.color;
import star.font;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.input_event;
import star.audio;
import star.map;
import star.byte_array;

typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef ZSTD_DCtx ZSTD_DStream;
typedef ZSTD_CCtx ZSTD_CStream;
import star.zstd_compression;

#include "StarLuaRoot.hpp"
import star.world_geometry;

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.tile_sector_array;
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
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
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
import star.chat_action;
import star.entity_rendering;
import star.world;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;

import star.world_camera;


import star.quest_indicator_painter;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.application;
import star.statistics_service;
import star.p2p_networking_service;
import star.user_generated_content_service;
import star.desktop_service;
import star.application_controller;
import star.renderer;
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
import star.host_address;
import star.ai_types;
import star.socket;
import star.tcp;
import star.net_packet_socket;
import star.universe_connection;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;

import star.animation;
import star.quest_manager;

namespace Star {

QuestIndicatorPainter::QuestIndicatorPainter(UniverseClientPtr const& client) {
  m_client = client;
}

AnimationPtr indicatorAnimation(String indicatorPath) {
  auto assets = Root::singleton().assets();
  return make_shared<Animation>(assets->json(indicatorPath), indicatorPath);
}

void QuestIndicatorPainter::update(float dt, WorldClientPtr const& world, WorldCamera const& camera) {
  m_camera = camera;

  Set<EntityId> foundIndicators;
  for (auto const& entity : world->query<Entity>(camera.worldScreenRect())) {
    auto indicator = m_client->questManager()->getQuestIndicator(entity);
    if (!indicator) continue;

    foundIndicators.insert(entity->entityId());
    Vec2F screenPos = camera.worldToScreen(indicator->worldPosition);

    if (auto currentIndicator = m_indicators.ptr(entity->entityId())) {
      currentIndicator->screenPos = screenPos;
      if (currentIndicator->indicatorName == indicator->indicatorImage) {
        currentIndicator->animation->update(dt);
      } else {
        currentIndicator->indicatorName = indicator->indicatorImage;
        currentIndicator->animation = indicatorAnimation(indicator->indicatorImage);
      }
    } else {
      m_indicators[entity->entityId()] = Indicator {
          entity->entityId(),
          screenPos,
          indicator->indicatorImage,
          indicatorAnimation(indicator->indicatorImage)
        };
    }
  }

  m_indicators = Map<EntityId, Indicator>::from(m_indicators.pairs().filtered([&foundIndicators](pair<EntityId, Indicator> indicator) {
      return foundIndicators.contains(indicator.first);
    }));
}

Drawable QuestIndicatorPainter::Indicator::render(float pixelRatio) const {
  return animation->drawable(pixelRatio);
}

void QuestIndicatorPainter::render() {
  auto& context = GuiContext::singleton();

  for (auto const& indicator : m_indicators.values()) {
    Drawable drawable = indicator.render(m_camera.pixelRatio());
    drawable.fullbright = true;
    context.drawDrawable(drawable, Vec2F(indicator.screenPos), 1, Vec4B(255, 255, 255, 255));
  }
}

}

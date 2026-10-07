#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarIdMap.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"
#include "StarRect.hpp"
#include "StarList.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
#include "StarVector.hpp"
import star.text;
#include "StarCasting.hpp"
#include "StarStrongTypedef.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarInputEvent.hpp"
#include "StarThread.hpp"
#include "StarVector.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarMap.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarConfig.hpp"
import star.version;
#include "StarEither.hpp"
#include "StarRandom.hpp"
import star.weighted_pool;
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarOrderedMap.hpp"
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
import star.perlin;
#include "StarBTree.hpp"
#include "StarBlockAllocator.hpp"
import star.lru_cache;
#include "StarDataStreamDevices.hpp"
import star.btree_database;
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarAStar.hpp"
import star.periodic_function;
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"

#include "StarLuaRoot.hpp"
import star.world_geometry;

import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
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



import star.chat_bubble_separation;
import star.renderer;
import star.font_texture_group;
import star.anchor_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.text_painter;

import star.world_camera;


import star.nameplate_painter;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.nametag_entity;
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
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.actor_movement_controller;
import star.ai_types;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.application;
#include "StarStatisticsService.hpp"
#include "StarP2PNetworkingService.hpp"
#include "StarUserGeneratedContentService.hpp"
#include "StarDesktopService.hpp"
import star.application_controller;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;

namespace Star {

NameplatePainter::NameplatePainter() {
  auto assets = Root::singleton().assets();

  Json nametagConfig = assets->json("/interface.config:nametag");
  m_showMasterNames = nametagConfig.getBool("showMasterNames");
  m_opacityRate = nametagConfig.getFloat("opacityRate");
  m_inspectOpacityRate = nametagConfig.queryFloat("inspectOpacityRate", m_opacityRate);
  m_offset = jsonToVec2F(nametagConfig.get("offset"));
  Json textStyle = nametagConfig.get("textStyle");
  m_textStyle = textStyle;
  m_statusTextStyle = nametagConfig.get("statusTextStyle", textStyle);
  m_statusOffset = jsonToVec2F(nametagConfig.get("statusOffset"));
  m_statusTextStyle.color = jsonToColor(nametagConfig.get("statusColor")).toRgba();
  m_opacityBoost = nametagConfig.getFloat("opacityBoost");
  m_nametags.setTweenFactor(nametagConfig.getFloat("tweenFactor"));
  m_nametags.setMovementThreshold(nametagConfig.getFloat("movementThreshold"));
}

void NameplatePainter::update(float dt, WorldClientPtr const& world, WorldCamera const& camera, bool inspectionMode) {
  m_camera = camera;

  Set<EntityId> foundEntities;
  for (auto const& entity : world->query<NametagEntity>(camera.worldScreenRect())) {
    if ((entity->isMaster() && !m_showMasterNames) || !entity->displayNametag())
      continue;
    if (auto player = as<Player>(entity)) {
      if (player->isTeleporting())
        continue;
    }
    foundEntities.insert(entity->entityId());

    if (!m_entitiesWithNametags.contains(entity->entityId())) {
      Nametag nametag = {entity->nametag(), entity->statusText(), entity->nametagColor(), 1.0f, entity->entityId()};
      RectF boundBox = determineBoundBox(Vec2F(), nametag);
      m_nametags.addBubble(Vec2F(), boundBox, std::move(nametag));
    }
  }

  m_nametags.filter([&foundEntities](
      BubbleState<Nametag> const&, Nametag const& nametag) { return foundEntities.contains(nametag.entityId); });

  m_nametags.forEach([&world, &camera, this, inspectionMode](BubbleState<Nametag>& bubbleState, Nametag& nametag) {
    if (auto entity = as<NametagEntity>(world->entity(nametag.entityId))) {
      bubbleState.idealDestination = camera.worldToScreen(entity->nametagOrigin()) + m_offset * camera.pixelRatio();
      bubbleState.boundBox = determineBoundBox(bubbleState.idealDestination, nametag);

      nametag.statusText = entity->statusText();
      nametag.name = entity->nametag();
      nametag.color = entity->nametagColor();
      bool fullyOnScreen = world->geometry().rectContains(camera.worldScreenRect(), entity->position());
      if (inspectionMode)
        nametag.opacity = approach(1.0f, nametag.opacity, m_inspectOpacityRate);
      else if (fullyOnScreen)
        nametag.opacity = approach(0.0f, nametag.opacity, m_opacityRate);
      else
        nametag.opacity = approach(m_opacityBoost, nametag.opacity, m_opacityRate);
    }
  });

  m_entitiesWithNametags = std::move(foundEntities);
  m_nametags.update(dt);
}

void NameplatePainter::render() {
  auto& context = GuiContext::singleton();

  m_nametags.forEach([&context, this](BubbleState<Nametag> const& bubble, Nametag const& nametag) {
    if (nametag.opacity == 0.0f)
      return;

    auto& setStyle = context.setTextStyle(m_textStyle);
    auto color = Color::rgb(nametag.color);
    color.setAlphaF(nametag.opacity);
    setStyle.color = color.toRgba();

    context.renderText(nametag.name, namePosition(bubble.currentPosition));

    if (nametag.statusText) {
      context.setTextStyle(m_statusTextStyle).color[3] *= nametag.opacity;
      context.renderText(*nametag.statusText, statusPosition(bubble.currentPosition));
    }
    context.clearTextStyle();
  });
}

TextPositioning NameplatePainter::namePosition(Vec2F bubblePosition) const {
  return TextPositioning(bubblePosition, HorizontalAnchor::HMidAnchor, VerticalAnchor::BottomAnchor);
}

TextPositioning NameplatePainter::statusPosition(Vec2F bubblePosition) const {
  auto& context = GuiContext::singleton();
  return TextPositioning(
      bubblePosition + m_statusOffset * context.interfaceScale(),
      HorizontalAnchor::HMidAnchor, VerticalAnchor::BottomAnchor);
}

RectF NameplatePainter::determineBoundBox(Vec2F bubblePosition, Nametag const& nametag) const {
  auto& context = GuiContext::singleton();
  context.setTextStyle(m_textStyle);
  RectF nametagBox = context.determineTextSize(nametag.name, namePosition(bubblePosition));
  if (nametag.statusText) {
    context.setTextStyle(m_statusTextStyle);
    nametagBox.combine(context.determineTextSize(*nametag.statusText, statusPosition(bubblePosition)));
  }
  context.clearTextStyle();
  return nametagBox;
}

}

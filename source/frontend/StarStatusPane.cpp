#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarXXHash.hpp"
#include "StarAssetPath.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRect.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarEither.hpp"
#include "StarVariant.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarOrderedSet.hpp"
#include "StarZSTDCompression.hpp"
#include "StarNetCompatibility.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
#include "StarIdMap.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarRpcPromise.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarImage.hpp"
#include "StarInterpolation.hpp"
#include "StarAStar.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarImageProcessing.hpp"


#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
#include "StarLuaRoot.hpp"
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
#include "StarP2PNetworkingService.hpp"
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
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
#include "StarLuaComponents.hpp"
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
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;

import star.simple_tooltip;
import star.status_effect_database;
import star.image_metadata_database;

namespace Star {

StatusPane::StatusPane(MainInterfacePaneManager* paneManager, UniverseClientPtr client) {
  m_paneManager = paneManager;
  m_client = client;
  m_player = m_client->mainPlayer();

  m_guiContext = GuiContext::singletonPtr();
  auto assets = Root::singleton().assets();

  GuiReader reader;
  reader.construct(assets->json("/interface/windowconfig/statuspane.config:paneLayout"), this);
  disableScissoring();
}

PanePtr StatusPane::createTooltip(Vec2I const& screenPosition) {
  auto interfaceScale = m_guiContext->interfaceScale();
  for (auto const& indicator : m_statusIndicators) {
    if (indicator.screenRect.contains(Vec2F(screenPosition * interfaceScale))) {
      if (!indicator.label.empty())
        return SimpleTooltipBuilder::buildTooltip(indicator.label);
    }
  }
  return {};
}

void StatusPane::renderImpl() {
  Pane::renderImpl();

  auto assets = Root::singleton().assets();
  auto interfaceScale = m_guiContext->interfaceScale();
  auto imageMetadataDatabase = Root::singleton().imageMetadataDatabase();

  String statusIconDarkenImage = assets->json("/interface.config:statusIconDarkenImage").toString();

  for (auto const& entry : m_statusIndicators) {
    String image = entry.icon;
    if (entry.durationPercentage) {
      int imageHeight = imageMetadataDatabase->imageSize(image)[1];
      int yOffset = -(int)(*entry.durationPercentage * imageHeight);
      image += "?" + imageOperationToString(BlendImageOperation{
                         BlendImageOperation::Multiply, {statusIconDarkenImage}, Vec2I(0, yOffset)});
    }
    m_guiContext->drawQuad(image, entry.screenRect.min(), interfaceScale);
  }
}

void StatusPane::update(float dt) {
  Pane::update(dt);

  auto assets = Root::singleton().assets();
  auto interfaceScale = m_guiContext->interfaceScale();
  int roundWindowHeight = ceil(windowHeight() / interfaceScale) * interfaceScale;

  auto imageMetadataDatabase = Root::singleton().imageMetadataDatabase();
  auto statusEffectDatabase = Root::singleton().statusEffectDatabase();

  Vec2I statusIconOffset = jsonToVec2I(assets->json("/interface.config:statusIconPos"));
  Vec2I statusIconPos = Vec2I(statusIconOffset[0] * interfaceScale, roundWindowHeight - statusIconOffset[1] * interfaceScale);
  Vec2I statusIconShift = jsonToVec2I(assets->json("/interface.config:statusIconShift")) * interfaceScale;

  RectF boundRect = RectF::null();

  m_statusIndicators.clear();
  for (auto const& pair : m_player->activeUniqueStatusEffectSummary()) {
    auto effectConfig = statusEffectDatabase->uniqueEffectConfig(pair.first);
    if (effectConfig.icon) {
      RectF rect = RectF::withSize(Vec2F(statusIconPos), Vec2F(imageMetadataDatabase->imageSize(*effectConfig.icon)) * interfaceScale);
      boundRect.combine(rect);
      m_statusIndicators.append(StatusEffectIndicator{*effectConfig.icon, pair.second, effectConfig.label, rect});
      statusIconPos += statusIconShift;
    }
  }

  setPosition(Vec2I::round(boundRect.min() / interfaceScale));
  setSize(Vec2I::round(boundRect.size() / interfaceScale));
}

}

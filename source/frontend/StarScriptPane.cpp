#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarXXHash.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
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
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarIdMap.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarNetElementSystem.hpp"
#include "StarEither.hpp"
#include "StarWeightedPool.hpp"
#include "StarStrongTypedef.hpp"
#include "StarInterpolation.hpp"
#include "StarVariant.hpp"
#include "StarRpcPromise.hpp"
#include "StarArray.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarAStar.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"
#include "StarZSTDCompression.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
#include "StarLua.hpp"



#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
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
#include "StarLuaComponents.hpp"
import star.widget_parsing;
import star.gui_reader;


import star.base_script_pane;


import star.script_pane;
#include "StarLuaGameConverters.hpp"
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
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
import star.world_geometry;
import star.wiring;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
#include "StarLuaRoot.hpp"
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
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.game_timers;
import star.world_client;
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
import star.ai_types;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.host_address;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.net_packet_socket;
import star.universe_connection;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.widget_lua_bindings;
import star.canvas_widget;
import star.item;
import star.progress_widget;
import star.animation;
import star.item_slot_widget;
import star.item_grid_widget;
import star.image_widget;

import star.player_lua_bindings;
import star.simple_tooltip;
import star.item_tooltip;
import star.config_lua_bindings;
import star.celestial_lua_bindings;
import star.status_controller_lua_bindings;

namespace Star {

ScriptPane::ScriptPane(UniverseClientPtr client, Json config, EntityId sourceEntityId) : BaseScriptPane(config) {
  auto& root = Root::singleton();
  auto assets = root.assets();

  m_client = std::move(client);
  m_sourceEntityId = sourceEntityId;

  m_script.addCallbacks("player", LuaBindings::makePlayerCallbacks(m_client->mainPlayer().get()));
  m_script.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(m_client->mainPlayer()->statusController()));
  m_script.addCallbacks("celestial", LuaBindings::makeCelestialCallbacks(m_client.get()));
}

void ScriptPane::displayed() {
  auto world = m_client->worldClient();
  if (world && world->inWorld()) {
    m_script.setLuaRoot(world->luaRoot());
    m_script.addCallbacks("world", LuaBindings::makeWorldCallbacks(world.get()));
  }
  BaseScriptPane::displayed();
}

void ScriptPane::dismissed() {
  BaseScriptPane::dismissed();
  m_script.removeCallbacks("world");
}

void ScriptPane::tick(float dt) {
  if (m_sourceEntityId != NullEntityId && !m_client->worldClient()->playerCanReachEntity(m_sourceEntityId))
    dismiss();

  BaseScriptPane::tick(dt);
}

PanePtr ScriptPane::createTooltip(Vec2I const& screenPosition) {
  auto result = m_script.invoke<Json>("createTooltip", screenPosition);
  if (result && !result.value().isNull()) {
    if (result->type() == Json::Type::String) {
      return SimpleTooltipBuilder::buildTooltip(result->toString());
    } else {
      PanePtr tooltip = make_shared<Pane>();
      m_reader->construct(*result, tooltip.get());
      return tooltip;
    }
  } else {
    ItemPtr item;
    if (auto child = getChildAt(screenPosition)) {
      if (auto itemSlot = as<ItemSlotWidget>(child))
        item = itemSlot->item();
      if (auto itemGrid = as<ItemGridWidget>(child))
        item = itemGrid->itemAt(screenPosition);
    }
    if (item)
      return ItemTooltipBuilder::buildItemTooltip(item, m_client->mainPlayer());
    return {};
  }
}

LuaCallbacks ScriptPane::makePaneCallbacks() {
  LuaCallbacks callbacks = BaseScriptPane::makePaneCallbacks();
  callbacks.registerCallback("sourceEntity", [this]() { return m_sourceEntityId; });
  return callbacks;
}

bool ScriptPane::openWithInventory() const {
  return m_config.getBool("openWithInventory", false);
}

bool ScriptPane::closeWithInventory() const {
  return m_config.getBool("closeWithInventory", openWithInventory());
}

EntityId ScriptPane::sourceEntityId() const {
  return m_sourceEntityId;
}

}

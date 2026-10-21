import std;
import star.json;
import star.json_extra;
import star.id_map;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.vector;
import star.bi_map;
import star.color;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;
import star.casting;
import star.strong_typedef;
import star.rpc_promise;
import star.a_star;
import star.ordered_set;
import star.either;
import star.random;
import star.periodic_function;
import star.ordered_map;

import star.lua_components;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;
import star.lua_animation_component;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.lua_root;
import star.force_regions;
import star.world;

import star.player_deployment;
import star.uuid;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
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
import star.ai_types;
import star.player_types;
import star.lua_actor_movement_component;
import star.radio_message_database;
import star.player;

import star.player_lua_bindings;
import star.entity_lua_bindings;
import star.config_lua_bindings;
import star.status_controller_lua_bindings;

namespace Star {

class Root;

PlayerDeployment::PlayerDeployment(Root& root, Json const& config) : m_root(root), m_config(config), m_scriptComponent(root) {
  m_deploying = false;
  m_deployed = false;
}

void PlayerDeployment::diskLoad(Json const& diskStore) {
  m_scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage", JsonObject{}));
}

Json PlayerDeployment::diskStore() const {
  JsonObject result;
  result["scriptStorage"] = m_scriptComponent.getScriptStorage();
  return result;
}

void PlayerDeployment::init(Entity* player, World* world) {
  m_world = world;

  if (m_deploying) {
    m_deployed = true;
    m_deploying = false;
  } else {
    m_deployed = false;
  }

  m_scriptComponent.setScripts(jsonToStringList(m_config.getArray("scripts", JsonArray())));
  m_scriptComponent.setUpdateDelta(m_config.getInt("scriptDelta", 10));

  m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(player));
  m_scriptComponent.addCallbacks("player", LuaBindings::makePlayerCallbacks(m_root, as<Player>(player)));
  m_scriptComponent.addCallbacks("status", LuaBindings::makeStatusControllerCallbacks(as<Player>(player)->statusController()));
  m_scriptComponent.addCallbacks("config",
      LuaBindings::makeConfigCallbacks([this](String const& name, Json const& def) { return m_config.query(name, def); }));

  m_scriptComponent.init(world);
}

bool PlayerDeployment::canDeploy() {
  std::optional<bool> res = m_scriptComponent.invoke<bool>("canDeploy");
  return res && *res;
}

void PlayerDeployment::setDeploying(bool deploying) {
  m_deploying = deploying;
}

bool PlayerDeployment::isDeploying() const {
  return m_deploying;
}

bool PlayerDeployment::isDeployed() const {
  return m_deployed;
}

void PlayerDeployment::uninit() {
  m_scriptComponent.uninit();
  m_scriptComponent.removeCallbacks("entity");
  m_scriptComponent.removeCallbacks("player");
  m_scriptComponent.removeCallbacks("status");
  m_scriptComponent.removeCallbacks("config");
  m_world = nullptr;
}

void PlayerDeployment::teleportOut() {
  m_scriptComponent.invoke("teleportOut");
}

std::optional<ChainableJsonMessageResponse> PlayerDeployment::receiveMessage(String const& message, bool localMessage, JsonArray const& args) {
  return m_scriptComponent.handleMessage(message, localMessage, args);
}

void PlayerDeployment::update(float dt) {
  m_scriptComponent.update(m_scriptComponent.updateDt(dt));
}

void PlayerDeployment::render(RenderCallback* renderCallback, Vec2F const& position) {
  for (auto drawablePair : m_scriptComponent.drawables()) {
    drawablePair.first.translate(position);
    renderCallback->addDrawable(drawablePair.first, drawablePair.second.value_or(RenderLayerPlayer));
  }
  renderCallback->addParticles(m_scriptComponent.pullNewParticles());
  for (auto audio : m_scriptComponent.pullNewAudios()) {
    audio->setPosition(position);
    renderCallback->addAudio(audio);
  }
}

void PlayerDeployment::renderLightSources(RenderCallback* renderCallback) {
  renderCallback->addLightSources(m_scriptComponent.lightSources());
}

}

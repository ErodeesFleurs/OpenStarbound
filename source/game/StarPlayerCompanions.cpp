import std;
import star.lua;
import star.json;
import star.json_extra;
import star.id_map;
import star.data_stream;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.variant;
import star.casting;
import star.game_types;
import star.strong_typedef;
import star.bi_map;
import star.rpc_promise;
import star.maybe;
import star.a_star;
import star.ordered_set;
import star.audio;
import star.map;
import star.set;
import star.either;
import star.random;
import star.periodic_function;
import star.ordered_map;

import star.lua_components;
import star.uuid;
import star.drawable;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
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

import star.player_companions;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
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
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.radio_message_database;
import star.player;
import star.root;

import star.player_lua_bindings;
import star.entity_lua_bindings;
import star.config_lua_bindings;
import star.status_controller_lua_bindings;

import star.algorithm;
namespace Star {

Companion::Companion(Root& root, Json const& json) : m_json(json) {
  m_portrait = json.getArray("portrait", JsonArray{}).transformed([&root](Json const& drawable) { return Drawable(*root.imageMetadataDatabase(), drawable); });
}

Json Companion::toJson() const {
  return m_json;
}

Uuid Companion::podUuid() const {
  return Uuid(m_json.getString("podUuid"));
}

Maybe<String> Companion::name() const {
  return m_json.optString("name");
}

Maybe<String> Companion::description() const {
  return m_json.optString("description");
}

List<Drawable> Companion::portrait() const {
  return m_portrait;
}

Maybe<float> Companion::resource(String const& resourceName) const {
  if (auto status = m_json.opt("status"))
    if (auto resources = status->opt("resources"))
      return resources->optFloat(resourceName);
  return {};
}

Maybe<float> Companion::resourceMax(String const& resourceName) const {
  if (auto status = m_json.opt("status"))
    if (auto resources = status->opt("resourceMax"))
      return resources->optFloat(resourceName);
  return {};
}

Maybe<float> Companion::stat(String const& statName) const {
  if (auto status = m_json.opt("status"))
    if (auto stats = status->opt("stats"))
      return stats->optFloat(statName);
  return {};
}

PlayerCompanions::PlayerCompanions(Root& root, Json const& config) : m_root(root), m_config(config), m_scriptComponent(root) {}

void PlayerCompanions::diskLoad(Json const& diskStore) {
  m_scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage", JsonObject{}));
  m_companions = jsonToMapV<StringMap<List<std::shared_ptr<Companion>>>>(diskStore.getObject("companions", JsonObject{}),
      [this](Json const& companions) {
        return companions.toArray().transformed([this](Json const& json) { return std::make_shared<Companion>(m_root, json); });
      });
}

Json PlayerCompanions::diskStore() const {
  JsonObject result;
  result["scriptStorage"] = m_scriptComponent.getScriptStorage();
  result["companions"] = jsonFromMapV(m_companions,
      [](List<std::shared_ptr<Companion>> const& companions) { return companions.transformed(std::mem_fn(&Companion::toJson)); });
  return result;
}

List<std::shared_ptr<Companion>> PlayerCompanions::getCompanions(String const& category) const {
  if (m_companions.contains(category))
    return m_companions.get(category);
  return {};
}

void PlayerCompanions::init(Entity* player, World* world) {
  m_world = world;

  m_scriptComponent.setScripts(jsonToStringList(m_config.getArray("scripts", JsonArray())));
  m_scriptComponent.setUpdateDelta(m_config.getInt("scriptDelta", 10));

  m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(player));
  m_scriptComponent.addCallbacks("player", LuaBindings::makePlayerCallbacks(m_root, as<Player>(player)));
  m_scriptComponent.addCallbacks(
      "status", LuaBindings::makeStatusControllerCallbacks(as<Player>(player)->statusController()));
  m_scriptComponent.addCallbacks("playerCompanions", makeCompanionsCallbacks());

  m_scriptComponent.addCallbacks("config",
      LuaBindings::makeConfigCallbacks([this](
          String const& name, Json const& def) { return m_config.query(name, def); }));

  m_scriptComponent.init(world);
}

void PlayerCompanions::uninit() {
  m_scriptComponent.uninit();
  m_scriptComponent.removeCallbacks("entity");
  m_scriptComponent.removeCallbacks("player");
  m_scriptComponent.removeCallbacks("status");
  m_scriptComponent.removeCallbacks("playerCompanions");
  m_scriptComponent.removeCallbacks("config");
  m_world = nullptr;
}

void PlayerCompanions::dismissCompanion(String const& category, Uuid const& podUuid) {
  m_scriptComponent.invoke("dismissCompanion", category, podUuid.hex());
}

Maybe<ChainableJsonMessageResponse> PlayerCompanions::receiveMessage(String const& message, bool localMessage, JsonArray const& args) {
  return m_scriptComponent.handleMessage(message, localMessage, args);
}

void PlayerCompanions::update(float dt) {
  m_scriptComponent.update(m_scriptComponent.updateDt(dt));
}

LuaCallbacks PlayerCompanions::makeCompanionsCallbacks() {
  LuaCallbacks callbacks;

  callbacks.registerCallback("getCompanions",
      [this](String const& category) {
        return m_companions[category].transformed([](std::shared_ptr<Companion> const& companion) { return companion->toJson(); });
      });
  callbacks.registerCallback("setCompanions",
      [this](String const& category, JsonArray const& companions) {
        m_companions[category] = companions.transformed([this](Json const& json) { return std::make_shared<Companion>(m_root, json); });
      });

  return callbacks;
}

}

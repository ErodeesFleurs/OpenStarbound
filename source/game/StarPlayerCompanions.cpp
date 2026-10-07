#include "StarJson.hpp"
#include "StarJsonExtra.hpp"
#include "StarJson.hpp"
#include "StarIdMap.hpp"
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarVariant.hpp"
#include "StarCasting.hpp"
#include "StarVector.hpp"
#include "StarGameTypes.hpp"
#include "StarStrongTypedef.hpp"
#include "StarBiMap.hpp"
#include "StarNetElementSystem.hpp"
#include "StarList.hpp"
#include "StarRpcPromise.hpp"
#include "StarMaybe.hpp"
#include "StarRect.hpp"
#include "StarAStar.hpp"
#include "StarOrderedSet.hpp"
#include "StarAudio.hpp"
#include "StarThread.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarEither.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.periodic_function;
#include "StarOrderedMap.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"

#include "StarLuaComponents.hpp"
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
#include "StarLuaRoot.hpp"
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
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;

import star.player_lua_bindings;
import star.entity_lua_bindings;
import star.config_lua_bindings;
import star.status_controller_lua_bindings;

namespace Star {

Companion::Companion(Json const& json) : m_json(json) {
  m_portrait = json.getArray("portrait", JsonArray{}).transformed(construct<Drawable>());
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

PlayerCompanions::PlayerCompanions(Json const& config) : m_config(config) {}

void PlayerCompanions::diskLoad(Json const& diskStore) {
  m_scriptComponent.setScriptStorage(diskStore.getObject("scriptStorage", JsonObject{}));
  m_companions = jsonToMapV<StringMap<List<CompanionPtr>>>(diskStore.getObject("companions", JsonObject{}),
      [](Json const& companions) {
        return companions.toArray().transformed([](Json const& json) { return make_shared<Companion>(json); });
      });
}

Json PlayerCompanions::diskStore() const {
  JsonObject result;
  result["scriptStorage"] = m_scriptComponent.getScriptStorage();
  result["companions"] = jsonFromMapV(m_companions,
      [](List<CompanionPtr> const& companions) { return companions.transformed(mem_fn(&Companion::toJson)); });
  return result;
}

List<CompanionPtr> PlayerCompanions::getCompanions(String const& category) const {
  if (m_companions.contains(category))
    return m_companions.get(category);
  return {};
}

void PlayerCompanions::init(Entity* player, World* world) {
  m_world = world;

  m_scriptComponent.setScripts(jsonToStringList(m_config.getArray("scripts", JsonArray())));
  m_scriptComponent.setUpdateDelta(m_config.getInt("scriptDelta", 10));

  m_scriptComponent.addCallbacks("entity", LuaBindings::makeEntityCallbacks(player));
  m_scriptComponent.addCallbacks("player", LuaBindings::makePlayerCallbacks(as<Player>(player)));
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
        return m_companions[category].transformed([](CompanionPtr const& companion) { return companion->toJson(); });
      });
  callbacks.registerCallback("setCompanions",
      [this](String const& category, JsonArray const& companions) {
        m_companions[category] = companions.transformed([](Json const& json) { return make_shared<Companion>(json); });
      });

  return callbacks;
}

}

module;
import std;
import star.json;
import star.json_extra;
import star.lua;
import star.list;
import star.observer_stream;
import star.strong_typedef;
import star.data_stream;
import star.id_map;
import star.either;
import star.game_types;
import star.random;
import star.periodic_function;
import star.ordered_map;
import star.string;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.bi_map;
import star.audio;
import star.map;
import star.set;


// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.status_types;
import star.damage_types;
import star.world_geometry;
import star.damage;
import star.lua_components;
import star.lua_actor_movement_component;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.entity_rendering_types;
import star.stat_collection;
import star.status_effect_database;
import star.status_controller;
import star.lua_game_converters;

module star.status_controller_lua_bindings;

namespace Star::LuaBindings::StatusControllerCallbacks {
  Json statusProperty(StatusController* statController, String const& arg1, Json const& arg2);
  void setStatusProperty(StatusController* statController, String const& arg1, Json const& arg2);
  float stat(StatusController* statController, String const& arg1);
  bool statPositive(StatusController* statController, String const& arg1);
  StringList resourceNames(StatusController* statController);
  bool isResource(StatusController* statController, String const& arg1);
  float resource(StatusController* statController, String const& arg1);
  bool resourcePositive(StatusController* statController, String const& arg1);
  void setResource(StatusController* statController, String const& arg1, float arg2);
  void modifyResource(StatusController* statController, String const& arg1, float arg2);
  float giveResource(StatusController* statController, String const& resourceName, float amount);
  bool consumeResource(StatusController* statController, String const& arg1, float arg2);
  bool overConsumeResource(StatusController* statController, String const& arg1, float arg2);
  bool resourceLocked(StatusController* statController, String const& arg1);
  void setResourceLocked(StatusController* statController, String const& arg1, bool arg2);
  void resetResource(StatusController* statController, String const& arg1);
  void resetAllResources(StatusController* statController);
  std::optional<float> resourceMax(StatusController* statController, String const& arg1);
  std::optional<float> resourcePercentage(StatusController* statController, String const& arg1);
  float setResourcePercentage(StatusController* statController, String const& arg1, float arg2);
  float modifyResourcePercentage(StatusController* statController, String const& arg1, float arg2);
  JsonArray getPersistentEffects(StatusController* statController, String const& arg1);
  void addPersistentEffect(StatusController* statController, String const& arg1, Json const& arg2);
  void addPersistentEffects(StatusController* statController, String const& arg1, JsonArray const& arg2);
  void setPersistentEffects(StatusController* statController, String const& arg1, JsonArray const& arg2);
  void clearPersistentEffects(StatusController* statController, String const& arg1);
  void clearAllPersistentEffects(StatusController* statController);
  void addEphemeralEffect(StatusController* statController,
      String const& uniqueEffect,
      std::optional<float> duration,
      std::optional<EntityId> sourceEntityId);
  void addEphemeralEffects(StatusController* statController, JsonArray const& arg1, std::optional<EntityId> sourceEntityId);
  void removeEphemeralEffect(StatusController* statController, String const& arg1);
  void clearEphemeralEffects(StatusController* statController);
  LuaTupleReturn<List<Json>, std::uint64_t> damageTakenSince(StatusController* statController, std::optional<std::uint64_t> timestep);
  LuaTupleReturn<List<Json>, std::uint64_t> inflictedHitsSince(StatusController* statController, std::optional<std::uint64_t> timestep);
  LuaTupleReturn<List<Json>, std::uint64_t> inflictedDamageSince(
      StatusController* statController, std::optional<std::uint64_t> timestep);
  List<JsonArray> activeUniqueStatusEffectSummary(StatusController* statController);
  bool uniqueStatusEffectActive(StatusController* statController, String const& effectName);
}

namespace Star {

LuaCallbacks LuaBindings::makeStatusControllerCallbacks(StatusController* statController) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Json, String, Json>(
      "statusProperty", std::bind(StatusControllerCallbacks::statusProperty, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, Json>(
      "setStatusProperty", std::bind(StatusControllerCallbacks::setStatusProperty, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<float, String>(
      "stat", std::bind(StatusControllerCallbacks::stat, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<bool, String>(
      "statPositive", std::bind(StatusControllerCallbacks::statPositive, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<StringList>(
      "resourceNames", std::bind(StatusControllerCallbacks::resourceNames, statController));
  callbacks.registerCallbackWithSignature<bool, String>(
      "isResource", std::bind(StatusControllerCallbacks::isResource, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<float, String>(
      "resource", std::bind(StatusControllerCallbacks::resource, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<bool, String>(
      "resourcePositive", std::bind(StatusControllerCallbacks::resourcePositive, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, String, float>(
      "setResource", std::bind(StatusControllerCallbacks::setResource, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, float>(
      "modifyResource", std::bind(StatusControllerCallbacks::modifyResource, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<float, String, float>(
      "giveResource", std::bind(StatusControllerCallbacks::giveResource, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool, String, float>(
      "consumeResource", std::bind(StatusControllerCallbacks::consumeResource, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool, String, float>(
      "overConsumeResource", std::bind(StatusControllerCallbacks::overConsumeResource, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool, String>(
      "resourceLocked", std::bind(StatusControllerCallbacks::resourceLocked, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, String, bool>(
      "setResourceLocked", std::bind(StatusControllerCallbacks::setResourceLocked, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String>(
      "resetResource", std::bind(StatusControllerCallbacks::resetResource, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void>(
      "resetAllResources", std::bind(StatusControllerCallbacks::resetAllResources, statController));
  callbacks.registerCallbackWithSignature<std::optional<float>, String>(
      "resourceMax", std::bind(StatusControllerCallbacks::resourceMax, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<std::optional<float>, String>(
      "resourcePercentage", std::bind(StatusControllerCallbacks::resourcePercentage, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<float, String, float>(
      "setResourcePercentage", std::bind(StatusControllerCallbacks::setResourcePercentage, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<float, String, float>(
      "modifyResourcePercentage", std::bind(StatusControllerCallbacks::modifyResourcePercentage, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<JsonArray, String>(
      "getPersistentEffects", std::bind(StatusControllerCallbacks::getPersistentEffects, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void, String, Json>(
      "addPersistentEffect", std::bind(StatusControllerCallbacks::addPersistentEffect, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, JsonArray>(
      "addPersistentEffects", std::bind(StatusControllerCallbacks::addPersistentEffects, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String, JsonArray>(
      "setPersistentEffects", std::bind(StatusControllerCallbacks::setPersistentEffects, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String>(
      "clearPersistentEffects", std::bind(StatusControllerCallbacks::clearPersistentEffects, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void>(
      "clearAllPersistentEffects", std::bind(StatusControllerCallbacks::clearAllPersistentEffects, statController));
  callbacks.registerCallbackWithSignature<void, String, std::optional<float>, std::optional<EntityId>>(
      "addEphemeralEffect", std::bind(StatusControllerCallbacks::addEphemeralEffect, statController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  callbacks.registerCallbackWithSignature<void, JsonArray, std::optional<EntityId>>(
      "addEphemeralEffects", std::bind(StatusControllerCallbacks::addEphemeralEffects, statController, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<void, String>(
      "removeEphemeralEffect", std::bind(StatusControllerCallbacks::removeEphemeralEffect, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<void>(
      "clearEphemeralEffects", std::bind(StatusControllerCallbacks::clearEphemeralEffects, statController));
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
      "damageTakenSince", std::bind(StatusControllerCallbacks::damageTakenSince, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
      "inflictedHitsSince", std::bind(StatusControllerCallbacks::inflictedHitsSince, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
      "inflictedDamageSince", std::bind(StatusControllerCallbacks::inflictedDamageSince, statController, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<List<JsonArray>>("activeUniqueStatusEffectSummary",
      std::bind(&StatusControllerCallbacks::activeUniqueStatusEffectSummary, statController));
  callbacks.registerCallbackWithSignature<bool, String>("uniqueStatusEffectActive",
      std::bind(&StatusControllerCallbacks::uniqueStatusEffectActive, statController, std::placeholders::_1));

  callbacks.registerCallbackWithSignature<Directives>("primaryDirectives", std::bind(&StatusController::primaryDirectives, statController));
  callbacks.registerCallback("setPrimaryDirectives", [statController](std::optional<String> const& directives) { statController->setPrimaryDirectives(directives.value_or(String{})); });

  callbacks.registerCallbackWithSignature<void, DamageRequest>("applySelfDamageRequest", std::bind(&StatusController::applySelfDamageRequest, statController, std::placeholders::_1));

  return callbacks;
}

Json LuaBindings::StatusControllerCallbacks::statusProperty(
    StatusController* statController, String const& arg1, Json const& arg2) {
  return statController->statusProperty(arg1, arg2);
}

void LuaBindings::StatusControllerCallbacks::setStatusProperty(
    StatusController* statController, String const& arg1, Json const& arg2) {
  statController->setStatusProperty(arg1, arg2);
}

float LuaBindings::StatusControllerCallbacks::stat(StatusController* statController, String const& arg1) {
  return statController->stat(arg1);
}

bool LuaBindings::StatusControllerCallbacks::statPositive(StatusController* statController, String const& arg1) {
  return statController->statPositive(arg1);
}

StringList LuaBindings::StatusControllerCallbacks::resourceNames(StatusController* statController) {
  return statController->resourceNames();
}

bool LuaBindings::StatusControllerCallbacks::isResource(StatusController* statController, String const& arg1) {
  return statController->isResource(arg1);
}

float LuaBindings::StatusControllerCallbacks::resource(StatusController* statController, String const& arg1) {
  return statController->resource(arg1);
}

bool LuaBindings::StatusControllerCallbacks::resourcePositive(StatusController* statController, String const& arg1) {
  return statController->resourcePositive(arg1);
}

void LuaBindings::StatusControllerCallbacks::setResource(
    StatusController* statController, String const& arg1, float arg2) {
  statController->setResource(arg1, arg2);
}

void LuaBindings::StatusControllerCallbacks::modifyResource(
    StatusController* statController, String const& arg1, float arg2) {
  statController->modifyResource(arg1, arg2);
}

float LuaBindings::StatusControllerCallbacks::giveResource(
    StatusController* statController, String const& resourceName, float amount) {
  return statController->giveResource(resourceName, amount);
}

bool LuaBindings::StatusControllerCallbacks::consumeResource(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->consumeResource(arg1, arg2);
}

bool LuaBindings::StatusControllerCallbacks::overConsumeResource(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->overConsumeResource(arg1, arg2);
}

bool LuaBindings::StatusControllerCallbacks::resourceLocked(StatusController* statController, String const& arg1) {
  return statController->resourceLocked(arg1);
}

void LuaBindings::StatusControllerCallbacks::setResourceLocked(
    StatusController* statController, String const& arg1, bool arg2) {
  statController->setResourceLocked(arg1, arg2);
}

void LuaBindings::StatusControllerCallbacks::resetResource(StatusController* statController, String const& arg1) {
  statController->resetResource(arg1);
}

void LuaBindings::StatusControllerCallbacks::resetAllResources(StatusController* statController) {
  statController->resetAllResources();
}

std::optional<float> LuaBindings::StatusControllerCallbacks::resourceMax(StatusController* statController, String const& arg1) {
  return statController->resourceMax(arg1);
}

std::optional<float> LuaBindings::StatusControllerCallbacks::resourcePercentage(
    StatusController* statController, String const& arg1) {
  return statController->resourcePercentage(arg1);
}

float LuaBindings::StatusControllerCallbacks::setResourcePercentage(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->setResourcePercentage(arg1, arg2);
}

float LuaBindings::StatusControllerCallbacks::modifyResourcePercentage(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->modifyResourcePercentage(arg1, arg2);
}

JsonArray LuaBindings::StatusControllerCallbacks::getPersistentEffects(
    StatusController* statController, String const& arg1) {
  return statController->getPersistentEffects(arg1).transformed(jsonFromPersistentStatusEffect);
}

void LuaBindings::StatusControllerCallbacks::addPersistentEffect(
    StatusController* statController, String const& arg1, Json const& arg2) {
  addPersistentEffects(statController, arg1, JsonArray{arg2});
}

void LuaBindings::StatusControllerCallbacks::addPersistentEffects(
    StatusController* statController, String const& arg1, JsonArray const& arg2) {
  statController->addPersistentEffects(arg1, arg2.transformed(jsonToPersistentStatusEffect));
}

void LuaBindings::StatusControllerCallbacks::setPersistentEffects(
    StatusController* statController, String const& arg1, JsonArray const& arg2) {
  statController->setPersistentEffects(arg1, arg2.transformed(jsonToPersistentStatusEffect));
}

void LuaBindings::StatusControllerCallbacks::clearPersistentEffects(
    StatusController* statController, String const& arg1) {
  statController->clearPersistentEffects(arg1);
}

void LuaBindings::StatusControllerCallbacks::clearAllPersistentEffects(StatusController* statController) {
  statController->clearAllPersistentEffects();
}

void LuaBindings::StatusControllerCallbacks::addEphemeralEffect(
    StatusController* statController, String const& arg1, std::optional<float> arg2, std::optional<EntityId> sourceEntityId) {
  statController->addEphemeralEffect(EphemeralStatusEffect{UniqueStatusEffect{arg1}, arg2}, sourceEntityId);
}

void LuaBindings::StatusControllerCallbacks::addEphemeralEffects(
    StatusController* statController, JsonArray const& arg1, std::optional<EntityId> sourceEntityId) {
  statController->addEphemeralEffects(arg1.transformed(jsonToEphemeralStatusEffect), sourceEntityId);
}

void LuaBindings::StatusControllerCallbacks::removeEphemeralEffect(
    StatusController* statController, String const& arg1) {
  statController->removeEphemeralEffect(UniqueStatusEffect{arg1});
}

void LuaBindings::StatusControllerCallbacks::clearEphemeralEffects(StatusController* statController) {
  statController->clearEphemeralEffects();
}

LuaTupleReturn<List<Json>, std::uint64_t> LuaBindings::StatusControllerCallbacks::damageTakenSince(
    StatusController* statController, std::optional<std::uint64_t> timestep) {
  auto pair = statController->damageTakenSince(timestep.value_or(std::uint64_t{}));
  return luaTupleReturn(pair.first.transformed(std::mem_fn(&DamageNotification::toJson)), pair.second);
}

LuaTupleReturn<List<Json>, std::uint64_t> LuaBindings::StatusControllerCallbacks::inflictedHitsSince(
    StatusController* statController, std::optional<std::uint64_t> timestep) {
  auto pair = statController->inflictedHitsSince(timestep.value_or(std::uint64_t{}));
  return luaTupleReturn(
      pair.first.transformed([](auto const& p) { return p.second.toJson().set("targetEntityId", p.first); }),
      pair.second);
}

LuaTupleReturn<List<Json>, std::uint64_t> LuaBindings::StatusControllerCallbacks::inflictedDamageSince(
    StatusController* statController, std::optional<std::uint64_t> timestep) {
  auto pair = statController->inflictedDamageSince(timestep.value_or(std::uint64_t{}));
  return luaTupleReturn(pair.first.transformed(std::mem_fn(&DamageNotification::toJson)), pair.second);
}

List<JsonArray> LuaBindings::StatusControllerCallbacks::activeUniqueStatusEffectSummary(
    StatusController* statController) {
  auto summary = statController->activeUniqueStatusEffectSummary();
  return summary.transformed([](std::pair<UniqueStatusEffect, std::optional<float>> effect) {
    JsonArray effectJson = {effect.first};
    if (effect.second)
      effectJson.append(*effect.second);
    return effectJson;
  });
}

bool LuaBindings::StatusControllerCallbacks::uniqueStatusEffectActive(
    StatusController* statController, String const& effectName) {
  return statController->uniqueStatusEffectActive(effectName);
}

}

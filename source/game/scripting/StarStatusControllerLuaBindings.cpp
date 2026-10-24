module;
import std;
import star.format;
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


// Match client include order for SIMD intrinsics used by xxhash.
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
  [[nodiscard]] std::expected<void, StarException> setResource(StatusController* statController, String const& arg1, float arg2);
  [[nodiscard]] std::expected<void, StarException> modifyResource(StatusController* statController, String const& arg1, float arg2);
  float giveResource(StatusController* statController, String const& resourceName, float amount);
  [[nodiscard]] std::expected<bool, StarException> consumeResource(StatusController* statController, String const& arg1, float arg2);
  [[nodiscard]] std::expected<bool, StarException> overConsumeResource(StatusController* statController, String const& arg1, float arg2);
  [[nodiscard]] std::expected<bool, StarException> resourceLocked(StatusController* statController, String const& arg1);
  [[nodiscard]] std::expected<void, StarException> setResourceLocked(StatusController* statController, String const& arg1, bool arg2);
  [[nodiscard]] std::expected<void, StarException> resetResource(StatusController* statController, String const& arg1);
  [[nodiscard]] std::expected<void, StarException> resetAllResources(StatusController* statController);
  [[nodiscard]] std::expected<std::optional<float>, StarException> resourceMax(StatusController* statController, String const& arg1);
  [[nodiscard]] std::expected<std::optional<float>, StarException> resourcePercentage(StatusController* statController, String const& arg1);
  [[nodiscard]] std::expected<float, StarException> setResourcePercentage(StatusController* statController, String const& arg1, float arg2);
  [[nodiscard]] std::expected<float, StarException> modifyResourcePercentage(StatusController* statController, String const& arg1, float arg2);
  [[nodiscard]] std::expected<JsonArray, StarException> getPersistentEffects(StatusController* statController, String const& arg1);
  [[nodiscard]] std::expected<void, StarException> addPersistentEffect(StatusController* statController, String const& arg1, Json const& arg2);
  [[nodiscard]] std::expected<void, StarException> addPersistentEffects(StatusController* statController, String const& arg1, JsonArray const& arg2);
  [[nodiscard]] std::expected<void, StarException> setPersistentEffects(StatusController* statController, String const& arg1, JsonArray const& arg2);
  [[nodiscard]] std::expected<void, StarException> clearPersistentEffects(StatusController* statController, String const& arg1);
  [[nodiscard]] std::expected<void, StarException> clearAllPersistentEffects(StatusController* statController);
  [[nodiscard]] std::expected<void, StarException> addEphemeralEffect(StatusController* statController,
      String const& uniqueEffect,
      std::optional<float> duration,
      std::optional<EntityId> sourceEntityId);
  [[nodiscard]] std::expected<void, StarException> addEphemeralEffects(StatusController* statController, JsonArray const& arg1, std::optional<EntityId> sourceEntityId);
  [[nodiscard]] std::expected<void, StarException> removeEphemeralEffect(StatusController* statController, String const& arg1);
  [[nodiscard]] std::expected<void, StarException> clearEphemeralEffects(StatusController* statController);
  [[nodiscard]] std::expected<LuaTupleReturn<List<Json>, std::uint64_t>, StarException> damageTakenSince(StatusController* statController, std::optional<std::uint64_t> timestep);
  [[nodiscard]] std::expected<LuaTupleReturn<List<Json>, std::uint64_t>, StarException> inflictedHitsSince(StatusController* statController, std::optional<std::uint64_t> timestep);
  [[nodiscard]] std::expected<LuaTupleReturn<List<Json>, std::uint64_t>, StarException> inflictedDamageSince(
      StatusController* statController, std::optional<std::uint64_t> timestep);
  List<JsonArray> activeUniqueStatusEffectSummary(StatusController* statController);
  bool uniqueStatusEffectActive(StatusController* statController, String const& effectName);
}

namespace Star {

std::expected<LuaCallbacks, StarException> LuaBindings::makeStatusControllerCallbacks(StatusController* statController) {
  LuaCallbacks callbacks;

  if (auto result = callbacks.registerCallbackWithSignature<Json, String, Json>(
      "statusProperty", std::bind(StatusControllerCallbacks::statusProperty, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Json>(
      "setStatusProperty", std::bind(StatusControllerCallbacks::setStatusProperty, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String>(
      "stat", std::bind(StatusControllerCallbacks::stat, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "statPositive", std::bind(StatusControllerCallbacks::statPositive, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<StringList>(
      "resourceNames", std::bind(StatusControllerCallbacks::resourceNames, statController)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "isResource", std::bind(StatusControllerCallbacks::isResource, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String>(
      "resource", std::bind(StatusControllerCallbacks::resource, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "resourcePositive", std::bind(StatusControllerCallbacks::resourcePositive, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, float>(
      "setResource", std::bind(StatusControllerCallbacks::setResource, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, float>(
      "modifyResource", std::bind(StatusControllerCallbacks::modifyResource, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String, float>(
      "giveResource", std::bind(StatusControllerCallbacks::giveResource, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String, float>(
      "consumeResource", std::bind(StatusControllerCallbacks::consumeResource, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String, float>(
      "overConsumeResource", std::bind(StatusControllerCallbacks::overConsumeResource, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>(
      "resourceLocked", std::bind(StatusControllerCallbacks::resourceLocked, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, bool>(
      "setResourceLocked", std::bind(StatusControllerCallbacks::setResourceLocked, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String>(
      "resetResource", std::bind(StatusControllerCallbacks::resetResource, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void>(
      "resetAllResources", std::bind(StatusControllerCallbacks::resetAllResources, statController)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<std::optional<float>, String>(
      "resourceMax", std::bind(StatusControllerCallbacks::resourceMax, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<std::optional<float>, String>(
      "resourcePercentage", std::bind(StatusControllerCallbacks::resourcePercentage, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String, float>(
      "setResourcePercentage", std::bind(StatusControllerCallbacks::setResourcePercentage, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float, String, float>(
      "modifyResourcePercentage", std::bind(StatusControllerCallbacks::modifyResourcePercentage, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<JsonArray, String>(
      "getPersistentEffects", std::bind(StatusControllerCallbacks::getPersistentEffects, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, Json>(
      "addPersistentEffect", std::bind(StatusControllerCallbacks::addPersistentEffect, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, JsonArray>(
      "addPersistentEffects", std::bind(StatusControllerCallbacks::addPersistentEffects, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, JsonArray>(
      "setPersistentEffects", std::bind(StatusControllerCallbacks::setPersistentEffects, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String>(
      "clearPersistentEffects", std::bind(StatusControllerCallbacks::clearPersistentEffects, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void>(
      "clearAllPersistentEffects", std::bind(StatusControllerCallbacks::clearAllPersistentEffects, statController)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String, std::optional<float>, std::optional<EntityId>>(
      "addEphemeralEffect", std::bind(StatusControllerCallbacks::addEphemeralEffect, statController, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, JsonArray, std::optional<EntityId>>(
      "addEphemeralEffects", std::bind(StatusControllerCallbacks::addEphemeralEffects, statController, std::placeholders::_1, std::placeholders::_2)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void, String>(
      "removeEphemeralEffect", std::bind(StatusControllerCallbacks::removeEphemeralEffect, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<void>(
      "clearEphemeralEffects", std::bind(StatusControllerCallbacks::clearEphemeralEffects, statController)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
      "damageTakenSince", std::bind(StatusControllerCallbacks::damageTakenSince, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
      "inflictedHitsSince", std::bind(StatusControllerCallbacks::inflictedHitsSince, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
      "inflictedDamageSince", std::bind(StatusControllerCallbacks::inflictedDamageSince, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<List<JsonArray>>("activeUniqueStatusEffectSummary",
      std::bind(&StatusControllerCallbacks::activeUniqueStatusEffectSummary, statController)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<bool, String>("uniqueStatusEffectActive",
      std::bind(&StatusControllerCallbacks::uniqueStatusEffectActive, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<Directives>("primaryDirectives", std::bind(&StatusController::primaryDirectives, statController)); !result) return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setPrimaryDirectives", [statController](std::optional<String> const& directives) { statController->setPrimaryDirectives(directives.value_or(String{})); }); !result) return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<void, DamageRequest>("applySelfDamageRequest", std::bind(&StatusController::applySelfDamageRequest, statController, std::placeholders::_1)); !result) return std::unexpected(std::move(result.error()));

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

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::setResource(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->setResource(arg1, arg2);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::modifyResource(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->modifyResource(arg1, arg2);
}

float LuaBindings::StatusControllerCallbacks::giveResource(
    StatusController* statController, String const& resourceName, float amount) {
  return statController->giveResource(resourceName, amount);
}

std::expected<bool, StarException> LuaBindings::StatusControllerCallbacks::consumeResource(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->consumeResource(arg1, arg2);
}

std::expected<bool, StarException> LuaBindings::StatusControllerCallbacks::overConsumeResource(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->overConsumeResource(arg1, arg2);
}

std::expected<bool, StarException> LuaBindings::StatusControllerCallbacks::resourceLocked(StatusController* statController, String const& arg1) {
  return statController->resourceLocked(arg1);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::setResourceLocked(
    StatusController* statController, String const& arg1, bool arg2) {
  return statController->setResourceLocked(arg1, arg2);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::resetResource(StatusController* statController, String const& arg1) {
  return statController->resetResource(arg1);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::resetAllResources(StatusController* statController) {
  return statController->resetAllResources();
}

std::expected<std::optional<float>, StarException> LuaBindings::StatusControllerCallbacks::resourceMax(StatusController* statController, String const& arg1) {
  return statController->resourceMax(arg1);
}

std::expected<std::optional<float>, StarException> LuaBindings::StatusControllerCallbacks::resourcePercentage(
    StatusController* statController, String const& arg1) {
  return statController->resourcePercentage(arg1);
}

std::expected<float, StarException> LuaBindings::StatusControllerCallbacks::setResourcePercentage(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->setResourcePercentage(arg1, arg2);
}

std::expected<float, StarException> LuaBindings::StatusControllerCallbacks::modifyResourcePercentage(
    StatusController* statController, String const& arg1, float arg2) {
  return statController->modifyResourcePercentage(arg1, arg2);
}

std::expected<JsonArray, StarException> LuaBindings::StatusControllerCallbacks::getPersistentEffects(
    StatusController* statController, String const& arg1) {
  JsonArray result;
  for (auto const& effect : statController->getPersistentEffects(arg1)) {
    auto json = jsonFromPersistentStatusEffect(effect);
    if (!json) return std::unexpected(std::move(json.error()));
    result.append(std::move(*json));
  }
  return result;
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::addPersistentEffect(
    StatusController* statController, String const& arg1, Json const& arg2) {
  return addPersistentEffects(statController, arg1, JsonArray{arg2});
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::addPersistentEffects(
    StatusController* statController, String const& arg1, JsonArray const& arg2) {
  List<PersistentStatusEffect> effects;
  for (auto const& json : arg2) {
    auto effect = jsonToPersistentStatusEffect(json);
    if (!effect) return std::unexpected(std::move(effect.error()));
    effects.append(std::move(*effect));
  }
  return statController->addPersistentEffects(arg1, effects);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::setPersistentEffects(
    StatusController* statController, String const& arg1, JsonArray const& arg2) {
  List<PersistentStatusEffect> effects;
  for (auto const& json : arg2) {
    auto effect = jsonToPersistentStatusEffect(json);
    if (!effect) return std::unexpected(std::move(effect.error()));
    effects.append(std::move(*effect));
  }
  return statController->setPersistentEffects(arg1, effects);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::clearPersistentEffects(
    StatusController* statController, String const& arg1) {
  return statController->clearPersistentEffects(arg1);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::clearAllPersistentEffects(StatusController* statController) {
  return statController->clearAllPersistentEffects();
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::addEphemeralEffect(
    StatusController* statController, String const& arg1, std::optional<float> arg2, std::optional<EntityId> sourceEntityId) {
  return statController->addEphemeralEffect(EphemeralStatusEffect{UniqueStatusEffect{arg1}, arg2}, sourceEntityId);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::addEphemeralEffects(
    StatusController* statController, JsonArray const& arg1, std::optional<EntityId> sourceEntityId) {
  List<EphemeralStatusEffect> effects;
  for (auto const& json : arg1) {
    auto effect = jsonToEphemeralStatusEffect(json);
    if (!effect) return std::unexpected(std::move(effect.error()));
    effects.append(std::move(*effect));
  }
  return statController->addEphemeralEffects(effects, sourceEntityId);
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::removeEphemeralEffect(
    StatusController* statController, String const& arg1) {
  auto removed = statController->removeEphemeralEffect(UniqueStatusEffect{arg1});
  if (!removed) return std::unexpected(std::move(removed.error()));
  return {};
}

std::expected<void, StarException> LuaBindings::StatusControllerCallbacks::clearEphemeralEffects(StatusController* statController) {
  return statController->clearEphemeralEffects();
}

std::expected<LuaTupleReturn<List<Json>, std::uint64_t>, StarException> LuaBindings::StatusControllerCallbacks::damageTakenSince(
    StatusController* statController, std::optional<std::uint64_t> timestep) {
  auto pair = statController->damageTakenSince(timestep.value_or(std::uint64_t{}));
  List<Json> notifications;
  for (auto const& notification : pair.first) {
    auto json = notification.toJson();
    if (!json) return std::unexpected(std::move(json.error()));
    notifications.append(std::move(*json));
  }
  return luaTupleReturn(std::move(notifications), pair.second);
}

std::expected<LuaTupleReturn<List<Json>, std::uint64_t>, StarException> LuaBindings::StatusControllerCallbacks::inflictedHitsSince(
    StatusController* statController, std::optional<std::uint64_t> timestep) {
  auto pair = statController->inflictedHitsSince(timestep.value_or(std::uint64_t{}));
  List<Json> hits;
  for (auto const& hit : pair.first) {
    auto json = hit.second.toJson();
    if (!json) return std::unexpected(std::move(json.error()));
    auto tagged = json->set("targetEntityId", hit.first);
    if (!tagged) return std::unexpected(std::move(tagged.error()));
    hits.append(std::move(*tagged));
  }
  return luaTupleReturn(std::move(hits), pair.second);
}

std::expected<LuaTupleReturn<List<Json>, std::uint64_t>, StarException> LuaBindings::StatusControllerCallbacks::inflictedDamageSince(
    StatusController* statController, std::optional<std::uint64_t> timestep) {
  auto pair = statController->inflictedDamageSince(timestep.value_or(std::uint64_t{}));
  List<Json> notifications;
  for (auto const& notification : pair.first) {
    auto json = notification.toJson();
    if (!json) return std::unexpected(std::move(json.error()));
    notifications.append(std::move(*json));
  }
  return luaTupleReturn(std::move(notifications), pair.second);
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

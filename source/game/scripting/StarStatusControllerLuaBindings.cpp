#include "StarStatusControllerLuaBindings.hpp"
#include "StarStatusController.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaGameConverters.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeStatusControllerCallbacks(StatusController* statController) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Json, String, Json>(
      "statusProperty", [statController](auto&&... args) { return StatusControllerCallbacks::statusProperty(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Json>(
      "setStatusProperty", [statController](auto&&... args) { StatusControllerCallbacks::setStatusProperty(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String>(
      "stat", [statController](auto&&... args) { return StatusControllerCallbacks::stat(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String>(
      "statPositive", [statController](auto&&... args) { return StatusControllerCallbacks::statPositive(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<StringList>(
      "resourceNames", [statController]() { return StatusControllerCallbacks::resourceNames(statController); });
  callbacks.registerCallbackWithSignature<bool, String>(
      "isResource", [statController](auto&&... args) { return StatusControllerCallbacks::isResource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String>(
      "resource", [statController](auto&&... args) { return StatusControllerCallbacks::resource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String>(
      "resourcePositive", [statController](auto&&... args) { return StatusControllerCallbacks::resourcePositive(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, float>(
      "setResource", [statController](auto&&... args) { StatusControllerCallbacks::setResource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, float>(
      "modifyResource", [statController](auto&&... args) { StatusControllerCallbacks::modifyResource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String, float>(
      "giveResource", [statController](auto&&... args) { return StatusControllerCallbacks::giveResource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String, float>(
      "consumeResource", [statController](auto&&... args) { return StatusControllerCallbacks::consumeResource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String, float>(
      "overConsumeResource", [statController](auto&&... args) { return StatusControllerCallbacks::overConsumeResource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<bool, String>(
      "resourceLocked", [statController](auto&&... args) { return StatusControllerCallbacks::resourceLocked(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, bool>(
      "setResourceLocked", [statController](auto&&... args) { StatusControllerCallbacks::setResourceLocked(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String>(
      "resetResource", [statController](auto&&... args) { StatusControllerCallbacks::resetResource(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void>(
      "resetAllResources", [statController]() { StatusControllerCallbacks::resetAllResources(statController); });
  callbacks.registerCallbackWithSignature<Maybe<float>, String>(
      "resourceMax", [statController](auto&&... args) { return StatusControllerCallbacks::resourceMax(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<Maybe<float>, String>(
      "resourcePercentage", [statController](auto&&... args) { return StatusControllerCallbacks::resourcePercentage(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String, float>(
      "setResourcePercentage", [statController](auto&&... args) { return StatusControllerCallbacks::setResourcePercentage(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<float, String, float>(
      "modifyResourcePercentage", [statController](auto&&... args) { return StatusControllerCallbacks::modifyResourcePercentage(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<JsonArray, String>(
      "getPersistentEffects", [statController](auto&&... args) { return StatusControllerCallbacks::getPersistentEffects(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, Json>(
      "addPersistentEffect", [statController](auto&&... args) { StatusControllerCallbacks::addPersistentEffect(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, JsonArray>(
      "addPersistentEffects", [statController](auto&&... args) { StatusControllerCallbacks::addPersistentEffects(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String, JsonArray>(
      "setPersistentEffects", [statController](auto&&... args) { StatusControllerCallbacks::setPersistentEffects(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String>(
      "clearPersistentEffects", [statController](auto&&... args) { StatusControllerCallbacks::clearPersistentEffects(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void>(
      "clearAllPersistentEffects", [statController]() { StatusControllerCallbacks::clearAllPersistentEffects(statController); });
  callbacks.registerCallbackWithSignature<void, String, Maybe<float>, Maybe<EntityId>>(
      "addEphemeralEffect", [statController](auto&&... args) { StatusControllerCallbacks::addEphemeralEffect(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, JsonArray, Maybe<EntityId>>(
      "addEphemeralEffects", [statController](auto&&... args) { StatusControllerCallbacks::addEphemeralEffects(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void, String>(
      "removeEphemeralEffect", [statController](auto&&... args) { StatusControllerCallbacks::removeEphemeralEffect(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<void>(
      "clearEphemeralEffects", [statController]() { StatusControllerCallbacks::clearEphemeralEffects(statController); });
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, uint64_t>, Maybe<uint64_t>>(
      "damageTakenSince", [statController](auto&&... args) { return StatusControllerCallbacks::damageTakenSince(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, uint64_t>, Maybe<uint64_t>>(
      "inflictedHitsSince", [statController](auto&&... args) { return StatusControllerCallbacks::inflictedHitsSince(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, uint64_t>, Maybe<uint64_t>>(
      "inflictedDamageSince", [statController](auto&&... args) { return StatusControllerCallbacks::inflictedDamageSince(statController, std::forward<decltype(args)>(args)...); });
  callbacks.registerCallbackWithSignature<List<JsonArray>>("activeUniqueStatusEffectSummary",
      [statController]() { return StatusControllerCallbacks::activeUniqueStatusEffectSummary(statController); });
  callbacks.registerCallbackWithSignature<bool, String>("uniqueStatusEffectActive",
      [statController](auto&&... args) { return StatusControllerCallbacks::uniqueStatusEffectActive(statController, std::forward<decltype(args)>(args)...); });

  callbacks.registerCallbackWithSignature<Directives>("primaryDirectives", [statController]() { return statController->primaryDirectives(); });
  callbacks.registerCallback("setPrimaryDirectives", [statController](Maybe<String> const& directives) { statController->setPrimaryDirectives(directives.value()); });

  callbacks.registerCallbackWithSignature<void, DamageRequest>("applySelfDamageRequest", [statController](auto&&... args) { return statController->applySelfDamageRequest(std::forward<decltype(args)>(args)...); });

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

Maybe<float> LuaBindings::StatusControllerCallbacks::resourceMax(StatusController* statController, String const& arg1) {
  return statController->resourceMax(arg1);
}

Maybe<float> LuaBindings::StatusControllerCallbacks::resourcePercentage(
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
    StatusController* statController, String const& arg1, Maybe<float> arg2, Maybe<EntityId> sourceEntityId) {
  statController->addEphemeralEffect(EphemeralStatusEffect{UniqueStatusEffect{arg1}, arg2}, sourceEntityId);
}

void LuaBindings::StatusControllerCallbacks::addEphemeralEffects(
    StatusController* statController, JsonArray const& arg1, Maybe<EntityId> sourceEntityId) {
  statController->addEphemeralEffects(arg1.transformed(jsonToEphemeralStatusEffect), sourceEntityId);
}

void LuaBindings::StatusControllerCallbacks::removeEphemeralEffect(
    StatusController* statController, String const& arg1) {
  statController->removeEphemeralEffect(UniqueStatusEffect{arg1});
}

void LuaBindings::StatusControllerCallbacks::clearEphemeralEffects(StatusController* statController) {
  statController->clearEphemeralEffects();
}

LuaTupleReturn<List<Json>, uint64_t> LuaBindings::StatusControllerCallbacks::damageTakenSince(
    StatusController* statController, Maybe<uint64_t> timestep) {
  auto pair = statController->damageTakenSince(timestep.value());
  return luaTupleReturn(pair.first.transformed(mem_fn(&DamageNotification::toJson)), pair.second);
}

LuaTupleReturn<List<Json>, uint64_t> LuaBindings::StatusControllerCallbacks::inflictedHitsSince(
    StatusController* statController, Maybe<uint64_t> timestep) {
  auto pair = statController->inflictedHitsSince(timestep.value());
  return luaTupleReturn(
      pair.first.transformed([](auto const& p) { return p.second.toJson().set("targetEntityId", p.first); }),
      pair.second);
}

LuaTupleReturn<List<Json>, uint64_t> LuaBindings::StatusControllerCallbacks::inflictedDamageSince(
    StatusController* statController, Maybe<uint64_t> timestep) {
  auto pair = statController->inflictedDamageSince(timestep.value());
  return luaTupleReturn(pair.first.transformed(mem_fn(&DamageNotification::toJson)), pair.second);
}

List<JsonArray> LuaBindings::StatusControllerCallbacks::activeUniqueStatusEffectSummary(
    StatusController* statController) {
  auto summary = statController->activeUniqueStatusEffectSummary();
  return summary.transformed([](pair<UniqueStatusEffect, Maybe<float>> effect) {
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

#include "StarStatusControllerLuaBindings.hpp"

#include "StarStatusController.hpp"

import std;

namespace Star {

auto LuaBindings::makeStatusControllerCallbacks(StatusController* statController) -> LuaCallbacks {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Json, String, Json>(
    "statusProperty", [statController](auto&& PH1, auto&& PH2) -> auto { return StatusControllerCallbacks::statusProperty(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, Json>(
    "setStatusProperty", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::setStatusProperty(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<float, String>(
    "stat", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::stat(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, String>(
    "statPositive", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::statPositive(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<StringList>(
    "resourceNames", [statController] -> StringList { return StatusControllerCallbacks::resourceNames(statController); });
  callbacks.registerCallbackWithSignature<bool, String>(
    "isResource", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::isResource(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<float, String>(
    "resource", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::resource(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, String>(
    "resourcePositive", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::resourcePositive(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, float>(
    "setResource", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::setResource(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, float>(
    "modifyResource", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::modifyResource(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<float, String, float>(
    "giveResource", [statController](auto&& PH1, auto&& PH2) -> auto { return StatusControllerCallbacks::giveResource(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, String, float>(
    "consumeResource", [statController](auto&& PH1, auto&& PH2) -> auto { return StatusControllerCallbacks::consumeResource(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, String, float>(
    "overConsumeResource", [statController](auto&& PH1, auto&& PH2) -> auto { return StatusControllerCallbacks::overConsumeResource(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, String>(
    "resourceLocked", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::resourceLocked(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, bool>(
    "setResourceLocked", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::setResourceLocked(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String>(
    "resetResource", [statController](auto&& PH1) -> auto { StatusControllerCallbacks::resetResource(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void>(
    "resetAllResources", [statController] -> void { StatusControllerCallbacks::resetAllResources(statController); });
  callbacks.registerCallbackWithSignature<std::optional<float>, String>(
    "resourceMax", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::resourceMax(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<std::optional<float>, String>(
    "resourcePercentage", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::resourcePercentage(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<float, String, float>(
    "setResourcePercentage", [statController](auto&& PH1, auto&& PH2) -> auto { return StatusControllerCallbacks::setResourcePercentage(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<float, String, float>(
    "modifyResourcePercentage", [statController](auto&& PH1, auto&& PH2) -> auto { return StatusControllerCallbacks::modifyResourcePercentage(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<JsonArray, String>(
    "getPersistentEffects", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::getPersistentEffects(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void, String, Json>(
    "addPersistentEffect", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::addPersistentEffect(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, JsonArray>(
    "addPersistentEffects", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::addPersistentEffects(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String, JsonArray>(
    "setPersistentEffects", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::setPersistentEffects(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String>(
    "clearPersistentEffects", [statController](auto&& PH1) -> auto { StatusControllerCallbacks::clearPersistentEffects(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void>(
    "clearAllPersistentEffects", [statController] -> void { StatusControllerCallbacks::clearAllPersistentEffects(statController); });
  callbacks.registerCallbackWithSignature<void, String, std::optional<float>, std::optional<EntityId>>(
    "addEphemeralEffect", [statController](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { StatusControllerCallbacks::addEphemeralEffect(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<void, JsonArray, std::optional<EntityId>>(
    "addEphemeralEffects", [statController](auto&& PH1, auto&& PH2) -> auto { StatusControllerCallbacks::addEphemeralEffects(statController, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<void, String>(
    "removeEphemeralEffect", [statController](auto&& PH1) -> auto { StatusControllerCallbacks::removeEphemeralEffect(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<void>(
    "clearEphemeralEffects", [statController] -> void { StatusControllerCallbacks::clearEphemeralEffects(statController); });
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
    "damageTakenSince", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::damageTakenSince(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
    "inflictedHitsSince", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::inflictedHitsSince(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<LuaTupleReturn<List<Json>, std::uint64_t>, std::optional<std::uint64_t>>(
    "inflictedDamageSince", [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::inflictedDamageSince(statController, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<List<JsonArray>>("activeUniqueStatusEffectSummary",
                                                           [statController] -> List<JsonArray> { return StatusControllerCallbacks::activeUniqueStatusEffectSummary(statController); });
  callbacks.registerCallbackWithSignature<bool, String>("uniqueStatusEffectActive",
                                                        [statController](auto&& PH1) -> auto { return StatusControllerCallbacks::uniqueStatusEffectActive(statController, std::forward<decltype(PH1)>(PH1)); });

  callbacks.registerCallbackWithSignature<Directives>("primaryDirectives", [statController] -> Directives { return statController->primaryDirectives(); });
  callbacks.registerCallback("setPrimaryDirectives", [statController](std::optional<String> const& directives) -> void { statController->setPrimaryDirectives(directives.value()); });

  callbacks.registerCallbackWithSignature<void, DamageRequest>("applySelfDamageRequest", [statController](auto&& PH1) -> auto { statController->applySelfDamageRequest(std::forward<decltype(PH1)>(PH1)); });

  return callbacks;
}

auto LuaBindings::StatusControllerCallbacks::statusProperty(
  StatusController* statController, String const& arg1, Json const& arg2) -> Json {
  return statController->statusProperty(arg1, arg2);
}

void LuaBindings::StatusControllerCallbacks::setStatusProperty(
  StatusController* statController, String const& arg1, Json const& arg2) {
  statController->setStatusProperty(arg1, arg2);
}

auto LuaBindings::StatusControllerCallbacks::stat(StatusController* statController, String const& arg1) -> float {
  return statController->stat(arg1);
}

auto LuaBindings::StatusControllerCallbacks::statPositive(StatusController* statController, String const& arg1) -> bool {
  return statController->statPositive(arg1);
}

auto LuaBindings::StatusControllerCallbacks::resourceNames(StatusController* statController) -> StringList {
  return statController->resourceNames();
}

auto LuaBindings::StatusControllerCallbacks::isResource(StatusController* statController, String const& arg1) -> bool {
  return statController->isResource(arg1);
}

auto LuaBindings::StatusControllerCallbacks::resource(StatusController* statController, String const& arg1) -> float {
  return statController->resource(arg1);
}

auto LuaBindings::StatusControllerCallbacks::resourcePositive(StatusController* statController, String const& arg1) -> bool {
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

auto LuaBindings::StatusControllerCallbacks::giveResource(
  StatusController* statController, String const& resourceName, float amount) -> float {
  return statController->giveResource(resourceName, amount);
}

auto LuaBindings::StatusControllerCallbacks::consumeResource(
  StatusController* statController, String const& arg1, float arg2) -> bool {
  return statController->consumeResource(arg1, arg2);
}

auto LuaBindings::StatusControllerCallbacks::overConsumeResource(
  StatusController* statController, String const& arg1, float arg2) -> bool {
  return statController->overConsumeResource(arg1, arg2);
}

auto LuaBindings::StatusControllerCallbacks::resourceLocked(StatusController* statController, String const& arg1) -> bool {
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

auto LuaBindings::StatusControllerCallbacks::resourceMax(StatusController* statController, String const& arg1) -> std::optional<float> {
  return statController->resourceMax(arg1);
}

auto LuaBindings::StatusControllerCallbacks::resourcePercentage(
  StatusController* statController, String const& arg1) -> std::optional<float> {
  return statController->resourcePercentage(arg1);
}

auto LuaBindings::StatusControllerCallbacks::setResourcePercentage(
  StatusController* statController, String const& arg1, float arg2) -> float {
  return statController->setResourcePercentage(arg1, arg2);
}

auto LuaBindings::StatusControllerCallbacks::modifyResourcePercentage(
  StatusController* statController, String const& arg1, float arg2) -> float {
  return statController->modifyResourcePercentage(arg1, arg2);
}

auto LuaBindings::StatusControllerCallbacks::getPersistentEffects(
  StatusController* statController, String const& arg1) -> JsonArray {
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
  statController->addEphemeralEffect(EphemeralStatusEffect{.uniqueEffect = UniqueStatusEffect{arg1}, .duration = arg2}, sourceEntityId);
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

auto LuaBindings::StatusControllerCallbacks::damageTakenSince(
  StatusController* statController, std::optional<std::uint64_t> timestep) -> LuaTupleReturn<List<Json>, std::uint64_t> {
  auto pair = statController->damageTakenSince(timestep.value());
  return luaTupleReturn(pair.first.transformed(std::mem_fn(&DamageNotification::toJson)), pair.second);
}

auto LuaBindings::StatusControllerCallbacks::inflictedHitsSince(
  StatusController* statController, std::optional<std::uint64_t> timestep) -> LuaTupleReturn<List<Json>, std::uint64_t> {
  auto pair = statController->inflictedHitsSince(timestep.value());
  return luaTupleReturn(
    pair.first.transformed([](auto const& p) -> auto { return p.second.toJson().set("targetEntityId", p.first); }),
    pair.second);
}

auto LuaBindings::StatusControllerCallbacks::inflictedDamageSince(
  StatusController* statController, std::optional<std::uint64_t> timestep) -> LuaTupleReturn<List<Json>, std::uint64_t> {
  auto pair = statController->inflictedDamageSince(timestep.value());
  return luaTupleReturn(pair.first.transformed(std::mem_fn(&DamageNotification::toJson)), pair.second);
}

auto LuaBindings::StatusControllerCallbacks::activeUniqueStatusEffectSummary(
  StatusController* statController) -> List<JsonArray> {
  auto summary = statController->activeUniqueStatusEffectSummary();
  return summary.transformed([](std::pair<UniqueStatusEffect, std::optional<float>> effect) -> JsonArray {
    JsonArray effectJson = {effect.first};
    if (effect.second)
      effectJson.append(*effect.second);
    return effectJson;
  });
}

auto LuaBindings::StatusControllerCallbacks::uniqueStatusEffectActive(
  StatusController* statController, String const& effectName) -> bool {
  return statController->uniqueStatusEffectActive(effectName);
}

}// namespace Star

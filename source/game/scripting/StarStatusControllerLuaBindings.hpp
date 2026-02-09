#pragma once

#include "StarGameTypes.hpp"
#include "StarLua.hpp"

import std;

namespace Star {

class StatusController;

namespace LuaBindings {
auto makeStatusControllerCallbacks(StatusController* statController) -> LuaCallbacks;

namespace StatusControllerCallbacks {
auto statusProperty(StatusController* statController, String const& arg1, Json const& arg2) -> Json;
void setStatusProperty(StatusController* statController, String const& arg1, Json const& arg2);
auto stat(StatusController* statController, String const& arg1) -> float;
auto statPositive(StatusController* statController, String const& arg1) -> bool;
auto resourceNames(StatusController* statController) -> StringList;
auto isResource(StatusController* statController, String const& arg1) -> bool;
auto resource(StatusController* statController, String const& arg1) -> float;
auto resourcePositive(StatusController* statController, String const& arg1) -> bool;
void setResource(StatusController* statController, String const& arg1, float arg2);
void modifyResource(StatusController* statController, String const& arg1, float arg2);
auto giveResource(StatusController* statController, String const& resourceName, float amount) -> float;
auto consumeResource(StatusController* statController, String const& arg1, float arg2) -> bool;
auto overConsumeResource(StatusController* statController, String const& arg1, float arg2) -> bool;
auto resourceLocked(StatusController* statController, String const& arg1) -> bool;
void setResourceLocked(StatusController* statController, String const& arg1, bool arg2);
void resetResource(StatusController* statController, String const& arg1);
void resetAllResources(StatusController* statController);
auto resourceMax(StatusController* statController, String const& arg1) -> std::optional<float>;
auto resourcePercentage(StatusController* statController, String const& arg1) -> std::optional<float>;
auto setResourcePercentage(StatusController* statController, String const& arg1, float arg2) -> float;
auto modifyResourcePercentage(StatusController* statController, String const& arg1, float arg2) -> float;
auto getPersistentEffects(StatusController* statController, String const& arg1) -> JsonArray;
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
auto damageTakenSince(StatusController* statController, std::optional<std::uint64_t> timestep) -> LuaTupleReturn<List<Json>, std::uint64_t>;
auto inflictedHitsSince(StatusController* statController, std::optional<std::uint64_t> timestep) -> LuaTupleReturn<List<Json>, std::uint64_t>;
auto inflictedDamageSince(
  StatusController* statController, std::optional<std::uint64_t> timestep) -> LuaTupleReturn<List<Json>, std::uint64_t>;
auto activeUniqueStatusEffectSummary(StatusController* statController) -> List<JsonArray>;
auto uniqueStatusEffectActive(StatusController* statController, String const& effectName) -> bool;
}// namespace StatusControllerCallbacks
}// namespace LuaBindings
}// namespace Star

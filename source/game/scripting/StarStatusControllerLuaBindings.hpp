#pragma once

#include "StarLua.hpp"
#include "StarEntity.hpp"

namespace Star {

class StatusController;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeStatusControllerCallbacks(StatusController& statController);

  namespace StatusControllerCallbacks {
    [[nodiscard]] Json statusProperty(StatusController& statController, String const& arg1, Json const& arg2);
    void setStatusProperty(StatusController& statController, String const& arg1, Json const& arg2);
    [[nodiscard]] float stat(StatusController& statController, String const& arg1);
    [[nodiscard]] bool statPositive(StatusController& statController, String const& arg1);
    [[nodiscard]] StringList resourceNames(StatusController& statController);
    [[nodiscard]] bool isResource(StatusController& statController, String const& arg1);
    [[nodiscard]] float resource(StatusController& statController, String const& arg1);
    [[nodiscard]] bool resourcePositive(StatusController& statController, String const& arg1);
    void setResource(StatusController& statController, String const& arg1, float arg2);
    void modifyResource(StatusController& statController, String const& arg1, float arg2);
    [[nodiscard]] float giveResource(StatusController& statController, String const& resourceName, float amount);
    [[nodiscard]] bool consumeResource(StatusController& statController, String const& arg1, float arg2);
    [[nodiscard]] bool overConsumeResource(StatusController& statController, String const& arg1, float arg2);
    [[nodiscard]] bool resourceLocked(StatusController& statController, String const& arg1);
    void setResourceLocked(StatusController& statController, String const& arg1, bool arg2);
    void resetResource(StatusController& statController, String const& arg1);
    void resetAllResources(StatusController& statController);
    [[nodiscard]] Maybe<float> resourceMax(StatusController& statController, String const& arg1);
    [[nodiscard]] Maybe<float> resourcePercentage(StatusController& statController, String const& arg1);
    [[nodiscard]] float setResourcePercentage(StatusController& statController, String const& arg1, float arg2);
    [[nodiscard]] float modifyResourcePercentage(StatusController& statController, String const& arg1, float arg2);
    [[nodiscard]] JsonArray getPersistentEffects(StatusController& statController, String const& arg1);
    void addPersistentEffect(StatusController& statController, String const& arg1, Json const& arg2);
    void addPersistentEffects(StatusController& statController, String const& arg1, JsonArray const& arg2);
    void setPersistentEffects(StatusController& statController, String const& arg1, JsonArray const& arg2);
    void clearPersistentEffects(StatusController& statController, String const& arg1);
    void clearAllPersistentEffects(StatusController& statController);
    void addEphemeralEffect(StatusController& statController,
        String const& uniqueEffect,
        Maybe<float> duration,
        Maybe<EntityId> sourceEntityId);
    void addEphemeralEffects(StatusController& statController, JsonArray const& arg1, Maybe<EntityId> sourceEntityId);
    void removeEphemeralEffect(StatusController& statController, String const& arg1);
    void clearEphemeralEffects(StatusController& statController);
    [[nodiscard]] LuaTupleReturn<List<Json>, uint64_t> damageTakenSince(StatusController& statController, Maybe<uint64_t> timestep);
    [[nodiscard]] LuaTupleReturn<List<Json>, uint64_t> inflictedHitsSince(StatusController& statController, Maybe<uint64_t> timestep);
    LuaTupleReturn<List<Json>, uint64_t> inflictedDamageSince(
        StatusController& statController, Maybe<uint64_t> timestep);
    [[nodiscard]] List<JsonArray> activeUniqueStatusEffectSummary(StatusController& statController);
    [[nodiscard]] bool uniqueStatusEffectActive(StatusController& statController, String const& effectName);
  }
}
}

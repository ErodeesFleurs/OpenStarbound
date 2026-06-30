#pragma once

#include "StarLua.hpp"

namespace Star {

class FireableItem;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeFireableItemCallbacks(FireableItem& fireableItem);

  namespace FireableItemCallbacks {
    void fire(FireableItem& fireableItem, Maybe<String> const& mode);
    void triggerCooldown(FireableItem& fireableItem);
    void setCooldown(FireableItem& fireableItem, float cooldownTime);
    void endCooldown(FireableItem& fireableItem);
    [[nodiscard]] float cooldownTime(FireableItem& fireableItem);
    [[nodiscard]] Json fireableParam(FireableItem& fireableItem, String const& name, Json const& def);
    [[nodiscard]] String fireMode(FireableItem& fireableItem);
    [[nodiscard]] bool ready(FireableItem& fireableItem);
    [[nodiscard]] bool firing(FireableItem& fireableItem);
    [[nodiscard]] bool windingUp(FireableItem& fireableItem);
    [[nodiscard]] bool coolingDown(FireableItem& fireableItem);
    [[nodiscard]] bool ownerFullEnergy(FireableItem& fireableItem);
    [[nodiscard]] bool ownerEnergy(FireableItem& fireableItem);
    [[nodiscard]] bool ownerEnergyLocked(FireableItem& fireableItem);
    [[nodiscard]] bool ownerConsumeEnergy(FireableItem& fireableItem, float energy);
  }
}
}

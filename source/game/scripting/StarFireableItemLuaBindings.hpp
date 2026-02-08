#pragma once

#include "StarLua.hpp"

import std;

namespace Star {

class FireableItem;

namespace LuaBindings {
auto makeFireableItemCallbacks(FireableItem* fireableItem) -> LuaCallbacks;

namespace FireableItemCallbacks {
void fire(FireableItem* fireableItem, std::optional<String> const& mode);
void triggerCooldown(FireableItem* fireableItem);
void setCooldown(FireableItem* fireableItem, float cooldownTime);
void endCooldown(FireableItem* fireableItem);
auto cooldownTime(FireableItem* fireableItem) -> float;
auto fireableParam(FireableItem* fireableItem, String const& name, Json const& def) -> Json;
auto fireMode(FireableItem* fireableItem) -> String;
auto ready(FireableItem* fireableItem) -> bool;
auto firing(FireableItem* fireableItem) -> bool;
auto windingUp(FireableItem* fireableItem) -> bool;
auto coolingDown(FireableItem* fireableItem) -> bool;
auto ownerFullEnergy(FireableItem* fireableItem) -> bool;
auto ownerEnergy(FireableItem* fireableItem) -> bool;
auto ownerEnergyLocked(FireableItem* fireableItem) -> bool;
auto ownerConsumeEnergy(FireableItem* fireableItem, float energy) -> bool;
}// namespace FireableItemCallbacks
}// namespace LuaBindings
}// namespace Star

#pragma once

#include "StarLua.hpp"

import std;

namespace Star {

class Item;

namespace LuaBindings {
auto makeItemCallbacks(Item* item) -> LuaCallbacks;

namespace ItemCallbacks {
auto name(Item* item) -> String;
auto count(Item* item) -> size_t;
auto setCount(Item* item, size_t count) -> size_t;
auto maxStack(Item* item) -> size_t;
auto matches(Item* item, Json const& descriptor, std::optional<bool> exactMatch) -> bool;
auto matchingDescriptors(Item* item) -> Json;
auto consume(Item* item, size_t count) -> bool;
auto empty(Item* item) -> bool;
auto descriptor(Item* item) -> Json;
auto description(Item* item) -> String;
auto friendlyName(Item* item) -> String;
auto rarity(Item* item) -> int;
auto rarityString(Item* item) -> String;
auto price(Item* item) -> size_t;
auto fuelAmount(Item* item) -> unsigned;
auto iconDrawables(Item* item) -> Json;
auto dropDrawables(Item* item) -> Json;
auto largeImage(Item* item) -> String;
auto tooltipKind(Item* item) -> String;
auto category(Item* item) -> String;
auto pickupSound(Item* item) -> String;
auto twoHanded(Item* item) -> bool;
auto timeToLive(Item* item) -> float;
auto learnBlueprintsOnPickup(Item* item) -> Json;
auto hasItemTag(Item* item, String const& itemTag) -> bool;
auto pickupQuestTemplates(Item* item) -> Json;
}// namespace ItemCallbacks
}// namespace LuaBindings
}// namespace Star

#pragma once

#include "StarLua.hpp"
#include "StarVector.hpp"

namespace Star {

class Item;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeItemCallbacks(Item& item);

  namespace ItemCallbacks {
    [[nodiscard]] String name(Item& item);
    [[nodiscard]] size_t count(Item& item);
    [[nodiscard]] size_t setCount(Item& item, size_t count);
    [[nodiscard]] size_t maxStack(Item& item);
    [[nodiscard]] bool matches(Item& item, Json const& descriptor, Maybe<bool> exactMatch);
    [[nodiscard]] Json matchingDescriptors(Item& item);
    [[nodiscard]] bool consume(Item& item, size_t count);
    [[nodiscard]] bool empty(Item& item);
    [[nodiscard]] Json descriptor(Item& item);
    [[nodiscard]] String description(Item& item);
    [[nodiscard]] String friendlyName(Item& item);
    [[nodiscard]] int rarity(Item& item);
    [[nodiscard]] String rarityString(Item& item);
    [[nodiscard]] size_t price(Item& item);
    [[nodiscard]] unsigned fuelAmount(Item& item);
    [[nodiscard]] Json iconDrawables(Item& item);
    [[nodiscard]] Json dropDrawables(Item& item);
    [[nodiscard]] String largeImage(Item& item);
    [[nodiscard]] String tooltipKind(Item& item);
    [[nodiscard]] String category(Item& item);
    [[nodiscard]] String pickupSound(Item& item);
    [[nodiscard]] bool twoHanded(Item& item);
    [[nodiscard]] float timeToLive(Item& item);
    [[nodiscard]] Json learnBlueprintsOnPickup(Item& item);
    [[nodiscard]] bool hasItemTag(Item& item, String const& itemTag);
    [[nodiscard]] Json pickupQuestTemplates(Item& item);
  }
}
}

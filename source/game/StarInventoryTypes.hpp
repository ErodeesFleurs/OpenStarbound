#pragma once

#include "StarAlgorithm.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"

import std;

namespace Star {

enum class EquipmentSlot : std::uint8_t {
  Head = 0,
  Chest = 1,
  Legs = 2,
  Back = 3,
  HeadCosmetic = 4,
  ChestCosmetic = 5,
  LegsCosmetic = 6,
  BackCosmetic = 7,
  Cosmetic1,
  Cosmetic2,
  Cosmetic3,
  Cosmetic4,
  Cosmetic5,
  Cosmetic6,
  Cosmetic7,
  Cosmetic8,
  Cosmetic9,
  Cosmetic10,
  Cosmetic11,
  Cosmetic12
};
extern EnumMap<EquipmentSlot> const EquipmentSlotNames;

using BagSlot = std::pair<String, std::uint8_t>;

using SwapSlot = StrongTypedef<Empty>;
using TrashSlot = StrongTypedef<Empty>;

// Any manageable location in the player inventory can be pointed to by an
// InventorySlot
using InventorySlot = Variant<EquipmentSlot, BagSlot, SwapSlot, TrashSlot>;

auto jsonToInventorySlot(Json const& json) -> InventorySlot;
auto jsonFromInventorySlot(InventorySlot const& slot) -> Json;

auto operator<<(std::ostream& ostream, InventorySlot const& slot) -> std::ostream&;

// Special items in the player inventory that are not generally manageable
enum class EssentialItem : std::uint8_t {
  BeamAxe = 0,
  WireTool = 1,
  PaintTool = 2,
  InspectionTool = 3
};
extern EnumMap<EssentialItem> const EssentialItemNames;

// A player's action bar is a collection of custom item shortcuts, and special
// hard coded shortcuts to the essential items.  There is one location selected
// at a time, which is either an entry on the custom bar, or one of the
// essential items, or nothing.
using CustomBarIndex = std::uint8_t;
using SelectedActionBarLocation = MVariant<CustomBarIndex, EssentialItem>;

auto jsonToSelectedActionBarLocation(Json const& json) -> SelectedActionBarLocation;
auto jsonFromSelectedActionBarLocation(SelectedActionBarLocation const& location) -> Json;

static std::uint8_t const EquipmentSize = 8;
static std::uint8_t const EssentialItemCount = 4;

}// namespace Star

template <>
struct std::formatter<Star::InventorySlot> : Star::ostream_formatter {};

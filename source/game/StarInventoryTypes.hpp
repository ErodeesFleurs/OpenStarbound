#pragma once

#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarStrongTypedef.hpp"

namespace Star {

enum class EquipmentSlot : uint8_t {
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

using BagSlot = pair<String, uint8_t>;

using SwapSlot = StrongTypedef<Empty, struct SwapSlotTag>;
using TrashSlot = StrongTypedef<Empty, struct TrashSlotTag>;

// Any manageable location in the player inventory can be pointed to by an
// InventorySlot
using InventorySlot = Variant<EquipmentSlot, BagSlot, SwapSlot, TrashSlot>;

InventorySlot jsonToInventorySlot(Json const& json);
Json jsonFromInventorySlot(InventorySlot const& slot);

std::ostream& operator<<(std::ostream& ostream, InventorySlot const& slot);

// Special items in the player inventory that are not generally manageable
enum class EssentialItem : uint8_t {
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
using CustomBarIndex = uint8_t;
using SelectedActionBarLocation = MVariant<CustomBarIndex, EssentialItem>;

SelectedActionBarLocation jsonToSelectedActionBarLocation(Json const& json);
Json jsonFromSelectedActionBarLocation(SelectedActionBarLocation const& location);

static uint8_t const EquipmentSize = 8;
static uint8_t const EssentialItemCount = 4;

}

template <> struct std::formatter<Star::InventorySlot> : Star::OstreamFormatter {};

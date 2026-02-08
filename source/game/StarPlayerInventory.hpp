#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarInventoryTypes.hpp"
#include "StarItemDescriptor.hpp"
#include "StarMultiArray.hpp"
#include "StarNetElementBasicFields.hpp"
#include "StarNetElementSyncGroup.hpp"

import std;

namespace Star {

class HeadArmor;
class ChestArmor;
class LegsArmor;
class BackArmor;
class ArmorItem;
class ItemBag;

using InventoryException = ExceptionDerived<"InventoryException">;

// Describes a player's entire inventory, including the main bag, material bag,
// object bag, reagent bag, food bag, weapon and armor slots, swap slot, trash
// slot, essential items, and currencies.
//
// Items in the inventory can be shorcutted in the "Action Bar", and one
// location in the action bar is selected at a time and the primary and
// secondary held items are the items pointed to in that action bar location.
//
// The special slot called the "swap" slot is used specifically for inventory
// management and is attached to the cursor.  When the swap slot is active,
// then whatever is in the slot swap temporarily becomes the only held item.
//
// The essential items are items that are not manageable and not pointable to
// by an ItemSlot, but are part of the action bar shortcut system.  They are
// used for permanent tools that need to be always quickly available.
//
// Currency items that enter the inventory are immediately put in the common currencies
// pool, and are also not manageable items.
class PlayerInventory : public NetElementSyncGroup {
public:
  // Whether the given item is allowed to go in the given slot type
  static auto itemAllowedInBag(Ptr<Item> const& item, String const& bagType) -> bool;
  static auto itemAllowedAsEquipment(Ptr<Item> const& item, EquipmentSlot equipmentSlot) -> bool;

  PlayerInventory();

  auto itemsAt(InventorySlot const& slot) const -> Ptr<Item>;

  // Attempts to combine the items with the given slot, and returns the items
  // left over (if any).
  auto stackWith(InventorySlot const& slot, Ptr<Item> const& items) -> Ptr<Item>;

  // Empty the slot and take what it contains, if any.
  auto takeSlot(InventorySlot const& slot) -> Ptr<Item>;

  // Try to exchange items between any two slots, returns true on success.
  auto exchangeItems(InventorySlot const& first, InventorySlot const& second) -> bool;

  // Forces the given item into the given slot, overriding what was already
  // there.  If the item is not allowed in the given location, does nothing and
  // returns false.
  auto setItem(InventorySlot const& slot, Ptr<Item> const& item) -> bool;

  auto consumeSlot(InventorySlot const& slot, std::uint64_t count = 1) -> bool;

  auto slotValid(InventorySlot const& slot) const -> bool;

  // Adds items to any slot except the trash or swap slots, returns stack left
  // over.
  auto addItems(Ptr<Item> items) -> Ptr<Item>;

  // Adds items to the first matching item bag, avoiding the equipment, swap,
  // or trash slots
  auto addToBags(Ptr<Item> items) -> Ptr<Item>;

  // Returns number of items in the given set that can fit anywhere in any item
  // slot except the trash slot (the number of items that would be added by a
  // call to addItems).
  auto itemsCanFit(Ptr<Item> const& items) const -> std::uint64_t;

  auto hasItem(ItemDescriptor const& descriptor, bool exactMatch = false) const -> bool;
  auto hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch = false) const -> std::uint64_t;

  // Consume items based on ItemDescriptor. Can take from any manageable item slot.
  auto consumeItems(ItemDescriptor const& descriptor, bool exactMatch = false) -> bool;
  auto takeItems(ItemDescriptor const& descriptor, bool takePartial = false, bool exactMatch = false) -> ItemDescriptor;
  // Return a summary of every item that can be consumed by ItemDescriptor.
  auto availableItems() const -> HashMap<ItemDescriptor, std::uint64_t>;

  auto headArmor() const -> Ptr<HeadArmor>;
  auto chestArmor() const -> Ptr<ChestArmor>;
  auto legsArmor() const -> Ptr<LegsArmor>;
  auto backArmor() const -> Ptr<BackArmor>;

  auto headCosmetic() const -> Ptr<HeadArmor>;
  auto chestCosmetic() const -> Ptr<ChestArmor>;
  auto legsCosmetic() const -> Ptr<LegsArmor>;
  auto backCosmetic() const -> Ptr<BackArmor>;

  auto equipment(EquipmentSlot slot, bool testMask = false) const -> Ptr<ArmorItem>;

  auto bagContents(String const& bag) const -> ConstPtr<ItemBag>;

  void condenseBagStacks(String const& bag);

  // Sorting a bag will not change the contents of an action bar location.  It
  // will instead potentially change the pointed to slot of an action bar
  // location to point to the new slot that contains the same item.
  void sortBag(String const& bag);

  // Either move the contents of the given slot into the swap slot, move the
  // contents of the swap slot into the given inventory slot, or swap the
  // contents of the swap slot and the inventory slot, or combine them,
  // whichever makes the most sense.
  void shiftSwap(InventorySlot const& slot);

  // Puts the swap slot back into the inventory, if there is room.  Returns
  // true if this was successful, and the swap slot is now empty.
  auto clearSwap() -> bool;

  auto swapSlotItem() const -> Ptr<Item>;
  void setSwapSlotItem(Ptr<Item> const& items);

  // Non-manageable essential items that are always available as action bar
  // entries.
  auto essentialItem(EssentialItem essentialItem) const -> Ptr<Item>;
  void setEssentialItem(EssentialItem essentialItem, Ptr<Item> item);

  // Non-manageable currencies
  auto availableCurrencies() const -> StringMap<std::uint64_t>;
  auto currency(String const& currencyType) const -> std::uint64_t;
  void addCurrency(String const& currencyType, std::uint64_t amount);
  auto consumeCurrency(String const& currencyType, std::uint64_t amount) -> bool;

  // A custom bar location primary and secondary cannot point to a slot that
  // has no item, and rather than set an empty slot to that location, the slot
  // will simply be cleared.  If a primary slot is set to a two handed item, it
  // will clear the secondary slot.  Any secondary slot that is set must be a
  // one handed item.
  auto customBarPrimarySlot(CustomBarIndex customBarIndex) const -> std::optional<InventorySlot>;
  auto customBarSecondarySlot(CustomBarIndex customBarIndex) const -> std::optional<InventorySlot>;
  void setCustomBarPrimarySlot(CustomBarIndex customBarIndex, std::optional<InventorySlot> slot);
  void setCustomBarSecondarySlot(CustomBarIndex customBarIndex, std::optional<InventorySlot> slot);

  // Add the given slot to a free place in the custom bar if one is available.
  void addToCustomBar(InventorySlot slot);

  // The custom bar has 'CustomBarGroups' groups that can be switched between.
  // This will not change the selected action bar location, but may change the
  // item if the selected location points to the custom bar and the contents
  // change.
  auto customBarGroup() const -> std::uint8_t;
  void setCustomBarGroup(std::uint8_t group);
  auto customBarGroups() const -> std::uint8_t;
  auto customBarIndexes() const -> std::uint8_t;

  // The action bar is the combination of the custom bar and the essential
  // items, and any of these locations can be selected.
  auto selectedActionBarLocation() const -> SelectedActionBarLocation;
  void selectActionBarLocation(SelectedActionBarLocation selectedActionBarLocation);

  // Held items are either the items shortcutted to in the currently selected
  // ActionBar primary / secondary locations, or if the swap slot is non-empty
  // then the swap slot.
  auto primaryHeldItem() const -> Ptr<Item>;
  auto secondaryHeldItem() const -> Ptr<Item>;

  // If the primary / secondary held items are valid manageable slots, returns
  // them.
  auto primaryHeldSlot() const -> std::optional<InventorySlot>;
  auto secondaryHeldSlot() const -> std::optional<InventorySlot>;

  auto pullOverflow() -> List<Ptr<Item>>;
  void setEquipmentVisibility(EquipmentSlot slot, bool visible);
  auto equipmentVisibility(EquipmentSlot slot) const -> bool;

  void load(Json const& store);
  auto store() const -> Json;

  // Loop over every manageable item and potentially mutate it.
  void forEveryItem(std::function<void(InventorySlot const&, Ptr<Item>&)> function);
  // Loop over every manageable item.
  void forEveryItem(std::function<void(InventorySlot const&, Ptr<Item> const&)> function) const;
  // Return every manageable item
  auto allItems() const -> List<Ptr<Item>>;
  // Return summary of every manageable item name and the count of that item
  auto itemSummary() const -> Map<String, std::uint64_t>;

  // Clears away any empty items and sets them as null, and updates action bar
  // slots to maintain the rules for the action bar.  Should be called every
  // tick.
  void cleanup();

private:
  using CustomBarLink = std::pair<std::optional<InventorySlot>, std::optional<InventorySlot>>;

  static auto checkInventoryFilter(Ptr<Item> const& items, String const& filterName) -> bool;

  auto retrieve(InventorySlot const& slot) const -> Ptr<Item> const&;
  auto retrieve(InventorySlot const& slot) -> Ptr<Item>&;

  void swapCustomBarLinks(InventorySlot a, InventorySlot b);
  void autoAddToCustomBar(InventorySlot slot);

  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  Map<EquipmentSlot, Ptr<Item>> m_equipment;
  Map<String, Ptr<ItemBag>> m_bags;
  Ptr<Item> m_swapSlot;
  std::optional<InventorySlot> m_swapReturnSlot;
  Ptr<Item> m_trashSlot;
  Map<EssentialItem, Ptr<Item>> m_essential;
  StringMap<std::uint64_t> m_currencies;
  std::uint8_t m_customBarGroup;
  MultiArray<CustomBarLink, 2> m_customBar;
  SelectedActionBarLocation m_selectedActionBar;

  Map<EquipmentSlot, NetElementData<ItemDescriptor>> m_equipmentNetState;
  Map<String, List<NetElementData<ItemDescriptor>>> m_bagsNetState;
  NetElementData<ItemDescriptor> m_swapSlotNetState;
  NetElementData<ItemDescriptor> m_trashSlotNetState;
  Map<EssentialItem, NetElementData<ItemDescriptor>> m_essentialNetState;
  NetElementData<StringMap<std::uint64_t>> m_currenciesNetState;
  NetElementUInt m_customBarGroupNetState;
  MultiArray<NetElementData<CustomBarLink>, 2> m_customBarNetState;
  NetElementData<SelectedActionBarLocation> m_selectedActionBarNetState;

  List<Ptr<Item>> m_inventoryLoadOverflow;
  unsigned m_equipmentVisibilityMask;
};

}// namespace Star

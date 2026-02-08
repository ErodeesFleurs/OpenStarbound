#pragma once

#include "StarConfig.hpp"
#include "StarItemDescriptor.hpp"
#include "StarMathCommon.hpp"

import std;

namespace Star {

// Manages a collection of items with non-zero counts, and putting them in /
// stacking them / consuming them.  As items are taken out of the ItemBag, any
// Item with a zero count is set to null, so that no ItemPtr returned by this
// class should ever be empty.  They will either be null, or of count >= 1.
// All methods are safe to call with null ItemPtrs.  Any non-const ItemPtr
// given to the ItemBag may be used internally depending on how the item
// stacks, so should not be used after passing to the method.
class ItemBag {
public:
  struct ItemsFitWhereResult {
    std::uint64_t leftover;
    List<std::size_t> slots;
  };

  ItemBag();
  explicit ItemBag(std::size_t size);

  static auto fromJson(Json const& spec) -> ItemBag;
  static auto loadStore(Json const& store) -> ItemBag;

  [[nodiscard]] auto toJson() const -> Json;
  [[nodiscard]] auto diskStore() const -> Json;

  [[nodiscard]] auto size() const -> std::size_t;
  // May reshape the container, but will try not to lose any container
  // contents.  Returns overflow.
  auto resize(std::size_t size) -> List<Ptr<Item>>;
  // Clears all item slots, does not change ItemBag size
  void clearItems();

  // Force a cleanup of any empty items from the ItemBag.  Even though no
  // methods should ever return a null item, it can be usefull to force cleanup
  // to remove empty items from memory.  If any action was done, will return
  // true.
  [[nodiscard]] auto cleanup() const -> bool;

  // Direct access to item list
  auto items() -> List<Ptr<Item>>&;
  [[nodiscard]] auto items() const -> List<Ptr<Item>> const&;

  [[nodiscard]] auto at(std::size_t i) const -> Ptr<Item> const&;
  auto at(std::size_t i) -> Ptr<Item>&;

  // Returns all non-empty items and clears container contents
  auto takeAll() -> List<Ptr<Item>>;

  // Directly set the value of an item at a given slot
  void setItem(std::size_t pos, Ptr<Item> item);

  // Put items into the given slot.  Returns number of items left over
  auto putItems(std::size_t pos, Ptr<Item> items) -> Ptr<Item>;
  // Take a maximum number of items from the given position, defaults to all.
  auto takeItems(std::size_t pos, std::uint64_t count = highest<std::uint64_t>()) -> Ptr<Item>;
  // Put items in the slot by combining, or swap the current items with the
  // given items.
  auto swapItems(std::size_t pos, Ptr<Item> items, bool tryCombine = true) -> Ptr<Item>;

  // Destroys the given number of items, only if the entirety of count is
  // available, returns success.
  auto consumeItems(std::size_t pos, std::uint64_t count) -> bool;

  // Consume any items from any stack that matches the given item descriptor,
  // only if the entirety of the count is available.  Returns success.
  auto consumeItems(ItemDescriptor const& descriptor, bool exactMatch = false) -> bool;

  // Returns the number of times this ItemDescriptor could be consumed using
  // the items in this container.
  [[nodiscard]] auto available(ItemDescriptor const& descriptor, bool exactMatch = false) const -> std::uint64_t;

  // Returns the number of items that can fit anywhere in the bag, including
  // being split up.
  [[nodiscard]] auto itemsCanFit(ConstPtr<Item> const& items) const -> std::uint64_t;
  // Returns the number of items that can be stacked with existing items
  // anywhere in the bag.
  [[nodiscard]] auto itemsCanStack(ConstPtr<Item> const& items) const -> std::uint64_t;

  // Returns where the items would fit if inserted, including any splitting up
  [[nodiscard]] auto itemsFitWhere(ConstPtr<Item> const& items, std::uint64_t max = highest<std::uint64_t>()) const -> ItemsFitWhereResult;

  // Add items anywhere in the bag. Tries to stack items first.  If any items
  // are left over, addItems returns them, otherwise null.
  auto addItems(Ptr<Item> items) -> Ptr<Item>;

  // Add items to the bag, but only if they stack with existing items in the
  // bag.
  auto stackItems(Ptr<Item> items) -> Ptr<Item>;

  // Attempt to condense all stacks in the given bag
  void condenseStacks();

  // Uses ItemDatabase to serialize / deserialize all items
  void read(DataStream& ds);
  void write(DataStream& ds) const;

private:
  // If the from item can stack into the given to item, returns the amount that
  // would be transfered.
  static auto stackTransfer(ConstPtr<Item> const& to, ConstPtr<Item> const& from) -> std::uint64_t;

  // Returns the slot that contains the item already and has the *highest*
  // stack count but not full, or an empty slot, or std::numeric_limits<std::size_t>::max() for no room.
  auto bestSlotAvailable(ConstPtr<Item> const& item, bool stacksOnly, std::function<bool(std::size_t)> test) const -> std::size_t;
  [[nodiscard]] auto bestSlotAvailable(ConstPtr<Item> const& item, bool stacksOnly) const -> std::size_t;

  List<Ptr<Item>> m_items;
};

}// namespace Star

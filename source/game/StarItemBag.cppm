module;
import std;
import star.list;
import star.json;

import star.math_common;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;

namespace Star {

class Item;

// Manages a collection of items with non-zero counts, and putting them in /
// stacking them / consuming them.  As items are taken out of the ItemBag, any
// Item with a zero count is set to null, so no shared item pointer returned by this
// class should ever be empty.  They will either be null, or of count >= 1.
// All methods are safe to call with null item pointers. Any non-const item pointer
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

  static ItemBag fromJson(Json const& spec);
  static ItemBag loadStore(Json const& store);

  Json toJson() const;
  Json diskStore() const;

  std::size_t size() const;
  // May reshape the container, but will try not to lose any container
  // contents.  Returns overflow.
  List<std::shared_ptr<Item>> resize(std::size_t size);
  // Clears all item slots, does not change ItemBag size
  void clearItems();

  // Force a cleanup of any empty items from the ItemBag.  Even though no
  // methods should ever return a null item, it can be usefull to force cleanup
  // to remove empty items from memory.  If any action was done, will return
  // true.
  bool cleanup() const;

  // Direct access to item list
  List<std::shared_ptr<Item>>& items();
  List<std::shared_ptr<Item>> const& items() const;

  std::shared_ptr<Item> const& at(std::size_t i) const;
  std::shared_ptr<Item>& at(std::size_t i);

  // Returns all non-empty items and clears container contents
  List<std::shared_ptr<Item>> takeAll();

  // Directly set the value of an item at a given slot
  void setItem(std::size_t pos, std::shared_ptr<Item> item);

  // Put items into the given slot.  Returns number of items left over
  std::shared_ptr<Item> putItems(std::size_t pos, std::shared_ptr<Item> items);
  // Take a maximum number of items from the given position, defaults to all.
  std::shared_ptr<Item> takeItems(std::size_t pos, std::uint64_t count = highest<std::uint64_t>());
  // Put items in the slot by combining, or swap the current items with the
  // given items.
  std::shared_ptr<Item> swapItems(std::size_t pos, std::shared_ptr<Item> items, bool tryCombine = true);

  // Destroys the given number of items, only if the entirety of count is
  // available, returns success.
  bool consumeItems(std::size_t pos, std::uint64_t count);

  // Consume any items from any stack that matches the given item descriptor,
  // only if the entirety of the count is available.  Returns success.
  bool consumeItems(ItemDescriptor const& descriptor, bool exactMatch = false);

  // Returns the number of times this ItemDescriptor could be consumed using
  // the items in this container.
  std::uint64_t available(ItemDescriptor const& descriptor, bool exactMatch = false) const;

  // Returns the number of items that can fit anywhere in the bag, including
  // being split up.
  std::uint64_t itemsCanFit(std::shared_ptr<Item const> const& items) const;
  // Returns the number of items that can be stacked with existing items
  // anywhere in the bag.
  std::uint64_t itemsCanStack(std::shared_ptr<Item const> const& items) const;

  // Returns where the items would fit if inserted, including any splitting up
  ItemsFitWhereResult itemsFitWhere(std::shared_ptr<Item const> const& items, std::uint64_t max = highest<std::uint64_t>()) const;

  // Add items anywhere in the bag. Tries to stack items first.  If any items
  // are left over, addItems returns them, otherwise null.
  std::shared_ptr<Item> addItems(std::shared_ptr<Item> items);

  // Add items to the bag, but only if they stack with existing items in the
  // bag.
  std::shared_ptr<Item> stackItems(std::shared_ptr<Item> items);

  // Attempt to condense all stacks in the given bag
  void condenseStacks();

  // Uses ItemDatabase to serialize / deserialize all items
  void read(DataStream& ds);
  void write(DataStream& ds) const;

private:
  // If the from item can stack into the given to item, returns the amount that
  // would be transfered.
  static std::uint64_t stackTransfer(std::shared_ptr<Item const> const& to, std::shared_ptr<Item const> const& from);

  // Returns the slot that contains the item already and has the *highest*
  // stack count but not full, or an empty slot, or NPos for no room.
  std::size_t bestSlotAvailable(std::shared_ptr<Item const> const& item, bool stacksOnly, std::function<bool(std::size_t)> test) const;
  std::size_t bestSlotAvailable(std::shared_ptr<Item const> const& item, bool stacksOnly) const;

  List<std::shared_ptr<Item>> m_items;
};

}

export module star.item_bag;

export namespace Star {
using ::Star::Item;
using ::Star::ItemBag;
}

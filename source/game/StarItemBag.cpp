#include "StarItemBag.hpp"
#include "StarConfig.hpp"
#include "StarItemDatabase.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

ItemBag::ItemBag() = default;

ItemBag::ItemBag(size_t size) {
  m_items.resize(size);
}

auto ItemBag::fromJson(Json const& store) -> ItemBag {
  ConstPtr<ItemDatabase> itemDatabase = Root::singleton().itemDatabase();
  ItemBag res;
  res.m_items = store.toArray().transformed([itemDatabase](Json const& v) -> Ptr<Item> { return itemDatabase->fromJson(v); });

  return res;
}

auto ItemBag::loadStore(Json const& store) -> ItemBag {
  auto itemDatabase = Root::singleton().itemDatabase();
  ItemBag res;
  res.m_items = store.toArray().transformed([itemDatabase](Json const& v) -> Ptr<Item> { return itemDatabase->diskLoad(v); });

  return res;
}

auto ItemBag::toJson() const -> Json {
  auto itemDatabase = Root::singleton().itemDatabase();
  return m_items.transformed([itemDatabase](ConstPtr<Item> const& item) -> Json { return itemDatabase->toJson(item); });
}

auto ItemBag::diskStore() const -> Json {
  auto itemDatabase = Root::singleton().itemDatabase();
  return m_items.transformed([itemDatabase](ConstPtr<Item> const& item) -> Json { return itemDatabase->diskStore(item); });
}

auto ItemBag::size() const -> size_t {
  return m_items.size();
}

auto ItemBag::resize(size_t size) -> List<Ptr<Item>> {
  List<Ptr<Item>> lost;
  while (m_items.size() > size) {
    auto lastItem = m_items.takeLast();
    lastItem = addItems(lastItem);
    if (lastItem && !lastItem->empty())
      lost.append(lastItem);
  }

  m_items.resize(size);

  return lost;
}

void ItemBag::clearItems() {
  size_t oldSize = m_items.size();
  m_items.clear();
  m_items.resize(oldSize);
}

auto ItemBag::cleanup() const -> bool {
  bool cleanupDone = false;
  for (auto& items : const_cast<ItemBag*>(this)->m_items) {
    if (items && items->empty()) {
      cleanupDone = true;
      items = {};
    }
  }

  return cleanupDone;
}

auto ItemBag::items() -> List<Ptr<Item>>& {
  // When returning the entire item collection, need to make sure that there
  // are no empty items before returning.
  auto _ = cleanup();

  return m_items;
}

auto ItemBag::items() const -> List<Ptr<Item>> const& {
  return const_cast<ItemBag*>(this)->items();
}

auto ItemBag::at(size_t i) const -> Ptr<Item> const& {
  return const_cast<ItemBag*>(this)->at(i);
}

auto ItemBag::at(size_t i) -> Ptr<Item>& {
  auto& item = m_items.at(i);
  if (item && item->empty())
    item = {};
  return item;
}

auto ItemBag::takeAll() -> List<Ptr<Item>> {
  List<Ptr<Item>> taken;
  for (size_t i = 0; i < size(); ++i) {
    if (auto& item = at(i))
      taken.append(std::move(item));
  }
  return taken;
}

void ItemBag::setItem(size_t pos, Ptr<Item> item) {
  auto& storedItem = at(pos);

  storedItem = item;
}

auto ItemBag::putItems(size_t pos, Ptr<Item> items) -> Ptr<Item> {
  if (!items || items->empty())
    return {};

  auto& storedItem = at(pos);

  if (storedItem) {
    // Try to stack with an item that is already there
    storedItem->stackWith(items);
    if (!items->empty())
      return items;
    else
      return {};
  } else {
    // Otherwise just put the items there and return nothing.
    storedItem = items;

    return {};
  }
}

auto ItemBag::takeItems(size_t pos, uint64_t count) -> Ptr<Item> {
  if (auto& storedItem = at(pos)) {
    auto taken = storedItem->take(count);
    if (storedItem->empty())
      storedItem = {};

    return taken;
  } else {
    return {};
  }
}

auto ItemBag::swapItems(size_t pos, Ptr<Item> items, bool tryCombine) -> Ptr<Item> {
  auto& storedItem = at(pos);

  auto swapItems = items;
  if (!swapItems || swapItems->empty()) {
    // If we are passed in nothing, simply return what's there, if anything.
    swapItems = storedItem;
    storedItem = {};
  } else if (storedItem) {
    // If something is there, try to stack with it first.  If we can't stack,
    // then swap.
    if (!tryCombine || !storedItem->stackWith(swapItems))
      std::swap(storedItem, swapItems);
  } else {
    // Otherwise just place the given items in the slot.
    storedItem = swapItems;
    swapItems = {};
  }

  return swapItems;
}

auto ItemBag::consumeItems(size_t pos, uint64_t count) -> bool {
  bool consumed = false;
  if (auto& storedItem = at(pos)) {
    consumed = storedItem->consume(count);
    if (storedItem->empty())
      storedItem = {};
  }

  return consumed;
}

auto ItemBag::consumeItems(ItemDescriptor const& descriptor, bool exactMatch) -> bool {
  uint64_t countLeft = descriptor.count();
  List<std::pair<size_t, uint64_t>> consumeLocations;
  for (size_t i = 0; i < m_items.size(); ++i) {
    auto& storedItem = at(i);
    if (storedItem && storedItem->matches(descriptor, exactMatch)) {
      uint64_t count = storedItem->count();
      uint64_t take = std::min(count, countLeft);
      consumeLocations.append({i, take});
      countLeft -= take;
      if (countLeft == 0)
        break;
    }
  }

  // Only consume any if we can consume them all
  if (countLeft > 0)
    return false;

  for (auto loc : consumeLocations) {
    bool _ = consumeItems(loc.first, loc.second);
  }

  return true;
}

auto ItemBag::available(ItemDescriptor const& descriptor, bool exactMatch) const -> uint64_t {
  uint64_t count = 0;
  for (auto const& items : m_items) {
    if (items && items->matches(descriptor, exactMatch))
      count += items->count();
  }

  return count / descriptor.count();
}

auto ItemBag::itemsCanFit(ConstPtr<Item> const& items) const -> uint64_t {
  auto itemsFit = itemsFitWhere(items);
  return items->count() - itemsFit.leftover;
}

auto ItemBag::itemsCanStack(ConstPtr<Item> const& items) const -> uint64_t {
  auto itemsFit = itemsFitWhere(items);
  uint64_t stackable = 0;
  for (auto slot : itemsFit.slots)
    if (m_items[slot])
      stackable += stackTransfer(at(slot), items);
  return stackable;
}

auto ItemBag::itemsFitWhere(ConstPtr<Item> const& items, uint64_t max) const -> ItemsFitWhereResult {
  if (!items || items->empty())
    return {};

  List<size_t> slots;
  StableHashSet<size_t> taken;
  uint64_t count = std::min(items->count(), max);

  while (true) {
    if (count == 0)
      break;

    size_t slot = bestSlotAvailable(items, false, [&](size_t i) -> bool {
      return !taken.contains(i);
    });
    if (slot == std::numeric_limits<std::size_t>::max())
      break;
    else {
      slots.append(slot);
      taken.insert(slot);
    }

    uint64_t available = stackTransfer(at(slot), items);
    if (available != 0)
      count -= std::min(available, count);
    else
      break;
  }

  return ItemsFitWhereResult{.leftover = count, .slots = slots};
}

auto ItemBag::addItems(Ptr<Item> items) -> Ptr<Item> {
  if (!items || items->empty())
    return {};

  while (true) {
    size_t slot = bestSlotAvailable(items, false);
    if (slot == std::numeric_limits<std::size_t>::max())
      return items;

    auto& storedItem = at(slot);
    if (storedItem) {
      storedItem->stackWith(items);
      if (items->empty())
        return {};
    } else {
      storedItem = std::move(items);
      return {};
    }
  }
}

auto ItemBag::stackItems(Ptr<Item> items) -> Ptr<Item> {
  if (!items || items->empty())
    return {};

  while (true) {
    size_t slot = bestSlotAvailable(items, true);
    if (slot == std::numeric_limits<std::size_t>::max())
      return items;

    auto& storedItem = at(slot);
    if (storedItem) {
      storedItem->stackWith(items);
      if (items->empty())
        return {};
    } else {
      storedItem = std::move(items);
      return {};
    }
  }
}

void ItemBag::condenseStacks() {
  for (size_t i = size() - 1; i > 0; --i) {
    if (auto& item = at(i)) {
      for (size_t j = 0; j < i; j++) {
        if (auto& stackWithItem = at(j))
          item->stackWith(stackWithItem);
        if (item->empty())
          break;
      }
    }
  }
}

void ItemBag::read(DataStream& ds) {
  auto itemDatabase = Root::singleton().itemDatabase();

  m_items.clear();
  m_items.resize(ds.readVlqU());

  size_t setItemsSize = ds.readVlqU();
  for (size_t i = 0; i < setItemsSize; ++i)
    itemDatabase->loadItem(ds.read<ItemDescriptor>(), at(i));
}

void ItemBag::write(DataStream& ds) const {
  // Try not to write the whole bag if a large part of the end of the bag is
  // empty.

  ds.writeVlqU(m_items.size());

  size_t setItemsSize = 0;
  for (size_t i = 0; i < m_items.size(); ++i) {
    if (at(i))
      setItemsSize = i + 1;
  }

  ds.writeVlqU(setItemsSize);
  for (size_t i = 0; i < setItemsSize; ++i)
    ds.write(itemSafeDescriptor(at(i)));
}

auto ItemBag::stackTransfer(ConstPtr<Item> const& to, ConstPtr<Item> const& from) -> uint64_t {
  if (!from)
    return 0;
  else if (!to)
    return from->count();
  else if (!to->stackableWith(from))
    return 0;
  else
    return std::min(to->maxStack() - to->count(), from->count());
}

auto ItemBag::bestSlotAvailable(ConstPtr<Item> const& item, bool stacksOnly, std::function<bool(size_t)> test) const -> size_t {
  // First look for any slots that can stack, before empty slots.
  for (size_t i = 0; i < m_items.size(); ++i) {
    if (!test(i))
      continue;
    auto const& storedItem = at(i);
    if (storedItem && stackTransfer(storedItem, item) != 0)
      return i;
  }

  if (!stacksOnly) {
    // Then, look for any empty slots.
    for (size_t i = 0; i < m_items.size(); ++i) {
      if (!at(i))
        return i;
    }
  }

  return std::numeric_limits<std::size_t>::max();
}

auto ItemBag::bestSlotAvailable(ConstPtr<Item> const& item, bool stacksOnly) const -> size_t {
  return bestSlotAvailable(item, stacksOnly, [](size_t) -> bool { return true; });
}

}// namespace Star

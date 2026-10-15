import star.memory;
import star.json_extra;
import star.json;
import star.set;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.strong_typedef;
import star.bi_map;
import star.game_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;

import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_descriptor;
import star.item_recipe;
import star.item_descriptor;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;


import star.item_database;
import star.item_bag;

namespace Star {

ItemBag::ItemBag() {}

ItemBag::ItemBag(std::size_t size) {
  m_items.resize(size);
}

ItemBag ItemBag::fromJson(Json const& store) {
  auto itemDatabase = Root::singleton().itemDatabase();
  ItemBag res;
  res.m_items = store.toArray().transformed([itemDatabase](Json const& v) { return itemDatabase->fromJson(v); });

  return res;
}

ItemBag ItemBag::loadStore(Json const& store) {
  auto itemDatabase = Root::singleton().itemDatabase();
  ItemBag res;
  res.m_items = store.toArray().transformed([itemDatabase](Json const& v) { return itemDatabase->diskLoad(v); });

  return res;
}

Json ItemBag::toJson() const {
  auto itemDatabase = Root::singleton().itemDatabase();
  return m_items.transformed([itemDatabase](std::shared_ptr<Item const> const& item) { return itemDatabase->toJson(item); });
}

Json ItemBag::diskStore() const {
  auto itemDatabase = Root::singleton().itemDatabase();
  return m_items.transformed([itemDatabase](std::shared_ptr<Item const> const& item) { return itemDatabase->diskStore(item); });
}

std::size_t ItemBag::size() const {
  return m_items.size();
}

List<std::shared_ptr<Item>> ItemBag::resize(std::size_t size) {
  List<std::shared_ptr<Item>> lost;
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
  std::size_t oldSize = m_items.size();
  m_items.clear();
  m_items.resize(oldSize);
}

bool ItemBag::cleanup() const {
  bool cleanupDone = false;
  for (auto& items : const_cast<ItemBag*>(this)->m_items) {
    if (items && items->empty()) {
      cleanupDone = true;
      items = {};
    }
  }

  return cleanupDone;
}

List<std::shared_ptr<Item>>& ItemBag::items() {
  // When returning the entire item collection, need to make sure that there
  // are no empty items before returning.
  cleanup();

  return m_items;
}

List<std::shared_ptr<Item>> const& ItemBag::items() const {
  return const_cast<ItemBag*>(this)->items();
}

std::shared_ptr<Item> const& ItemBag::at(std::size_t i) const {
  return const_cast<ItemBag*>(this)->at(i);
}

std::shared_ptr<Item>& ItemBag::at(std::size_t i) {
  auto& item = m_items.at(i);
  if (item && item->empty())
    item = {};
  return item;
}

List<std::shared_ptr<Item>> ItemBag::takeAll() {
  List<std::shared_ptr<Item>> taken;
  for (std::size_t i = 0; i < size(); ++i) {
    if (auto& item = at(i))
      taken.append(std::move(item));
  }
  return taken;
}

void ItemBag::setItem(std::size_t pos, std::shared_ptr<Item> item) {
  auto& storedItem = at(pos);

  storedItem = item;
}

std::shared_ptr<Item> ItemBag::putItems(std::size_t pos, std::shared_ptr<Item> items) {
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

std::shared_ptr<Item> ItemBag::takeItems(std::size_t pos, std::uint64_t count) {
  if (auto& storedItem = at(pos)) {
    auto taken = storedItem->take(count);
    if (storedItem->empty())
      storedItem = {};

    return taken;
  } else {
    return {};
  }
}

std::shared_ptr<Item> ItemBag::swapItems(std::size_t pos, std::shared_ptr<Item> items, bool tryCombine) {
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

bool ItemBag::consumeItems(std::size_t pos, std::uint64_t count) {
  bool consumed = false;
  if (auto& storedItem = at(pos)) {
    consumed = storedItem->consume(count);
    if (storedItem->empty())
      storedItem = {};
  }

  return consumed;
}

bool ItemBag::consumeItems(ItemDescriptor const& descriptor, bool exactMatch) {
  std::uint64_t countLeft = descriptor.count();
  List<std::pair<std::size_t, std::uint64_t>> consumeLocations;
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    auto& storedItem = at(i);
    if (storedItem && storedItem->matches(descriptor, exactMatch)) {
      std::uint64_t count = storedItem->count();
      std::uint64_t take = std::min(count, countLeft);
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
    bool res = consumeItems(loc.first, loc.second);
    ((void)(res));
    if constexpr (Star::DebugEnabled) {
      Star::starAssertFailed((res));
    }
  }

  return true;
}

std::uint64_t ItemBag::available(ItemDescriptor const& descriptor, bool exactMatch) const {
  std::uint64_t count = 0;
  for (auto const& items : m_items) {
    if (items && items->matches(descriptor, exactMatch))
      count += items->count();
  }

  return count / descriptor.count();
}

std::uint64_t ItemBag::itemsCanFit(std::shared_ptr<Item const> const& items) const {
  auto itemsFit = itemsFitWhere(items);
  return items->count() - itemsFit.leftover;
}

std::uint64_t ItemBag::itemsCanStack(std::shared_ptr<Item const> const& items) const {
  auto itemsFit = itemsFitWhere(items);
  std::uint64_t stackable = 0;
  for (auto slot : itemsFit.slots)
    if (m_items[slot])
      stackable += stackTransfer(at(slot), items);
  return stackable;
}

auto ItemBag::itemsFitWhere(std::shared_ptr<Item const> const& items, std::uint64_t max) const -> ItemsFitWhereResult {
  if (!items || items->empty())
    return ItemsFitWhereResult();

  List<std::size_t> slots;
  StableHashSet<std::size_t> taken;
  std::uint64_t count = std::min(items->count(), max);

  while (true) {
    if (count == 0)
      break;

    std::size_t slot = bestSlotAvailable(items, false, [&](std::size_t i) {
      return !taken.contains(i);
    });
    if (slot == NPos)
      break;
    else {
      slots.append(slot);
      taken.insert(slot);
    }

    std::uint64_t available = stackTransfer(at(slot), items);
    if (available != 0)
      count -= std::min(available, count);
    else
      break;
  }

  return ItemsFitWhereResult{count, slots};
}

std::shared_ptr<Item> ItemBag::addItems(std::shared_ptr<Item> items) {
  if (!items || items->empty())
    return {};

  while (true) {
    std::size_t slot = bestSlotAvailable(items, false);
    if (slot == NPos)
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

std::shared_ptr<Item> ItemBag::stackItems(std::shared_ptr<Item> items) {
  if (!items || items->empty())
    return {};

  while (true) {
    std::size_t slot = bestSlotAvailable(items, true);
    if (slot == NPos)
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
  for (std::size_t i = size() - 1; i > 0; --i) {
    if (auto& item = at(i)) {
      for (std::size_t j = 0; j < i; j++) {
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

  std::size_t setItemsSize = ds.readVlqU();
  for (std::size_t i = 0; i < setItemsSize; ++i)
    itemDatabase->loadItem(ds.read<ItemDescriptor>(), at(i));
}

void ItemBag::write(DataStream& ds) const {
  // Try not to write the whole bag if a large part of the end of the bag is
  // empty.

  ds.writeVlqU(m_items.size());

  std::size_t setItemsSize = 0;
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    if (at(i))
      setItemsSize = i + 1;
  }

  ds.writeVlqU(setItemsSize);
  for (std::size_t i = 0; i < setItemsSize; ++i)
    ds.write(itemSafeDescriptor(at(i)));
}

std::uint64_t ItemBag::stackTransfer(std::shared_ptr<Item const> const& to, std::shared_ptr<Item const> const& from) {
  if (!from)
    return 0;
  else if (!to)
    return from->count();
  else if (!to->stackableWith(from))
    return 0;
  else
    return std::min(to->maxStack() - to->count(), from->count());
}

std::size_t ItemBag::bestSlotAvailable(std::shared_ptr<Item const> const& item, bool stacksOnly, std::function<bool(std::size_t)> test) const {
  // First look for any slots that can stack, before empty slots.
  for (std::size_t i = 0; i < m_items.size(); ++i) {
    if (!test(i))
      continue;
    auto const& storedItem = at(i);
    if (storedItem && stackTransfer(storedItem, item) != 0)
      return i;
  }

  if (!stacksOnly) {
    // Then, look for any empty slots.
    for (std::size_t i = 0; i < m_items.size(); ++i) {
      if (!at(i))
        return i;
    }
  }

  return NPos;
}

std::size_t ItemBag::bestSlotAvailable(std::shared_ptr<Item const> const& item, bool stacksOnly) const {
  return bestSlotAvailable(item, stacksOnly, [](std::size_t) { return true; });
}

}

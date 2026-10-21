import star.json;
import star.json_extra;
import star.data_stream_extra;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.list;
import star.rpc_promise;
import star.optional;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.strong_typedef;
import star.multi_array;
import star.string;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.set;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;
import star.array;
import star.ordered_set;
import star.audio;
import star.map;
import star.either;
import star.periodic_function;

import star.lua_root;

import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.inventory_types;
import star.asset_source;
import star.assets;
import star.configuration;
import star.root;
import star.status_effect_item;
import star.effect_source_item;
import star.previewable_item;
import star.animation;
import star.particle;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.fireable_item;
import star.swingable_item;
import star.armors;
import star.drawable;
import star.pointable_item;

// Import before Player's global forward declaration so GCC 16 emits the vtable.
import star.player_inventory;

import star.uuid;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.lounging_entities;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inspectable_entity;
import star.ai_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.player_types;
import star.radio_message_database;
import star.player;

import star.currency_item;
import star.material_item;
import star.object_item;
import star.item_bag;


import star.net_element_basic_fields;
import star.net_element_sync_group;

import star.net_compatibility;
import star.math_common;
import star.algorithm;
namespace Star {

bool PlayerInventory::itemAllowedInBag(std::shared_ptr<Item> const& items, String const& bagType) const {
  // any inventory type can have empty slots
  if (!items || items->empty())
    return true;

  return checkInventoryFilter(items, bagType);
}

bool PlayerInventory::itemAllowedAsEquipment(std::shared_ptr<Item> const& item, EquipmentSlot equipmentSlot) {
  // any equipment slot can be empty
  if (!item || item->empty())
    return true;

  if (equipmentSlot == EquipmentSlot::Head || equipmentSlot == EquipmentSlot::HeadCosmetic)
    return is<HeadArmor>(item);
  else if (equipmentSlot == EquipmentSlot::Chest || equipmentSlot == EquipmentSlot::ChestCosmetic)
    return is<ChestArmor>(item);
  else if (equipmentSlot == EquipmentSlot::Legs || equipmentSlot == EquipmentSlot::LegsCosmetic)
    return is<LegsArmor>(item);
  else if (equipmentSlot == EquipmentSlot::Back || equipmentSlot == EquipmentSlot::BackCosmetic)
    return is<BackArmor>(item);
  else
    return is<ArmorItem>(item);
}

PlayerInventory::PlayerInventory(Root& root)
  : PlayerInventory(root, root.assets()->json("/player.config:inventory"), root.assets()->json("/currencies.config")) {}

PlayerInventory::PlayerInventory(Root& root, Json inventoryConfig, Json currenciesConfig)
  : m_root(&root), m_inventoryConfig(std::move(inventoryConfig)), m_currenciesConfig(std::move(currenciesConfig)) {
  auto const& config = m_inventoryConfig;

  auto bags = config.get("itemBags");
  auto bagOrder = bags.toObject().keys().sorted([&bags](String const& a, String const& b) {
    return bags.get(a).getInt("priority", 0) < bags.get(b).getInt("priority", 0);
  });
  for (auto name : bagOrder) {
    std::size_t size = bags.get(name).getUInt("size");
    m_bags[name] = std::make_shared<ItemBag>(root, size);
    m_bagsNetState[name].resize(size);
  }

  for (auto p : m_currenciesConfig.iterateObject())
    m_currencies[p.first] = 0;

  std::size_t customBarGroups = config.getUInt("customBarGroups");
  std::size_t customBarIndexes = config.getUInt("customBarIndexes");
  m_customBarGroup = 0;
  m_customBar.resize(customBarGroups, customBarIndexes);

  for (auto slot : EquipmentSlotNames) {
    auto& element = m_equipmentNetState[slot.first];
    if (slot.first > EquipmentSlot::BackCosmetic)
      element.setCompatibilityVersion(9);
    addNetElement(&element);
  }

  for (auto& p : m_bagsNetState) {
    for (auto& e : p.second)
      addNetElement(&e);
  }

  addNetElement(&m_swapSlotNetState);
  addNetElement(&m_trashSlotNetState);

  addNetElement(&m_currenciesNetState);

  addNetElement(&m_customBarGroupNetState);
  m_customBarNetState.resize(customBarGroups, customBarIndexes);
  m_customBarNetState.forEach([this](Array2S const&, NetElementData<CustomBarLink>& e) {
      addNetElement(&e);
    });

  addNetElement(&m_selectedActionBarNetState);

  addNetElement(&m_essentialNetState[EssentialItem::BeamAxe]);
  addNetElement(&m_essentialNetState[EssentialItem::WireTool]);
  addNetElement(&m_essentialNetState[EssentialItem::PaintTool]);
  addNetElement(&m_essentialNetState[EssentialItem::InspectionTool]);

  m_equipmentVisibilityMask = 0xFFFFFFFF;
  m_player = nullptr;
}

std::shared_ptr<Item> PlayerInventory::itemsAt(InventorySlot const& slot) const {
  return retrieve(slot);
}

std::shared_ptr<Item> PlayerInventory::stackWith(InventorySlot const& slot, std::shared_ptr<Item> const& items) {
  if (!items || items->empty())
    return {};

  if (auto es = std::get_if<EquipmentSlot>(&slot)) {
    auto& itemSlot = retrieveForMutation(slot);
    if (!itemSlot && itemAllowedAsEquipment(items, *es))
      m_equipment[*es] = items->take(1);
  } else {
    auto& dest = retrieveForMutation(slot);
    if (dest && dest->stackableWith(items))
      dest->stackWith(items);
    if (!dest)
      dest = items->take(items->count());
    normalizeItem(dest);
  }

  if (items->empty())
    return {};

  return items;
}

std::shared_ptr<Item> PlayerInventory::takeSlot(InventorySlot const& slot) {
  if (std::holds_alternative<SwapSlot>(slot))
    m_swapReturnSlot = {};
  return take(retrieveForMutation(slot));
}

bool PlayerInventory::exchangeItems(InventorySlot const& first, InventorySlot const& second) {
  auto& firstItems = retrieveForMutation(first);
  auto& secondItems = retrieveForMutation(second);

  if (std::holds_alternative<BagSlot>(first) && !itemAllowedInBag(secondItems, std::get<BagSlot>(first).first))
    return false;
  if (std::holds_alternative<BagSlot>(second) && !itemAllowedInBag(firstItems, std::get<BagSlot>(second).first))
    return false;
  if (std::holds_alternative<EquipmentSlot>(first) && (itemSafeCount(secondItems) > 1 || !itemAllowedAsEquipment(secondItems, std::get<EquipmentSlot>(first))))
    return false;
  if (std::holds_alternative<EquipmentSlot>(second) && (itemSafeCount(firstItems) > 1 || !itemAllowedAsEquipment(firstItems, std::get<EquipmentSlot>(second))))
    return false;

  std::swap(firstItems, secondItems);
  swapCustomBarLinks(first, second);

  return true;
}

bool PlayerInventory::setItem(InventorySlot const& slot, std::shared_ptr<Item> const& item) {
  if (item && item->empty())
    return setItem(slot, {});

  if (auto currencyItem = as<CurrencyItem>(item)) {
    m_currencies[currencyItem->currencyType()] += currencyItem->totalValue();
    return true;
  } else if (auto es = std::get_if<EquipmentSlot>(&slot)) {
    if (itemAllowedAsEquipment(item, *es)) {
      m_equipment[*es] = item;
      return true;
    }
  } else if (std::holds_alternative<SwapSlot>(slot)) {
    m_swapSlot = item;
    return true;
  } else if (std::holds_alternative<TrashSlot>(slot)) {
    m_trashSlot = item;
    return true;
  } else {
    auto bs = std::get<BagSlot>(slot);
    if (itemAllowedInBag(item, bs.first)) {
      m_bags[bs.first]->setItem(bs.second, item);
      return true;
    }
  }

  return false;
}

bool PlayerInventory::consumeSlot(InventorySlot const& slot, std::uint64_t count) {
  if (count == 0)
    return true;

  auto& item = retrieveForMutation(slot);
  if (!item)
    return false;

  bool consumed = item->consume(count);
  normalizeItem(item);

  return consumed;
}

bool PlayerInventory::slotValid(InventorySlot const& slot) const {
  if (auto bagSlot = std::get_if<BagSlot>(&slot)) {
    if (auto bag = bagContents(bagSlot->first)) {
      if ((std::size_t)bagSlot->second >= bag->size())
        return false;
    }
    else return false;
  }
  return true;
}

std::shared_ptr<Item> PlayerInventory::addItems(std::shared_ptr<Item> items) {
  if (!items || items->empty())
    return {};

  // First, add coins as monetary value.
  if (auto currencyItem = as<CurrencyItem>(items)) {
    addCurrency(currencyItem->currencyType(), currencyItem->totalValue());
    return {};
  }

  // Then, try adding equipment to the equipment slots.
  if (is<HeadArmor>(items) && !headArmor())
    m_equipment[EquipmentSlot::Head] = items->take(1);
  if (is<ChestArmor>(items) && !chestArmor())
    m_equipment[EquipmentSlot::Chest] = items->take(1);
  if (is<LegsArmor>(items) && !legsArmor())
    m_equipment[EquipmentSlot::Legs] = items->take(1);
  if (is<BackArmor>(items) && !backArmor())
    m_equipment[EquipmentSlot::Back] = items->take(1);

  if (is<MaterialItem>(items)) {
    if (auto primary = primaryHeldItem()) {
      primary->stackWith(items);
      if (items->empty())
        return {};
    }
  }

  // Then, finally the bags
  return addToBags(std::move(items));
}

std::shared_ptr<Item> PlayerInventory::addToBags(std::shared_ptr<Item> items) {
  if (!items || items->empty())
    return {};

  for (auto pair : m_bags) {
    if (!itemAllowedInBag(items, pair.first))
      continue;

    items = pair.second->stackItems(items);
    if (!items)
      break;

    for (unsigned i = 0; i < pair.second->size(); ++i) {
      if (!pair.second->at(i)) {
        pair.second->setItem(i, take(items));
        autoAddToCustomBar(BagSlot(pair.first, i));
        break;
      }
    }
  }

  return items;
}

std::uint64_t PlayerInventory::itemsCanFit(std::shared_ptr<Item> const& items) const {
  if (!items || items->empty())
    return 0;

  if (is<CurrencyItem>(items))
    return items->count();

  std::uint64_t canFit = 0;

  // First, check the equipment slots
  if (is<HeadArmor>(items) && !headArmor())
    ++canFit;
  if (is<ChestArmor>(items) && !chestArmor())
    ++canFit;
  if (is<LegsArmor>(items) && !legsArmor())
    ++canFit;
  if (is<BackArmor>(items) && !backArmor())
    ++canFit;

  // Then add into bags
  for (auto const& pair : m_bags) {
    if (itemAllowedInBag(items, pair.first))
      canFit += pair.second->itemsCanFit(items);
  }

  return std::min(canFit, items->count());
}

bool PlayerInventory::hasItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  return hasCountOfItem(descriptor, exactMatch) >= descriptor.count();
}

std::uint64_t PlayerInventory::hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  auto one = descriptor.singular();

  std::uint64_t count = 0;
  auto countItem = [&](std::shared_ptr<Item> const& ptr) {
    if (ptr)
      count += ptr->matches(one, exactMatch) ? ptr->count() : 0;
  };

  countItem(m_swapSlot);
  countItem(m_trashSlot);
  for (auto const& p : m_equipment)
    countItem(p.second);

  for (auto const& pair : m_bags)
    count += pair.second->available(one, exactMatch);

  return count;
}

bool PlayerInventory::consumeItems(ItemDescriptor const& descriptor, bool exactMatch) {
  if (descriptor.count() == 0)
    return true;

  auto one = descriptor.singular();

  Map<String, std::uint64_t> consumeFromItemBags;
  for (auto pair : m_bags)
    consumeFromItemBags[pair.first] = pair.second->available(one);

  std::uint64_t consumeFromEquipment = 0;
  for (auto const& p : m_equipment) {
    if (p.second)
      consumeFromEquipment += p.second->matches(one, exactMatch) ? p.second->count() : 0;
  }

  std::uint64_t consumeFromSwap = 0;
  if (m_swapSlot)
    consumeFromSwap += m_swapSlot->matches(one, exactMatch) ? m_swapSlot->count() : 0;

  std::uint64_t consumeFromTrash = 0;
  if (m_trashSlot)
    consumeFromTrash += m_trashSlot->matches(one, exactMatch) ? m_trashSlot->count() : 0;

  auto totalAvailable = consumeFromEquipment + consumeFromSwap + consumeFromTrash;
  for (auto pair : consumeFromItemBags)
    totalAvailable += pair.second;

  if (totalAvailable < descriptor.count())
    return false;

  std::uint64_t leftoverCount = descriptor.count();
  std::uint64_t quantity;
  for (auto pair : m_bags) {
    quantity = std::min(leftoverCount, consumeFromItemBags[pair.first]);
    if (quantity > 0) {
      auto res = pair.second->consumeItems(one.multiply(quantity), exactMatch);
      ((void)(res));
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((res));
      }
      leftoverCount -= quantity;
    }
  }

  quantity = std::min(leftoverCount, consumeFromEquipment);
  if (quantity > 0) {
    auto leftoverQuantity = quantity;
    ((void)(leftoverQuantity));
    for (auto& p : m_equipment) {
      if (p.second && p.second->matches(one, exactMatch)) {
        auto toConsume = std::min(p.second->count(), quantity);
        auto res = p.second->consume(toConsume);
        normalizeItem(p.second);
        ((void)(res));
        if constexpr (Star::DebugEnabled) {
          Star::starAssertFailed((res));
        }

        leftoverQuantity -= toConsume;
      }
    }
    if constexpr (Star::DebugEnabled) {
      Star::starAssertFailed((leftoverQuantity == 0));
    }
    leftoverCount -= quantity;
  }

  quantity = std::min(leftoverCount, consumeFromSwap);
  if (quantity > 0) {
    if (m_swapSlot && m_swapSlot->matches(one, exactMatch)) {
      auto toConsume = std::min(m_swapSlot->count(), quantity);
      auto res = m_swapSlot->consume(toConsume);
      normalizeItem(m_swapSlot);
      ((void)(res));
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((res));
      }

      quantity -= toConsume;
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((quantity == 0));
      }
    }
    leftoverCount -= std::min(leftoverCount, consumeFromSwap);
  }

  quantity = std::min(leftoverCount, consumeFromTrash);
  if (quantity > 0) {
    if (m_trashSlot && m_trashSlot->matches(one, exactMatch)) {
      auto toConsume = std::min(m_trashSlot->count(), quantity);
      auto res = m_trashSlot->consume(toConsume);
      normalizeItem(m_trashSlot);
      ((void)(res));
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((res));
      }

      quantity -= toConsume;
      if constexpr (Star::DebugEnabled) {
        Star::starAssertFailed((quantity == 0));
      }
    }
    leftoverCount -= std::min(leftoverCount, consumeFromTrash);
  }

  if constexpr (Star::DebugEnabled) {
    Star::starAssertFailed((leftoverCount == 0));
  }
  return true;
}

ItemDescriptor PlayerInventory::takeItems(ItemDescriptor const& descriptor, bool takePartial, bool exactMatch) {
  std::uint64_t hasCount = hasCountOfItem(descriptor, exactMatch);

  if (hasCount >= descriptor.count() || (takePartial && hasCount > 0)) {
    ItemDescriptor consumeDescriptor = descriptor.withCount(std::min(descriptor.count(), hasCount));
    consumeItems(consumeDescriptor, exactMatch);
    return consumeDescriptor;
  }

  return {};
}

HashMap<ItemDescriptor, std::uint64_t> PlayerInventory::availableItems() const {
  return ItemDatabase::normalizeBag(allItems());
}

std::shared_ptr<HeadArmor> PlayerInventory::headArmor() const {
  return as<HeadArmor>(retrieve(EquipmentSlot::Head));
}

std::shared_ptr<ChestArmor> PlayerInventory::chestArmor() const {
  return as<ChestArmor>(retrieve(EquipmentSlot::Chest));
}

std::shared_ptr<LegsArmor> PlayerInventory::legsArmor() const {
  return as<LegsArmor>(retrieve(EquipmentSlot::Legs));
}

std::shared_ptr<BackArmor> PlayerInventory::backArmor() const {
  return as<BackArmor>(retrieve(EquipmentSlot::Back));
}

std::shared_ptr<HeadArmor> PlayerInventory::headCosmetic() const {
  return as<HeadArmor>(retrieve(EquipmentSlot::HeadCosmetic));
}

std::shared_ptr<ChestArmor> PlayerInventory::chestCosmetic() const {
  return as<ChestArmor>(retrieve(EquipmentSlot::ChestCosmetic));
}

std::shared_ptr<LegsArmor> PlayerInventory::legsCosmetic() const {
  return as<LegsArmor>(retrieve(EquipmentSlot::LegsCosmetic));
}

std::shared_ptr<BackArmor> PlayerInventory::backCosmetic() const {
  return as<BackArmor>(retrieve(EquipmentSlot::BackCosmetic));
}

std::shared_ptr<ArmorItem> PlayerInventory::equipment(EquipmentSlot slot, bool testMask) const {
  if (testMask && !equipmentVisibility(slot))
    return {};

  return as<ArmorItem>(retrieve(slot));
}

std::shared_ptr<ItemBag const> PlayerInventory::bagContents(String const& type) const {
  if (!m_bags.contains(type)) return nullptr;
  return m_bags.get(type);
}

void PlayerInventory::condenseBagStacks(String const& bagType) {
  auto bag = m_bags[bagType];

  bag->condenseStacks();

  m_customBar.forEach([&](auto const&, CustomBarLink& link) {
      if (link.first) {
        if (auto bs = std::get_if<BagSlot>(&*link.first)) {
          if (bs->first == bagType && !bag->at(bs->second))
            link.first = {};
        }
      }
      if (link.second) {
        if (auto bs = std::get_if<BagSlot>(&*link.second)) {
          if (bs->first == bagType && !bag->at(bs->second))
            link.second = {};
        }
      }
    });
}

void PlayerInventory::sortBag(String const& bagType) {
  auto bag = m_bags[bagType];

  // When sorting bags, we need to record where all the action bar links were
  // pointing if any of them were pointing to the bag we are about to sort.
  MultiArray<std::pair<std::shared_ptr<Item>, std::shared_ptr<Item>>, 2> savedCustomBar(m_customBar.size());
  m_customBar.forEach([&](auto const& index, CustomBarLink const& link) {
      if (link.first) {
        if (auto bs = std::get_if<BagSlot>(&*link.first)) {
          if (bs->first == bagType)
            savedCustomBar(index).first = bag->at(bs->second);
        }
      }
      if (link.second) {
        if (auto bs = std::get_if<BagSlot>(&*link.second)) {
          if (bs->first == bagType)
            savedCustomBar(index).second = bag->at(bs->second);
        }
      }
    });

  auto itemDatabase = m_root->itemDatabase();
  bag->items().sort([itemDatabase](std::shared_ptr<Item> const& a, std::shared_ptr<Item> const& b) {
      if (a && !b)
        return true;
      if (!a)
        return false;

      auto aType = itemDatabase->itemType(a->name());
      auto bType = itemDatabase->itemType(b->name());
      if (aType != bType)
        return aType < bType;

      if (a->rarity() != b->rarity())
        return a->rarity() > b->rarity();

      if (a->name().compare(b->name()) != 0)
        return a->name().compare(b->name()) < 0;

      if (a->count() != b->count())
        return a->count() > b->count();

      return false;
    });

  // Once we are done sorting, we need to restore the potentially action bar
  // links to point to where the item with the same identity is now residing.

  Map<std::shared_ptr<Item>, std::size_t> itemIndexes;
  for (std::size_t i = 0; i < bag->size(); ++i) {
    if (auto item = bag->at(i))
      itemIndexes[item] = i;
  }

  savedCustomBar.forEach([&](auto const& index, auto const& savedItems) {
      if (savedItems.first)
        m_customBar.at(index).first.emplace(BagSlot(bagType, itemIndexes.get(savedItems.first)));
      if (savedItems.second)
        m_customBar.at(index).second.emplace(BagSlot(bagType, itemIndexes.get(savedItems.second)));
    });
}

void PlayerInventory::shiftSwap(InventorySlot const& slot) {
  normalizeItem(m_swapSlot);
  if (auto es = std::get_if<EquipmentSlot>(&slot)) {
    if (itemAllowedAsEquipment(m_swapSlot, *es)) {
      auto& equipSlot = retrieveForMutation(slot);
      if (itemSafeCount(m_swapSlot) <= 1) {
        std::swap(m_swapSlot, equipSlot);
        swapCustomBarLinks(SwapSlot(), slot);
      } else if (itemSafeCount(equipSlot) == 0) {
        equipSlot = m_swapSlot->take(1);
      }
    }
  } else if (std::holds_alternative<TrashSlot>(slot)) {
    normalizeItem(m_trashSlot);
    std::swap(m_swapSlot, m_trashSlot);
    swapCustomBarLinks(SwapSlot(), slot);
  } else if (auto bs = std::get_if<BagSlot>(&slot)) {
    if (itemAllowedInBag(m_swapSlot, bs->first)) {
      m_swapSlot = m_bags[bs->first]->swapItems(bs->second, m_swapSlot);
      swapCustomBarLinks(SwapSlot(), slot);
    }
  }
  normalizeItem(m_swapSlot);

  if (!m_swapSlot)
    m_swapReturnSlot = {};
  else
    m_swapReturnSlot = slot;
}

bool PlayerInventory::clearSwap() {
  normalizeItem(m_swapSlot);
  auto trySlot = [&](InventorySlot slot) {
    if (!m_swapSlot)
      return;

    m_swapSlot = stackWith(slot, m_swapSlot);
    if (!m_swapSlot)
      swapCustomBarLinks(SwapSlot(), slot);
  };

  auto tryBag = [&](String const& bagType) {
    for (unsigned i = 0; i < m_bags[bagType]->size(); ++i) {
      if (!m_swapSlot || !itemAllowedInBag(m_swapSlot, bagType))
        break;
      trySlot(BagSlot(bagType, i));
    }
  };

  if (m_swapReturnSlot)
    trySlot(takeOptional(m_swapReturnSlot));

  trySlot(EquipmentSlot::Head);
  trySlot(EquipmentSlot::Chest);
  trySlot(EquipmentSlot::Legs);
  trySlot(EquipmentSlot::Back);

  for (auto bagType : m_bags.keys())
    tryBag(bagType);

  return !m_swapSlot;
}

std::shared_ptr<Item> PlayerInventory::swapSlotItem() const {
  return retrieve(SwapSlot());
}

void PlayerInventory::setSwapSlotItem(std::shared_ptr<Item> const& items) {
  if (auto currencyItem = as<CurrencyItem>(items)) {
    addCurrency(currencyItem->currencyType(), currencyItem->totalValue());
    m_swapSlot = {};
  } else {
    m_swapSlot = items;
    normalizeItem(m_swapSlot);
    autoAddToCustomBar(SwapSlot());
  }
}

std::shared_ptr<Item> PlayerInventory::essentialItem(EssentialItem essentialItem) const {
  return m_essential.value(essentialItem);
}

void PlayerInventory::setEssentialItem(EssentialItem essentialItem, std::shared_ptr<Item> item) {
  m_essential[essentialItem] = item;
}

StringMap<std::uint64_t> PlayerInventory::availableCurrencies() const {
  return m_currencies;
}

std::uint64_t PlayerInventory::currency(String const& currencyType) const {
  return m_currencies.value(currencyType, 0);
}

void PlayerInventory::addCurrency(String const& currencyType, std::uint64_t amount) {
  std::uint64_t previousTotal = m_currencies[currencyType];
  std::uint64_t newTotal = previousTotal + amount;
  if (newTotal < previousTotal)
    newTotal = std::numeric_limits<std::uint64_t>::max();
  m_currencies[currencyType] = std::min(m_root->assets()->json("/currencies.config").get(currencyType).getUInt("playerMax", std::numeric_limits<std::uint64_t>::max()), newTotal);
}

bool PlayerInventory::consumeCurrency(String const& currencyType, std::uint64_t amount) {
  if (m_currencies[currencyType] >= amount) {
    m_currencies[currencyType] -= amount;
    return true;
  } else {
    return false;
  }
}

std::optional<InventorySlot> PlayerInventory::customBarPrimarySlot(CustomBarIndex customBarIndex) const {
  return m_customBar.at(m_customBarGroup, customBarIndex).first;
}

std::optional<InventorySlot> PlayerInventory::customBarSecondarySlot(CustomBarIndex customBarIndex) const {
  return m_customBar.at(m_customBarGroup, customBarIndex).second;
}

void PlayerInventory::setCustomBarPrimarySlot(CustomBarIndex customBarIndex, std::optional<InventorySlot> slot) {
  // The primary slot is not allowed to point to an empty item.
  if (slot) {
    if (!itemsAt(*slot))
      slot = {};
  }

  auto& cbl = m_customBar.at(m_customBarGroup, customBarIndex);
  if (slot && cbl.second == slot) {
    // If we match the secondary slot, just swap the slots for primary and
    // secondary
    std::swap(cbl.first, cbl.second);
  } else {
    cbl.first = slot;
  }
}

void PlayerInventory::setCustomBarSecondarySlot(CustomBarIndex customBarIndex, std::optional<InventorySlot> slot) {
  auto& cbl = m_customBar.at(m_customBarGroup, customBarIndex);
  // The secondary slot is not allowed to point to an empty item or a two
  // handed item.
  if (slot) {
    if (!itemsAt(*slot) || itemSafeTwoHanded(itemsAt(*slot)))
      slot = {};
  }

  if (cbl.first && cbl.first == slot && !itemSafeTwoHanded(itemsAt(*cbl.first))) {
    // If we match the primary slot and the primary slot is not a two handed
    // item, then just swap the two slots.
    std::swap(cbl.first, cbl.second);
  } else {
    cbl.second = slot;
    // If the primary slot was two handed, it is no longer valid so clear it.
    if (cbl.first && itemSafeTwoHanded(itemsAt(*cbl.first)))
      cbl.first = {};
  }
}

void PlayerInventory::addToCustomBar(InventorySlot slot) {
  for (std::size_t j = 0; j < m_customBar.size(1); ++j) {
    auto& cbl = m_customBar.at(m_customBarGroup, j);
    if (!cbl.first && !cbl.second) {
      cbl.first.emplace(slot);
      break;
    }
  }
}

std::uint8_t PlayerInventory::customBarGroup() const {
  return m_customBarGroup;
}

void PlayerInventory::setCustomBarGroup(std::uint8_t group) {
  m_customBarGroup = group;
}

std::uint8_t PlayerInventory::customBarGroups() const {
  return m_customBar.size(0);
}

std::uint8_t PlayerInventory::customBarIndexes() const {
  return m_customBar.size(1);
}

SelectedActionBarLocation PlayerInventory::selectedActionBarLocation() const {
  return m_selectedActionBar;
}

void PlayerInventory::selectActionBarLocation(SelectedActionBarLocation location) {
  m_selectedActionBar = location;
}

std::shared_ptr<Item> PlayerInventory::primaryHeldItem() const {
  if (auto swapItem = swapSlotItem())
    return swapItem;

  if (std::holds_alternative<EssentialItem>(m_selectedActionBar))
    return m_essential.value(std::get<EssentialItem>(m_selectedActionBar));

  if (std::holds_alternative<CustomBarIndex>(m_selectedActionBar)) {
    if (auto slot = m_customBar.at(m_customBarGroup, std::get<CustomBarIndex>(m_selectedActionBar)).first)
      return itemsAt(*slot);
  }

  return {};
}

std::shared_ptr<Item> PlayerInventory::secondaryHeldItem() const {
  auto pri = primaryHeldItem();
  if (itemSafeTwoHanded(pri) || swapSlotItem() || !std::holds_alternative<CustomBarIndex>(m_selectedActionBar))
    return {};

  auto const& cbl = m_customBar.at(m_customBarGroup, std::get<CustomBarIndex>(m_selectedActionBar));

  if (cbl.first && itemSafeTwoHanded(itemsAt(*cbl.first)))
    return {};

  if (cbl.second)
    return itemsAt(*cbl.second);

  return {};
}

std::optional<InventorySlot> PlayerInventory::primaryHeldSlot() const {
  if (swapSlotItem())
    return InventorySlot(SwapSlot());
  if (std::holds_alternative<CustomBarIndex>(m_selectedActionBar))
    return customBarPrimarySlot(std::get<CustomBarIndex>(m_selectedActionBar));
  return {};
}

std::optional<InventorySlot> PlayerInventory::secondaryHeldSlot() const {
  if (swapSlotItem() || itemSafeTwoHanded(primaryHeldItem()))
    return {};
  if (std::holds_alternative<CustomBarIndex>(m_selectedActionBar))
    return customBarSecondarySlot(std::get<CustomBarIndex>(m_selectedActionBar));
  return {};
}

List<std::shared_ptr<Item>> PlayerInventory::pullOverflow() {
  return std::move(m_inventoryLoadOverflow);
}

bool PlayerInventory::equipmentVisibility(EquipmentSlot slot) const {
  return (m_equipmentVisibilityMask >> (std::uint8_t)slot) & 0x1;
}

void PlayerInventory::setEquipmentVisibility(EquipmentSlot slot, bool visible) {
  if (visible)
    m_equipmentVisibilityMask |= (1 << (std::uint8_t)slot);
  else
    m_equipmentVisibilityMask &= ~(1 << (std::uint8_t)slot);
}

void PlayerInventory::load(Json const& store) {
  auto itemDatabase = m_root->itemDatabase();

  for (auto slot : EquipmentSlotNames) {
    auto jItem = store.get(strf("{}Slot", slot.second), Json());
    m_equipment[slot.first] = itemDatabase->diskLoad(jItem);
    normalizeItem(m_equipment[slot.first]);
  }

  //reuse ItemBags so the Inventory pane still works after load()'ing into the same PlayerInventory again (from swap)
  auto itemBags = store.get("itemBags").toObject();
  m_inventoryLoadOverflow.clear();
  for (auto const& p : itemBags) {
    auto& bagType = p.first;
    auto newBag = ItemBag::loadStore(*m_root, p.second);
    newBag.cleanup();
    if (m_bags.contains(bagType)) {
      auto& bag = m_bags.at(bagType);
      m_inventoryLoadOverflow.appendAll(newBag.resize(bag->size()));
      *bag = std::move(newBag);
    } else {
      m_inventoryLoadOverflow.appendAll(newBag.items());
    }
  }

  m_swapSlot = itemDatabase->diskLoad(store.get("swapSlot"));
  m_trashSlot = itemDatabase->diskLoad(store.get("trashSlot"));
  normalizeItem(m_swapSlot);
  normalizeItem(m_trashSlot);

  m_currencies = jsonToMapV<StringMap<std::uint64_t>>(store.get("currencies"), std::mem_fn(&Json::toUInt));

  m_customBarGroup = store.getUInt("customBarGroup");

  for (std::size_t i = 0; i < m_customBar.size(0); ++i) {
    for (std::size_t j = 0; j < m_customBar.size(1); ++j) {
      Json cbl = store.get("customBar").get(i, JsonArray()).get(j, JsonArray());
      auto validateLink = [this](std::optional<InventorySlot> link) -> std::optional<InventorySlot> {
        if (link && std::holds_alternative<BagSlot>(*link)) {
          auto& slot = std::get<BagSlot>(*link);
          if (m_bags.contains(slot.first) && std::size_t(slot.second) < m_bags[slot.first]->size())
            return link;
          else
            return {};
        }
        return link;
      };
      m_customBar.at(i, j) = CustomBarLink{
        validateLink(jsonToOptional<InventorySlot>(cbl.get(0, {}), jsonToInventorySlot)),
        validateLink(jsonToOptional<InventorySlot>(cbl.get(1, {}), jsonToInventorySlot))
      };
    }
  }

  m_selectedActionBar = jsonToSelectedActionBarLocation(store.get("selectedActionBar"));

  m_essential.clear();
  m_essential[EssentialItem::BeamAxe] = itemDatabase->diskLoad(store.get("beamAxe"));
  m_essential[EssentialItem::WireTool] = itemDatabase->diskLoad(store.get("wireTool"));
  m_essential[EssentialItem::PaintTool] = itemDatabase->diskLoad(store.get("paintTool"));
  m_essential[EssentialItem::InspectionTool] = itemDatabase->diskLoad(store.get("inspectionTool"));

  m_equipmentVisibilityMask = (unsigned)store.optUInt("equipmentVisibilityMask").value_or(0xFFFFFFFF);
}

Json PlayerInventory::store() const {
  auto itemDatabase = m_root->itemDatabase();

  JsonArray customBar;
  for (std::size_t i = 0; i < m_customBar.size(0); ++i) {
    JsonArray customBarGroup;
    for (std::size_t j = 0; j < m_customBar.size(1); ++j) {
      auto const& cbl  = m_customBar.at(i, j);
      customBarGroup.append(JsonArray{jsonFromOptional(cbl.first, jsonFromInventorySlot), jsonFromOptional(cbl.second, jsonFromInventorySlot)});
    }
    customBar.append(take(customBarGroup));
  }

  JsonObject itemBags;
  for (auto& bag : m_bags)
    itemBags.add(bag.first, bag.second->diskStore());

  auto data = JsonObject{
    {"itemBags", itemBags},
    {"swapSlot", itemDatabase->diskStore(m_swapSlot)},
    {"trashSlot", itemDatabase->diskStore(m_trashSlot)},
    {"currencies", jsonFromMap(m_currencies)},
    {"customBarGroup", m_customBarGroup},
    {"customBar", std::move(customBar)},
    {"selectedActionBar", jsonFromSelectedActionBarLocation(m_selectedActionBar)},
    {"beamAxe", itemDatabase->diskStore(m_essential.value(EssentialItem::BeamAxe))},
    {"wireTool", itemDatabase->diskStore(m_essential.value(EssentialItem::WireTool))},
    {"paintTool", itemDatabase->diskStore(m_essential.value(EssentialItem::PaintTool))},
    {"inspectionTool", itemDatabase->diskStore(m_essential.value(EssentialItem::InspectionTool))}
  };

  for (auto& equipment : m_equipment) {
    if (equipment.first <= EquipmentSlot::BackCosmetic || equipment.second)
      data.set(strf("{}Slot", EquipmentSlotNames.getRight(equipment.first)), itemDatabase->diskStore(equipment.second));
  }

  data.set("equipmentVisibilityMask", m_equipmentVisibilityMask);

  return data;
}

void PlayerInventory::forEveryItem(std::function<void(InventorySlot const&, std::shared_ptr<Item>&)> function) {
  auto checkedFunction = [&function](InventorySlot const& slot, std::shared_ptr<Item>& item) {
    normalizeItem(item);
    if (item) {
      function(slot, item);
      normalizeItem(item);
    }
  };

  for (auto& p : m_equipment)
    checkedFunction(p.first, p.second);
  for (auto const& p : m_bags) {
    for (std::size_t i = 0; i < p.second->size(); ++i)
      checkedFunction(BagSlot(p.first, i), p.second->at(i));
  }
  checkedFunction(SwapSlot(), m_swapSlot);
  checkedFunction(TrashSlot(), m_trashSlot);
}

void PlayerInventory::forEveryItem(std::function<void(InventorySlot const&, std::shared_ptr<Item> const&)> function) const {
  auto checkedFunction = [&function](InventorySlot const& slot, std::shared_ptr<Item> const& item) {
    if (item && !item->empty())
      function(slot, item);
  };

  for (auto const& p : m_equipment)
    checkedFunction(p.first, p.second);
  for (auto const& p : m_bags) {
    auto const& bag = std::as_const(*p.second);
    for (std::size_t i = 0; i < bag.size(); ++i)
      checkedFunction(BagSlot(p.first, i), bag.at(i));
  }
  checkedFunction(SwapSlot(), m_swapSlot);
  checkedFunction(TrashSlot(), m_trashSlot);
}

List<std::shared_ptr<Item>> PlayerInventory::allItems() const {
  List<std::shared_ptr<Item>> items;
  forEveryItem([&items](InventorySlot const&, std::shared_ptr<Item> const& item) {
      items.append(item);
    });
  return items;
}

Map<String, std::uint64_t> PlayerInventory::itemSummary() const {
  Map<String, std::uint64_t> result;
  forEveryItem([&result](auto const&, auto const& item) {
      result[item->name()] += item->count();
    });
  return result;
}

void PlayerInventory::cleanup() {
  for (auto pair : m_bags)
    pair.second->cleanup();

  for (auto& p : m_equipment)
    normalizeItem(p.second);

  normalizeItem(m_swapSlot);
  normalizeItem(m_trashSlot);

  m_customBar.forEach([this](Array2S const&, CustomBarLink& p) {
      std::shared_ptr<Item> primary = p.first ? retrieve(*p.first) : std::shared_ptr<Item>();
      std::shared_ptr<Item> secondary = p.second ? retrieve(*p.second) : std::shared_ptr<Item>();

      // Reset the primary and secondary action bar link if the item is gone
      if (!primary)
        p.first.reset();
      if (!secondary)
        p.second.reset();

      // If the primary hand item is two handed, the secondary hand should not be
      // set
      if (itemSafeTwoHanded(primary))
        p.second.reset();
      // Two handed items are not allowed in the secondary slot
      if (itemSafeTwoHanded(secondary))
        p.second.reset();
    });
}

void PlayerInventory::setPlayer(Player* player) {
  m_player = player;
}

PlayerInventory const& PlayerInventory::blankInventory() const {
  if (!m_blankInventory)
    m_blankInventory.reset(new PlayerInventory(*m_root, m_inventoryConfig, m_currenciesConfig));
  return *m_blankInventory;
}

void PlayerInventory::netStore(DataStream& ds, NetCompatibilityRules rules) const {
  if (m_player && !m_player->isMaster() && !rules.isAdmin())
    return blankInventory().netStore(ds, rules);
  else
    return NetElementSyncGroup::netStore(ds, rules);
}


bool PlayerInventory::writeNetDelta(DataStream& ds, std::uint64_t fromVersion, NetCompatibilityRules rules) const {
  if (m_player && !m_player->isMaster() && !rules.isAdmin())
    return blankInventory().writeNetDelta(ds, fromVersion, rules);
  else
    return NetElementSyncGroup::writeNetDelta(ds, fromVersion, rules);
}

bool PlayerInventory::checkInventoryFilter(std::shared_ptr<Item> const& items, String const& filterName) const {
  Json filterConfig;

  auto itemFilters = items->instanceValue("inventoryFilters");
  if (itemFilters.isType(Json::Type::Object)) {
    filterConfig = itemFilters.opt(filterName).value_or(Json{});
    if (!filterConfig.isType(Json::Type::Object))
      filterConfig = itemFilters.opt("default").value_or(Json{});
  }

  if (!filterConfig.isType(Json::Type::Object)) {
    auto config = m_root->assets()->json("/player.config:inventoryFilters");
    filterConfig = config.opt(filterName).value_or(Json{});
    if (!filterConfig.isType(Json::Type::Object))
      filterConfig = config.get("default", JsonObject());
  }

  // filter by item type if an itemTypes filter is set
  auto itemDatabase = m_root->itemDatabase();
  auto itemTypeName = ItemTypeNames.getRight(itemDatabase->itemType(items->name()));
  if (filterConfig.contains("typeWhitelist") && !filterConfig.getArray("typeWhitelist").contains(itemTypeName))
    return false;

  if (filterConfig.contains("typeBlacklist") && filterConfig.getArray("typeBlacklist").contains(itemTypeName))
    return false;

  // filter by item tags if an itemTags filter is set
  // this is an inclusive filter
  auto itemTags = itemDatabase->itemTags(items->name());
  if (filterConfig.contains("tagWhitelist")) {
    auto whitelistedTags = filterConfig.getArray("tagWhitelist").filtered([itemTags](Json const& tag) {
        return itemTags.contains(tag.toString());
      });
    if (whitelistedTags.size() == 0)
      return false;
  }

  if (filterConfig.contains("tagBlacklist")) {
    auto blacklistedTags = filterConfig.getArray("tagBlacklist").filtered([itemTags](Json const& tag) {
        return itemTags.contains(tag.toString());
      });
    if (blacklistedTags.size() > 0)
      return false;
  }

  auto itemCategory = items->category();
  if (auto categoryWhitelist = filterConfig.optArray("categoryWhitelist")) {
    auto categoryWhiteset = jsonToStringSet(*categoryWhitelist);
    if (!categoryWhiteset.contains(itemCategory))
      return false;
  }

  if (auto categoryBlacklist = filterConfig.optArray("categoryBlacklist")) {
    auto categoryBlackset = jsonToStringSet(*categoryBlacklist);
    if (categoryBlackset.contains(itemCategory))
      return false;
  }

  return true;
}

void PlayerInventory::normalizeItem(std::shared_ptr<Item>& item) {
  if (item && item->empty())
    item.reset();
}

std::shared_ptr<Item> PlayerInventory::retrieve(InventorySlot const& slot) const {
  if (auto es = std::get_if<EquipmentSlot>(&slot)) {
    auto const* item = m_equipment.ptr(*es);
    return item && *item && !(*item)->empty() ? *item : std::shared_ptr<Item>();
  } else if (auto bs = std::get_if<BagSlot>(&slot)) {
    if (auto bag = m_bags.ptr(bs->first))
      return std::as_const(**bag).at(bs->second);
  } else {
    auto const& item = std::holds_alternative<SwapSlot>(slot) ? m_swapSlot : m_trashSlot;
    return item && !item->empty() ? item : std::shared_ptr<Item>();
  }

  throw ItemException::format("Invalid inventory slot {}", jsonFromInventorySlot(slot));
}

std::shared_ptr<Item>& PlayerInventory::retrieveForMutation(InventorySlot const& slot) {
  auto normalized = [](std::shared_ptr<Item>& item) -> std::shared_ptr<Item>& {
    normalizeItem(item);
    return item;
  };

  if (auto es = std::get_if<EquipmentSlot>(&slot))
    return normalized(m_equipment[*es]);
  else if (auto bs = std::get_if<BagSlot>(&slot)) {
    if (auto bag = m_bags.ptr(bs->first))
      return normalized((*bag)->at(bs->second));
  } else if (std::holds_alternative<SwapSlot>(slot))
    return normalized(m_swapSlot);
  else
    return normalized(m_trashSlot);

  throw ItemException::format("Invalid inventory slot {}", jsonFromInventorySlot(slot));
}

void PlayerInventory::swapCustomBarLinks(InventorySlot a, InventorySlot b) {
  m_customBar.forEach([&](Array2S const&, CustomBarLink& p) {
      if (p.first == a)
        p.first = b;
      else if (p.first == b)
        p.first = a;

      if (p.second == a)
        p.second = b;
      else if (p.second == b)
        p.second = a;
    });
}

void PlayerInventory::autoAddToCustomBar(InventorySlot slot) {
  if (!m_root->configuration()->getPath("inventory.pickupToActionBar").toBool())
    return;

  auto items = itemsAt(slot);
  if (items && !items->empty() && checkInventoryFilter(items, "autoAddToCustomBar"))
    addToCustomBar(slot);
}

void PlayerInventory::netElementsNeedLoad(bool) {
  auto itemDatabase = m_root->itemDatabase();

  auto deserializeItem = [&itemDatabase](NetElementData<ItemDescriptor>& netState, std::shared_ptr<Item>& item) {
    if (netState.pullUpdated())
      itemDatabase->loadItem(netState.get(), item);
  };

  auto deserializeItemList = [&](List<NetElementData<ItemDescriptor>>& netStatesList, List<std::shared_ptr<Item>>& itemList) {
    for (std::size_t i = 0; i < netStatesList.size(); ++i)
      deserializeItem(netStatesList[i], itemList[i]);
  };

  auto deserializeItemMap = [&](auto& netStatesMap, auto& itemMap) {
    for (auto k : netStatesMap.keys())
      deserializeItem(netStatesMap[k], itemMap[k]);
  };

  deserializeItemMap(m_equipmentNetState, m_equipment);

  for (auto bagType : m_bagsNetState.keys())
    deserializeItemList(m_bagsNetState[bagType], m_bags[bagType]->items());

  deserializeItem(m_swapSlotNetState, m_swapSlot);
  deserializeItem(m_trashSlotNetState, m_trashSlot);

  m_currencies = m_currenciesNetState.get();

  m_customBarGroup = m_customBarGroupNetState.get();
  m_customBarNetState.forEach([&](auto const& index, auto& ns) {
      m_customBar.at(index) = ns.get();
    });

  m_selectedActionBar = m_selectedActionBarNetState.get();

  deserializeItemMap(m_essentialNetState, m_essential);

  cleanup();
}

void PlayerInventory::netElementsNeedStore() {
  cleanup();

  auto serializeItem = [](NetElementData<ItemDescriptor>& netState, std::shared_ptr<Item>& item) {
    netState.set(itemSafeDescriptor(item));
  };

  auto serializeItemList = [&](List<NetElementData<ItemDescriptor>>& netStatesList, List<std::shared_ptr<Item>>& itemList) {
    for (std::size_t i = 0; i < netStatesList.size(); ++i)
      serializeItem(netStatesList[i], itemList[i]);
  };

  auto serializeItemMap = [&](auto& netStatesMap, auto& itemMap) {
    for (auto k : netStatesMap.keys())
      serializeItem(netStatesMap[k], itemMap[k]);
  };

  serializeItemMap(m_equipmentNetState, m_equipment);

  for (auto bagType : m_bagsNetState.keys())
    serializeItemList(m_bagsNetState[bagType], m_bags[bagType]->items());

  serializeItem(m_swapSlotNetState, m_swapSlot);
  serializeItem(m_trashSlotNetState, m_trashSlot);

  m_currenciesNetState.set(m_currencies);

  m_customBarGroupNetState.set(m_customBarGroup);
  m_customBar.forEach([&](auto const& index, auto& cbl) {
      m_customBarNetState.at(index).set(cbl);
    });

  m_selectedActionBarNetState.set(m_selectedActionBar);

  serializeItemMap(m_essentialNetState, m_essential);
}

}

#include "StarPlayerInventory.hpp"

#include <utility>

#include "StarCurrency.hpp"
#include "StarArmors.hpp"
#include "StarLiquidItem.hpp"
#include "StarMaterialItem.hpp"
#include "StarObjectItem.hpp"
#include "StarItemDatabase.hpp"
#include "StarPointableItem.hpp"
#include "StarItemBag.hpp"
#include "StarConfiguration.hpp"
#include "StarJsonExtra.hpp"
#include "StarPlayer.hpp"
#include "StarAlgorithm.hpp"
#include "StarPythonic.hpp"

namespace Star {

namespace {

struct SavedCustomBarItems {
  ItemPtr primary;
  ItemPtr secondary;
};

}

bool PlayerInventory::itemAllowedInBag(ItemPtr const& items, String const& bagType) const {
  // any inventory type can have empty slots
  if (!items)
    return true;

  return checkInventoryFilter(items, bagType);
}

bool PlayerInventory::itemAllowedAsEquipment(ItemPtr const& item, EquipmentSlot equipmentSlot) {
  // any equipment slot can be empty
  if (!item)
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

PlayerInventory::PlayerInventory(AssetsConstPtr assets, ItemDatabaseConstPtr itemDatabase, ConfigurationPtr configuration)
  : m_assets(requireServiceValueAs<InventoryException>(std::move(assets), "PlayerInventory", "assets")),
    m_itemDatabase(requireServiceValueAs<InventoryException>(std::move(itemDatabase), "PlayerInventory", "item database")),
    m_configuration(requireServiceValueAs<InventoryException>(std::move(configuration), "PlayerInventory", "configuration")) {
  auto config = m_assets->json("/player.config:inventory");

  auto bags = config.get("itemBags");
  auto bagOrder = bags.toObject().keys().sorted([&bags](String const& a, String const& b) {
    return bags.get(a).getInt("priority", 0) < bags.get(b).getInt("priority", 0);
  });
  for (auto name : bagOrder) {
    size_t size = bags.get(name).getUInt("size");
    m_bags[name] = make_shared<ItemBag>(size, m_itemDatabase);
    m_bagsNetState[name].resize(size);
  }

  auto currenciesConfig = m_assets->json("/currencies.config");
  for (auto const& [currencyName, currencyConfig] : currenciesConfig.iterateObject())
    m_currencies[currencyName] = 0;

  size_t customBarGroups = config.getUInt("customBarGroups");
  size_t customBarIndexes = config.getUInt("customBarIndexes");
  m_customBarGroup = 0;
  m_customBar.resize(customBarGroups, customBarIndexes);

  for (auto [equipmentSlot, _] : EquipmentSlotNames) {
    auto& element = m_equipmentNetState[equipmentSlot];
    if (equipmentSlot > EquipmentSlot::BackCosmetic)
      element.setCompatibilityVersion(9);
    addNetElement(&element);
  }

  for (auto& bagNetState : m_bagsNetState.values()) {
    for (auto& itemNetState : bagNetState)
      addNetElement(&itemNetState);
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

ItemPtr PlayerInventory::itemsAt(InventorySlot const& slot) const {
  return retrieve(slot);
}

ItemPtr PlayerInventory::stackWith(InventorySlot const& slot, ItemPtr const& items) {
  if (!items || items->empty())
    return {};

  if (auto es = slot.ptr<EquipmentSlot>()) {
    auto& itemSlot = retrieve(slot);
    if (!itemSlot && itemAllowedAsEquipment(items, *es))
      m_equipment[*es] = items->take(1);
  } else {
    auto& dest = retrieve(slot);
    if (dest && dest->stackableWith(items))
      dest->stackWith(items);
    if (!dest)
      dest = items->take(items->count());
  }

  if (items->empty())
    return {};

  return items;
}

ItemPtr PlayerInventory::takeSlot(InventorySlot const& slot) {
  if (slot.is<SwapSlot>())
    m_swapReturnSlot = {};
  return take(retrieve(slot));
}

bool PlayerInventory::exchangeItems(InventorySlot const& first, InventorySlot const& second) {
  auto& firstItems = retrieve(first);
  auto& secondItems = retrieve(second);

  if (first.is<BagSlot>() && !itemAllowedInBag(secondItems, first.get<BagSlot>().first))
    return false;
  if (second.is<BagSlot>() && !itemAllowedInBag(firstItems, second.get<BagSlot>().first))
    return false;
  if (first.is<EquipmentSlot>() && (secondItems->count() > 1 || !itemAllowedAsEquipment(secondItems, first.get<EquipmentSlot>())))
    return false;
  if (second.is<EquipmentSlot>() && (firstItems->count() > 1 || !itemAllowedAsEquipment(firstItems, second.get<EquipmentSlot>())))
    return false;

  swap(firstItems, secondItems);
  swapCustomBarLinks(first, second);

  return true;
}

bool PlayerInventory::setItem(InventorySlot const& slot, ItemPtr const& item) {
  if (auto currencyItem = as<CurrencyItem>(item)) {
    m_currencies[currencyItem->currencyType()] += currencyItem->totalValue();
    return true;
  } else if (auto es = slot.ptr<EquipmentSlot>()) {
    if (itemAllowedAsEquipment(item, *es)) {
      m_equipment[*es] = item;
      return true;
    }
  } else if (slot.is<SwapSlot>()) {
    m_swapSlot = item;
    return true;
  } else if (slot.is<TrashSlot>()) {
    m_trashSlot = item;
    return true;
  } else {
    auto [bagType, bagIndex] = slot.get<BagSlot>();
    if (itemAllowedInBag(item, bagType)) {
      m_bags[bagType]->setItem(bagIndex, item);
      return true;
    }
  }

  return false;
}

bool PlayerInventory::consumeSlot(InventorySlot const& slot, uint64_t count) {
  if (count == 0)
    return true;

  auto& item = retrieve(slot);
  if (!item)
    return false;

  bool consumed = item->consume(count);
  if (item->empty())
    item = {};

  return consumed;
}

bool PlayerInventory::slotValid(InventorySlot const& slot) const {
  if (auto bagSlot = slot.ptr<BagSlot>()) {
    if (auto bag = bagContents(bagSlot->first)) {
      if (static_cast<size_t>(bagSlot->second) >= bag->size())
        return false;
    }
    else return false;
  }
  return true;
}

ItemPtr PlayerInventory::addItems(ItemPtr items) {
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

ItemPtr PlayerInventory::addToBags(ItemPtr items) {
  if (!items || items->empty())
    return {};

  for (auto const& [bagType, bag] : m_bags) {
    if (!itemAllowedInBag(items, bagType))
      continue;

    items = bag->stackItems(items);
    if (!items)
      break;

    for (size_t i = 0; i < bag->size(); ++i) {
      if (!bag->at(i)) {
        bag->setItem(i, take(items));
        autoAddToCustomBar(BagSlot(bagType, i));
        break;
      }
    }
  }

  return items;
}

uint64_t PlayerInventory::itemsCanFit(ItemPtr const& items) const {
  if (!items || items->empty())
    return 0;

  if (is<CurrencyItem>(items))
    return items->count();

  uint64_t canFit = 0;

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
  for (auto const& [bagType, bag] : m_bags) {
    if (itemAllowedInBag(items, bagType))
      canFit += bag->itemsCanFit(items);
  }

  return min(canFit, items->count());
}

bool PlayerInventory::hasItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  return hasCountOfItem(descriptor, exactMatch) >= descriptor.count();
}

uint64_t PlayerInventory::hasCountOfItem(ItemDescriptor const& descriptor, bool exactMatch) const {
  auto one = descriptor.singular();

  uint64_t count = 0;
  auto countItem = [&](ItemPtr const& ptr) {
    if (ptr)
      count += ptr->matches(one, exactMatch) ? ptr->count() : 0;
  };

  countItem(m_swapSlot);
  countItem(m_trashSlot);
  for (auto const& [_, item] : m_equipment)
    countItem(item);

  for (auto const& [_, bag] : m_bags)
    count += bag->available(one, exactMatch);

  return count;
}

bool PlayerInventory::consumeItems(ItemDescriptor const& descriptor, bool exactMatch) {
  if (descriptor.count() == 0)
    return true;

  auto one = descriptor.singular();

  Map<String, uint64_t> consumeFromItemBags;
  for (auto const& [bagType, bag] : m_bags)
    consumeFromItemBags[bagType] = bag->available(one);

  uint64_t consumeFromEquipment = 0;
  for (auto const& [_, item] : m_equipment) {
    if (item)
      consumeFromEquipment += item->matches(one, exactMatch) ? item->count() : 0;
  }

  uint64_t consumeFromSwap = 0;
  if (m_swapSlot)
    consumeFromSwap += m_swapSlot->matches(one, exactMatch) ? m_swapSlot->count() : 0;

  uint64_t consumeFromTrash = 0;
  if (m_trashSlot)
    consumeFromTrash += m_trashSlot->matches(one, exactMatch) ? m_trashSlot->count() : 0;

  auto totalAvailable = consumeFromEquipment + consumeFromSwap + consumeFromTrash;
  for (auto const& [_, available] : consumeFromItemBags)
    totalAvailable += available;

  if (totalAvailable < descriptor.count())
    return false;

  uint64_t leftoverCount = descriptor.count();
  uint64_t quantity;
  for (auto const& [bagType, bag] : m_bags) {
    quantity = min(leftoverCount, consumeFromItemBags[bagType]);
    if (quantity > 0) {
      [[maybe_unused]] auto res = bag->consumeItems(one.multiply(quantity), exactMatch);
      starAssert(res);
      leftoverCount -= quantity;
    }
  }

  quantity = min(leftoverCount, consumeFromEquipment);
  if (quantity > 0) {
    [[maybe_unused]] auto leftoverQuantity = quantity;
    for (auto const& [_, item] : m_equipment) {
      if (item && item->matches(one, exactMatch)) {
        auto toConsume = min(item->count(), quantity);
        [[maybe_unused]] auto res = item->consume(toConsume);
        starAssert(res);

        leftoverQuantity -= toConsume;
      }
    }
    starAssert(leftoverQuantity == 0);
    leftoverCount -= quantity;
  }

  quantity = std::min(leftoverCount, consumeFromSwap);
  if (quantity > 0) {
    if (m_swapSlot && m_swapSlot->matches(one, exactMatch)) {
      auto toConsume = std::min(m_swapSlot->count(), quantity);
      [[maybe_unused]] auto res = m_swapSlot->consume(toConsume);
      starAssert(res);

      quantity -= toConsume;
      starAssert(quantity == 0);
    }
    leftoverCount -= std::min(leftoverCount, consumeFromSwap);
  }

  quantity = std::min(leftoverCount, consumeFromTrash);
  if (quantity > 0) {
    if (m_trashSlot && m_trashSlot->matches(one, exactMatch)) {
      auto toConsume = std::min(m_trashSlot->count(), quantity);
      [[maybe_unused]] auto res = m_trashSlot->consume(toConsume);
      starAssert(res);

      quantity -= toConsume;
      starAssert(quantity == 0);
    }
    leftoverCount -= std::min(leftoverCount, consumeFromTrash);
  }

  starAssert(leftoverCount == 0);
  return true;
}

ItemDescriptor PlayerInventory::takeItems(ItemDescriptor const& descriptor, bool takePartial, bool exactMatch) {
  uint64_t hasCount = hasCountOfItem(descriptor, exactMatch);

  if (hasCount >= descriptor.count() || (takePartial && hasCount > 0)) {
    ItemDescriptor consumeDescriptor = descriptor.withCount(min(descriptor.count(), hasCount));
    consumeItems(consumeDescriptor, exactMatch);
    return consumeDescriptor;
  }

  return {};
}

HashMap<ItemDescriptor, uint64_t> PlayerInventory::availableItems() const {
  return ItemDatabase::normalizeBag(allItems());
}

HeadArmorPtr PlayerInventory::headArmor() const {
  return as<HeadArmor>(m_equipment.value(EquipmentSlot::Head));
}

ChestArmorPtr PlayerInventory::chestArmor() const {
  return as<ChestArmor>(m_equipment.value(EquipmentSlot::Chest));
}

LegsArmorPtr PlayerInventory::legsArmor() const {
  return as<LegsArmor>(m_equipment.value(EquipmentSlot::Legs));
}

BackArmorPtr PlayerInventory::backArmor() const {
  return as<BackArmor>(m_equipment.value(EquipmentSlot::Back));
}

HeadArmorPtr PlayerInventory::headCosmetic() const {
  return as<HeadArmor>(m_equipment.value(EquipmentSlot::HeadCosmetic));
}

ChestArmorPtr PlayerInventory::chestCosmetic() const {
  return as<ChestArmor>(m_equipment.value(EquipmentSlot::ChestCosmetic));
}

LegsArmorPtr PlayerInventory::legsCosmetic() const {
  return as<LegsArmor>(m_equipment.value(EquipmentSlot::LegsCosmetic));
}

BackArmorPtr PlayerInventory::backCosmetic() const {
  return as<BackArmor>(m_equipment.value(EquipmentSlot::BackCosmetic));
}

ArmorItemPtr PlayerInventory::equipment(EquipmentSlot slot, bool testMask) const {
  if (testMask && !equipmentVisibility(slot))
    return {};

  if (auto item = m_equipment.ptr(slot))
    if (auto armor = as<ArmorItem>(*item))
      return armor;

  return {};
}

ItemBagConstPtr PlayerInventory::bagContents(String const& type) const {
  if (!m_bags.contains(type)) return nullptr;
  return m_bags.get(type);
}

void PlayerInventory::condenseBagStacks(String const& bagType) {
  auto bag = m_bags[bagType];

  bag->condenseStacks();

  m_customBar.forEach([&](auto const&, CustomBarLink& link) {
      auto& [primaryLink, secondaryLink] = link;
      if (primaryLink) {
        if (auto bs = primaryLink->ptr<BagSlot>()) {
          if (bs->first == bagType && !bag->at(bs->second))
            primaryLink = {};
        }
      }
      if (secondaryLink) {
        if (auto bs = secondaryLink->ptr<BagSlot>()) {
          if (bs->first == bagType && !bag->at(bs->second))
            secondaryLink = {};
        }
      }
    });
}

void PlayerInventory::sortBag(String const& bagType) {
  auto bag = m_bags[bagType];

  // When sorting bags, we need to record where all the action bar links were
  // pointing if any of them were pointing to the bag we are about to sort.
  MultiArray<SavedCustomBarItems, 2> savedCustomBar(m_customBar.size());
  m_customBar.forEach([&](auto const& index, CustomBarLink const& link) {
      auto const& [primaryLink, secondaryLink] = link;
      if (primaryLink) {
        if (auto bs = primaryLink->ptr<BagSlot>()) {
          if (bs->first == bagType)
            savedCustomBar(index).primary = bag->at(bs->second);
        }
      }
      if (secondaryLink) {
        if (auto bs = secondaryLink->ptr<BagSlot>()) {
          if (bs->first == bagType)
            savedCustomBar(index).secondary = bag->at(bs->second);
        }
      }
    });

  bag->items().sort([this](ItemPtr const& a, ItemPtr const& b) {
      if (a && !b)
        return true;
      if (!a)
        return false;

      auto aType = m_itemDatabase->itemType(a->name());
      auto bType = m_itemDatabase->itemType(b->name());
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

  Map<ItemPtr, size_t> itemIndexes;
  for (size_t i = 0; i < bag->size(); ++i) {
    if (auto item = bag->at(i))
      itemIndexes[item] = i;
  }

  savedCustomBar.forEach([&](auto const& index, SavedCustomBarItems const& savedItems) {
      auto& [primaryLink, secondaryLink] = m_customBar.at(index);
      if (savedItems.primary)
        primaryLink.set(BagSlot(bagType, itemIndexes.get(savedItems.primary)));
      if (savedItems.secondary)
        secondaryLink.set(BagSlot(bagType, itemIndexes.get(savedItems.secondary)));
    });
}

void PlayerInventory::shiftSwap(InventorySlot const& slot) {
  if (auto es = slot.ptr<EquipmentSlot>()) {
    if (itemAllowedAsEquipment(m_swapSlot, *es)) {
      auto& equipSlot = m_equipment[*es];
      if (itemSafeCount(m_swapSlot) <= 1) {
        swap(m_swapSlot, equipSlot);
        swapCustomBarLinks(SwapSlot(), slot);
      } else if (itemSafeCount(equipSlot) == 0) {
        equipSlot = m_swapSlot->take(1);
      }
    }
  } else if (slot.is<TrashSlot>()) {
    swap(m_swapSlot, m_trashSlot);
    swapCustomBarLinks(SwapSlot(), slot);
  } else if (auto bs = slot.ptr<BagSlot>()) {
    if (itemAllowedInBag(m_swapSlot, bs->first)) {
      m_swapSlot = m_bags[bs->first]->swapItems(bs->second, m_swapSlot);
      swapCustomBarLinks(SwapSlot(), slot);
    }
  }

  if (!m_swapSlot)
    m_swapReturnSlot = {};
  else
    m_swapReturnSlot = slot;
}

bool PlayerInventory::clearSwap() {
  auto trySlot = [&](InventorySlot slot) {
    if (!m_swapSlot)
      return;

    m_swapSlot = stackWith(slot, m_swapSlot);
    if (!m_swapSlot)
      swapCustomBarLinks(SwapSlot(), slot);
  };

  auto tryBag = [&](String const& bagType) {
    for (size_t i = 0; i < m_bags[bagType]->size(); ++i) {
      if (!m_swapSlot || !itemAllowedInBag(m_swapSlot, bagType))
        break;
      trySlot(BagSlot(bagType, i));
    }
  };

  if (m_swapReturnSlot)
    trySlot(m_swapReturnSlot.take());

  trySlot(EquipmentSlot::Head);
  trySlot(EquipmentSlot::Chest);
  trySlot(EquipmentSlot::Legs);
  trySlot(EquipmentSlot::Back);

  for (auto const& bagType : m_bags.keys())
    tryBag(bagType);

  return !m_swapSlot;
}

ItemPtr PlayerInventory::swapSlotItem() const {
  return m_swapSlot;
}

void PlayerInventory::setSwapSlotItem(ItemPtr const& items) {
  if (auto currencyItem = as<CurrencyItem>(items)) {
    addCurrency(currencyItem->currencyType(), currencyItem->totalValue());
    m_swapSlot = {};
  } else {
    m_swapSlot = items;
    autoAddToCustomBar(SwapSlot());
  }
}

ItemPtr PlayerInventory::essentialItem(EssentialItem essentialItem) const {
  return m_essential.value(essentialItem);
}

void PlayerInventory::setEssentialItem(EssentialItem essentialItem, ItemPtr item) {
  m_essential[essentialItem] = item;
}

StringMap<uint64_t> PlayerInventory::availableCurrencies() const {
  return m_currencies;
}

uint64_t PlayerInventory::currency(String const& currencyType) const {
  return m_currencies.value(currencyType, 0);
}

void PlayerInventory::addCurrency(String const& currencyType, uint64_t amount) {
  uint64_t previousTotal = m_currencies[currencyType];
  uint64_t newTotal = previousTotal + amount;
  if (newTotal < previousTotal)
    newTotal = highest<uint64_t>();
  m_currencies[currencyType] = min(m_assets->json("/currencies.config").get(currencyType).getUInt("playerMax", highest<uint64_t>()), newTotal);
}

bool PlayerInventory::consumeCurrency(String const& currencyType, uint64_t amount) {
  if (m_currencies[currencyType] >= amount) {
    m_currencies[currencyType] -= amount;
    return true;
  } else {
    return false;
  }
}

Maybe<InventorySlot> PlayerInventory::customBarPrimarySlot(CustomBarIndex customBarIndex) const {
  return m_customBar.at(m_customBarGroup, customBarIndex).first;
}

Maybe<InventorySlot> PlayerInventory::customBarSecondarySlot(CustomBarIndex customBarIndex) const {
  return m_customBar.at(m_customBarGroup, customBarIndex).second;
}

void PlayerInventory::setCustomBarPrimarySlot(CustomBarIndex customBarIndex, Maybe<InventorySlot> slot) {
  // The primary slot is not allowed to point to an empty item.
  if (slot) {
    if (!itemsAt(*slot))
      slot = {};
  }

  auto& [primaryLink, secondaryLink] = m_customBar.at(m_customBarGroup, customBarIndex);
  if (slot && secondaryLink == slot) {
    // If we match the secondary slot, just swap the slots for primary and
    // secondary
    swap(primaryLink, secondaryLink);
  } else {
    primaryLink = slot;
  }
}

void PlayerInventory::setCustomBarSecondarySlot(CustomBarIndex customBarIndex, Maybe<InventorySlot> slot) {
  auto& [primaryLink, secondaryLink] = m_customBar.at(m_customBarGroup, customBarIndex);
  // The secondary slot is not allowed to point to an empty item or a two
  // handed item.
  if (slot) {
    if (!itemsAt(*slot) || itemSafeTwoHanded(itemsAt(*slot)))
      slot = {};
  }

  if (primaryLink && primaryLink == slot && !itemSafeTwoHanded(itemsAt(*primaryLink))) {
    // If we match the primary slot and the primary slot is not a two handed
    // item, then just swap the two slots.
    swap(primaryLink, secondaryLink);
  } else {
    secondaryLink = slot;
    // If the primary slot was two handed, it is no longer valid so clear it.
    if (primaryLink && itemSafeTwoHanded(itemsAt(*primaryLink)))
      primaryLink = {};
  }
}

void PlayerInventory::addToCustomBar(InventorySlot slot) {
  for (size_t j = 0; j < m_customBar.size(1); ++j) {
    auto& [primaryLink, secondaryLink] = m_customBar.at(m_customBarGroup, j);
    if (!primaryLink && !secondaryLink) {
      primaryLink.set(slot);
      break;
    }
  }
}

uint8_t PlayerInventory::customBarGroup() const {
  return m_customBarGroup;
}

void PlayerInventory::setCustomBarGroup(uint8_t group) {
  m_customBarGroup = group;
}

uint8_t PlayerInventory::customBarGroups() const {
  return m_customBar.size(0);
}

uint8_t PlayerInventory::customBarIndexes() const {
  return m_customBar.size(1);
}

SelectedActionBarLocation PlayerInventory::selectedActionBarLocation() const {
  return m_selectedActionBar;
}

void PlayerInventory::selectActionBarLocation(SelectedActionBarLocation location) {
  m_selectedActionBar = location;
}

ItemPtr PlayerInventory::primaryHeldItem() const {
  if (m_swapSlot)
    return m_swapSlot;

  if (m_selectedActionBar.is<EssentialItem>())
    return m_essential.value(m_selectedActionBar.get<EssentialItem>());

  if (m_selectedActionBar.is<CustomBarIndex>()) {
    auto const& primaryLink = m_customBar.at(m_customBarGroup, m_selectedActionBar.get<CustomBarIndex>()).first;
    if (auto slot = primaryLink)
      return itemsAt(*slot);
  }

  return {};
}

ItemPtr PlayerInventory::secondaryHeldItem() const {
  auto pri = primaryHeldItem();
  if (itemSafeTwoHanded(pri) || m_swapSlot || !m_selectedActionBar || m_selectedActionBar.is<EssentialItem>())
    return {};

  auto const& [primaryLink, secondaryLink] = m_customBar.at(m_customBarGroup, m_selectedActionBar.get<CustomBarIndex>());

  if (primaryLink && itemSafeTwoHanded(itemsAt(*primaryLink)))
    return {};

  if (secondaryLink)
    return itemsAt(*secondaryLink);

  return {};
}

Maybe<InventorySlot> PlayerInventory::primaryHeldSlot() const {
  if (m_swapSlot)
    return InventorySlot(SwapSlot());
  if (m_selectedActionBar.is<CustomBarIndex>())
    return customBarPrimarySlot(m_selectedActionBar.get<CustomBarIndex>());
  return {};
}

Maybe<InventorySlot> PlayerInventory::secondaryHeldSlot() const {
  if (m_swapSlot || itemSafeTwoHanded(primaryHeldItem()))
    return {};
  if (m_selectedActionBar.is<CustomBarIndex>())
    return customBarSecondarySlot(m_selectedActionBar.get<CustomBarIndex>());
  return {};
}

List<ItemPtr> PlayerInventory::pullOverflow() {
  return std::exchange(m_inventoryLoadOverflow, {});
}

bool PlayerInventory::equipmentVisibility(EquipmentSlot slot) const {
  return (m_equipmentVisibilityMask >> static_cast<uint8_t>(slot)) & 0x1;
}

void PlayerInventory::setEquipmentVisibility(EquipmentSlot slot, bool visible) {
  if (visible)
    m_equipmentVisibilityMask |= (1u << static_cast<uint8_t>(slot));
  else
    m_equipmentVisibilityMask &= ~(1u << static_cast<uint8_t>(slot));
}

void PlayerInventory::load(Json const& store) {
  for (auto [equipmentSlot, equipmentSlotName] : EquipmentSlotNames) {
    auto jItem = store.get(strf("{}Slot", equipmentSlotName), Json());
    m_equipment[equipmentSlot] = m_itemDatabase->diskLoad(jItem);
  }

  //reuse ItemBags so the Inventory pane still works after load()'ing into the same PlayerInventory again (from swap)
  auto itemBags = store.get("itemBags").toObject();
  m_inventoryLoadOverflow.clear();
  for (auto const& [bagType, bagStore] : itemBags) {
    auto newBag = ItemBag::loadStore(bagStore, m_itemDatabase);
    if (m_bags.contains(bagType)) {
      auto& bag = m_bags.at(bagType);
      m_inventoryLoadOverflow.appendAll(newBag.resize(bag->size()));
      *bag = std::move(newBag);
    } else {
      m_inventoryLoadOverflow.appendAll(newBag.items());
    }
  }

  m_swapSlot = m_itemDatabase->diskLoad(store.get("swapSlot"));
  m_trashSlot = m_itemDatabase->diskLoad(store.get("trashSlot"));

  m_currencies = jsonToMapV<StringMap<uint64_t>>(store.get("currencies"), mem_fn(&Json::toUInt));

  m_customBarGroup = store.getUInt("customBarGroup");

  for (size_t i = 0; i < m_customBar.size(0); ++i) {
    for (size_t j = 0; j < m_customBar.size(1); ++j) {
      Json cbl = store.get("customBar").get(i, JsonArray()).get(j, JsonArray());
      auto validateLink = [this](Maybe<InventorySlot> link) -> Maybe<InventorySlot> {
        if (link && link->is<BagSlot>()) {
          auto const& [bagType, bagIndex] = link->get<BagSlot>();
          if (m_bags.contains(bagType) && size_t(bagIndex) < m_bags[bagType]->size())
            return link;
          else
            return {};
        }
        return link;
      };
      m_customBar.at(i, j) = CustomBarLink{
        validateLink(jsonToMaybe<InventorySlot>(cbl.get(0, {}), jsonToInventorySlot)),
        validateLink(jsonToMaybe<InventorySlot>(cbl.get(1, {}), jsonToInventorySlot))
      };
    }
  }

  m_selectedActionBar = jsonToSelectedActionBarLocation(store.get("selectedActionBar"));

  m_essential.clear();
  m_essential[EssentialItem::BeamAxe] = m_itemDatabase->diskLoad(store.get("beamAxe"));
  m_essential[EssentialItem::WireTool] = m_itemDatabase->diskLoad(store.get("wireTool"));
  m_essential[EssentialItem::PaintTool] = m_itemDatabase->diskLoad(store.get("paintTool"));
  m_essential[EssentialItem::InspectionTool] = m_itemDatabase->diskLoad(store.get("inspectionTool"));

  m_equipmentVisibilityMask = static_cast<unsigned>(store.optUInt("equipmentVisibilityMask").value(0xFFFFFFFF));
}

Json PlayerInventory::store() const {
  JsonArray customBar;
  for (size_t i = 0; i < m_customBar.size(0); ++i) {
    JsonArray customBarGroup;
    for (size_t j = 0; j < m_customBar.size(1); ++j) {
      auto const& [primaryLink, secondaryLink] = m_customBar.at(i, j);
      customBarGroup.append(JsonArray{jsonFromMaybe(primaryLink, jsonFromInventorySlot), jsonFromMaybe(secondaryLink, jsonFromInventorySlot)});
    }
    customBar.append(take(customBarGroup));
  }

  JsonObject itemBags;
  for (auto& [bagType, bag] : m_bags)
    itemBags.add(bagType, bag->diskStore());

  auto data = JsonObject{
    {"itemBags", itemBags},
    {"swapSlot", m_itemDatabase->diskStore(m_swapSlot)},
    {"trashSlot", m_itemDatabase->diskStore(m_trashSlot)},
    {"currencies", jsonFromMap(m_currencies)},
    {"customBarGroup", m_customBarGroup},
    {"customBar", std::move(customBar)},
    {"selectedActionBar", jsonFromSelectedActionBarLocation(m_selectedActionBar)},
    {"beamAxe", m_itemDatabase->diskStore(m_essential.value(EssentialItem::BeamAxe))},
    {"wireTool", m_itemDatabase->diskStore(m_essential.value(EssentialItem::WireTool))},
    {"paintTool", m_itemDatabase->diskStore(m_essential.value(EssentialItem::PaintTool))},
    {"inspectionTool", m_itemDatabase->diskStore(m_essential.value(EssentialItem::InspectionTool))}
  };

  for (auto& [equipmentSlot, item] : m_equipment) {
    if (equipmentSlot <= EquipmentSlot::BackCosmetic || item)
      data.set(strf("{}Slot", EquipmentSlotNames.getRight(equipmentSlot)), m_itemDatabase->diskStore(item));
  }

  data.set("equipmentVisibilityMask", m_equipmentVisibilityMask);

  return data;
}

void PlayerInventory::forEveryItem(function<void(InventorySlot const&, ItemPtr&)> function) {
  auto checkedFunction = [function = std::move(function)](InventorySlot const& slot, ItemPtr& item) {
    if (item)
      function(slot, item);
  };

  for (auto& [slot, item] : m_equipment)
    checkedFunction(slot, item);
  for (auto const& [bagType, bag] : m_bags) {
    for (size_t i = 0; i < bag->size(); ++i)
      checkedFunction(BagSlot(bagType, i), bag->at(i));
  }
  checkedFunction(SwapSlot(), m_swapSlot);
  checkedFunction(TrashSlot(), m_trashSlot);
}

void PlayerInventory::forEveryItem(function<void(InventorySlot const&, ItemPtr const&)> function) const {
  return const_cast<PlayerInventory*>(this)->forEveryItem([function = std::move(function)](InventorySlot const& slot, ItemPtr& item) {
      function(slot, item);
    });
}

List<ItemPtr> PlayerInventory::allItems() const {
  List<ItemPtr> items;
  forEveryItem([&items](InventorySlot const&, ItemPtr const& item) {
      items.append(item);
    });
  return items;
}

Map<String, uint64_t> PlayerInventory::itemSummary() const {
  Map<String, uint64_t> result;
  forEveryItem([&result](auto const&, auto const& item) {
      result[item->name()] += item->count();
    });
  return result;
}

void PlayerInventory::cleanup() {
  for (auto const& [_, bag] : m_bags)
    bag->cleanup();

  for (auto& [_, item] : m_equipment)
    if (item && item->empty())
      item = ItemPtr();

  if (m_swapSlot && m_swapSlot->empty())
    m_swapSlot = ItemPtr();

  if (m_trashSlot && m_trashSlot->empty())
    m_trashSlot = ItemPtr();

  m_customBar.forEach([this](Array2S const&, CustomBarLink& customBarLink) {
      auto& [primaryLink, secondaryLink] = customBarLink;
      ItemPtr primary = primaryLink ? retrieve(*primaryLink) : ItemPtr();
      ItemPtr secondary = secondaryLink ? retrieve(*secondaryLink) : ItemPtr();

      // Reset the primary and secondary action bar link if the item is gone
      if (!primary)
        primaryLink.reset();
      if (!secondary)
        secondaryLink.reset();

      // If the primary hand item is two handed, the secondary hand should not be
      // set
      if (itemSafeTwoHanded(primary))
        secondaryLink.reset();
      // Two handed items are not allowed in the secondary slot
      if (itemSafeTwoHanded(secondary))
        secondaryLink.reset();
    });
}

void PlayerInventory::setPlayer(Player& player) {
  m_player = &player;
}

PlayerInventory const& PlayerInventory::blankInventory() const {
  static thread_local PlayerInventoryPtr inventory;
  if (!inventory || inventory->m_assets != m_assets || inventory->m_configuration != m_configuration)
    inventory = make_shared<PlayerInventory>(m_assets, m_itemDatabase, m_configuration);
  return *inventory;
}

void PlayerInventory::netStore(DataStream& ds, NetCompatibilityRules rules) const {
  if (m_player && !m_player->isMaster() && !rules.isAdmin())
    return blankInventory().netStore(ds, rules);
  else
    return NetElementSyncGroup::netStore(ds, rules);
}


bool PlayerInventory::writeNetDelta(DataStream& ds, uint64_t fromVersion, NetCompatibilityRules rules) const {
  if (m_player && !m_player->isMaster() && !rules.isAdmin())
    return blankInventory().writeNetDelta(ds, fromVersion, rules);
  else
    return NetElementSyncGroup::writeNetDelta(ds, fromVersion, rules);
}

bool PlayerInventory::checkInventoryFilter(ItemPtr const& items, String const& filterName) const {
  Json filterConfig;

  auto itemFilters = items->instanceValue("inventoryFilters");
  if (itemFilters.isType(Json::Type::Object)) {
    filterConfig = itemFilters.opt(filterName).value();
    if (!filterConfig.isType(Json::Type::Object))
      filterConfig = itemFilters.opt("default").value();
  }

  if (!filterConfig.isType(Json::Type::Object)) {
    auto config = m_assets->json("/player.config:inventoryFilters");
    filterConfig = config.opt(filterName).value();
    if (!filterConfig.isType(Json::Type::Object))
      filterConfig = config.get("default");
  }

  // filter by item type if an itemTypes filter is set
  auto itemTypeName = ItemTypeNames.getRight(m_itemDatabase->itemType(items->name()));
  if (filterConfig.contains("typeWhitelist") && !filterConfig.getArray("typeWhitelist").contains(itemTypeName))
    return false;

  if (filterConfig.contains("typeBlacklist") && filterConfig.getArray("typeBlacklist").contains(itemTypeName))
    return false;

  // filter by item tags if an itemTags filter is set
  // this is an inclusive filter
  auto itemTags = m_itemDatabase->itemTags(items->name());
  if (filterConfig.contains("tagWhitelist")) {
    auto whitelistedTags = filterConfig.getArray("tagWhitelist").filtered([itemTags](Json const& tag) {
        return itemTags.contains(tag.toString());
      });
    if (whitelistedTags.empty())
      return false;
  }

  if (filterConfig.contains("tagBlacklist")) {
    auto blacklistedTags = filterConfig.getArray("tagBlacklist").filtered([itemTags](Json const& tag) {
        return itemTags.contains(tag.toString());
      });
    if (!blacklistedTags.empty())
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

ItemPtr const& PlayerInventory::retrieve(InventorySlot const& slot) const {
  return const_cast<PlayerInventory*>(this)->retrieve(slot);
}

ItemPtr& PlayerInventory::retrieve(InventorySlot const& slot) {
  auto guardEmpty = [](ItemPtr& item) -> ItemPtr& {
    if (item && item->empty())
      item = {};
    return item;
  };

  if (auto es = slot.ptr<EquipmentSlot>())
    return guardEmpty(m_equipment[*es]);
  else if (auto bs = slot.ptr<BagSlot>()) {
    if (auto bag = m_bags.ptr(bs->first))
      return guardEmpty((*bag)->at(bs->second));
  } else if (slot.is<SwapSlot>())
    return guardEmpty(m_swapSlot);
  else
    return guardEmpty(m_trashSlot);

  throw ItemException::format("Invalid inventory slot {}", jsonFromInventorySlot(slot));
}

void PlayerInventory::swapCustomBarLinks(InventorySlot a, InventorySlot b) {
  m_customBar.forEach([&](Array2S const&, CustomBarLink& customBarLink) {
      auto& [primaryLink, secondaryLink] = customBarLink;
      if (primaryLink == a)
        primaryLink = b;
      else if (primaryLink == b)
        primaryLink = a;

      if (secondaryLink == a)
        secondaryLink = b;
      else if (secondaryLink == b)
        secondaryLink = a;
    });
}

void PlayerInventory::autoAddToCustomBar(InventorySlot slot) {
  if (!m_configuration->getPath("inventory.pickupToActionBar").toBool())
    return;

  auto items = itemsAt(slot);
  if (items && !items->empty() && checkInventoryFilter(items, "autoAddToCustomBar"))
    addToCustomBar(slot);
}

void PlayerInventory::netElementsNeedLoad(bool) {
  auto deserializeItem = [this](NetElementData<ItemDescriptor>& netState, ItemPtr& item) {
    if (netState.pullUpdated())
      m_itemDatabase->loadItem(netState.get(), item);
  };

  auto deserializeItemList = [&](List<NetElementData<ItemDescriptor>>& netStatesList, List<ItemPtr>& itemList) {
    for (auto [netState, item] : zipIterator(netStatesList, itemList))
      deserializeItem(netState, item);
  };

  auto deserializeItemMap = [&](auto& netStatesMap, auto& itemMap) {
    for (auto k : netStatesMap.keys())
      deserializeItem(netStatesMap[k], itemMap[k]);
  };

  deserializeItemMap(m_equipmentNetState, m_equipment);

  for (auto const& bagType : m_bagsNetState.keys())
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

  auto serializeItem = [](NetElementData<ItemDescriptor>& netState, ItemPtr& item) {
    netState.set(itemSafeDescriptor(item));
  };

  auto serializeItemList = [&](List<NetElementData<ItemDescriptor>>& netStatesList, List<ItemPtr>& itemList) {
    for (auto [netState, item] : zipIterator(netStatesList, itemList))
      serializeItem(netState, item);
  };

  auto serializeItemMap = [&](auto& netStatesMap, auto& itemMap) {
    for (auto k : netStatesMap.keys())
      serializeItem(netStatesMap[k], itemMap[k]);
  };

  serializeItemMap(m_equipmentNetState, m_equipment);

  for (auto const& bagType : m_bagsNetState.keys())
    serializeItemList(m_bagsNetState[bagType], m_bags[bagType]->items());

  serializeItem(m_swapSlotNetState, m_swapSlot);
  serializeItem(m_trashSlotNetState, m_trashSlot);

  m_currenciesNetState.set(m_currencies);

  m_customBarGroupNetState.set(m_customBarGroup);
  m_customBar.forEach([&](auto const& index, auto& customBarLink) {
      m_customBarNetState.at(index).set(customBarLink);
    });

  m_selectedActionBarNetState.set(m_selectedActionBar);

  serializeItemMap(m_essentialNetState, m_essential);
}

}

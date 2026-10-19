import star.json;
import star.set;
import star.string;
import star.color;
import star.list;
import star.maybe;
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
import star.version;
import std;

import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_descriptor;
import star.item_recipe;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;


#include "gtest/gtest.h"


import star.item_database;
import star.item_bag;
import star.player_inventory;
import star.inventory_types;
import star.armors;
import star.net_element_top;
import star.format;

using namespace Star;

TEST(ItemTest, ItemDescriptorConstruction) {
  ItemDescriptor testItemDescriptor;

  testItemDescriptor = ItemDescriptor();

  testItemDescriptor = ItemDescriptor(Json());

  String nameOnly = "perfectlygenericitem";
  testItemDescriptor = ItemDescriptor(nameOnly);

  List<JsonArray> arrayFormats = List<JsonArray>{
      JsonArray{"perfectlygenericitem"},
      JsonArray{"perfectlygenericitem", 1},
      JsonArray{"perfectlygenericitem", 1, JsonObject()},
      JsonArray{"perfectlygenericitem", 1, JsonObject{{"testParameter", "testValue"}}}
    };

  for (auto arrayFormat : arrayFormats)
    testItemDescriptor = ItemDescriptor(arrayFormat);

  List<JsonObject> objectFormats = List<JsonObject>{
      JsonObject{{"name", "perfectlygenericitem"}},
      JsonObject{{"item", "perfectlygenericitem"}},
      JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}},
      JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}, {"parameters", JsonObject()}},
      JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}, {"parameters", JsonObject{{"testParameter", "testValue"}}}}
    };

  for (auto objectFormat : objectFormats)
    testItemDescriptor = ItemDescriptor(objectFormat);

  testItemDescriptor = ItemDescriptor("perfectlygenericitem", 1);
  testItemDescriptor = ItemDescriptor("perfectlygenericitem", 1, JsonObject{{"testParameter", "testValue"}});
}

TEST(ItemTest, ItemComparison) {
  auto itemDatabase = Root::singleton().itemDatabase();
  std::shared_ptr<::Star::Item> testItem = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1));
  std::shared_ptr<::Star::Item> testItemParams = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1, JsonObject{{"testParameter", "testValue"}}));

  List<ItemDescriptor> testItemDescriptors = List<ItemDescriptor>{
    ItemDescriptor("perfectlygenericitem", 1),
    ItemDescriptor(JsonArray{"perfectlygenericitem"}),
    ItemDescriptor(JsonArray{"perfectlygenericitem", 1}),
    ItemDescriptor(JsonArray{"perfectlygenericitem", 1, JsonObject()}),
    ItemDescriptor(JsonObject{{"name", "perfectlygenericitem"}}),
    ItemDescriptor(JsonObject{{"item", "perfectlygenericitem"}}),
    ItemDescriptor(JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}}),
    ItemDescriptor(JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}, {"parameters", JsonObject()}})
  };

  List<ItemDescriptor> testItemDescriptorsParams = List<ItemDescriptor>{
    ItemDescriptor(JsonArray{"perfectlygenericitem", 1, JsonObject{{"testParameter", "testValue"}}}),
    ItemDescriptor(JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}, {"parameters", JsonObject{{"testParameter", "testValue"}}}})
  };

  // Creating an item can add parameters to it: the perfectly generic item is an
  // object item, and objects that retain their placement parameters start with
  // an empty 'scriptStorage' entry so that they stack correctly.  Exact
  // comparisons therefore work on the parameters the created items carry.
  ItemDescriptor testItemExact = testItem->descriptor().singular();
  ItemDescriptor testItemParamsExact = testItemParams->descriptor().singular();
  EXPECT_EQ(testItemExact.parameters(), JsonObject({{"scriptStorage", JsonObject()}}));
  EXPECT_EQ(testItemParamsExact.parameters(),
      JsonObject({{"testParameter", "testValue"}, {"scriptStorage", JsonObject()}}));

  // The same descriptor spellings again, carrying the items' own parameters.
  List<ItemDescriptor> testItemExactDescriptors = List<ItemDescriptor>{
    testItemExact,
    ItemDescriptor(JsonArray{"perfectlygenericitem", 1, testItemExact.parameters()}),
    ItemDescriptor(JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}, {"parameters", testItemExact.parameters()}})
  };
  List<ItemDescriptor> testItemParamsExactDescriptors = List<ItemDescriptor>{
    testItemParamsExact,
    ItemDescriptor(JsonArray{"perfectlygenericitem", 1, testItemParamsExact.parameters()}),
    ItemDescriptor(JsonObject{{"name", "perfectlygenericitem"}, {"count", 1}, {"parameters", testItemParamsExact.parameters()}})
  };

  // comparisons WITHOUT exactMatch
  for (ItemDescriptor const& id : testItemDescriptors) {
    EXPECT_TRUE(testItem->matches(id));
    EXPECT_TRUE(testItemParams->matches(id));
    EXPECT_TRUE(id.matches(testItem));
    EXPECT_TRUE(id.matches(testItemParams));
    for (ItemDescriptor const& id2 : testItemDescriptors)
      EXPECT_TRUE(id.matches(id2));
    for (ItemDescriptor const& id2 : testItemDescriptorsParams)
      EXPECT_TRUE(id.matches(id2));
  }
  for (ItemDescriptor const& id : testItemDescriptorsParams) {
    EXPECT_TRUE(testItem->matches(id));
    EXPECT_TRUE(testItemParams->matches(id));
    EXPECT_TRUE(id.matches(testItem));
    EXPECT_TRUE(id.matches(testItemParams));
    for (ItemDescriptor const& id2 : testItemDescriptors)
      EXPECT_TRUE(id.matches(id2));
    for (ItemDescriptor const& id2 : testItemDescriptorsParams)
      EXPECT_TRUE(id.matches(id2));
  }
  EXPECT_TRUE(testItem->matches(testItemParams));
  EXPECT_TRUE(testItemParams->matches(testItem));

  // comparisons WITH exactMatch: the parameterless descriptors match neither
  // item, and the descriptors carrying an item's own parameters match exactly.
  for (ItemDescriptor const& id : testItemDescriptors) {
    EXPECT_FALSE(testItem->matches(id, true));
    EXPECT_FALSE(testItemParams->matches(id, true));
    EXPECT_FALSE(id.matches(testItem, true));
    EXPECT_FALSE(id.matches(testItemParams, true));
    for (ItemDescriptor const& id2 : testItemDescriptors)
      EXPECT_TRUE(id.matches(id2, true));
    for (ItemDescriptor const& id2 : testItemExactDescriptors)
      EXPECT_FALSE(id.matches(id2, true));
    for (ItemDescriptor const& id2 : testItemParamsExactDescriptors)
      EXPECT_FALSE(id.matches(id2, true));
  }
  for (ItemDescriptor const& id : testItemExactDescriptors) {
    EXPECT_TRUE(testItem->matches(id, true));
    EXPECT_FALSE(testItemParams->matches(id, true));
    EXPECT_TRUE(id.matches(testItem, true));
    EXPECT_FALSE(id.matches(testItemParams, true));
    for (ItemDescriptor const& id2 : testItemDescriptors)
      EXPECT_FALSE(id.matches(id2, true));
    for (ItemDescriptor const& id2 : testItemExactDescriptors)
      EXPECT_TRUE(id.matches(id2, true));
    for (ItemDescriptor const& id2 : testItemParamsExactDescriptors)
      EXPECT_FALSE(id.matches(id2, true));
  }
  for (ItemDescriptor const& id : testItemParamsExactDescriptors) {
    EXPECT_FALSE(testItem->matches(id, true));
    EXPECT_TRUE(testItemParams->matches(id, true));
    EXPECT_FALSE(id.matches(testItem, true));
    EXPECT_TRUE(id.matches(testItemParams, true));
    for (ItemDescriptor const& id2 : testItemDescriptors)
      EXPECT_FALSE(id.matches(id2, true));
    for (ItemDescriptor const& id2 : testItemExactDescriptors)
      EXPECT_FALSE(id.matches(id2, true));
    for (ItemDescriptor const& id2 : testItemParamsExactDescriptors)
      EXPECT_TRUE(id.matches(id2, true));
  }
  EXPECT_FALSE(testItem->matches(testItemParams, true));
  EXPECT_FALSE(testItemParams->matches(testItem, true));
}

TEST(ItemTest, ConstructItems) {
  auto itemDatabase = Root::singleton().itemDatabase();

  for (auto itemName : itemDatabase->allItems())
    std::shared_ptr<::Star::Item> item = itemDatabase->item(ItemDescriptor(itemName, 1));
}

TEST(ItemTest, ConstBagObservesEmptyStacksWithoutReleasingThem) {
  auto itemDatabase = Root::singleton().itemDatabase();
  auto makeEmptyBag = [&] {
    ItemBag bag(2);
    bag.setItem(0, itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1)));
    bag.setItem(1, itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1)));
    return bag;
  };

  ItemBag const indexedBag = makeEmptyBag();
  auto first = indexedBag.at(0);
  auto second = indexedBag.at(1);
  first->setCount(0);
  second->setCount(0);
  auto firstOwners = first.use_count();
  EXPECT_FALSE(indexedBag.at(0));
  EXPECT_EQ(first.use_count(), firstOwners);
  first->setCount(1);
  EXPECT_EQ(indexedBag.at(0), first);
  first->setCount(0);
  EXPECT_EQ(indexedBag.size(), 2u);
  EXPECT_TRUE(indexedBag.cleanup());
  EXPECT_FALSE(indexedBag.at(1));
  EXPECT_FALSE(indexedBag.cleanup());

  ItemBag const listedBag = makeEmptyBag();
  auto emptyItem = listedBag.at(0);
  auto liveItem = listedBag.at(1);
  emptyItem->setCount(0);
  auto const& slots = listedBag.items();
  ASSERT_EQ(slots.size(), 2u);
  EXPECT_FALSE(slots[0]);
  EXPECT_EQ(slots[1], liveItem);
  EXPECT_FALSE(listedBag.cleanup());
}

namespace {

std::shared_ptr<Item> inventoryTestHeadArmor() {
  auto itemDatabase = Root::singleton().itemDatabase();
  for (auto const& name : itemDatabase->allItems()) {
    if (itemDatabase->itemType(name) == ItemType::HeadArmor)
      return itemDatabase->item(ItemDescriptor(name, 1));
  }
  throw ItemException("Inventory tests require a head armor item");
}

BagSlot inventoryTestBagSlot(PlayerInventory const& inventory, std::shared_ptr<Item> const& item) {
  // The perfectly generic item is an ObjectItem, not a main-bag generic item.
  // Use the real configured filters rather than forcing it into a fixed bag.
  auto bags = Root::singleton().assets()->json("/player.config:inventory").get("itemBags");
  for (auto const& name : bags.toObject().keys()) {
    auto bag = inventory.bagContents(name);
    if (bag && bag->size() != 0 && PlayerInventory::itemAllowedInBag(item, name))
      return BagSlot(name, 0);
  }
  throw ItemException("Inventory tests require a bag accepting the fixture item");
}

}

TEST(ItemTest, ConstInventoryObservesExternallyConsumedItemsWithoutMutation) {
  auto itemDatabase = Root::singleton().itemDatabase();
  PlayerInventory inventory;
  auto const& view = std::as_const(inventory);
  auto bagItem = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1));
  auto bagSlot = inventoryTestBagSlot(view, bagItem);

  List<std::pair<InventorySlot, std::shared_ptr<Item>>> items{
    {EquipmentSlot::Head, inventoryTestHeadArmor()},
    {bagSlot, bagItem},
    {SwapSlot(), itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1))},
    {TrashSlot(), itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1))}
  };
  for (auto const& entry : items)
    ASSERT_TRUE(inventory.setItem(entry.first, entry.second));
  inventory.setCustomBarPrimarySlot(0, InventorySlot(bagSlot));
  inventory.selectActionBarLocation(CustomBarIndex(0));

  List<long> owners;
  for (auto const& entry : items) {
    ASSERT_TRUE(entry.second->consume(1));
    owners.append(entry.second.use_count());
    EXPECT_FALSE(view.itemsAt(entry.first));
  }

  EXPECT_FALSE(view.headArmor());
  EXPECT_FALSE(view.equipment(EquipmentSlot::Head));
  EXPECT_FALSE(view.bagContents(bagSlot.first)->at(bagSlot.second));
  EXPECT_FALSE(view.swapSlotItem());
  EXPECT_FALSE(view.primaryHeldItem());
  EXPECT_FALSE(view.secondaryHeldItem());
  EXPECT_EQ(view.primaryHeldSlot(), Maybe<InventorySlot>(InventorySlot(bagSlot)));
  EXPECT_TRUE(view.allItems().empty());
  EXPECT_TRUE(view.itemSummary().empty());
  EXPECT_TRUE(view.availableItems().empty());
  std::size_t visited = 0;
  view.forEveryItem([&](InventorySlot const&, std::shared_ptr<Item> const&) { ++visited; });
  EXPECT_EQ(visited, 0u);

  // Restoring a shared alias makes the same slots visible again: no getter
  // released the inventory's ownership of the exhausted items.
  for (std::size_t i = 0; i < items.size(); ++i) {
    auto const& entry = items[i];
    EXPECT_EQ(entry.second.use_count(), owners[i]);
    entry.second->setCount(1);
    EXPECT_EQ(view.itemsAt(entry.first), entry.second);
  }
  EXPECT_EQ(view.allItems().size(), items.size());

  for (auto const& entry : items)
    ASSERT_TRUE(entry.second->consume(1));
  inventory.cleanup();
  for (std::size_t i = 0; i < items.size(); ++i)
    EXPECT_EQ(items[i].second.use_count(), owners[i] - 1);
  EXPECT_FALSE(view.customBarPrimarySlot(0));
}

TEST(ItemTest, InventoryMutationNormalizesEmptySlots) {
  auto itemDatabase = Root::singleton().itemDatabase();
  PlayerInventory inventory;
  auto makeItems = [&](std::uint64_t count) {
    return itemDatabase->item(ItemDescriptor("perfectlygenericitem", count));
  };

  auto items = makeItems(2);
  auto bagSlot = inventoryTestBagSlot(inventory, items);
  ASSERT_TRUE(inventory.setItem(bagSlot, items));
  EXPECT_TRUE(inventory.consumeSlot(bagSlot, 2));
  EXPECT_FALSE(inventory.itemsAt(bagSlot));
  EXPECT_EQ(items.use_count(), 1);
  EXPECT_FALSE(inventory.consumeSlot(bagSlot));
  EXPECT_TRUE(inventory.consumeSlot(bagSlot, 0));

  auto externallyConsumed = makeItems(1);
  ASSERT_TRUE(inventory.setItem(bagSlot, externallyConsumed));
  ASSERT_TRUE(externallyConsumed->consume(1));
  EXPECT_FALSE(inventory.stackWith(bagSlot, makeItems(3)));
  ASSERT_TRUE(inventory.itemsAt(bagSlot));
  EXPECT_EQ(inventory.itemsAt(bagSlot)->count(), 3u);
  EXPECT_EQ(externallyConsumed.use_count(), 1);
  EXPECT_FALSE(inventory.stackWith(bagSlot, makeItems(2)));
  EXPECT_EQ(inventory.itemsAt(bagSlot)->count(), 5u);
  EXPECT_TRUE(inventory.consumeItems(ItemDescriptor("perfectlygenericitem", 5)));
  EXPECT_FALSE(inventory.itemsAt(bagSlot));

  auto armor = inventoryTestHeadArmor();
  auto armorDescriptor = armor->descriptor();
  ASSERT_TRUE(inventory.setItem(EquipmentSlot::Head, armor));
  EXPECT_TRUE(inventory.exchangeItems(EquipmentSlot::Head, TrashSlot()));
  EXPECT_FALSE(inventory.itemsAt(EquipmentSlot::Head));
  EXPECT_EQ(inventory.itemsAt(TrashSlot()), armor);
  EXPECT_TRUE(inventory.exchangeItems(EquipmentSlot::Head, TrashSlot()));
  EXPECT_EQ(inventory.itemsAt(EquipmentSlot::Head), armor);
  ASSERT_TRUE(inventory.setItem(TrashSlot(), makeItems(1)));
  EXPECT_FALSE(inventory.exchangeItems(EquipmentSlot::Head, TrashSlot()));
  EXPECT_EQ(inventory.itemsAt(EquipmentSlot::Head), armor);

  ASSERT_TRUE(inventory.setItem(SwapSlot(), makeItems(1)));
  ASSERT_TRUE(inventory.consumeSlot(SwapSlot()));
  EXPECT_FALSE(inventory.swapSlotItem());
  inventory.shiftSwap(EquipmentSlot::Head);
  EXPECT_EQ(inventory.swapSlotItem(), armor);
  EXPECT_FALSE(inventory.headArmor());
  EXPECT_TRUE(inventory.clearSwap());
  auto equippedArmor = inventory.itemsAt(EquipmentSlot::Head);
  ASSERT_TRUE(equippedArmor);
  EXPECT_EQ(equippedArmor->descriptor(), armorDescriptor);
  EXPECT_TRUE(armor->empty());

  ASSERT_TRUE(inventory.consumeItems(armorDescriptor));
  EXPECT_FALSE(inventory.headArmor());
  EXPECT_TRUE(equippedArmor->empty());
  EXPECT_EQ(equippedArmor.use_count(), 1);
  EXPECT_EQ(armor.use_count(), 1);
  ASSERT_TRUE(inventory.consumeItems(ItemDescriptor("perfectlygenericitem", 1)));
  EXPECT_FALSE(inventory.itemsAt(TrashSlot()));
  auto takenEmpty = makeItems(1);
  ASSERT_TRUE(inventory.setItem(TrashSlot(), takenEmpty));
  ASSERT_TRUE(takenEmpty->consume(1));
  EXPECT_FALSE(inventory.takeSlot(TrashSlot()));
  EXPECT_EQ(takenEmpty.use_count(), 1);

  auto empty = makeItems(1);
  ASSERT_TRUE(empty->consume(1));
  EXPECT_TRUE(inventory.setItem(EquipmentSlot::Head, empty));
  EXPECT_TRUE(inventory.setItem(bagSlot, empty));
  EXPECT_TRUE(inventory.setItem(TrashSlot(), empty));
  inventory.setSwapSlotItem(empty);
  EXPECT_EQ(empty.use_count(), 1);
  EXPECT_TRUE(inventory.allItems().empty());
  EXPECT_TRUE(inventory.exchangeItems(EquipmentSlot::Head, TrashSlot()));

  EXPECT_THROW(std::as_const(inventory).itemsAt(BagSlot("missingInventoryTestBag", 0)), ItemException);
  EXPECT_THROW(inventory.takeSlot(BagSlot("missingInventoryTestBag", 0)), ItemException);
  auto bagSize = inventory.bagContents(bagSlot.first)->size();
  ASSERT_LE(bagSize, std::numeric_limits<std::uint8_t>::max());
  BagSlot pastEnd(bagSlot.first, static_cast<std::uint8_t>(bagSize));
  EXPECT_THROW(std::as_const(inventory).itemsAt(pastEnd), OutOfRangeException);
  EXPECT_THROW(inventory.takeSlot(pastEnd), OutOfRangeException);
}

TEST(ItemTest, InventoryMutableTraversalAndNetworkNormalizeConsumption) {
  auto itemDatabase = Root::singleton().itemDatabase();
  NetElementTop<PlayerInventory> master;
  NetElementTop<PlayerInventory> slave;
  auto item = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 2));
  auto bagSlot = inventoryTestBagSlot(master, item);
  ASSERT_TRUE(master.setItem(bagSlot, item));
  ASSERT_TRUE(master.setItem(TrashSlot(), itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1))));
  master.setCustomBarPrimarySlot(0, InventorySlot(bagSlot));

  auto initial = master.writeNetState();
  slave.readNetState(initial.first);
  ASSERT_TRUE(slave.itemsAt(bagSlot));
  EXPECT_EQ(slave.itemsAt(bagSlot)->count(), 2u);
  EXPECT_TRUE(slave.itemsAt(TrashSlot()));

  std::size_t visited = 0;
  master.forEveryItem([&](InventorySlot const&, std::shared_ptr<Item>& current) {
    ++visited;
    EXPECT_TRUE(current->consume(current->count()));
  });
  EXPECT_EQ(visited, 2u);
  EXPECT_EQ(item.use_count(), 1);
  EXPECT_TRUE(master.allItems().empty());
  auto delta = master.writeNetState(initial.second);
  slave.readNetState(delta.first);
  EXPECT_TRUE(slave.allItems().empty());
  EXPECT_FALSE(slave.customBarPrimarySlot(0));

  ASSERT_TRUE(master.setItem(SwapSlot(), itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1))));
  auto liveSwap = master.writeNetState(delta.second);
  slave.readNetState(liveSwap.first);
  EXPECT_TRUE(slave.swapSlotItem());
  auto swapAlias = master.swapSlotItem();
  ASSERT_TRUE(swapAlias->consume(1));
  auto emptySwap = master.writeNetState(liveSwap.second);
  slave.readNetState(emptySwap.first);
  EXPECT_FALSE(slave.swapSlotItem());
  EXPECT_EQ(swapAlias.use_count(), 1);

  PlayerInventory restored;
  restored.load(master.store());
  EXPECT_TRUE(restored.allItems().empty());
}

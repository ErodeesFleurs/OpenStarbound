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
import star.game_test;
import star.algorithm;
import star.file;
import star.random;
import star.collection_database;

using namespace Star;

const GameTestRegistration itemDescriptorConstruction("ItemTest", "ItemDescriptorConstruction", [](GameTest&) {
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
});

const GameTestRegistration itemComparison("ItemTest", "ItemComparison", [](GameTest& fixture) {
  auto itemDatabase = fixture.root().itemDatabase();
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
});

const GameTestRegistration constructItems("ItemTest", "ConstructItems", [](GameTest& fixture) {
  auto itemDatabase = fixture.root().itemDatabase();

  for (auto itemName : itemDatabase->allItems())
    std::shared_ptr<::Star::Item> item = itemDatabase->item(ItemDescriptor(itemName, 1));
});

const GameTestRegistration constBagObservesEmptyStacksWithoutReleasingThem("ItemTest", "ConstBagObservesEmptyStacksWithoutReleasingThem", [](GameTest& fixture) {
  auto itemDatabase = fixture.root().itemDatabase();
  auto makeEmptyBag = [&] {
    ItemBag bag(fixture.root(), 2);
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
});

namespace {

std::shared_ptr<Item> inventoryTestHeadArmor(Root& root) {
  auto itemDatabase = root.itemDatabase();
  for (auto const& name : itemDatabase->allItems()) {
    if (itemDatabase->itemType(name) == ItemType::HeadArmor)
      return itemDatabase->item(ItemDescriptor(name, 1));
  }
  throw ItemException("Inventory tests require a head armor item");
}

BagSlot inventoryTestBagSlot(Root& root, PlayerInventory const& inventory, std::shared_ptr<Item> const& item) {
  // The perfectly generic item is an ObjectItem, not a main-bag generic item.
  // Use the real configured filters rather than forcing it into a fixed bag.
  auto bags = root.assets()->json("/player.config:inventory").get("itemBags");
  for (auto const& name : bags.toObject().keys()) {
    auto bag = inventory.bagContents(name);
    if (bag && bag->size() != 0 && inventory.itemAllowedInBag(item, name))
      return BagSlot(name, 0);
  }
  throw ItemException("Inventory tests require a bag accepting the fixture item");
}

}

const GameTestRegistration constInventoryObservesExternallyConsumedItemsWithoutMutation("ItemTest", "ConstInventoryObservesExternallyConsumedItemsWithoutMutation", [](GameTest& fixture) {
  auto itemDatabase = fixture.root().itemDatabase();
  PlayerInventory inventory(fixture.root());
  auto const& view = std::as_const(inventory);
  auto bagItem = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1));
  auto bagSlot = inventoryTestBagSlot(fixture.root(), view, bagItem);

  List<std::pair<InventorySlot, std::shared_ptr<Item>>> items{
    {EquipmentSlot::Head, inventoryTestHeadArmor(fixture.root())},
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
  EXPECT_EQ(view.primaryHeldSlot(), std::optional<InventorySlot>(InventorySlot(bagSlot)));
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
});

const GameTestRegistration inventoryMutationNormalizesEmptySlots("ItemTest", "InventoryMutationNormalizesEmptySlots", [](GameTest& fixture) {
  auto itemDatabase = fixture.root().itemDatabase();
  PlayerInventory inventory(fixture.root());
  auto makeItems = [&](std::uint64_t count) {
    return itemDatabase->item(ItemDescriptor("perfectlygenericitem", count));
  };

  auto items = makeItems(2);
  auto bagSlot = inventoryTestBagSlot(fixture.root(), inventory, items);
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

  auto armor = inventoryTestHeadArmor(fixture.root());
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
});

const GameTestRegistration inventoryMutableTraversalAndNetworkNormalizeConsumption("ItemTest", "InventoryMutableTraversalAndNetworkNormalizeConsumption", [](GameTest& fixture) {
  auto itemDatabase = fixture.root().itemDatabase();
  NetElementTop<PlayerInventory> master(fixture.root());
  NetElementTop<PlayerInventory> slave(fixture.root());
  auto item = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 2));
  auto bagSlot = inventoryTestBagSlot(fixture.root(), master, item);
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

  PlayerInventory restored(fixture.root());
  restored.load(master.store());
  EXPECT_TRUE(restored.allItems().empty());
});

namespace {

class ReadyCatalogErrorSink : public LogSink {
public:
  ReadyCatalogErrorSink() { setLevel(LogLevel::Error); }
  void log(char const* message, LogLevel) override {
    ADD_FAILURE() << "Ready-catalog fixture logged an error: " << message;
  }
};

}

const GameTestRegistration nestedReadyCatalog("ItemTest", "BuildersAndRebuildersKeepReadyCatalogAcrossNestedServices", [](GameTest& fixture) {
  auto directory = File::temporaryDirectory(fixture.root().random());
  auto cleanup = finally([&] { File::removeDirectoryRecursive(directory); });
  auto overlay = File::relativeTo(directory, "assets");
  auto writeAsset = [&](String const& path, String const& contents) {
    auto destination = File::relativeTo(overlay, path.beginsWith("/") ? path.substr(1) : path);
    File::makeDirectoryRecursive(File::dirName(destination));
    File::writeFile(contents, destination);
  };

  // Reuse the actual fixture's legal species/NPC assets, including their real
  // creation/build scripts. The wrappers add observable nested item consumers.
  auto baseAssets = fixture.root().assets();
  String humanPath;
  Json humanConfig;
  for (auto const& path : baseAssets->scanExtension("species")) {
    auto config = baseAssets->json(path);
    if (config.getString("kind") == "human") {
      humanPath = path;
      humanConfig = config;
      break;
    }
  }
  ASSERT_FALSE(humanPath.empty());
  Json npcConfig;
  for (auto const& path : baseAssets->scanExtension("npctype")) {
    auto config = baseAssets->json(path);
    if (config.getString("type") == "villager") {
      npcConfig = config;
      break;
    }
  }
  ASSERT_FALSE(npcConfig.isNull());
  auto creationScripts = humanConfig.getArray("createScripts", {});
  if (creationScripts.empty())
    creationScripts.append("/humanoid/opensb/create.lua");
  creationScripts.append("/readycatalog/create.lua");
  auto buildScripts = humanConfig.getArray("buildScripts", {});
  if (buildScripts.empty())
    buildScripts.append("/humanoid/opensb/build.lua");
  buildScripts.append("/readycatalog/humanoid.lua");
  humanConfig = humanConfig.set("createScripts", creationScripts).set("buildScripts", buildScripts);
  writeAsset(humanPath, humanConfig.repr());
  npcConfig = npcConfig.set("type", "readycatalognpc")
      .set("humanoidConfig", "/humanoid.config")
      .set("items", Json::parse(R"({"override":[[0,[{}]]]})"));
  writeAsset("/readycatalog/probe.npctype", npcConfig.repr());
  writeAsset("/readycatalog/create.lua", R"(
local originalCreate = create
function create(...)
  local identity, parameters, armor = originalCreate(...)
  local descriptor = root.createItem("readycatalogleaf")
  assert(descriptor.name == "readycatalogleaf")
  parameters.readyCreateMarker = root.itemConfig("readycatalogleaf").config.shortdescription
  return identity, parameters, armor
end
)");
  writeAsset("/readycatalog/humanoid.lua", R"(
local originalBuild = build
function build(...)
  local config = originalBuild(...)
  local descriptor = root.createItem("readycatalogleaf")
  assert(descriptor.name == "readycatalogleaf")
  root.setConfiguration("readySpeciesBuild", root.itemConfig("readycatalogleaf").config.shortdescription)
  return config
end
)");

  writeAsset("/_metadata", R"({
    "name":"ready_catalog_fixture",
    "errorHandlers":{"item":["/readycatalog/rebuild.lua"]}
  })");
  writeAsset("/readycatalog/leaf.item", R"({
    "itemName":"readycatalogleaf","rarity":"common",
    "shortdescription":"LeafOwner","description":"Ready catalog leaf"
  })");
  writeAsset("/readycatalog/built.item", R"({
    "itemName":"readycatalogbuilt","rarity":"common",
    "shortdescription":"Unbuilt","builder":"/readycatalog/build.lua"
  })");
  writeAsset("/readycatalog/broken.item", R"({
    "itemName":"readycatalogbroken","rarity":"common",
    "shortdescription":"Unrebuilt","builder":"/readycatalog/broken.lua"
  })");
  writeAsset("/readycatalog/broken.lua", R"(
function build()
  error("intentional ready-catalog rebuild")
end
)");
  writeAsset("/readycatalog/probe.treasurepools", R"({
    "readycatalogpool":[[0,{"fill":[{"item":"readycatalogleaf"}],"poolRounds":0}]]
  })");
  writeAsset("/readycatalog/probe.collection", R"({
    "name":"readycatalogcollection","type":"item",
    "collectables":{"leaf":{"item":"readycatalogleaf","icon":"/assetmissing.png"}}
  })");
  writeAsset("/versioning.config", baseAssets->json("/versioning.config").set("ReadyCatalog", 1).repr());
  writeAsset("/versioning/ReadyCatalog_0_1.lua", R"(
function update(data)
  return {
    name = root.createItem("readycatalogleaf").name,
    marker = root.itemConfig("readycatalogleaf").config.shortdescription
  }
end
)");

  String checkNestedServices = R"(
local function checkNestedServices()
  local leaf = root.itemConfig("readycatalogleaf").config.shortdescription
  assert(leaf == "LeafOwner")
  assert(root.createItem("readycatalogleaf").name == "readycatalogleaf")
  local treasure = root.createTreasure("readycatalogpool", 1, 123)
  assert(#treasure == 1 and treasure[1].name == "readycatalogleaf")
  local upgraded = root.loadVersionedJson(
    {id="ReadyCatalog",version=0,content={}}, "ReadyCatalog")
  assert(upgraded.name == "readycatalogleaf" and upgraded.marker == leaf)
  assert(root.collection("readycatalogcollection").name == "readycatalogcollection")
  local collectables = root.collectables("readycatalogcollection")
  assert(#collectables == 1 and collectables[1].title == leaf)
  local variant = root.npcVariant("human", "readycatalognpc", 1, 456)
  assert(variant.humanoidParameters.readyCreateMarker == leaf)
  local portrait = root.npcPortrait("head", "human", "readycatalognpc", 1, 456)
  assert(#portrait > 0)
  assert(root.getConfiguration("readySpeciesBuild") == leaf)
  return leaf
end
)";
  writeAsset("/readycatalog/build.lua", checkNestedServices + R"(
function build(directory, config, parameters)
  config.shortdescription = "NestedReady_" .. checkNestedServices()
  parameters.nestedCatalog = "builder"
  return config, parameters
end
)");
  writeAsset("/readycatalog/rebuild.lua", checkNestedServices + R"(
function error(descriptor, message)
  if descriptor.name ~= "readycatalogbroken" then return descriptor end
  assert(string.find(message, "intentional ready%-catalog rebuild"))
  return root.createItem({
    name = "readycatalogleaf",
    count = descriptor.count,
    parameters = {
      shortdescription = "RebuiltReady_" .. checkNestedServices(),
      nestedCatalog = "rebuilder"
    }
  })
end
)");
  writeAsset("/readycatalog/built.recipe", R"({
    "input":[],"output":{"name":"readycatalogbuilt","count":1},
    "groups":[],"collectables":{}
  })");
  writeAsset("/readycatalog/broken.recipe", R"({
    "input":[],"output":{"name":"readycatalogbroken","count":1},
    "groups":[],"collectables":{}
  })");

  auto settings = fixture.rootSettings();
  settings.assetSources.append(overlay);
  settings.storageDirectory = File::relativeTo(directory, "storage");
  settings.runtimeConfigFile = {};
  settings.logDirectory = {};
  settings.logFile = {};
  settings.includeUGC = false;
  Root root(settings);
  root.logger().addSink(std::make_shared<ReadyCatalogErrorSink>());

  // These strings are recorded by the INITIAL recipe scan while the Root has
  // not published ItemDatabase. A later db.item() alone could hide a fallback.
  auto database = root.itemDatabase();
  auto builtRecipes = database->recipesForOutputItem("readycatalogbuilt");
  ASSERT_EQ(builtRecipes.size(), 1u);
  EXPECT_TRUE(builtRecipes.begin()->guiFilterString.contains("nestedready_leafowner"));
  auto rebuiltRecipes = database->recipesForOutputItem("readycatalogbroken");
  ASSERT_EQ(rebuiltRecipes.size(), 1u);
  EXPECT_TRUE(rebuiltRecipes.begin()->guiFilterString.contains("rebuiltready_leafowner"));
  auto built = database->item(ItemDescriptor("readycatalogbuilt"));
  ASSERT_TRUE(built);
  EXPECT_EQ(built->name(), "readycatalogbuilt");
  EXPECT_EQ(built->friendlyName(), "NestedReady_LeafOwner");
  EXPECT_EQ(built->parameters().getString("nestedCatalog"), "builder");
  auto rebuilt = database->item(ItemDescriptor("readycatalogbroken", 3));
  ASSERT_TRUE(rebuilt);
  EXPECT_EQ(rebuilt->name(), "readycatalogleaf");
  EXPECT_EQ(rebuilt->count(), 3u);
  EXPECT_EQ(rebuilt->friendlyName(), "RebuiltReady_LeafOwner");
  EXPECT_EQ(rebuilt->parameters().getString("nestedCatalog"), "rebuilder");
  EXPECT_EQ(root.collectionDatabase()->collectable("readycatalogcollection", "leaf").title, "LeafOwner");
  EXPECT_EQ(root.configuration()->get("readySpeciesBuild").toString(), "LeafOwner");
});

#include "StarJson.hpp"
#include "StarSet.hpp"
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarAssetPath.hpp"
#include "StarStrongTypedef.hpp"
#include "StarVector.hpp"
#include "StarDataStream.hpp"
#include "StarBiMap.hpp"
#include "StarGameTypes.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarListener.hpp"
#include "StarVersion.hpp"
#include <list>

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
  ItemPtr testItem = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1));
  ItemPtr testItemParams = itemDatabase->item(ItemDescriptor("perfectlygenericitem", 1, JsonObject{{"testParameter", "testValue"}}));

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
    ItemPtr item = itemDatabase->item(ItemDescriptor(itemName, 1));
}

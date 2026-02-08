#pragma once

#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarItem.hpp"
#include "StarItemRecipe.hpp"
#include "StarLuaRoot.hpp"
#include "StarThread.hpp"
#include "StarTtlCache.hpp"

import std;

namespace Star {

class AugmentItem;
class Rebuilder;

using ItemDatabaseException = ExceptionDerived<"ItemDatabaseException", ItemException>;

enum class ItemType {
  Generic,
  LiquidItem,
  MaterialItem,
  ObjectItem,
  CurrencyItem,
  MiningTool,
  Flashlight,
  WireTool,
  BeamMiningTool,
  HarvestingTool,
  TillingTool,
  PaintingBeamTool,
  HeadArmor,
  ChestArmor,
  LegsArmor,
  BackArmor,
  Consumable,
  Blueprint,
  Codex,
  InspectionTool,
  InstrumentItem,
  GrapplingHook,
  ThrownItem,
  UnlockItem,
  ActiveItem,
  AugmentItem
};
extern EnumMap<ItemType> ItemTypeNames;

class ItemDatabase {
public:
  // During item loading, the ItemDatabase takes the ItemDescriptor and
  // produces a set of things from it:
  struct ItemConfig {
    // The relative path in assets to the base config
    String directory;

    // A possibly modified / generated config from the base config that is
    // re-constructed each time an ItemDescriptor is loaded.  Become's the
    // Item's base config.
    Json config;

    // The parameters from the ItemDescriptor, also possibly modified during
    // loading.  Since this become's the Item's parameters, it will be
    // subsequently stored with the Item as the new ItemDescriptor.
    Json parameters;
  };

  static auto getCountOfItem(List<Ptr<Item>> const& bag, ItemDescriptor const& item, bool exactMatch = false) -> std::uint64_t;
  static auto getCountOfItem(HashMap<ItemDescriptor, std::uint64_t> const& bag, ItemDescriptor const& item, bool exactMatch = false) -> std::uint64_t;
  static auto normalizeBag(List<Ptr<Item>> const& bag) -> HashMap<ItemDescriptor, std::uint64_t>;
  static auto canMakeRecipe(ItemRecipe const& recipe, HashMap<ItemDescriptor, std::uint64_t> const& availableIngredients, StringMap<std::uint64_t> const& availableCurrencies) -> bool;
  static auto recipesFromSubset(HashMap<ItemDescriptor, std::uint64_t> const& normalizedBag, StringMap<std::uint64_t> const& availableCurrencies, HashSet<ItemRecipe> const& subset) -> HashSet<ItemRecipe>;
  static auto recipesFromSubset(HashMap<ItemDescriptor, std::uint64_t> const& normalizedBag, StringMap<std::uint64_t> const& availableCurrencies, HashSet<ItemRecipe> const& subset, StringSet const& allowedTypes) -> HashSet<ItemRecipe>;
  static auto guiFilterString(Ptr<Item> const& item) -> String;

  ItemDatabase();

  void cleanup();

  // Load an item based on item descriptor.  If loadItem is called with a
  // live ptr, and the ptr matches the descriptor read, then no new item is
  // constructed.  If ItemT is some other type than Item, then loadItem will
  // clear the item if the new item is not castable to it.  Returns whether
  // itemPtr was changed.  No exception will be thrown if there is an error
  // spawning the new item, it will be logged and the itemPtr will be set to a
  // default item.
  template <typename ItemT>
  auto loadItem(ItemDescriptor const& descriptor, std::shared_ptr<ItemT>& itemPtr) const -> bool;

  // Protects against re-instantiating an item in the same was as loadItem
  template <typename ItemT>
  auto diskLoad(Json const& diskStore, std::shared_ptr<ItemT>& itemPtr) const -> bool;

  auto diskLoad(Json const& diskStore) const -> Ptr<Item>;
  auto fromJson(Json const& spec) const -> Ptr<Item>;

  auto diskStore(ConstPtr<Item> const& itemPtr) const -> Json;

  auto toJson(ConstPtr<Item> const& itemPtr) const -> Json;

  auto hasItem(String const& itemName) const -> bool;
  auto itemType(String const& itemName) const -> ItemType;
  // Friendly name here can be different than the final friendly name, as it
  // can be modified by custom config or builder scripts.
  auto itemFriendlyName(String const& itemName) const -> String;
  auto itemTags(String const& itemName) const -> StringSet;

  // Generate an item config for the given itemName, parameters, level and seed.
  // Level and seed are used by generation in some item types, and may be stored as part
  // of the unique item data or may be ignored.
  auto itemConfig(String const& itemName, Json parameters, std::optional<float> level = {}, std::optional<std::uint64_t> seed = {}) const -> ItemConfig;

  // Returns the path to the item's json file in the assets.
  auto itemFile(String const& itemName) const -> std::optional<String>;

  // Generates the config for the given item descriptor and then loads the item
  // from the appropriate factory.  If there is a problem instantiating the
  // item, will return a default item instead.  If item is passed a null
  // ItemDescriptor, it will return a null pointer.
  // The returned item pointer will be shared. Either call ->clone() or use item() instead for a copy.
  auto itemShared(ItemDescriptor descriptor, std::optional<float> level = {}, std::optional<std::uint64_t> seed = {}) const -> Ptr<Item>;
  // Same as itemShared, but makes a copy instead. Does not cache.
  auto item(ItemDescriptor descriptor, std::optional<float> level = {}, std::optional<std::uint64_t> seed = {}, bool ignoreInvalid = false) const -> Ptr<Item>;

  auto hasRecipeToMake(ItemDescriptor const& item) const -> bool;
  auto hasRecipeToMake(ItemDescriptor const& item, StringSet const& allowedTypes) const -> bool;

  auto recipesForOutputItem(String itemName) const -> HashSet<ItemRecipe>;

  auto recipesFromBagContents(List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const -> HashSet<ItemRecipe>;
  auto recipesFromBagContents(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const -> HashSet<ItemRecipe>;

  auto recipesFromBagContents(List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const -> HashSet<ItemRecipe>;
  auto recipesFromBagContents(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const -> HashSet<ItemRecipe>;

  auto maxCraftableInBag(List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const -> std::uint64_t;
  auto maxCraftableInBag(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const -> std::uint64_t;

  auto getPreciseRecipeForMaterials(String const& group, List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const -> ItemRecipe;

  auto parseRecipe(Json const& config) const -> ItemRecipe;

  auto allRecipes() const -> HashSet<ItemRecipe> const&;
  auto allRecipes(StringSet const& types) const -> HashSet<ItemRecipe>;

  auto applyAugment(Ptr<Item> const item, AugmentItem* augment) const -> Ptr<Item>;
  auto ageItem(Ptr<Item>& item, double aging) const -> bool;

  auto allItems() const -> List<String>;

private:
  struct ItemData {
    ItemType type;
    String name;
    String friendlyName;
    StringSet itemTags;
    StringList agingScripts;
    std::optional<String> assetsConfig;
    JsonObject customConfig;
    String directory;
    String filename;
  };

  static auto createItem(ItemType type, ItemConfig const& config) -> Ptr<Item>;
  auto tryCreateItem(ItemDescriptor const& descriptor, std::optional<float> level = {}, std::optional<std::uint64_t> seed = {}, bool ignoreInvalid = false) const -> Ptr<Item>;

  auto itemData(String const& name) const -> ItemData const&;
  auto makeRecipe(List<ItemDescriptor> inputs, ItemDescriptor output, float duration, StringSet groups) const -> ItemRecipe;

  void addItemSet(ItemType type, String const& extension);
  void addObjectDropItem(String const& objectPath, Json const& objectConfig);

  void scanItems();
  void addObjectItems();
  void scanRecipes();
  void addBlueprints();
  void addCodexes();

  StringMap<ItemData> m_items;
  HashSet<ItemRecipe> m_recipes;

  mutable RecursiveMutex m_luaMutex;
  Ptr<LuaRoot> m_luaRoot;
  Ptr<Rebuilder> m_rebuilder;

  using ItemCacheEntry = std::tuple<ItemDescriptor, std::optional<float>, std::optional<std::uint64_t>>;

  mutable Mutex m_cacheMutex;
  mutable HashTtlCache<ItemCacheEntry, Ptr<Item>> m_itemCache;
};

template <typename ItemT>
auto ItemDatabase::loadItem(ItemDescriptor const& descriptor, std::shared_ptr<ItemT>& itemPtr) const -> bool {
  if (descriptor.isNull()) {
    if (itemPtr) {
      itemPtr.reset();
      return true;
    }
  } else {
    if (!itemPtr || !itemPtr->matches(descriptor, true)) {
      itemPtr = as<ItemT>(item(descriptor));
      return true;
    } else if (itemPtr->count() != descriptor.count()) {
      itemPtr->setCount(descriptor.count());
      return true;
    }
  }
  return false;
}

template <typename ItemT>
auto ItemDatabase::diskLoad(Json const& diskStore, std::shared_ptr<ItemT>& itemPtr) const -> bool {
  try {
    return loadItem(ItemDescriptor::loadStore(diskStore), itemPtr);
  } catch (StarException const&) {
    return false;
  }
}
}// namespace Star

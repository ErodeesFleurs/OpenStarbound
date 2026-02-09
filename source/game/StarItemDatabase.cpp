#include "StarItemDatabase.hpp"

#include "StarActiveItem.hpp"
#include "StarArmors.hpp"
#include "StarAugmentItem.hpp"
#include "StarBlueprintItem.hpp"
#include "StarCasting.hpp"
#include "StarCodexDatabase.hpp"
#include "StarCodexItem.hpp"
#include "StarConfig.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarConsumableItem.hpp"
#include "StarCurrency.hpp"
#include "StarInspectionTool.hpp"
#include "StarInstrumentItem.hpp"
#include "StarItemLuaBindings.hpp"
#include "StarJsonExtra.hpp"
#include "StarLiquidItem.hpp"
#include "StarLuaRoot.hpp"
#include "StarMaterialItem.hpp"
#include "StarObjectDatabase.hpp"
#include "StarObjectItem.hpp"
#include "StarRebuilder.hpp"
#include "StarRoot.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarString.hpp"
#include "StarThrownItem.hpp"
#include "StarTools.hpp"
#include "StarUnlockItem.hpp"
#include "StarUtilityLuaBindings.hpp"

import std;

namespace Star {

EnumMap<ItemType> ItemTypeNames{
  {ItemType::Generic, "generic"},
  {ItemType::LiquidItem, "liquid"},
  {ItemType::MaterialItem, "material"},
  {ItemType::ObjectItem, "object"},
  {ItemType::CurrencyItem, "currency"},
  {ItemType::MiningTool, "miningtool"},
  {ItemType::Flashlight, "flashlight"},
  {ItemType::WireTool, "wiretool"},
  {ItemType::BeamMiningTool, "beamminingtool"},
  {ItemType::HarvestingTool, "harvestingtool"},
  {ItemType::TillingTool, "tillingtool"},
  {ItemType::PaintingBeamTool, "paintingbeamtool"},
  {ItemType::HeadArmor, "headarmor"},
  {ItemType::ChestArmor, "chestarmor"},
  {ItemType::LegsArmor, "legsarmor"},
  {ItemType::BackArmor, "backarmor"},
  {ItemType::Consumable, "consumable"},
  {ItemType::Blueprint, "blueprint"},
  {ItemType::Codex, "codex"},
  {ItemType::InspectionTool, "inspectiontool"},
  {ItemType::InstrumentItem, "instrument"},
  {ItemType::ThrownItem, "thrownitem"},
  {ItemType::UnlockItem, "unlockitem"},
  {ItemType::ActiveItem, "activeitem"},
  {ItemType::AugmentItem, "augmentitem"}};

auto ItemDatabase::getCountOfItem(List<Ptr<Item>> const& bag, ItemDescriptor const& item, bool exactMatch) -> std::uint64_t {
  auto normalizedBag = normalizeBag(bag);
  return getCountOfItem(normalizedBag, item, exactMatch);
}

auto ItemDatabase::getCountOfItem(HashMap<ItemDescriptor, std::uint64_t> const& bag, ItemDescriptor const& item, bool exactMatch) -> std::uint64_t {
  ItemDescriptor matchItem = exactMatch ? item.singular() : ItemDescriptor(item.name(), 1);
  if (!bag.contains(matchItem)) {
    return 0;
  } else {
    return bag.get(matchItem);
  }
}

auto ItemDatabase::normalizeBag(List<Ptr<Item>> const& bag) -> HashMap<ItemDescriptor, std::uint64_t> {
  HashMap<ItemDescriptor, std::uint64_t> normalizedBag;
  for (auto const& item : bag) {
    if (!item)
      continue;

    normalizedBag[ItemDescriptor(item->name(), 1)] += item->count();

    if (!item->parameters().toObject().empty())
      normalizedBag[ItemDescriptor(item->name(), 1, item->parameters())] += item->count();
  }

  return normalizedBag;
}

auto ItemDatabase::recipesFromSubset(HashMap<ItemDescriptor, std::uint64_t> const& normalizedBag, StringMap<std::uint64_t> const& availableCurrencies, HashSet<ItemRecipe> const& subset) -> HashSet<ItemRecipe> {
  HashSet<ItemRecipe> res;
  for (auto const& recipe : subset) {
    // add this recipe if we can make it.
    if (canMakeRecipe(recipe, normalizedBag, availableCurrencies))
      res.add(recipe);
  }

  return res;
}

auto ItemDatabase::recipesFromSubset(HashMap<ItemDescriptor, std::uint64_t> const& normalizedBag, StringMap<std::uint64_t> const& availableCurrencies,
                                     HashSet<ItemRecipe> const& subset, StringSet const& allowedTypes) -> HashSet<ItemRecipe> {
  HashSet<ItemRecipe> res;
  for (auto const& recipe : subset) {
    // is it the right kind of recipe for this check ?
    if (recipe.groups.hasIntersection(allowedTypes) || allowedTypes.empty() || recipe.groups.empty()) {
      // do we have the ingredients to make it.
      if (canMakeRecipe(recipe, normalizedBag, availableCurrencies)) {
        res.add(recipe);
      }
    }
  }

  return res;
}

auto ItemDatabase::guiFilterString(Ptr<Item> const& item) -> String {
  return (item->name() + item->friendlyName() + item->description()).toLower().splitAny(" ,.?*\\+/|\t").join("");
}

auto ItemDatabase::canMakeRecipe(ItemRecipe const& recipe, HashMap<ItemDescriptor, std::uint64_t> const& availableIngredients, StringMap<std::uint64_t> const& availableCurrencies) -> bool {
  for (auto const& p : recipe.currencyInputs) {
    if (availableCurrencies.value(p.first, 0) < p.second)
      return false;
  }

  for (auto const& input : recipe.inputs) {
    ItemDescriptor matchInput = recipe.matchInputParameters ? input.singular() : ItemDescriptor(input.name(), 1);
    if (availableIngredients.value(matchInput) < input.count())
      return false;
  }

  return true;
}

ItemDatabase::ItemDatabase()
    : m_luaRoot(std::make_shared<LuaRoot>()), m_rebuilder(std::make_shared<Rebuilder>("item")) {
  scanItems();
  addObjectItems();
  addCodexes();
  scanRecipes();
  addBlueprints();
}

void ItemDatabase::cleanup() {
  {
    MutexLocker locker(m_cacheMutex);
    m_itemCache.cleanup([](ItemCacheEntry const&, Ptr<Item> const& item) -> bool {
      return item.use_count() != 1;
    });
  }
}

auto ItemDatabase::diskLoad(Json const& diskStore) const -> Ptr<Item> {
  if (diskStore) {
    return item(ItemDescriptor::loadStore(diskStore));
  } else {
    return {};
  }
}

auto ItemDatabase::fromJson(Json const& spec) const -> Ptr<Item> {
  return item(ItemDescriptor(spec));
}

auto ItemDatabase::diskStore(ConstPtr<Item> const& itemPtr) const -> Json {
  if (itemPtr)
    return itemPtr->descriptor().diskStore();
  else
    return {};
}

auto ItemDatabase::toJson(ConstPtr<Item> const& itemPtr) const -> Json {
  if (itemPtr)
    return itemPtr->descriptor().toJson();
  else
    return {};
}

auto ItemDatabase::hasItem(String const& itemName) const -> bool {
  return m_items.contains(itemName);
}

auto ItemDatabase::itemType(String const& itemName) const -> ItemType {
  return itemData(itemName).type;
}

auto ItemDatabase::itemFriendlyName(String const& itemName) const -> String {
  return itemData(itemName).friendlyName;
}

auto ItemDatabase::itemTags(String const& itemName) const -> StringSet {
  return itemData(itemName).itemTags;
}

auto ItemDatabase::itemConfig(String const& itemName, Json parameters, std::optional<float> level, std::optional<std::uint64_t> seed) const -> ItemDatabase::ItemConfig {
  auto const& data = itemData(itemName);

  ItemConfig itemConfig;
  if (data.assetsConfig)
    itemConfig.config = Root::singleton().assets()->json(*data.assetsConfig);
  itemConfig.directory = data.directory;
  itemConfig.config = jsonMerge(itemConfig.config, data.customConfig);
  itemConfig.parameters = parameters;

  if (auto builder = itemConfig.config.optString("builder")) {
    RecursiveMutexLocker locker(m_luaMutex);
    auto context = m_luaRoot->createContext(*builder);
    context.setCallbacks("root", LuaBindings::makeRootCallbacks());
    context.setCallbacks("sb", LuaBindings::makeUtilityCallbacks());
    luaTie(itemConfig.config, itemConfig.parameters) = context.invokePath<LuaTupleReturn<Json, Json>>(
      "build", itemConfig.directory, itemConfig.config, itemConfig.parameters, level, seed);
  }

  return itemConfig;
}

auto ItemDatabase::itemFile(String const& itemName) const -> std::optional<String> {
  if (!hasItem(itemName)) {
    return {};
  }
  auto const& data = itemData(itemName);
  return data.directory + data.filename;
}

auto ItemDatabase::itemShared(ItemDescriptor descriptor, std::optional<float> level, std::optional<std::uint64_t> seed) const -> Ptr<Item> {
  if (!descriptor)
    return {};

  ItemCacheEntry entry{descriptor, level, seed};
  MutexLocker locker(m_cacheMutex);
  if (Ptr<Item>* cached = m_itemCache.ptr(entry))
    return *cached;
  else {
    locker.unlock();

    Ptr<Item> item = tryCreateItem(descriptor, level, seed);
    get<2>(entry) = item->parameters().optUInt("seed");// Seed could've been changed by the buildscript

    locker.lock();
    return m_itemCache.get(entry, [&](ItemCacheEntry const&) -> Ptr<Item> { return std::move(item); });
  }
}

auto ItemDatabase::item(ItemDescriptor descriptor, std::optional<float> level, std::optional<std::uint64_t> seed, bool ignoreInvalid) const -> Ptr<Item> {
  if (!descriptor)
    return {};
  else
    return tryCreateItem(descriptor, level, seed, ignoreInvalid);
}

auto ItemDatabase::hasRecipeToMake(ItemDescriptor const& item) const -> bool {
  auto si = item.singular();
  for (auto const& recipe : m_recipes)
    if (recipe.output.singular() == si)
      return true;
  return false;
}

auto ItemDatabase::hasRecipeToMake(ItemDescriptor const& item, StringSet const& allowedTypes) const -> bool {
  auto si = item.singular();
  for (auto const& recipe : m_recipes)
    if (recipe.output.singular() == si)
      for (auto allowedType : allowedTypes)
        if (recipe.groups.contains(allowedType))
          return true;
  return false;
}

auto ItemDatabase::recipesForOutputItem(String itemName) const -> HashSet<ItemRecipe> {
  HashSet<ItemRecipe> result;
  for (auto const& recipe : m_recipes)
    if (recipe.output.name() == itemName)
      result.add(recipe);
  return result;
}

auto ItemDatabase::recipesFromBagContents(List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const -> HashSet<ItemRecipe> {
  auto normalizedBag = normalizeBag(bag);
  return recipesFromBagContents(normalizedBag, availableCurrencies);
}

auto ItemDatabase::recipesFromBagContents(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const -> HashSet<ItemRecipe> {
  return recipesFromSubset(bag, availableCurrencies, m_recipes);
}

auto ItemDatabase::recipesFromBagContents(List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const -> HashSet<ItemRecipe> {
  auto normalizedBag = normalizeBag(bag);
  return recipesFromBagContents(normalizedBag, availableCurrencies, allowedTypes);
}

auto ItemDatabase::recipesFromBagContents(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const -> HashSet<ItemRecipe> {
  return recipesFromSubset(bag, availableCurrencies, m_recipes, allowedTypes);
}

auto ItemDatabase::maxCraftableInBag(List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const -> std::uint64_t {
  auto normalizedBag = normalizeBag(bag);

  return maxCraftableInBag(normalizedBag, availableCurrencies, recipe);
}

auto ItemDatabase::maxCraftableInBag(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const -> std::uint64_t {
  auto res = highest<std::uint64_t>();

  for (auto const& p : recipe.currencyInputs) {
    std::uint64_t available = availableCurrencies.value(p.first, 0);
    if (available == 0)
      return 0;
    else if (p.second > 0)
      res = std::min(available / p.second, res);
  }

  for (auto const& input : recipe.inputs) {
    if (!bag.contains(input.singular()))
      return 0;
    else if (input.count() > 0)
      res = std::min(bag.get(input.singular()) / input.count(), res);
  }

  return res;
}

auto ItemDatabase::getPreciseRecipeForMaterials(String const& group, List<Ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const -> ItemRecipe {
  // picks the recipe that:
  // * can be crafted (duh)
  // * uses all the input material types
  // * uses the most materials (if recipes exist with the same input materials)

  auto options = recipesFromBagContents(bag, availableCurrencies);
  ItemRecipe result;
  int ingredientsCount = 0;
  for (auto const& recipe : options) {
    if (!recipe.groups.contains(group))
      continue;
    bool usesAllItemTypes = true;
    for (auto const& item : bag) {
      bool match = false;
      for (auto const& input : recipe.inputs)
        if (item->matches(input, recipe.matchInputParameters))
          match = true;
      if (!match)
        usesAllItemTypes = false;
    }
    if (!usesAllItemTypes)
      continue;
    int count = 0;
    for (auto const& input : recipe.inputs)
      count += input.count();
    if (count > ingredientsCount)
      result = recipe;
  }
  return result;
}

auto ItemDatabase::parseRecipe(Json const& config) const -> ItemRecipe {
  ItemRecipe res;
  try {
    res.currencyInputs = jsonToMapV<StringMap<std::uint64_t>>(config.get("currencyInputs", JsonObject()), std::mem_fn(&Json::toUInt));

    // parse currency items into currency inputs
    for (auto input : config.getArray("input")) {
      auto id = ItemDescriptor(input);
      if (itemType(id.name()) == ItemType::CurrencyItem) {
        auto currencyItem = as<CurrencyItem>(itemShared(id));
        res.currencyInputs[currencyItem->currencyType()] += currencyItem->totalValue();
      } else {
        res.inputs.push_back(id);
      }
    }

    res.output = ItemDescriptor(config.get("output"));
    res.duration = config.getFloat("duration", Root::singleton().assets()->json("/items/defaultParameters.config:defaultCraftDuration").toFloat());
    res.groups = StringSet::from(jsonToStringList(config.get("groups", JsonArray())));
    if (auto item = ItemDatabase::itemShared(res.output)) {
      res.outputRarity = item->rarity();
      res.guiFilterString = guiFilterString(item);
    }
    res.collectables = jsonToMapV<StringMap<String>>(config.get("collectables", JsonObject()), std::mem_fn(&Json::toString));
    res.matchInputParameters = config.getBool("matchInputParameters", false);

  } catch (JsonException const& e) {
    throw RecipeException(strf("Recipe missing required ingredient: {}", outputException(e, false)));
  }

  return res;
}

auto ItemDatabase::allRecipes() const -> HashSet<ItemRecipe> const& {
  return m_recipes;
}

auto ItemDatabase::allRecipes(StringSet const& types) const -> HashSet<ItemRecipe> {
  HashSet<ItemRecipe> res;
  for (auto const& i : m_recipes) {
    if (i.groups.hasIntersection(types))
      res.add(i);
  }
  return res;
}

auto ItemDatabase::applyAugment(Ptr<Item> const item, AugmentItem* augment) const -> Ptr<Item> {
  if (item) {
    RecursiveMutexLocker locker(m_luaMutex);
    LuaBaseComponent script;
    script.setLuaRoot(m_luaRoot);
    script.setScripts(augment->augmentScripts());
    script.addCallbacks("item", LuaBindings::makeItemCallbacks(augment));
    script.addCallbacks("config", LuaBindings::makeConfigCallbacks([augment](auto&& PH1, auto&& PH2) -> auto { return augment->instanceValue(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); }));
    script.init();
    auto luaResult = script.invoke<LuaTupleReturn<Json, std::optional<std::uint64_t>>>("apply", item->descriptor().toJson());
    script.uninit();
    locker.unlock();

    if (luaResult) {
      if (!get<0>(*luaResult).isNull()) {
        augment->take(get<1>(*luaResult).value_or(1));
        return ItemDatabase::item(ItemDescriptor(get<0>(*luaResult)));
      }
    }
  }

  return item;
}

auto ItemDatabase::ageItem(Ptr<Item>& item, double aging) const -> bool {
  if (!item)
    return false;

  auto const& itemData = ItemDatabase::itemData(item->name());
  if (itemData.agingScripts.empty())
    return false;

  ItemDescriptor original = item->descriptor();

  RecursiveMutexLocker locker(m_luaMutex);
  LuaBaseComponent script;
  script.setLuaRoot(m_luaRoot);
  script.setScripts(itemData.agingScripts);
  script.init();
  auto aged = script.invoke<Json>("ageItem", original.toJson(), aging).transform(construct<ItemDescriptor>());
  script.uninit();
  locker.unlock();

  if (aged && *aged != original) {
    item = ItemDatabase::item(*aged);
    return true;
  }

  return false;
}

auto ItemDatabase::allItems() const -> List<String> {
  return m_items.keys();
}

auto ItemDatabase::createItem(ItemType type, ItemConfig const& config) -> Ptr<Item> {
  if (type == ItemType::Generic) {
    return std::make_shared<GenericItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::LiquidItem) {
    return std::make_shared<LiquidItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::MaterialItem) {
    return std::make_shared<MaterialItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::ObjectItem) {
    return std::make_shared<ObjectItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::CurrencyItem) {
    return std::make_shared<CurrencyItem>(config.config, config.directory);
  } else if (type == ItemType::MiningTool) {
    return std::make_shared<MiningTool>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::Flashlight) {
    return std::make_shared<Flashlight>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::WireTool) {
    return std::make_shared<WireTool>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::BeamMiningTool) {
    return std::make_shared<BeamMiningTool>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::PaintingBeamTool) {
    return std::make_shared<PaintingBeamTool>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::TillingTool) {
    return std::make_shared<TillingTool>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::HarvestingTool) {
    return std::make_shared<HarvestingTool>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::HeadArmor) {
    return std::make_shared<HeadArmor>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::ChestArmor) {
    return std::make_shared<ChestArmor>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::LegsArmor) {
    return std::make_shared<LegsArmor>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::BackArmor) {
    return std::make_shared<BackArmor>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::Consumable) {
    return std::make_shared<ConsumableItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::Blueprint) {
    return std::make_shared<BlueprintItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::Codex) {
    return std::make_shared<CodexItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::InspectionTool) {
    return std::make_shared<InspectionTool>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::InstrumentItem) {
    return std::make_shared<InstrumentItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::ThrownItem) {
    return std::make_shared<ThrownItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::UnlockItem) {
    return std::make_shared<UnlockItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::ActiveItem) {
    return std::make_shared<ActiveItem>(config.config, config.directory, config.parameters);
  } else if (type == ItemType::AugmentItem) {
    return std::make_shared<AugmentItem>(config.config, config.directory, config.parameters);
  } else {
    throw ItemException(strf("Unknown item type {}", (int)type));
  }
}

auto ItemDatabase::tryCreateItem(ItemDescriptor const& descriptor, std::optional<float> level, std::optional<std::uint64_t> seed, bool ignoreInvalid) const -> Ptr<Item> {
  Ptr<Item> result;
  ItemDescriptor newDescriptor = descriptor;

  try {
    if (newDescriptor.name() == "perfectlygenericitem" && newDescriptor.parameters().contains("genericItemStorage"))
      newDescriptor = ItemDescriptor(descriptor.parameters().get("genericItemStorage"));
    result = createItem(m_items.get(newDescriptor.name()).type, itemConfig(newDescriptor.name(), newDescriptor.parameters(), level, seed));
    result->setCount(descriptor.count());
  } catch (std::exception const& e) {
    if (!ignoreInvalid) {
      bool success = m_rebuilder->rebuild(descriptor.toJson(), strf("{}", outputException(e, false)), [&](Json const& store) -> String {
        try {
          ItemDescriptor newDescriptor(store);
          result = createItem(m_items.get(newDescriptor.name()).type, itemConfig(newDescriptor.name(), newDescriptor.parameters(), level, seed));
          result->setCount(newDescriptor.count());
        } catch (std::exception const& e) {
          return strf("{}", outputException(e, false));
        }
        return {};
      });

      if (!success) {
        if (descriptor.name() == "perfectlygenericitem") {
          Logger::error("Could not re-instantiate item '{}'. {}", descriptor, outputException(e, false));
          result = createItem(m_items.get("perfectlygenericitem").type, itemConfig("perfectlygenericitem", descriptor.parameters(), level, seed));
        } else {
          Logger::error(std::string_view("Could not instantiate item '{}'. {}"), descriptor, outputException(e, false));
          result = createItem(m_items.get("perfectlygenericitem").type, itemConfig("perfectlygenericitem", JsonObject({{"genericItemStorage", descriptor.toJson()}, {"shortdescription", descriptor.name()}, {"description", "Reinstall the parent mod to return this item to normal"}}), {}, {}));
        }
      }
    } else
      throw;
  }
  return result;
}

auto ItemDatabase::itemData(String const& name) const -> ItemDatabase::ItemData const& {
  if (auto p = m_items.ptr(name))
    return *p;
  throw ItemException::format("No such item '{}'", name);
}

auto ItemDatabase::makeRecipe(List<ItemDescriptor> inputs, ItemDescriptor output, float duration, StringSet groups) const -> ItemRecipe {
  ItemRecipe res;
  res.inputs = std::move(inputs);
  res.output = std::move(output);
  res.duration = duration;
  res.groups = std::move(groups);
  if (auto item = ItemDatabase::itemShared(res.output)) {
    res.outputRarity = item->rarity();
    res.guiFilterString = guiFilterString(item);
  }
  return res;
}

void ItemDatabase::addItemSet(ItemType type, String const& extension) {
  auto assets = Root::singleton().assets();
  for (auto& file : assets->scanExtension(extension)) {
    ItemData data;
    try {
      auto config = assets->json(file);
      data.type = type;
      data.assetsConfig = file;
      data.name = config.get("itemName").toString();
      data.friendlyName = config.getString("shortdescription", {});
      data.itemTags = config.opt("itemTags").transform(jsonToStringSet).value_or(StringSet());
      data.agingScripts = config.opt("itemAgingScripts").transform(jsonToStringList).value_or(StringList());
      data.directory = AssetPath::directory(file);
      data.filename = AssetPath::filename(file);

      data.agingScripts = data.agingScripts.transformed([capture0 = data.directory](auto&& PH1) -> auto { return AssetPath::relativeTo(capture0, std::forward<decltype(PH1)>(PH1)); });
    } catch (std::exception const& e) {
      throw ItemException(strf("Could not load item asset {}", file), e);
    }

    if (m_items.contains(data.name))
      throw ItemException(strf("Duplicate item name '{}' found", data.name));

    m_items[data.name] = data;
  }
}

void ItemDatabase::addObjectDropItem(String const& objectPath, Json const& objectConfig) {
  auto assets = Root::singleton().assets();

  ItemData data;
  data.type = ItemType::ObjectItem;
  data.name = objectConfig.get("objectName").toString();
  data.friendlyName = objectConfig.getString("shortdescription", {});
  data.itemTags = objectConfig.opt("itemTags").transform(jsonToStringSet).value_or(StringSet());
  data.agingScripts = objectConfig.opt("itemAgingScripts").transform(jsonToStringList).value_or(StringList());
  data.directory = AssetPath::directory(objectPath);
  data.filename = AssetPath::filename(objectPath);
  JsonObject customConfig = objectConfig.toObject();
  if (!customConfig.contains("inventoryIcon")) {
    customConfig["inventoryIcon"] = assets->json("/objects/defaultParameters.config:missingIcon");
    Logger::warn("Missing inventoryIcon for {}, using default", data.name);
  }
  customConfig["itemName"] = data.name;
  if (!customConfig.contains("tooltipKind"))
    customConfig["tooltipKind"] = "object";

  if (!customConfig.contains("printable"))
    customConfig["printable"] = customConfig.contains("price");

  // Don't inherit object scripts. this is kind of a crappy solution to prevent
  // ObjectItems (which are firable and therefore scripted) from trying to
  // execute scripts intended for objects
  customConfig.remove("scripts");

  data.customConfig = std::move(customConfig);

  if (m_items.contains(data.name))
    throw ItemException(strf("Object drop '{}' shares name with existing item", data.name));

  m_items[data.name] = std::move(data);
}

void ItemDatabase::scanItems() {
  auto assets = Root::singleton().assets();

  List<std::pair<ItemType, String>> itemSets;
  auto scanItemType = [&itemSets, assets](ItemType type, String const& extension) -> void {
    itemSets.append(std::make_pair(type, extension));
    assets->queueJsons(assets->scanExtension(extension));
  };

  scanItemType(ItemType::Generic, "item");
  scanItemType(ItemType::LiquidItem, "liqitem");
  scanItemType(ItemType::MaterialItem, "matitem");
  scanItemType(ItemType::MiningTool, "miningtool");
  scanItemType(ItemType::Flashlight, "flashlight");
  scanItemType(ItemType::WireTool, "wiretool");
  scanItemType(ItemType::BeamMiningTool, "beamaxe");
  scanItemType(ItemType::TillingTool, "tillingtool");
  scanItemType(ItemType::PaintingBeamTool, "painttool");
  scanItemType(ItemType::HarvestingTool, "harvestingtool");
  scanItemType(ItemType::HeadArmor, "head");
  scanItemType(ItemType::ChestArmor, "chest");
  scanItemType(ItemType::LegsArmor, "legs");
  scanItemType(ItemType::BackArmor, "back");
  scanItemType(ItemType::CurrencyItem, "currency");
  scanItemType(ItemType::Consumable, "consumable");
  scanItemType(ItemType::Blueprint, "blueprint");
  scanItemType(ItemType::InspectionTool, "inspectiontool");
  scanItemType(ItemType::InstrumentItem, "instrument");
  scanItemType(ItemType::ThrownItem, "thrownitem");
  scanItemType(ItemType::UnlockItem, "unlock");
  scanItemType(ItemType::ActiveItem, "activeitem");
  scanItemType(ItemType::AugmentItem, "augment");

  for (auto const& itemset : itemSets)
    addItemSet(itemset.first, itemset.second);
}

void ItemDatabase::addObjectItems() {
  auto objectDatabase = Root::singleton().objectDatabase();

  for (auto const& objectName : objectDatabase->allObjects()) {
    auto objectConfig = objectDatabase->getConfig(objectName);

    if (objectConfig->hasObjectItem)
      addObjectDropItem(objectConfig->path, objectConfig->config);
  }
}

void ItemDatabase::scanRecipes() {
  auto assets = Root::singleton().assets();

  auto& files = assets->scanExtension("recipe");
  assets->queueJsons(files);
  for (auto& file : files) {
    try {
      m_recipes.add(parseRecipe(assets->json(file)));
    } catch (std::exception const& e) {
      Logger::error("Could not load recipe {}: {}", file, outputException(e, false));
    }
  }
}

void ItemDatabase::addBlueprints() {
  auto assets = Root::singleton().assets();

  for (auto const& recipe : m_recipes) {
    auto baseDesc = recipe.output;
    auto baseItem = itemShared(baseDesc);

    String blueprintName = strf("{}-recipe", baseItem->name());
    if (m_items.contains(blueprintName))
      continue;

    try {
      ItemData blueprintData;

      blueprintData.type = ItemType::Blueprint;
      JsonObject configInfo;
      configInfo["recipe"] = baseDesc.singular().toJson();

      String description = assets->json("/blueprint.config:description").toString();
      description = description.replace("<item>", baseItem->friendlyName());
      configInfo["description"] = Json(description);

      String shortDesc = assets->json("/blueprint.config:shortdescription").toString();
      shortDesc = shortDesc.replace("<item>", baseItem->friendlyName());
      configInfo["shortdescription"] = Json(shortDesc);

      configInfo["category"] = assets->json("/blueprint.config:category").toString();

      blueprintData.name = blueprintName;
      blueprintData.friendlyName = shortDesc;
      configInfo["itemName"] = blueprintData.name;

      if (baseItem->instanceValue("inventoryIcon", false))
        configInfo["inventoryIcon"] = baseItem->instanceValue("inventoryIcon");

      configInfo["rarity"] = RarityNames.getRight(baseItem->rarity());

      configInfo["price"] = baseItem->price();

      blueprintData.customConfig = std::move(configInfo);
      blueprintData.directory = itemData(baseDesc.name()).directory;

      m_items[blueprintData.name] = blueprintData;
    } catch (std::exception const& e) {
      Logger::error("Could not create blueprint item from recipe: {}", outputException(e, false));
    }
  }
}

void ItemDatabase::addCodexes() {
  auto assets = Root::singleton().assets();
  auto codexConfig = assets->json("/codex.config");

  ConstPtr<CodexDatabase> codexDatabase = Root::singleton().codexDatabase();
  for (auto const& codexPair : codexDatabase->codexes()) {
    String codexItemName = strf("{}-codex", codexPair.second->id());
    if (m_items.contains(codexItemName)) {
      Logger::warn("Couldn't create codex item {} because an item with that name is already defined", codexItemName);
      continue;
    }

    try {
      ItemData codexItemData;

      codexItemData.type = ItemType::Codex;
      codexItemData.name = codexItemName;
      codexItemData.friendlyName = codexPair.second->title();
      codexItemData.directory = codexPair.second->directory();
      codexItemData.filename = codexPair.second->filename();
      auto customConfig = jsonMerge(codexConfig.get("defaultItemConfig"), codexPair.second->itemConfig()).toObject();
      customConfig["itemName"] = codexItemName;
      customConfig["codexId"] = codexPair.second->id();
      customConfig["shortdescription"] = codexPair.second->title();
      customConfig["description"] = codexPair.second->description();
      customConfig["codexIcon"] = codexPair.second->icon();
      codexItemData.customConfig = customConfig;

      m_items[codexItemName] = codexItemData;
    } catch (std::exception const& e) {
      Logger::error("Could not create item for codex {}: {}", codexPair.second->id(), outputException(e, false));
    }
  }
}

}// namespace Star

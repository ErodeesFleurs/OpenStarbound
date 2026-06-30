#include "StarItemDatabase.hpp"
#include "StarActiveItem.hpp"
#include "StarAlgorithm.hpp"
#include "StarArmors.hpp"
#include "StarAssets.hpp"
#include "StarAugmentItem.hpp"
#include "StarBlueprintItem.hpp"
#include "StarCasting.hpp"
#include "StarCodexDatabase.hpp"
#include "StarCodexItem.hpp"
#include "StarConfigLuaBindings.hpp"
#include "StarConsumableItem.hpp"
#include "StarCurrency.hpp"
#include "StarInspectionTool.hpp"
#include "StarInstrumentItem.hpp"
#include "StarItemDrop.hpp"
#include "StarItemLuaBindings.hpp"
#include "StarJsonExtra.hpp"
#include "StarLiquidItem.hpp"
#include "StarLuaRoot.hpp"
#include "StarMaterialItem.hpp"
#include "StarObjectDatabase.hpp"
#include "StarObjectItem.hpp"
#include "StarRebuilder.hpp"
#include "StarRootLuaBindings.hpp"
#include "StarThrownItem.hpp"
#include "StarTools.hpp"
#include "StarUnlockItem.hpp"
#include "StarUtilityLuaBindings.hpp"

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

uint64_t ItemDatabase::getCountOfItem(List<ItemPtr> const& bag, ItemDescriptor const& item, bool exactMatch) {
  auto normalizedBag = normalizeBag(bag);
  return getCountOfItem(normalizedBag, item, exactMatch);
}

uint64_t ItemDatabase::getCountOfItem(HashMap<ItemDescriptor, uint64_t> const& bag, ItemDescriptor const& item, bool exactMatch) {
  ItemDescriptor matchItem = exactMatch ? item.singular() : ItemDescriptor(item.name(), 1);
  if (!bag.contains(matchItem)) {
    return 0;
  } else {
    return bag.get(matchItem);
  }
}

HashMap<ItemDescriptor, uint64_t> ItemDatabase::normalizeBag(List<ItemPtr> const& bag) {
  HashMap<ItemDescriptor, uint64_t> normalizedBag;
  for (auto const& item : bag) {
    if (!item)
      continue;

    normalizedBag[ItemDescriptor(item->name(), 1)] += item->count();

    if (!item->parameters().toObject().empty())
      normalizedBag[ItemDescriptor(item->name(), 1, item->parameters())] += item->count();
  }

  return normalizedBag;
}

HashSet<ItemRecipe> ItemDatabase::recipesFromSubset(HashMap<ItemDescriptor, uint64_t> const& normalizedBag, StringMap<uint64_t> const& availableCurrencies, HashSet<ItemRecipe> const& subset) {
  HashSet<ItemRecipe> res;
  for (auto const& recipe : subset) {
    // add this recipe if we can make it.
    if (canMakeRecipe(recipe, normalizedBag, availableCurrencies))
      res.add(recipe);
  }

  return res;
}

HashSet<ItemRecipe> ItemDatabase::recipesFromSubset(HashMap<ItemDescriptor, uint64_t> const& normalizedBag, StringMap<uint64_t> const& availableCurrencies,
                                                    HashSet<ItemRecipe> const& subset, StringSet const& allowedTypes) {
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

String ItemDatabase::guiFilterString(ItemPtr const& item) {
  return (item->name() + item->friendlyName() + item->description()).toLower().splitAny(" ,.?*\\+/|\t").join("");
}

bool ItemDatabase::canMakeRecipe(ItemRecipe const& recipe, HashMap<ItemDescriptor, uint64_t> const& availableIngredients, StringMap<uint64_t> const& availableCurrencies) {
  for (auto const& [currencyName, currencyCount] : recipe.currencyInputs) {
    if (availableCurrencies.value(currencyName, 0) < currencyCount)
      return false;
  }

  for (auto const& input : recipe.inputs) {
    ItemDescriptor matchInput = recipe.matchInputParameters ? input.singular() : ItemDescriptor(input.name(), 1);
    if (availableIngredients.value(matchInput) < input.count())
      return false;
  }

  return true;
}

ItemDatabase::ItemDatabase(AssetsConstPtr assets,
                           function<ObjectDatabaseConstPtr()> objectDatabase,
                           LiquidsDatabaseConstPtr liquidsDatabase,
                           FunctionDatabaseConstPtr functionDatabase,
                           CodexDatabaseConstPtr codexDatabase,
                           MaterialDatabaseConstPtr materialDatabase,
                           VersioningDatabaseConstPtr versioningDatabase,
                           ParticleDatabaseConstPtr particleDatabase,
                           ImageMetadataDatabaseConstPtr imageMetadataDatabase,
                           LuaRootServices luaRootServices)
    : m_assets(requireServiceValueAs<ItemException>(std::move(assets), "ItemDatabase", "assets")),
      m_objectDatabase(requireDependencyValueAs<ItemException>(std::move(objectDatabase), "ItemDatabase", "object database provider")),
      m_liquidsDatabase(requireServiceValueAs<ItemException>(std::move(liquidsDatabase), "ItemDatabase", "liquids database")),
      m_functionDatabase(requireServiceValueAs<ItemException>(std::move(functionDatabase), "ItemDatabase", "function database")),
      m_codexDatabase(requireServiceValueAs<ItemException>(std::move(codexDatabase), "ItemDatabase", "codex database")),
      m_materialDatabase(requireServiceValueAs<ItemException>(std::move(materialDatabase), "ItemDatabase", "material database")),
      m_versioningDatabase(requireServiceValueAs<ItemException>(std::move(versioningDatabase), "ItemDatabase", "versioning database")),
      m_particleDatabase(requireServiceValueAs<ItemException>(std::move(particleDatabase), "ItemDatabase", "particle database")),
      m_imageMetadataDatabase(requireServiceValueAs<ItemException>(std::move(imageMetadataDatabase), "ItemDatabase", "image metadata database")),
      m_luaRoot(make_shared<LuaRoot>(requireLuaRootServices(luaRootServices, "ItemDatabase"))),
      m_rebuilder(make_shared<Rebuilder>(m_assets, "item", requireLuaRootServices(std::move(luaRootServices), "ItemDatabase"))) {
  scanItems();
  addObjectItems();
  addCodexes();
  scanRecipes();
  addBlueprints();
}

void ItemDatabase::cleanup() {
  {
    MutexLocker locker(m_cacheMutex);
    m_itemCache.cleanup([](ItemCacheEntry const&, ItemPtr const& item) {
      return item.use_count() != 1;
    });
  }
}

ItemPtr ItemDatabase::diskLoad(Json const& diskStore) const {
  if (diskStore) {
    return item(ItemDescriptor::loadStore(diskStore, m_versioningDatabase));
  } else {
    return {};
  }
}

ItemPtr ItemDatabase::fromJson(Json const& spec) const {
  return item(ItemDescriptor(spec));
}

bool ItemDatabase::loadItem(ItemDescriptor const& descriptor, ItemPtr& itemPtr) const {
  return loadItem<Item>(descriptor, itemPtr);
}

Json ItemDatabase::diskStore(ItemConstPtr const& itemPtr) const {
  if (itemPtr)
    return itemPtr->descriptor().diskStore(m_versioningDatabase);
  else
    return Json();
}

Json ItemDatabase::toJson(ItemConstPtr const& itemPtr) const {
  if (itemPtr)
    return itemPtr->descriptor().toJson();
  else
    return Json();
}

bool ItemDatabase::hasItem(String const& itemName) const {
  return m_items.contains(itemName);
}

ItemType ItemDatabase::itemType(String const& itemName) const {
  return itemData(itemName).type;
}

String ItemDatabase::itemFriendlyName(String const& itemName) const {
  return itemData(itemName).friendlyName;
}

StringSet ItemDatabase::itemTags(String const& itemName) const {
  return itemData(itemName).itemTags;
}

ItemDatabase::ItemConfig ItemDatabase::itemConfig(String const& itemName, Json parameters, Maybe<float> level, Maybe<uint64_t> seed) const {
  auto const& data = itemData(itemName);

  ItemConfig itemConfig;
  if (data.assetsConfig)
    itemConfig.config = m_assets->json(*data.assetsConfig);
  itemConfig.directory = data.directory;
  itemConfig.config = jsonMerge(itemConfig.config, data.customConfig);
  itemConfig.parameters = parameters;

  if (auto builder = itemConfig.config.optString("builder")) {
    RecursiveMutexLocker locker(m_luaMutex);
    auto context = m_luaRoot->createContext(*builder);
    context.setCallbacks("sb", LuaBindings::makeUtilityCallbacks());
    luaTie(itemConfig.config, itemConfig.parameters) = context.invokePath<LuaTupleReturn<Json, Json>>(
      "build", itemConfig.directory, itemConfig.config, itemConfig.parameters, level, seed);
  }

  return itemConfig;
}

Maybe<String> ItemDatabase::itemFile(String const& itemName) const {
  if (!hasItem(itemName)) {
    return {};
  }
  auto const& data = itemData(itemName);
  return data.directory + data.filename;
}

ItemPtr ItemDatabase::itemShared(ItemDescriptor descriptor, Maybe<float> level, Maybe<uint64_t> seed) const {
  if (!descriptor)
    return {};

  ItemCacheEntry entry{descriptor, level, seed};
  MutexLocker locker(m_cacheMutex);
  if (ItemPtr* cached = m_itemCache.ptr(entry))
    return *cached;
  else {
    locker.unlock();

    ItemPtr item = tryCreateItem(descriptor, level, seed);
    get<2>(entry) = item->parameters().optUInt("seed");// Seed could've been changed by the buildscript

    locker.lock();
    return m_itemCache.get(entry, [&](ItemCacheEntry const&) -> ItemPtr { return std::move(item); });
  }
}

ItemPtr ItemDatabase::item(ItemDescriptor descriptor, Maybe<float> level, Maybe<uint64_t> seed, bool ignoreInvalid) const {
  if (!descriptor)
    return {};
  else
    return tryCreateItem(descriptor, level, seed, ignoreInvalid);
}

bool ItemDatabase::hasRecipeToMake(ItemDescriptor const& item) const {
  auto si = item.singular();
  for (auto const& recipe : m_recipes)
    if (recipe.output.singular() == si)
      return true;
  return false;
}

bool ItemDatabase::hasRecipeToMake(ItemDescriptor const& item, StringSet const& allowedTypes) const {
  auto si = item.singular();
  for (auto const& recipe : m_recipes)
    if (recipe.output.singular() == si)
      for (auto allowedType : allowedTypes)
        if (recipe.groups.contains(allowedType))
          return true;
  return false;
}

HashSet<ItemRecipe> ItemDatabase::recipesForOutputItem(String itemName) const {
  HashSet<ItemRecipe> result;
  for (auto const& recipe : m_recipes)
    if (recipe.output.name() == itemName)
      result.add(recipe);
  return result;
}

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(List<ItemPtr> const& bag, StringMap<uint64_t> const& availableCurrencies) const {
  auto normalizedBag = normalizeBag(bag);
  return recipesFromBagContents(normalizedBag, availableCurrencies);
}

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(HashMap<ItemDescriptor, uint64_t> const& bag, StringMap<uint64_t> const& availableCurrencies) const {
  return recipesFromSubset(bag, availableCurrencies, m_recipes);
}

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(List<ItemPtr> const& bag, StringMap<uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const {
  auto normalizedBag = normalizeBag(bag);
  return recipesFromBagContents(normalizedBag, availableCurrencies, allowedTypes);
}

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(HashMap<ItemDescriptor, uint64_t> const& bag, StringMap<uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const {
  return recipesFromSubset(bag, availableCurrencies, m_recipes, allowedTypes);
}

uint64_t ItemDatabase::maxCraftableInBag(List<ItemPtr> const& bag, StringMap<uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const {
  auto normalizedBag = normalizeBag(bag);

  return maxCraftableInBag(normalizedBag, availableCurrencies, recipe);
}

uint64_t ItemDatabase::maxCraftableInBag(HashMap<ItemDescriptor, uint64_t> const& bag, StringMap<uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const {
  uint64_t res = highest<uint64_t>();

  for (auto const& [currencyName, currencyCount] : recipe.currencyInputs) {
    uint64_t available = availableCurrencies.value(currencyName, 0);
    if (available == 0)
      return 0;
    else if (currencyCount > 0)
      res = min(available / currencyCount, res);
  }

  for (auto const& input : recipe.inputs) {
    if (!bag.contains(input.singular()))
      return 0;
    else if (input.count() > 0)
      res = min(bag.get(input.singular()) / input.count(), res);
  }

  return res;
}

ItemRecipe ItemDatabase::getPreciseRecipeForMaterials(String const& group, List<ItemPtr> const& bag, StringMap<uint64_t> const& availableCurrencies) const {
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

ItemRecipe ItemDatabase::parseRecipe(Json const& config) const {
  ItemRecipe res;
  try {
    res.currencyInputs = jsonToMapV<StringMap<uint64_t>>(config.get("currencyInputs", JsonObject()), mem_fn(&Json::toUInt));

    // parse currency items into currency inputs
    auto inputArray = config.getArray("input");
    res.inputs.reserve(res.inputs.size() + inputArray.size());
    for (auto input : inputArray) {
      auto id = ItemDescriptor(input);
      if (itemType(id.name()) == ItemType::CurrencyItem) {
        auto currencyItem = as<CurrencyItem>(itemShared(id));
        res.currencyInputs[currencyItem->currencyType()] += currencyItem->totalValue();
      } else {
        res.inputs.push_back(id);
      }
    }

    res.output = ItemDescriptor(config.get("output"));
    res.duration = config.getFloat("duration", m_assets->json("/items/defaultParameters.config:defaultCraftDuration").toFloat());
    res.groups = StringSet::from(jsonToStringList(config.get("groups", JsonArray())));
    if (auto item = ItemDatabase::itemShared(res.output)) {
      res.outputRarity = item->rarity();
      res.guiFilterString = guiFilterString(item);
    }
    res.collectables = jsonToMapV<StringMap<String>>(config.get("collectables", JsonObject()), mem_fn(&Json::toString));
    res.matchInputParameters = config.getBool("matchInputParameters", false);

  } catch (JsonException const& e) {
    throw RecipeException(strf("Recipe missing required ingredient: {}", outputException(e, false)));
  }

  return res;
}

HashSet<ItemRecipe> const& ItemDatabase::allRecipes() const {
  return m_recipes;
}

HashSet<ItemRecipe> ItemDatabase::allRecipes(StringSet const& types) const {
  HashSet<ItemRecipe> res;
  for (auto const& i : m_recipes) {
    if (i.groups.hasIntersection(types))
      res.add(i);
  }
  return res;
}

ItemPtr ItemDatabase::applyAugment(ItemPtr const item, AugmentItem& augment) const {
  if (item) {
    RecursiveMutexLocker locker(m_luaMutex);
    LuaBaseComponent script;
    script.setLuaRoot(m_luaRoot);
    script.setScripts(augment.augmentScripts());
    script.addCallbacks("item", LuaBindings::makeItemCallbacks(augment));
    script.addCallbacks("config", LuaBindings::makeConfigCallbacks([&augment](String const& name, Json const& def) { return augment.instanceValue(name, def); }));
    script.init();
    auto luaResult = script.invoke<LuaTupleReturn<Json, Maybe<uint64_t>>>("apply", item->descriptor().toJson());
    script.uninit();
    locker.unlock();

    if (luaResult) {
      if (!get<0>(*luaResult).isNull()) {
        augment.take(get<1>(*luaResult).value(1));
        return ItemDatabase::item(ItemDescriptor(get<0>(*luaResult)));
      }
    }
  }

  return item;
}

bool ItemDatabase::ageItem(ItemPtr& item, double aging) const {
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
  auto aged = script.invoke<Json>("ageItem", original.toJson(), aging).apply(construct<ItemDescriptor>());
  script.uninit();
  locker.unlock();

  if (aged && *aged != original) {
    item = ItemDatabase::item(*aged);
    return true;
  }

  return false;
}

List<String> ItemDatabase::allItems() const {
  return m_items.keys();
}

ItemPtr ItemDatabase::createItem(AssetsConstPtr assets, ItemDatabase const& itemDatabase, ItemType type, ItemConfig const& config) {
  if (type == ItemType::Generic) {
    return make_shared<GenericItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::LiquidItem) {
    return make_shared<LiquidItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters, itemDatabase.m_liquidsDatabase);
  } else if (type == ItemType::MaterialItem) {
    return make_shared<MaterialItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters, itemDatabase.m_materialDatabase);
  } else if (type == ItemType::ObjectItem) {
    return make_shared<ObjectItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters, itemDatabase.m_objectDatabase());
  } else if (type == ItemType::CurrencyItem) {
    return make_shared<CurrencyItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory);
  } else if (type == ItemType::MiningTool) {
    return make_shared<MiningTool>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::Flashlight) {
    return make_shared<Flashlight>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::WireTool) {
    return make_shared<WireTool>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::BeamMiningTool) {
    return make_shared<BeamMiningTool>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::PaintingBeamTool) {
    return make_shared<PaintingBeamTool>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::TillingTool) {
    return make_shared<TillingTool>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::HarvestingTool) {
    return make_shared<HarvestingTool>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::HeadArmor) {
    return make_shared<HeadArmor>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters, itemDatabase.m_functionDatabase);
  } else if (type == ItemType::ChestArmor) {
    return make_shared<ChestArmor>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters, itemDatabase.m_functionDatabase);
  } else if (type == ItemType::LegsArmor) {
    return make_shared<LegsArmor>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters, itemDatabase.m_functionDatabase);
  } else if (type == ItemType::BackArmor) {
    return make_shared<BackArmor>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters, itemDatabase.m_functionDatabase);
  } else if (type == ItemType::Consumable) {
    return make_shared<ConsumableItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::Blueprint) {
    return make_shared<BlueprintItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::Codex) {
    return make_shared<CodexItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::InspectionTool) {
    return make_shared<InspectionTool>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::InstrumentItem) {
    return make_shared<InstrumentItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::ThrownItem) {
    return make_shared<ThrownItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::UnlockItem) {
    return make_shared<UnlockItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::ActiveItem) {
    return make_shared<ActiveItem>(assets, itemDatabase.m_imageMetadataDatabase, itemDatabase.m_particleDatabase, config.config, config.directory, config.parameters);
  } else if (type == ItemType::AugmentItem) {
    return make_shared<AugmentItem>(assets, itemDatabase.m_imageMetadataDatabase, config.config, config.directory, itemDatabase, config.parameters);
  } else {
    throw ItemException(strf("Unknown item type {}", static_cast<int>(type)));
  }
}

ItemPtr ItemDatabase::tryCreateItem(ItemDescriptor const& descriptor, Maybe<float> level, Maybe<uint64_t> seed, bool ignoreInvalid) const {
  ItemPtr result;
  ItemDescriptor newDescriptor = descriptor;

  try {
    if (newDescriptor.name() == "perfectlygenericitem" && newDescriptor.parameters().contains("genericItemStorage"))
      newDescriptor = ItemDescriptor(descriptor.parameters().get("genericItemStorage"));
    result = createItem(m_assets, *this, m_items.get(newDescriptor.name()).type, itemConfig(newDescriptor.name(), newDescriptor.parameters(), level, seed));
    result->setCount(descriptor.count());
  } catch (std::exception const& e) {
    if (!ignoreInvalid) {
      bool success = m_rebuilder->rebuild(descriptor.toJson(), strf("{}", outputException(e, false)), [&](Json const& store) -> String {
        try {
          ItemDescriptor newDescriptor(store);
          result = createItem(m_assets, *this, m_items.get(newDescriptor.name()).type, itemConfig(newDescriptor.name(), newDescriptor.parameters(), level, seed));
          result->setCount(newDescriptor.count());
        } catch (std::exception const& e) {
          return strf("{}", outputException(e, false));
        }
        return {};
      });

      if (!success) {
        if (descriptor.name() == "perfectlygenericitem") {
          Logger::error("Could not re-instantiate item '{}'. {}", descriptor, outputException(e, false));
          result = createItem(m_assets, *this, m_items.get("perfectlygenericitem").type, itemConfig("perfectlygenericitem", descriptor.parameters(), level, seed));
        } else {
          Logger::error("Could not instantiate item '{}'. {}", descriptor, outputException(e, false));
          result = createItem(m_assets, *this, m_items.get("perfectlygenericitem").type, itemConfig("perfectlygenericitem", JsonObject({{"genericItemStorage", descriptor.toJson()}, {"shortdescription", descriptor.name()}, {"description", "Reinstall the parent mod to return this item to normal"}}), {}, {}));
        }
      }
    } else
      throw;
  }
  return result;
}

ItemDatabase::ItemData const& ItemDatabase::itemData(String const& name) const {
  if (auto p = m_items.ptr(name))
    return *p;
  throw ItemException::format("No such item '{}'", name);
}

ItemRecipe ItemDatabase::makeRecipe(List<ItemDescriptor> inputs, ItemDescriptor output, float duration, StringSet groups) const {
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
  for (auto& file : m_assets->scanExtension(extension)) {
    ItemData data;
    try {
      auto config = m_assets->json(file);
      data.type = type;
      data.assetsConfig = file;
      data.name = config.get("itemName").toString();
      data.friendlyName = config.getString("shortdescription", {});
      data.itemTags = config.opt("itemTags").apply(jsonToStringSet).value();
      data.agingScripts = config.opt("itemAgingScripts").apply(jsonToStringList).value();
      data.directory = AssetPath::directory(file);
      data.filename = AssetPath::filename(file);

      data.agingScripts = data.agingScripts.transformed([dir = data.directory](String const& s) { return AssetPath::relativeTo(dir, s); });
    } catch (std::exception const& e) {
      throw ItemException(strf("Could not load item asset {}", file), e);
    }

    if (m_items.contains(data.name))
      throw ItemException(strf("Duplicate item name '{}' found", data.name));

    m_items[data.name] = data;
  }
}

void ItemDatabase::addObjectDropItem(String const& objectPath, Json const& objectConfig) {
  ItemData data;
  data.type = ItemType::ObjectItem;
  data.name = objectConfig.get("objectName").toString();
  data.friendlyName = objectConfig.getString("shortdescription", {});
  data.itemTags = objectConfig.opt("itemTags").apply(jsonToStringSet).value();
  data.agingScripts = objectConfig.opt("itemAgingScripts").apply(jsonToStringList).value();
  data.directory = AssetPath::directory(objectPath);
  data.filename = AssetPath::filename(objectPath);
  JsonObject customConfig = objectConfig.toObject();
  if (!customConfig.contains("inventoryIcon")) {
    customConfig["inventoryIcon"] = m_assets->json("/objects/defaultParameters.config:missingIcon");
    Logger::warn(strf("Missing inventoryIcon for {}, using default", data.name).c_str());
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
  List<std::pair<ItemType, String>> itemSets;
  auto scanItemType = [this, &itemSets](ItemType type, String const& extension) {
    itemSets.append(make_pair(type, extension));
    m_assets->queueJsons(m_assets->scanExtension(extension));
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
  auto objectDatabase = m_objectDatabase();

  for (auto const& objectName : objectDatabase->allObjects()) {
    auto objectConfig = objectDatabase->getConfig(objectName);

    if (objectConfig->hasObjectItem)
      addObjectDropItem(objectConfig->path, objectConfig->config);
  }
}

void ItemDatabase::scanRecipes() {
  auto& files = m_assets->scanExtension("recipe");
  m_assets->queueJsons(files);
  for (auto& file : files) {
    try {
      m_recipes.add(parseRecipe(m_assets->json(file)));
    } catch (std::exception const& e) {
      Logger::error("Could not load recipe {}: {}", file, outputException(e, false));
    }
  }
}

void ItemDatabase::addBlueprints() {
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

      String description = m_assets->json("/blueprint.config:description").toString();
      description = description.replace("<item>", baseItem->friendlyName());
      configInfo["description"] = Json(description);

      String shortDesc = m_assets->json("/blueprint.config:shortdescription").toString();
      shortDesc = shortDesc.replace("<item>", baseItem->friendlyName());
      configInfo["shortdescription"] = Json(shortDesc);

      configInfo["category"] = m_assets->json("/blueprint.config:category").toString();

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
  auto codexConfig = m_assets->json("/codex.config");

  for (auto const& [_, codex] : m_codexDatabase->codexes()) {
    String codexItemName = strf("{}-codex", codex->id());
    if (m_items.contains(codexItemName)) {
      Logger::warn("Couldn't create codex item {} because an item with that name is already defined", codexItemName);
      continue;
    }

    try {
      ItemData codexItemData;

      codexItemData.type = ItemType::Codex;
      codexItemData.name = codexItemName;
      codexItemData.friendlyName = codex->title();
      codexItemData.directory = codex->directory();
      codexItemData.filename = codex->filename();
      auto customConfig = jsonMerge(codexConfig.get("defaultItemConfig"), codex->itemConfig()).toObject();
      customConfig["itemName"] = codexItemName;
      customConfig["codexId"] = codex->id();
      customConfig["shortdescription"] = codex->title();
      customConfig["description"] = codex->description();
      customConfig["codexIcon"] = codex->icon();
      codexItemData.customConfig = customConfig;

      m_items[codexItemName] = codexItemData;
    } catch (std::exception const& e) {
      Logger::error("Could not create item for codex {}: {}", codex->id(), outputException(e, false));
    }
  }
}

}// namespace Star

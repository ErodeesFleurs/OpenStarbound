import star.json_extra;
import star.json;
import star.thread;
import star.data_stream;
import star.bi_map;
import star.ordered_map;
import star.lru_cache;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.set;
import star.string;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.strong_typedef;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;
import star.id_map;
import star.maybe;
import star.variant;
import star.rpc_promise;
import star.lua;
import star.spline;
import star.a_star;
import star.audio;
import star.map;

import star.lua_root;
import star.lua_components;
import star.lua_actor_movement_component;
import star.drawable;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.item;
import star.item_database;
import star.item_descriptor;
import star.item_recipe;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.mobile_entity;
import star.scripted_entity;
import star.game_timers;
import star.item_drop;
import star.augment_item;
import star.non_rotated_drawables_item;
import star.platformer_astar_types;
import star.anchorable_entity;
import star.actor_movement_controller;
import star.actor_entity;
import star.tool_user_entity;
import star.tool_user_item;
import star.beam_item;
import star.status_effect_item;
import star.fireable_item;
import star.swingable_item;
import star.durability_item;
import star.pointable_item;
import star.mixer;
import star.entity_rendering_types;
import star.animation;
import star.particle;
import star.entity_rendering;
struct PreviewTile;
import star.preview_tile_tool;
import star.tools;
import star.effect_source_item;
import star.previewable_item;
import star.armors;
import star.utility_lua_bindings;
import star.rebuilder;

import star.thrown_item;
import star.object_item;
import star.unlock_item;
import star.blueprint_item;
import star.codex_item;
import star.currency_item;
import star.consumable_item;
import star.inspection_tool;
import star.instrument_item;
import star.liquid_item;
import star.material_item;
import star.active_item;
import star.flashlight;
import star.tilling_tool;
import star.config_lua_bindings;
import star.root_lua_bindings;
import star.item_lua_bindings;
import star.codex_database;
import star.codex;
import star.object_database;
import star.versioning_database;

import star.math_common;
import star.algorithm;
namespace Star {

extern EnumMap<ItemType> const ItemTypeNames{
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
  {ItemType::AugmentItem, "augmentitem"}
};

std::uint64_t ItemDatabase::getCountOfItem(List<std::shared_ptr<Item>> const& bag, ItemDescriptor const& item, bool exactMatch) {
  auto normalizedBag = normalizeBag(bag);
  return getCountOfItem(normalizedBag, item, exactMatch);
}

std::uint64_t ItemDatabase::getCountOfItem(HashMap<ItemDescriptor, std::uint64_t> const& bag, ItemDescriptor const& item, bool exactMatch) {
  ItemDescriptor matchItem = exactMatch ? item.singular() : ItemDescriptor(item.name(), 1);
  if (!bag.contains(matchItem)) {
    return 0;
  } else {
    return bag.get(matchItem);
  }
}

HashMap<ItemDescriptor, std::uint64_t> ItemDatabase::normalizeBag(List<std::shared_ptr<Item>> const& bag) {
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

HashSet<ItemRecipe> ItemDatabase::recipesFromSubset(HashMap<ItemDescriptor, std::uint64_t> const& normalizedBag, StringMap<std::uint64_t> const& availableCurrencies, HashSet<ItemRecipe> const& subset) {
  HashSet<ItemRecipe> res;
  for (auto const& recipe : subset) {
    // add this recipe if we can make it.
    if (canMakeRecipe(recipe, normalizedBag, availableCurrencies))
      res.add(recipe);
  }

  return res;
}

HashSet<ItemRecipe> ItemDatabase::recipesFromSubset(HashMap<ItemDescriptor, std::uint64_t> const& normalizedBag, StringMap<std::uint64_t> const& availableCurrencies,
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

String ItemDatabase::guiFilterString(std::shared_ptr<Item> const& item) {
  return (item->name() + item->friendlyName() + item->description()).toLower().splitAny(" ,.?*\\+/|\t").join("");
}

bool ItemDatabase::canMakeRecipe(ItemRecipe const& recipe, HashMap<ItemDescriptor, std::uint64_t> const& availableIngredients, StringMap<std::uint64_t> const& availableCurrencies) {
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

ItemDatabase::ItemDatabase(Root& root)
  : m_root(root), m_luaRoot(std::make_shared<LuaRoot>(root)),
    m_itemCache(root.random()) {
  scanItems();
  addObjectItems();
  addCodexes();
  // Builders below can query the complete base catalog without re-entering
  // Root's still-unpublished ItemDatabase loader.
  m_rebuilder = std::make_shared<Rebuilder>(root, "item", this);
  scanRecipes();
  addBlueprints();
}

void ItemDatabase::cleanup() {
  {
    MutexLocker locker(m_cacheMutex);
    m_itemCache.cleanup([](ItemCacheEntry const&, std::shared_ptr<Item> const& item) {
      return item.use_count() != 1;
    });
  }
}


ItemDescriptor ItemDatabase::loadDescriptor(Json const& store) const {
  return ItemDescriptor::loadStore(*m_root.versioningDatabase(), store);
}
std::shared_ptr<Item> ItemDatabase::diskLoad(Json const& diskStore) const {
  if (diskStore) {
    return item(loadDescriptor(diskStore));
  } else {
    return {};
  }
}

std::shared_ptr<Item> ItemDatabase::fromJson(Json const& spec) const {
  return item(ItemDescriptor(spec));
}

Json ItemDatabase::diskStore(std::shared_ptr<Item const> const& itemPtr) const {
  if (itemPtr)
    return itemPtr->descriptor().diskStore(*m_root.versioningDatabase());
  else
    return Json();
}

Json ItemDatabase::toJson(std::shared_ptr<Item const> const& itemPtr) const {
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

ItemDatabase::ItemConfig ItemDatabase::itemConfig(String const& itemName, Json parameters, Maybe<float> level, Maybe<std::uint64_t> seed) const {
  auto const& data = itemData(itemName);

  ItemConfig itemConfig;
  if (data.assetsConfig)
    itemConfig.config = m_root.assets()->json(*data.assetsConfig);
  itemConfig.directory = data.directory;
  itemConfig.config = jsonMerge(itemConfig.config, data.customConfig);
  itemConfig.parameters = parameters;

  if (auto builder = itemConfig.config.optString("builder")) {
    RecursiveMutexLocker locker(m_luaMutex);
    auto context = m_luaRoot->createContext(*builder);
    context.setCallbacks("root", LuaBindings::makeRootCallbacks(m_root, this));
    context.setCallbacks("sb", LuaBindings::makeUtilityCallbacks(m_root.random(), m_root.logger(), m_root.logMap()));
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

std::shared_ptr<Item> ItemDatabase::itemShared(ItemDescriptor descriptor, Maybe<float> level, Maybe<std::uint64_t> seed) const {
  if (!descriptor)
    return {};

  ItemCacheEntry entry{ descriptor, level, seed };
  MutexLocker locker(m_cacheMutex);
  if (std::shared_ptr<Item>* cached = m_itemCache.ptr(entry))
    return *cached;
  else {
    locker.unlock();

    std::shared_ptr<Item> item = tryCreateItem(descriptor, level, seed);
    std::get<2>(entry) = item->parameters().optUInt("seed"); // Seed could've been changed by the buildscript

    locker.lock();
    return m_itemCache.get(entry, [&](ItemCacheEntry const&) -> std::shared_ptr<Item> { return std::move(item); });
  }
}

std::shared_ptr<Item> ItemDatabase::item(ItemDescriptor descriptor, Maybe<float> level, Maybe<std::uint64_t> seed, bool ignoreInvalid) const {
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

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(List<std::shared_ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const {
  auto normalizedBag = normalizeBag(bag);
  return recipesFromBagContents(normalizedBag, availableCurrencies);
}

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const {
  return recipesFromSubset(bag, availableCurrencies, m_recipes);
}

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(List<std::shared_ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const {
  auto normalizedBag = normalizeBag(bag);
  return recipesFromBagContents(normalizedBag, availableCurrencies, allowedTypes);
}

HashSet<ItemRecipe> ItemDatabase::recipesFromBagContents(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies, StringSet const& allowedTypes) const {
  return recipesFromSubset(bag, availableCurrencies, m_recipes, allowedTypes);
}

std::uint64_t ItemDatabase::maxCraftableInBag(List<std::shared_ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const {
  auto normalizedBag = normalizeBag(bag);

  return maxCraftableInBag(normalizedBag, availableCurrencies, recipe);
}

std::uint64_t ItemDatabase::maxCraftableInBag(HashMap<ItemDescriptor, std::uint64_t> const& bag, StringMap<std::uint64_t> const& availableCurrencies, ItemRecipe const& recipe) const {
  std::uint64_t res = std::numeric_limits<std::uint64_t>::max();

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

ItemRecipe ItemDatabase::getPreciseRecipeForMaterials(String const& group, List<std::shared_ptr<Item>> const& bag, StringMap<std::uint64_t> const& availableCurrencies) const {
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
    res.currencyInputs = jsonToMapV<StringMap<std::uint64_t>>(config.get("currencyInputs", JsonObject()), std::mem_fn(&Json::toUInt));

    // parse currency items into currency inputs
    for (auto input : config.getArray("input")) {
      auto id = ItemDescriptor(input);
      if (itemType(id.name()) == ItemType::CurrencyItem) {
        auto currencyItem = as<CurrencyItem>(itemShared(id, {}, {}));
        res.currencyInputs[currencyItem->currencyType()] += currencyItem->totalValue();
      } else {
        res.inputs.push_back(id);
      }
    }

    res.output = ItemDescriptor(config.get("output"));
    res.duration = config.getFloat("duration", m_root.assets()->json("/items/defaultParameters.config:defaultCraftDuration").toFloat());
    res.groups = StringSet::from(jsonToStringList(config.get("groups", JsonArray())));
    if (auto item = ItemDatabase::itemShared(res.output, {}, {})) {
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

std::shared_ptr<Item> ItemDatabase::applyAugment(std::shared_ptr<Item> const item, AugmentItem* augment) const {
  if (item) {
    RecursiveMutexLocker locker(m_luaMutex);
    LuaBaseComponent script(m_root);
    script.removeCallbacks("root");
    script.addCallbacks("root", LuaBindings::makeRootCallbacks(m_root, this));
    script.setLuaRoot(m_luaRoot);
    script.setScripts(augment->augmentScripts());
    script.addCallbacks("item", LuaBindings::makeItemCallbacks(augment));
    script.addCallbacks("config", LuaBindings::makeConfigCallbacks(std::bind(&Item::instanceValue, augment, std::placeholders::_1, std::placeholders::_2)));
    script.init();
    auto luaResult = script.invoke<LuaTupleReturn<Json, Maybe<std::uint64_t>>>("apply", item->descriptor().toJson());
    script.uninit();
    locker.unlock();

    if (luaResult) {
      if (!std::get<0>(*luaResult).isNull()) {
        augment->take(std::get<1>(*luaResult).value(1));
        return ItemDatabase::item(ItemDescriptor(std::get<0>(*luaResult)), {}, {}, false);
      }
    }
  }

  return item;
}

bool ItemDatabase::ageItem(std::shared_ptr<Item>& item, double aging) const {
  if (!item)
    return false;

  auto const& itemData = ItemDatabase::itemData(item->name());
  if (itemData.agingScripts.empty())
    return false;

  ItemDescriptor original = item->descriptor();

  RecursiveMutexLocker locker(m_luaMutex);
  LuaBaseComponent script(m_root);
  script.removeCallbacks("root");
  script.addCallbacks("root", LuaBindings::makeRootCallbacks(m_root, this));
  script.setLuaRoot(m_luaRoot);
  script.setScripts(itemData.agingScripts);
  script.init();
  auto aged = script.invoke<Json>("ageItem", original.toJson(), aging).apply(construct<ItemDescriptor>());
  script.uninit();
  locker.unlock();

  if (aged && *aged != original) {
    item = ItemDatabase::item(*aged, {}, {}, false);
    return true;
  }

  return false;
}

List<String> ItemDatabase::allItems() const {
  return m_items.keys();
}

std::shared_ptr<Item> ItemDatabase::createItem(ItemType type, ItemConfig const& config) const {
  if (type == ItemType::Generic) {
    return std::make_shared<GenericItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::LiquidItem) {
    return std::make_shared<LiquidItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::MaterialItem) {
    return std::make_shared<MaterialItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::ObjectItem) {
    return std::make_shared<ObjectItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::CurrencyItem) {
    return std::make_shared<CurrencyItem>(m_root, config.config, config.directory);
  } else if (type == ItemType::MiningTool) {
    return std::make_shared<MiningTool>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::Flashlight) {
    return std::make_shared<Flashlight>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::WireTool) {
    return std::make_shared<WireTool>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::BeamMiningTool) {
    return std::make_shared<BeamMiningTool>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::PaintingBeamTool) {
    return std::make_shared<PaintingBeamTool>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::TillingTool) {
    return std::make_shared<TillingTool>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::HarvestingTool) {
    return std::make_shared<HarvestingTool>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::HeadArmor) {
    return std::make_shared<HeadArmor>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::ChestArmor) {
    return std::make_shared<ChestArmor>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::LegsArmor) {
    return std::make_shared<LegsArmor>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::BackArmor) {
    return std::make_shared<BackArmor>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::Consumable) {
    return std::make_shared<ConsumableItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::Blueprint) {
    return std::make_shared<BlueprintItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::Codex) {
    return std::make_shared<CodexItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::InspectionTool) {
    return std::make_shared<InspectionTool>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::InstrumentItem) {
    return std::make_shared<InstrumentItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::ThrownItem) {
    return std::make_shared<ThrownItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::UnlockItem) {
    return std::make_shared<UnlockItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::ActiveItem) {
    return std::make_shared<ActiveItem>(m_root, config.config, config.directory, config.parameters);
  } else if (type == ItemType::AugmentItem) {
    return std::make_shared<AugmentItem>(m_root, config.config, config.directory, config.parameters);
  } else {
    throw ItemException(strf("Unknown item type {}", (int)type));
  }
}

std::shared_ptr<Item> ItemDatabase::tryCreateItem(ItemDescriptor const& descriptor, Maybe<float> level, Maybe<std::uint64_t> seed, bool ignoreInvalid) const {
  std::shared_ptr<Item> result;
  ItemDescriptor newDescriptor = descriptor;

  try {
    if (newDescriptor.name() == "perfectlygenericitem" && newDescriptor.parameters().contains("genericItemStorage"))
      newDescriptor = ItemDescriptor(descriptor.parameters().get("genericItemStorage"));
    result = createItem(m_items.get(newDescriptor.name()).type, itemConfig(newDescriptor.name(), newDescriptor.parameters(), level, seed));
    result->setCount(descriptor.count());
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    if (!ignoreInvalid) {
      bool success = m_rebuilder->rebuild(descriptor.toJson(), strf("{}", outputException(e, false)), [&](Json const& store) -> String {
        try {
          ItemDescriptor newDescriptor(store);
          result = createItem(m_items.get(newDescriptor.name()).type, itemConfig(newDescriptor.name(), newDescriptor.parameters(), level, seed));
          result->setCount(newDescriptor.count());
        }
        catch (std::exception const&) {
          ExceptionDiagnostic const e(std::current_exception());
          return strf("{}", outputException(e, false));
        }
        return {};
      });

      if (!success) {
        if (descriptor.name() == "perfectlygenericitem") {
          m_root.logger().error("Could not re-instantiate item '{}'. {}", descriptor, outputException(e, false));
          result = createItem(m_items.get("perfectlygenericitem").type, itemConfig("perfectlygenericitem", descriptor.parameters(), level, seed));
          result->setCount(descriptor.count());
        } else {
          m_root.logger().error("Could not instantiate item '{}'. {}", descriptor, outputException(e, false));
          result = createItem(m_items.get("perfectlygenericitem").type, itemConfig("perfectlygenericitem", JsonObject({
            {"genericItemStorage", descriptor.toJson().eraseKey("count")},
            {"shortdescription", descriptor.name()},
            {"description", "Reinstall the parent mod to return this item to normal"}
          }), {}, {}));
          result->setCount(descriptor.count());
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
  if (auto item = ItemDatabase::itemShared(res.output, {}, {})) {
    res.outputRarity = item->rarity();
    res.guiFilterString = guiFilterString(item);
  }
  return res;
}

void ItemDatabase::addItemSet(ItemType type, String const& extension) {
  auto assets = m_root.assets();
  for (auto& file : assets->scanExtension(extension)) {
    ItemData data;
    try {
      auto config = assets->json(file);
      data.type = type;
      data.assetsConfig = file;
      data.name = config.get("itemName").toString();
      data.friendlyName = config.getString("shortdescription", {});
      data.itemTags = config.opt("itemTags").apply(jsonToStringSet).value();
      data.agingScripts = config.opt("itemAgingScripts").apply(jsonToStringList).value();
      data.directory = AssetPath::directory(file);
      data.filename = AssetPath::filename(file);

      data.agingScripts = data.agingScripts.transformed(std::bind(&AssetPath::relativeTo, data.directory, std::placeholders::_1));
    } catch (std::exception const&) {
      ExceptionDiagnostic const e(std::current_exception());
      throw ItemException(strf("Could not load item asset {}", file), e);
    }

    if (m_items.contains(data.name))
      throw ItemException(strf("Duplicate item name '{}' found", data.name));

    m_items[data.name] = data;
  }
}

void ItemDatabase::addObjectDropItem(String const& objectPath, Json const& objectConfig) {
  auto assets = m_root.assets();

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
    customConfig["inventoryIcon"] = assets->json("/objects/defaultParameters.config:missingIcon");
    m_root.logger().warn("Missing inventoryIcon for {}, using default", data.name);
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
  auto assets = m_root.assets();

  List<std::pair<ItemType, String>> itemSets;
  auto scanItemType = [&itemSets, assets](ItemType type, String const& extension) {
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
  auto objectDatabase = m_root.objectDatabase();

  for (auto const& objectName : objectDatabase->allObjects()) {
    auto objectConfig = objectDatabase->getConfig(objectName);

    if (objectConfig->hasObjectItem)
      addObjectDropItem(objectConfig->path, objectConfig->config);
  }
}

void ItemDatabase::scanRecipes() {
  auto assets = m_root.assets();

  auto& files = assets->scanExtension("recipe");
  assets->queueJsons(files);
  for (auto& file : files) {
    try {
      m_recipes.add(parseRecipe(assets->json(file)));
    } catch (std::exception const&) {
      ExceptionDiagnostic const e(std::current_exception());
      m_root.logger().error("Could not load recipe {}: {}", file, outputException(e, false));
    }
  }
}

void ItemDatabase::addBlueprints() {
  auto assets = m_root.assets();

  for (auto const& recipe : m_recipes) {
    auto baseDesc = recipe.output;
    auto baseItem = itemShared(baseDesc, {}, {});

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
    } catch (std::exception const&) {
      ExceptionDiagnostic const e(std::current_exception());
      m_root.logger().error("Could not create blueprint item from recipe: {}", outputException(e, false));
    }
  }
}

void ItemDatabase::addCodexes() {
  auto assets = m_root.assets();
  auto codexConfig = assets->json("/codex.config");

  auto codexDatabase = m_root.codexDatabase();
  for (auto const& codexPair : codexDatabase->codexes()) {
    String codexItemName = strf("{}-codex", codexPair.second->id());
    if (m_items.contains(codexItemName)) {
      m_root.logger().warn("Couldn't create codex item {} because an item with that name is already defined", codexItemName);
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
    } catch (std::exception const&) {
      ExceptionDiagnostic const e(std::current_exception());
      m_root.logger().error("Could not create item for codex {}: {}", codexPair.second->id(), outputException(e, false));
    }
  }
}

}

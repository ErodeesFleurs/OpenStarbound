module;
import star.system_world;
import star.json_extra;
import star.json;
import star.xx_hash;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
import star.casting;
import star.game_types;
import star.liquid_types;
import star.material_types;
import star.rpc_promise;
import star.vector;
import star.rect;
import star.bi_map;
import star.a_star;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.perlin;
import star.weighted_pool;
import star.strong_typedef;
import star.lua;
import star.parametric_function;
import star.multi_array_interpolator;
import star.multi_table;
import star.set;
import star.string;
import star.color;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;
import star.periodic_function;
import star.audio;
import star.map;
import star.image;


import star.lua_root;

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
import star.drawable;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.collision_block;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.spawn_type_database;
import star.biome;
import star.lua_game_converters;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.animation;
import star.particle;
import star.animated_part_set;
import star.mixer;
import star.networked_animator;
import star.humanoid;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.emote_entity;
import star.entity_rendering_types;
import star.lounging_entities;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lua_components;
import star.lua_actor_movement_component;
import star.lua_animation_component;
import star.effect_emitter;
import star.npc;
import star.entity_rendering;
import star.aggressive_entity;
import star.monster_database;
import star.monster;
import star.dungeon_generator;
import star.image_lua_bindings;

module star.root_lua_bindings;
import star.stored_functions;
import star.treasure;
import star.name_generator;
import star.projectile_database;
import star.liquids_database;
import star.behavior_database;
import star.species_database;
import star.tech_database;
import star.collection_database;
import star.damage_database;
import star.status_effect_database;
import star.quest_template_database;
import star.tenant_database;
import star.versioning_database;
import star.npc_database;
import star.image_metadata_database;
import star.biome_database;
import star.material_database;


namespace Star::LuaBindings {
  namespace RootCallbacks {
    String assetData(Root* root, String const& path);
    Image assetImage(Root* root, String const& path);
    Json assetFrames(Root* root, String const& path);
    Json assetJson(Root* root, String const& path);
    Json makeCurrentVersionedJson(Root* root, String const& identifier, Json const& content);
    Json loadVersionedJson(Root* root, ItemDatabase const* itemCatalog, Json const& versionedJson, String const& expectedIdentifier);
    double evalFunction(Root* root, String const& arg1, double arg2);
    double evalFunction2(Root* root, String const& arg1, double arg2, double arg3);
    Vec2U imageSize(Root* root, String const& arg1);
    List<Vec2I> imageSpaces(Root* root, String const& arg1, Vec2F const& arg2, float arg3, bool arg4);
    RectU nonEmptyRegion(Root* root, String const& arg1);
    Json npcConfig(Root* root, String const& arg1);
    float projectileGravityMultiplier(Root* root, String const& arg1);
    Json projectileConfig(Root* root, String const& arg1);
    JsonArray recipesForItem(ItemDatabase const* database, String const& arg1);
    JsonArray allRecipes(ItemDatabase const* database, std::optional<StringSet> filter);
    String itemType(ItemDatabase const* database, String const& itemName);
    Json itemTags(ItemDatabase const* database, String const& itemName);
    bool itemHasTag(ItemDatabase const* database, String const& itemName, String const& itemTag);
    Json itemConfig(ItemDatabase const* database, Json const& descriptor, std::optional<float> const& level, std::optional<std::uint64_t> const& seed);
    Json createItem(ItemDatabase const* database, Json const& descriptor, std::optional<float> const& level, std::optional<std::uint64_t> const& seed);
    Json tenantConfig(Root* root, String const& tenantName);
    JsonArray getMatchingTenants(Root* root, StringMap<unsigned> const& colonyTags);
    Json liquidStatusEffects(Root* root, LiquidId arg1);
    String generateName(Root* root, String const& rulesAsset, std::optional<std::uint64_t> seed);
    Json questConfig(Root* root, String const& templateId);
    JsonArray npcPortrait(Root* root, ItemDatabase const* itemCatalog,
        String const& portraitMode,
        String const& species,
        String const& typeName,
        float level,
        std::optional<std::uint64_t> seed,
        std::optional<JsonObject> const& parameters);
    Json npcVariant(Root* root, ItemDatabase const* itemCatalog,
        String const& species,
        String const& typeName,
        float level,
        std::optional<std::uint64_t> seed,
        std::optional<JsonObject> const& parameters);
    JsonArray monsterPortrait(Root* root, String const& typeName, std::optional<JsonObject> const& parameters);
    bool isTreasurePool(Root* root, String const& pool);
    JsonArray createTreasure(Root* root, ItemDatabase const* itemCatalog, String const& pool, float level, std::optional<std::uint64_t> seed);
    std::optional<String> materialMiningSound(Root* root, String const& materialName, std::optional<String> const& modName);
    std::optional<String> materialFootstepSound(Root* root, String const& materialName, std::optional<String> const& modName);
  }
}

namespace Star {

enum VariantTerrainLayer { Background, Platform, Foreground };
EnumMap<VariantTerrainLayer> const VariantTerrainLayerNames{
  {VariantTerrainLayer::Background, "Background"},
  {VariantTerrainLayer::Platform, "Platform"},
  {VariantTerrainLayer::Foreground, "Foreground"},
};

LuaCallbacks LuaBindings::makeRootCallbacks(Root& owner, ItemDatabase const* itemCatalog) {
  LuaCallbacks callbacks;

  auto root = &owner;
  // Builders borrow their ready catalog; regular contexts resolve the current
  // resource per invocation so reload does not freeze an obsolete database.
  auto itemCallback = [root, itemCatalog]<typename Result, typename... Args>(Result (*callback)(ItemDatabase const*, Args...)) {
    return [root, itemCatalog, callback](Args... args) -> Result {
      auto databaseOwner = itemCatalog ? std::shared_ptr<ItemDatabase>() : root->itemDatabase();
      return callback(itemCatalog ? itemCatalog : databaseOwner.get(), std::forward<Args>(args)...);
    };
  };

  callbacks.registerCallbackWithSignature<String, String>("assetData", std::bind(RootCallbacks::assetData, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Image, String>("assetImage", std::bind(RootCallbacks::assetImage, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Json, String>("assetFrames", std::bind(RootCallbacks::assetFrames, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Json, String>("assetJson", std::bind(RootCallbacks::assetJson, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Json, String, Json>("makeCurrentVersionedJson", std::bind(RootCallbacks::makeCurrentVersionedJson, root, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<Json, Json, String>("loadVersionedJson", std::bind(RootCallbacks::loadVersionedJson, root, itemCatalog, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<double, String, double>("evalFunction", std::bind(RootCallbacks::evalFunction, root, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<double, String, double, double>("evalFunction2", std::bind(RootCallbacks::evalFunction2, root, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  callbacks.registerCallbackWithSignature<Vec2U, String>("imageSize", std::bind(RootCallbacks::imageSize, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<List<Vec2I>, String, Vec2F, float, bool>("imageSpaces", std::bind(RootCallbacks::imageSpaces, root, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  callbacks.registerCallbackWithSignature<RectU, String>("nonEmptyRegion", std::bind(RootCallbacks::nonEmptyRegion, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Json, String>("npcConfig", std::bind(RootCallbacks::npcConfig, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<float, String>("projectileGravityMultiplier", std::bind(RootCallbacks::projectileGravityMultiplier, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Json, String>("projectileConfig", std::bind(RootCallbacks::projectileConfig, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<JsonArray, String>("recipesForItem", itemCallback(RootCallbacks::recipesForItem));
  callbacks.registerCallbackWithSignature<JsonArray, std::optional<StringSet>>("allRecipes", itemCallback(RootCallbacks::allRecipes));
  callbacks.registerCallbackWithSignature<String, String>("itemType", itemCallback(RootCallbacks::itemType));
  callbacks.registerCallbackWithSignature<Json, String>("itemTags", itemCallback(RootCallbacks::itemTags));
  callbacks.registerCallbackWithSignature<bool, String, String>("itemHasTag", itemCallback(RootCallbacks::itemHasTag));
  callbacks.registerCallbackWithSignature<Json, Json, std::optional<float>, std::optional<std::uint64_t>>("itemConfig", itemCallback(RootCallbacks::itemConfig));
  callbacks.registerCallbackWithSignature<Json, Json, std::optional<float>, std::optional<std::uint64_t>>("createItem", itemCallback(RootCallbacks::createItem));
  callbacks.registerCallbackWithSignature<Json, String>("tenantConfig", std::bind(RootCallbacks::tenantConfig, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Json, StringMap<unsigned>>("getMatchingTenants", std::bind(RootCallbacks::getMatchingTenants, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Json, LiquidId>("liquidStatusEffects", std::bind(RootCallbacks::liquidStatusEffects, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<String, String, std::optional<std::uint64_t>>("generateName", std::bind(RootCallbacks::generateName, root, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<Json, String>("questConfig", std::bind(RootCallbacks::questConfig, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<JsonArray, String, String, String, float, std::optional<std::uint64_t>, std::optional<JsonObject>>("npcPortrait", std::bind(RootCallbacks::npcPortrait, root, itemCatalog, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  callbacks.registerCallbackWithSignature<Json, String, String, float, std::optional<std::uint64_t>, std::optional<JsonObject>>("npcVariant", std::bind(RootCallbacks::npcVariant, root, itemCatalog, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  callbacks.registerCallbackWithSignature<JsonArray, String, std::optional<JsonObject>>("monsterPortrait", std::bind(RootCallbacks::monsterPortrait, root, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<bool, String>("isTreasurePool", std::bind(RootCallbacks::isTreasurePool, root, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<JsonArray, String, float, std::optional<std::uint64_t>>("createTreasure", std::bind(RootCallbacks::createTreasure, root, itemCatalog, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  callbacks.registerCallbackWithSignature<std::optional<String>, String, std::optional<String>>("materialMiningSound", std::bind(RootCallbacks::materialMiningSound, root, std::placeholders::_1, std::placeholders::_2));
  callbacks.registerCallbackWithSignature<std::optional<String>, String, std::optional<String>>("materialFootstepSound", std::bind(RootCallbacks::materialFootstepSound, root, std::placeholders::_1, std::placeholders::_2));

  auto getVariant = [](unsigned x, unsigned y, bool isMatMod, VariantTerrainLayer layer, unsigned variants) -> unsigned {
    XXHash32 state(2938728349);
    state.push(reinterpret_cast<char const*>(&x), sizeof(x));
    state.push(reinterpret_cast<char const*>(&y), sizeof(y));
    state.push(reinterpret_cast<char const*>(&layer), sizeof(layer));
    if (isMatMod)
      state.push("mod", 3);
    else
      state.push("main", 4);
    return state.digest() % variants;
  };

  callbacks.registerCallback("materialVariant", [getVariant](Vec2U const& tilePosition, String const& layer, unsigned variants) -> unsigned {
    return getVariant(tilePosition.x(), tilePosition.y(), false, VariantTerrainLayerNames.getLeft(layer), variants);
  });

  callbacks.registerCallback("modVariant", [getVariant](Vec2U const& tilePosition, String const& layer, unsigned variants) -> unsigned {
    return getVariant(tilePosition.x(), tilePosition.y(), true, VariantTerrainLayerNames.getLeft(layer), variants);
  });

  callbacks.registerCallback("assetsByExtension", [root](String const& extension) -> CaseInsensitiveStringSet {
    return root->assets()->scanExtension(extension);
  });

  callbacks.registerCallback("assetsScan", [root]( std::optional<String> const& a, std::optional<String> const& b) -> StringList {
    return b ? root->assets()->scan(a.value_or(String{}), *b) : root->assets()->scan(a.value_or(String{}));
  });

  callbacks.registerCallback("assetOrigin", [root](String const& path) -> std::optional<String> {
      auto assets = root->assets();
      if (auto descriptor = assets->assetDescriptor(path))
        return assets->assetSourcePath(descriptor->source);
      return {};
    });

  callbacks.registerCallback("assetPatches", [root](LuaEngine& engine, String const& path) -> std::optional<LuaTable> {
      auto assets = root->assets();
      if (auto descriptor = assets->assetDescriptor(path)) {
        auto& patches = descriptor->patchSources;
        auto table = engine.createTable(patches.size(), 0);
        for (std::size_t i = 0; i != patches.size(); ++i) {
          auto& patch = patches.at(i);
          auto patchTable = engine.createTable(2, 0);
          if (auto sourcePath = assets->assetSourcePath(patch.second))
            patchTable.set(1, *sourcePath);
          patchTable.set(2, patch.first);
          table.set(i + 1, patchTable);
        }
        return table;
      }
      return {};
    });

  callbacks.registerCallback("assetSourcePaths", [root](LuaEngine& engine, std::optional<bool> withMetadata) -> LuaTable {
      auto assets = root->assets();
      auto assetSources = assets->assetSources();
      auto table = engine.createTable(assetSources.size(), 0);
      if (withMetadata.value_or(false)) {
        for (auto& assetSource : assetSources)
          table.set(assetSource, assets->assetSourceMetadata(assetSource));
      }
      else {
        std::size_t i = 0;
        for (auto& assetSource : assetSources)
          table.set(++i, assetSource);
      }
      return table;
    });

  callbacks.registerCallback("assetSourceMetadata", [root](String const& assetSourcePath) {
    auto assets = root->assets();
    return assets->assetSourceMetadata(assetSourcePath);
  });

  callbacks.registerCallback("itemFile", [root, itemCatalog](String const& itemName) -> std::optional<String> {
      auto databaseOwner = itemCatalog ? std::shared_ptr<ItemDatabase>() : root->itemDatabase();
      return (itemCatalog ? itemCatalog : databaseOwner.get())->itemFile(itemName);
    });

  callbacks.registerCallback("materialConfig", [root](std::variant<MaterialId, String> const& materialNameOrId) -> Json {
      MaterialId materialId;
      if (auto id = std::get_if<MaterialId>(&materialNameOrId))
        materialId = *id;
      else
        materialId = root->materialDatabase()->materialId(std::get<String>(materialNameOrId));

      if (auto path = root->materialDatabase()->materialPath(materialId))
        return JsonObject{{"path", *path}, {"config", root->materialDatabase()->materialConfig(materialId).value()}};
      return {};
    });

  callbacks.registerCallback("modConfig", [root](std::variant<ModId, String> const& modNameOrId) -> Json {
      ModId modId;
      if (auto id = std::get_if<ModId>(&modNameOrId))
        modId = *id;
      else
        modId = root->materialDatabase()->modId(std::get<String>(modNameOrId));

      if (auto path = root->materialDatabase()->modPath(modId))
        return JsonObject{{"path", *path}, {"config", root->materialDatabase()->modConfig(modId).value()}};
      return {};
    });

  callbacks.registerCallback("liquidConfig", [root](LuaEngine& engine, LuaValue nameOrId) -> Json {
      LiquidId liquidId;
      if (auto id = engine.luaMaybeTo<std::uint8_t>(nameOrId))
        liquidId = *id;
      else if (auto name = engine.luaMaybeTo<String>(nameOrId))
        liquidId = root->liquidsDatabase()->liquidId(*name);
      else
        return {};

      if (auto path = root->liquidsDatabase()->liquidPath(liquidId))
        return JsonObject{{"path", *path}, {"config", root->liquidsDatabase()->liquidConfig(liquidId).value()}};
      return {};
    });

  callbacks.registerCallback("liquidName", [root](LiquidId liquidId) -> String {
      return root->liquidsDatabase()->liquidName(liquidId);
    });

  callbacks.registerCallback("liquidId", [root](String liquidName) -> LiquidId {
      return root->liquidsDatabase()->liquidId(liquidName);
    });

  callbacks.registerCallback("monsterSkillParameter", [root](String const& skillName, String const& configParameterName) {
      return root->monsterDatabase()->skillConfigParameter(skillName, configParameterName);
    });

  callbacks.registerCallback("monsterParameters", [root](String const& monsterType, std::optional<std::uint64_t> seed) {
      return root->monsterDatabase()->monsterVariant(monsterType, seed.value_or(0)).parameters;
    });

  callbacks.registerCallback("monsterMovementSettings", [root](String const& monsterType, std::optional<std::uint64_t> seed) {
      return root->monsterDatabase()->monsterVariant(monsterType, seed.value_or(0)).movementSettings;
    });

  callbacks.registerCallback("createBiome", [root](String const& biomeName, std::uint64_t seed, float verticalMidPoint, float threatLevel) {
      try {
        return root->biomeDatabase()->createBiome(biomeName, seed, verticalMidPoint, threatLevel)->toJson();
      } catch (BiomeException const&) {
        return Json();
      }
    });

  callbacks.registerCallback("materialHealth", [root](String const& materialName) {
      auto materialId = root->materialDatabase()->materialId(materialName);
      return root->materialDatabase()->materialDamageParameters(materialId).totalHealth();
    });

  callbacks.registerCallback("techType", [root](String const& techName) {
      return TechTypeNames.getRight(root->techDatabase()->tech(techName).type);
    });

  callbacks.registerCallback("hasTech", [root](String const& tech) {
      return root->techDatabase()->contains(tech);
    });

  callbacks.registerCallback("techConfig", [root](String const& tech) {
      return root->techDatabase()->tech(tech).parameters;
    });

  callbacks.registerCallbackWithSignature<std::optional<String>, String>("treeStemDirectory", [root](String const& stemName) {
      return root->plantDatabase()->treeStemDirectory(stemName);
    });

  callbacks.registerCallbackWithSignature<std::optional<String>, String>("treeFoliageDirectory", [root](String const& foliageName) {
      return root->plantDatabase()->treeFoliageDirectory(foliageName);
    });

  callbacks.registerCallback("collection", [root](String const& collectionName) {
      return root->collectionDatabase()->collection(collectionName);
    });

  callbacks.registerCallback("collectables", [root, itemCatalog](String const& collectionName) {
      return root->collectionDatabase()->collectables(collectionName, itemCatalog);
    });

  callbacks.registerCallback("elementalResistance", [root](String const& damageKindName) -> String {
      DamageKind const& damageKind = root->damageDatabase()->damageKind(damageKindName);
      return root->damageDatabase()->elementalType(damageKind.elementalType).resistanceStat;
    });
  callbacks.registerCallback("elementalType", [root](String const& damageKindName) -> String {
      return root->damageDatabase()->damageKind(damageKindName).elementalType;
    });

  callbacks.registerCallback("dungeonMetadata", [root](String const& name) {
      return root->dungeonDefinitions()->getMetadata(name);
    });

  callbacks.registerCallback("systemObjectTypeConfig", [root](String const& name) -> Json {
      return SystemWorld::systemObjectTypeConfig(*root, name);
    });

  callbacks.registerCallback("itemDescriptorsMatch", [](Json const& descriptor1, Json const& descriptor2, std::optional<bool> exactMatch) -> bool {
      return ItemDescriptor(descriptor1).matches(ItemDescriptor(descriptor2), exactMatch.value_or(false));
    });

  callbacks.registerCallback("getConfiguration", [root](String const& key) -> Json {
      if (key == "title")
        throw StarException(strf("Cannot get {}", key));
      else
        return root->configuration()->get(key);
    });

  callbacks.registerCallback("setConfiguration", [root](String const& key, Json const& value) {
    if (key == "safeScripts" || key == "safe")
      throw StarException(strf("Cannot set {}", key));
    else
      root->configuration()->set(key, value);
    });


  callbacks.registerCallback("getConfigurationPath", [root](String const& path) -> Json {
    if (path.empty() || path.beginsWith("title"))
      throw ConfigurationException(strf("cannot get {}", path));
    else
      return root->configuration()->getPath(path);
    });

  callbacks.registerCallback("setConfigurationPath", [root](String const& path, Json const& value) {
    if (path.empty() || path.beginsWith("safeScripts") || path.splitAny("[].").get(0) == "safe")
      throw ConfigurationException(strf("cannot set {}", path));
    else
      root->configuration()->setPath(path, value);
    });

  callbacks.registerCallback("speciesConfig", [root](String const& species) -> Json {
    return root->speciesDatabase()->species(species)->config();
  });

  callbacks.registerCallback("generateHumanoidIdentity", [root, itemCatalog](String const& species, std::optional<std::uint64_t> seed, std::optional<String> gender) -> LuaTupleReturn<Json, JsonObject, JsonObject> {
    auto result = root->speciesDatabase()->generateHumanoid(species, seed.value_or(root->random().randu64()), gender.has_value() ? GenderNames.getLeft(*gender) : std::optional<Gender>(), itemCatalog);
    return LuaTupleReturn<Json, JsonObject, JsonObject>{result.identity.toJson(), result.humanoidParameters, result.armor};
  });
  callbacks.copyCallback("generateHumanoidIdentity", "generateHumanoid");
  callbacks.registerCallback("createHumanoid", [root, itemCatalog](
    String name,
    String speciesChoice,
    std::size_t genderChoice,
    std::size_t bodyColorChoice,
    std::size_t alty,
    std::size_t hairChoice,
    std::size_t heady,
    std::size_t shirtChoice,
    std::size_t shirtColor,
    std::size_t pantsChoice,
    std::size_t pantsColor,
    std::size_t personality,
    LuaVariadic<LuaValue> ext
  ) -> LuaTupleReturn<Json, JsonObject, JsonObject> {
    auto result = root->speciesDatabase()->createHumanoid(
      name,
      speciesChoice,
      genderChoice,
      bodyColorChoice,
      alty,
      hairChoice,
      heady,
      shirtChoice,
      shirtColor,
      pantsChoice,
      pantsColor,
      personality,
      ext,
      itemCatalog
    );
    return LuaTupleReturn<Json, JsonObject, JsonObject>{result.identity.toJson(), result.humanoidParameters, result.armor};
  });

  callbacks.registerCallback("effectConfig", [root](String const& effect) -> Json {
    return root->statusEffectDatabase()->uniqueEffectConfig(effect).toJson();
  });

  callbacks.registerCallback("monsterConfig", [root](String const& typeName) -> Json {
    return root->monsterDatabase()->monsterConfig(typeName);
  });

  callbacks.registerCallback("biomeConfig", [root](String const& typeName) -> Json {
    return root->biomeDatabase()->biomeConfig(typeName);
  });

  callbacks.registerCallback("treeFoliageConfig", [root](String const& typeName) -> Json {
    return root->plantDatabase()->treeFoliageConfig(typeName);
  });
  callbacks.registerCallback("treeStemConfig", [root](String const& typeName) -> Json {
    return root->plantDatabase()->treeStemConfig(typeName);
  });

  return callbacks;
}

String LuaBindings::RootCallbacks::assetData(Root* root, String const& path) {
  auto bytes = root->assets()->bytes(path);
  return String(bytes->ptr(), bytes->size());
}

Image LuaBindings::RootCallbacks::assetImage(Root* root, String const& path) {
  return *root->assets()->image(path);
}

Json LuaBindings::RootCallbacks::assetFrames(Root* root, String const& path) {
  if (auto frames = root->assets()->imageFrames(path))
    return frames->toJson();
  return Json();
}

Json LuaBindings::RootCallbacks::assetJson(Root* root, String const& path) {
  return root->assets()->json(path);
}

Json LuaBindings::RootCallbacks::makeCurrentVersionedJson(Root* root, String const& identifier, Json const& content) {
  return root->versioningDatabase()->makeCurrentVersionedJson(identifier, content).toJson();
}

Json LuaBindings::RootCallbacks::loadVersionedJson(Root* root, ItemDatabase const* itemCatalog, Json const& versionedJson, String const& identifier) {
  return root->versioningDatabase()->loadVersionedJson(VersionedJson::fromJson(versionedJson), identifier, itemCatalog);
}

double LuaBindings::RootCallbacks::evalFunction(Root* root, String const& arg1, double arg2) {
  return root->functionDatabase()->function(arg1)->evaluate(arg2);
}

double LuaBindings::RootCallbacks::evalFunction2(Root* root, String const& arg1, double arg2, double arg3) {
  return root->functionDatabase()->function2(arg1)->evaluate(arg2, arg3);
}

Vec2U LuaBindings::RootCallbacks::imageSize(Root* root, String const& arg1) {
  return root->imageMetadataDatabase()->imageSize(arg1);
}

List<Vec2I> LuaBindings::RootCallbacks::imageSpaces(
    Root* root, String const& arg1, Vec2F const& arg2, float arg3, bool arg4) {
  return root->imageMetadataDatabase()->imageSpaces(arg1, arg2, arg3, arg4);
}

RectU LuaBindings::RootCallbacks::nonEmptyRegion(Root* root, String const& arg1) {
  return root->imageMetadataDatabase()->nonEmptyRegion(arg1);
}

Json LuaBindings::RootCallbacks::npcConfig(Root* root, String const& arg1) {
  return root->npcDatabase()->buildConfig(arg1);
}

float LuaBindings::RootCallbacks::projectileGravityMultiplier(Root* root, String const& arg1) {
  auto projectileDatabase = root->projectileDatabase();
  return projectileDatabase->gravityMultiplier(arg1);
}

Json LuaBindings::RootCallbacks::projectileConfig(Root* root, String const& arg1) {
  auto projectileDatabase = root->projectileDatabase();
  return projectileDatabase->projectileConfig(arg1);
}

JsonArray LuaBindings::RootCallbacks::recipesForItem(ItemDatabase const* database, String const& arg1) {
  auto recipes = database->recipesForOutputItem(arg1);
  JsonArray result;
  result.reserve(recipes.size());
  for (auto& recipe : recipes)
    result.append(recipe.toJson());
  return result;
}

JsonArray LuaBindings::RootCallbacks::allRecipes(ItemDatabase const* database, std::optional<StringSet> filter) {
  HashSet<ItemRecipe> recipes;
  if (filter.has_value())
    recipes = database->allRecipes(filter.value());
  else
    recipes = database->allRecipes();

  JsonArray result;
  result.reserve(recipes.size());
  for (auto& recipe : recipes)
    result.append(recipe.toJson());
  return result;
}

String LuaBindings::RootCallbacks::itemType(ItemDatabase const* database, String const& itemName) {
  return ItemTypeNames.getRight(database->itemType(itemName));
}

Json LuaBindings::RootCallbacks::itemTags(ItemDatabase const* database, String const& itemName) {
  return jsonFromStringSet(database->itemTags(itemName));
}

bool LuaBindings::RootCallbacks::itemHasTag(ItemDatabase const* database, String const& itemName, String const& itemTag) {
  return database->itemTags(itemName).contains(itemTag);
}

Json LuaBindings::RootCallbacks::itemConfig(ItemDatabase const* database, Json const& descJson, std::optional<float> const& level, std::optional<std::uint64_t> const& seed) {
  ItemDescriptor descriptor(descJson);
  if (!database->hasItem(descriptor.name()))
    return {};
  auto config = database->itemConfig(descriptor.name(), descriptor.parameters(), level, seed);
  return JsonObject{{"directory", config.directory}, {"config", config.config}, {"parameters", config.parameters}};
}

Json LuaBindings::RootCallbacks::createItem(ItemDatabase const* database, Json const& descriptor, std::optional<float> const& level, std::optional<std::uint64_t> const& seed) {
  auto item = database->item(ItemDescriptor(descriptor), level, seed);
  return item->descriptor().toJson();
}

Json LuaBindings::RootCallbacks::tenantConfig(Root* root, String const& tenantName) {
  return root->tenantDatabase()->getTenant(tenantName)->config;
}

JsonArray LuaBindings::RootCallbacks::getMatchingTenants(Root* root, StringMap<unsigned> const& colonyTags) {
  return root->tenantDatabase()
      ->getMatchingTenants(colonyTags)
      .transformed([](std::shared_ptr<Tenant> const& tenant) { return tenant->config; });
}

Json LuaBindings::RootCallbacks::liquidStatusEffects(Root* root, LiquidId arg1) {
  if (auto liquidSettings = root->liquidsDatabase()->liquidSettings(arg1))
    return liquidSettings->statusEffects;
  return Json();
}

String LuaBindings::RootCallbacks::generateName(Root* root, String const& rulesAsset, std::optional<std::uint64_t> seed) {
  return root->nameGenerator()->generateName(rulesAsset, seed.value_or(root->random().randu64()));
}

Json LuaBindings::RootCallbacks::questConfig(Root* root, String const& templateId) {
  if (auto quest = root->questTemplateDatabase()->questTemplate(templateId))
    return quest->config;
  throw StarException(strf("No quest template found for id '{}'", templateId));
}

JsonArray LuaBindings::RootCallbacks::npcPortrait(Root* root, ItemDatabase const* itemCatalog,
    String const& portraitMode,
    String const& species,
    String const& typeName,
    float level,
    std::optional<std::uint64_t> seed,
    std::optional<JsonObject> const& parameters) {
  auto npcDatabase = root->npcDatabase();
  auto npcVariant = npcDatabase->generateNpcVariant(species, typeName, level, seed.value_or(root->random().randu64()), parameters.value_or(JsonObject{}), itemCatalog);

  auto drawables = npcDatabase->npcPortrait(npcVariant, PortraitModeNames.getLeft(portraitMode), itemCatalog);
  return drawables.transformed(std::mem_fn(&Drawable::toJson));
}

Json LuaBindings::RootCallbacks::npcVariant(Root* root, ItemDatabase const* itemCatalog, String const& species, String const& typeName, float level, std::optional<std::uint64_t> seed, std::optional<JsonObject> const& parameters) {
  auto npcDatabase = root->npcDatabase();
  auto npcVariant = npcDatabase->generateNpcVariant(
      species, typeName, level, seed.value_or(root->random().randu64()), parameters.value_or(JsonObject{}), itemCatalog);
  return npcDatabase->writeNpcVariantToJson(npcVariant);
}

JsonArray LuaBindings::RootCallbacks::monsterPortrait(Root* root, String const& typeName, std::optional<JsonObject> const& parameters) {
  auto monsterDatabase = root->monsterDatabase();
  auto seed = 0; // use a static seed to utilize caching
  auto monsterVariant = monsterDatabase->monsterVariant(typeName, seed, parameters.value_or(JsonObject{}));
  auto drawables = monsterDatabase->monsterPortrait(monsterVariant);
  return drawables.transformed(std::mem_fn(&Drawable::toJson));
}

bool LuaBindings::RootCallbacks::isTreasurePool(Root* root, String const& pool) {
  return root->treasureDatabase()->isTreasurePool(pool);
}

JsonArray LuaBindings::RootCallbacks::createTreasure(
    Root* root, ItemDatabase const* itemCatalog, String const& pool, float level, std::optional<std::uint64_t> seed) {
  auto treasure = root->treasureDatabase()->createTreasure(pool, level, seed.value_or(root->random().randu64()), itemCatalog);
  return treasure.transformed([](std::shared_ptr<Item> const& item) { return item->descriptor().toJson(); });
}

std::optional<String> LuaBindings::RootCallbacks::materialMiningSound(
    Root* root, String const& materialName, std::optional<String> const& modName) {
  auto materialDatabase = root->materialDatabase();
  auto materialId = materialDatabase->materialId(materialName);
  auto modId = modName.transform(std::bind(&MaterialDatabase::modId, materialDatabase, std::placeholders::_1)).value_or(NoModId);
  auto sound = materialDatabase->miningSound(materialId, modId);
  if (sound.empty())
    return {};
  return sound;
}

std::optional<String> LuaBindings::RootCallbacks::materialFootstepSound(
    Root* root, String const& materialName, std::optional<String> const& modName) {
  auto materialDatabase = root->materialDatabase();
  auto materialId = materialDatabase->materialId(materialName);
  auto modId = modName.transform(std::bind(&MaterialDatabase::modId, materialDatabase, std::placeholders::_1)).value_or(NoModId);
  auto sound = materialDatabase->footstepSound(materialId, modId);
  if (sound.empty())
    return {};
  return sound;
}

}

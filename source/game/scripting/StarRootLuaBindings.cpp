#include "StarRootLuaBindings.hpp"

#include "StarBiomeDatabase.hpp"// IWYU pragma: export
#include "StarCollectionDatabase.hpp"
#include "StarConfig.hpp"
#include "StarDamageDatabase.hpp"
#include "StarDungeonGenerator.hpp"     // IWYU pragma: export
#include "StarImageMetadataDatabase.hpp"// IWYU pragma: export
#include "StarItemDatabase.hpp"
#include "StarJsonExtra.hpp"
#include "StarLiquidsDatabase.hpp"      // IWYU pragma: export
#include "StarMaterialDatabase.hpp"     // IWYU pragma: export
#include "StarMonsterDatabase.hpp"      // IWYU pragma: export
#include "StarNameGenerator.hpp"        // IWYU pragma: export
#include "StarNpcDatabase.hpp"          // IWYU pragma: export
#include "StarProjectileDatabase.hpp"   // IWYU pragma: export
#include "StarQuestTemplateDatabase.hpp"// IWYU pragma: export
#include "StarRoot.hpp"
#include "StarSpeciesDatabase.hpp"
#include "StarStatusEffectDatabase.hpp"
#include "StarStoredFunctions.hpp"// IWYU pragma: export
#include "StarSystemWorld.hpp"
#include "StarTechDatabase.hpp"
#include "StarTenantDatabase.hpp"
#include "StarTreasure.hpp"// IWYU pragma: export
#include "StarVersioningDatabase.hpp"

import std;

namespace Star {

auto LuaBindings::makeRootCallbacks() -> LuaCallbacks {
  LuaCallbacks callbacks;

  auto root = Root::singletonPtr();

  callbacks.registerCallbackWithSignature<String, String>("assetData", [root](auto&& PH1) -> auto { return RootCallbacks::assetData(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Image, String>("assetImage", [root](auto&& PH1) -> auto { return RootCallbacks::assetImage(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, String>("assetFrames", [root](auto&& PH1) -> auto { return RootCallbacks::assetFrames(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, String>("assetJson", [root](auto&& PH1) -> auto { return RootCallbacks::assetJson(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, String, Json>("makeCurrentVersionedJson", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::makeCurrentVersionedJson(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, Json, String>("loadVersionedJson", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::loadVersionedJson(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<double, String, double>("evalFunction", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::evalFunction(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<double, String, double, double>("evalFunction2", [root](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return RootCallbacks::evalFunction2(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<Vec2U, String>("imageSize", [root](auto&& PH1) -> auto { return RootCallbacks::imageSize(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<List<Vec2I>, String, Vec2F, float, bool>("imageSpaces", [root](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4) -> auto { return RootCallbacks::imageSpaces(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4)); });
  callbacks.registerCallbackWithSignature<RectU, String>("nonEmptyRegion", [root](auto&& PH1) -> auto { return RootCallbacks::nonEmptyRegion(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, String>("npcConfig", [root](auto&& PH1) -> auto { return RootCallbacks::npcConfig(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<float, String>("projectileGravityMultiplier", [root](auto&& PH1) -> auto { return RootCallbacks::projectileGravityMultiplier(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, String>("projectileConfig", [root](auto&& PH1) -> auto { return RootCallbacks::projectileConfig(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<JsonArray, String>("recipesForItem", [root](auto&& PH1) -> auto { return RootCallbacks::recipesForItem(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<JsonArray>("allRecipes", [root] -> JsonArray { return RootCallbacks::allRecipes(root); });
  callbacks.registerCallbackWithSignature<String, String>("itemType", [root](auto&& PH1) -> auto { return RootCallbacks::itemType(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, String>("itemTags", [root](auto&& PH1) -> auto { return RootCallbacks::itemTags(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<bool, String, String>("itemHasTag", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::itemHasTag(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, Json, std::optional<float>, std::optional<std::uint64_t>>("itemConfig", [root](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return RootCallbacks::itemConfig(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<Json, Json, std::optional<float>, std::optional<std::uint64_t>>("createItem", [root](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return RootCallbacks::createItem(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });
  callbacks.registerCallbackWithSignature<Json, String>("tenantConfig", [root](auto&& PH1) -> auto { return RootCallbacks::tenantConfig(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, StringMap<unsigned>>("getMatchingTenants", [root](auto&& PH1) -> auto { return RootCallbacks::getMatchingTenants(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<Json, LiquidId>("liquidStatusEffects", [root](auto&& PH1) -> auto { return RootCallbacks::liquidStatusEffects(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<String, String, std::optional<std::uint64_t>>("generateName", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::generateName(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<Json, String>("questConfig", [root](auto&& PH1) -> auto { return RootCallbacks::questConfig(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<JsonArray, String, String, String, float, std::optional<std::uint64_t>, std::optional<JsonObject>>("npcPortrait", [root](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5, auto&& PH6) -> auto { return RootCallbacks::npcPortrait(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6)); });
  callbacks.registerCallbackWithSignature<Json, String, String, float, std::optional<std::uint64_t>, std::optional<JsonObject>>("npcVariant", [root](auto&& PH1, auto&& PH2, auto&& PH3, auto&& PH4, auto&& PH5) -> auto { return RootCallbacks::npcVariant(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3), std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5)); });
  callbacks.registerCallbackWithSignature<JsonArray, String, std::optional<JsonObject>>("monsterPortrait", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::monsterPortrait(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<bool, String>("isTreasurePool", [root](auto&& PH1) -> auto { return RootCallbacks::isTreasurePool(root, std::forward<decltype(PH1)>(PH1)); });
  callbacks.registerCallbackWithSignature<JsonArray, String, float, std::optional<std::uint64_t>>("createTreasure", [root](auto&& PH1, auto&& PH2, auto&& PH3) -> auto { return RootCallbacks::createTreasure(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3)); });

  callbacks.registerCallbackWithSignature<std::optional<String>, String, std::optional<String>>("materialMiningSound", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::materialMiningSound(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
  callbacks.registerCallbackWithSignature<std::optional<String>, String, std::optional<String>>("materialFootstepSound", [root](auto&& PH1, auto&& PH2) -> auto { return RootCallbacks::materialFootstepSound(root, std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });

  callbacks.registerCallback("assetsByExtension", [root](String const& extension) -> CaseInsensitiveStringSet {
    return root->assets()->scanExtension(extension);
  });

  callbacks.registerCallback("assetsScan", [root](std::optional<String> const& a, std::optional<String> const& b) -> StringList {
    return b ? root->assets()->scan(a.value(), *b) : root->assets()->scan(a.value());
  });

  callbacks.registerCallback("assetOrigin", [root](String const& path) -> std::optional<String> {
    auto assets = root->assets();
    if (auto descriptor = assets->assetDescriptor(path))
      return assets->assetSourcePath(descriptor->source);
    return std::nullopt;
  });

  callbacks.registerCallback("assetPatches", [root](LuaEngine& engine, String const& path) -> std::optional<LuaTable> {
    auto assets = root->assets();
    if (auto descriptor = assets->assetDescriptor(path)) {
      auto& patches = descriptor->patchSources;
      auto table = engine.createTable(patches.size(), 0);
      for (size_t i = 0; i != patches.size(); ++i) {
        auto& patch = patches.at(i);
        auto patchTable = engine.createTable(2, 0);
        if (auto sourcePath = assets->assetSourcePath(patch.second))
          patchTable.set(1, *sourcePath);
        patchTable.set(2, patch.first);
        table.set(i + 1, patchTable);
      }
      return table;
    }
    return std::nullopt;
  });

  callbacks.registerCallback("assetSourcePaths", [root](LuaEngine& engine, std::optional<bool> withMetadata) -> LuaTable {
    auto assets = root->assets();
    auto assetSources = assets->assetSources();
    auto table = engine.createTable(assetSources.size(), 0);
    if (withMetadata.value()) {
      for (auto& assetSource : assetSources)
        table.set(assetSource, assets->assetSourceMetadata(assetSource));
    } else {
      size_t i = 0;
      for (auto& assetSource : assetSources)
        table.set(++i, assetSource);
    }
    return table;
  });

  callbacks.registerCallback("assetSourceMetadata", [root](String const& assetSourcePath) -> JsonObject {
    auto assets = root->assets();
    return assets->assetSourceMetadata(assetSourcePath);
  });

  callbacks.registerCallback("itemFile", [root](String const& itemName) -> std::optional<String> {
    return root->itemDatabase()->itemFile(itemName);
  });

  callbacks.registerCallback("materialConfig", [root](String const& materialName) -> Json {
    auto materialId = root->materialDatabase()->materialId(materialName);
    if (auto path = root->materialDatabase()->materialPath(materialId))
      return JsonObject{{"path", *path}, {"config", root->materialDatabase()->materialConfig(materialId).value()}};
    return {};
  });

  callbacks.registerCallback("modConfig", [root](String const& modName) -> Json {
    auto modId = root->materialDatabase()->modId(modName);
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

  callbacks.registerCallback("monsterSkillParameter", [root](String const& skillName, String const& configParameterName) -> Json {
    return root->monsterDatabase()->skillConfigParameter(skillName, configParameterName);
  });

  callbacks.registerCallback("monsterParameters", [root](String const& monsterType, std::optional<std::uint64_t> seed) -> Json {
    return root->monsterDatabase()->monsterVariant(monsterType, seed.value_or(0)).parameters;
  });

  callbacks.registerCallback("monsterMovementSettings", [root](String const& monsterType, std::optional<std::uint64_t> seed) -> ActorMovementParameters {
    return root->monsterDatabase()->monsterVariant(monsterType, seed.value_or(0)).movementSettings;
  });

  callbacks.registerCallback("createBiome", [root](String const& biomeName, std::uint64_t seed, float verticalMidPoint, float threatLevel) -> auto {
    try {
      return root->biomeDatabase()->createBiome(biomeName, seed, verticalMidPoint, threatLevel)->toJson();
    } catch (BiomeException const&) {
      return Json();
    }
  });

  callbacks.registerCallback("materialHealth", [root](String const& materialName) -> float {
    auto materialId = root->materialDatabase()->materialId(materialName);
    return root->materialDatabase()->materialDamageParameters(materialId).totalHealth();
  });

  callbacks.registerCallback("techType", [root](String const& techName) -> auto {
    return TechTypeNames.getRight(root->techDatabase()->tech(techName).type);
  });

  callbacks.registerCallback("hasTech", [root](String const& tech) -> bool {
    return root->techDatabase()->contains(tech);
  });

  callbacks.registerCallback("techConfig", [root](String const& tech) -> Json {
    return root->techDatabase()->tech(tech).parameters;
  });

  callbacks.registerCallbackWithSignature<std::optional<String>, String>("treeStemDirectory", [root](String const& stemName) -> std::optional<String> {
    return root->plantDatabase()->treeStemDirectory(stemName);
  });

  callbacks.registerCallbackWithSignature<std::optional<String>, String>("treeFoliageDirectory", [root](String const& foliageName) -> std::optional<String> {
    return root->plantDatabase()->treeFoliageDirectory(foliageName);
  });

  callbacks.registerCallback("collection", [root](String const& collectionName) -> Collection {
    return root->collectionDatabase()->collection(collectionName);
  });

  callbacks.registerCallback("collectables", [root](String const& collectionName) -> List<Collectable> {
    return root->collectionDatabase()->collectables(collectionName);
  });

  callbacks.registerCallback("elementalResistance", [root](String const& damageKindName) -> String {
    DamageKind const& damageKind = root->damageDatabase()->damageKind(damageKindName);
    return root->damageDatabase()->elementalType(damageKind.elementalType).resistanceStat;
  });
  callbacks.registerCallback("elementalType", [root](String const& damageKindName) -> String {
    return root->damageDatabase()->damageKind(damageKindName).elementalType;
  });

  callbacks.registerCallback("dungeonMetadata", [root](String const& name) -> JsonObject {
    return root->dungeonDefinitions()->getMetadata(name);
  });

  callbacks.registerCallback("systemObjectTypeConfig", [](String const& name) -> Json {
    return SystemWorld::systemObjectTypeConfig(name);
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

  callbacks.registerCallback("setConfiguration", [root](String const& key, Json const& value) -> void {
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

  callbacks.registerCallback("setConfigurationPath", [root](String const& path, Json const& value) -> void {
    if (path.empty() || path.beginsWith("safeScripts") || path.splitAny("[].").get(0) == "safe")
      throw ConfigurationException(strf("cannot set {}", path));
    else
      root->configuration()->setPath(path, value);
  });

  callbacks.registerCallback("speciesConfig", [root](String const& species) -> Json {
    return root->speciesDatabase()->species(species)->config();
  });

  callbacks.registerCallback("generateHumanoidIdentity", [root](String const& species, std::optional<std::uint64_t> seed, std::optional<String> gender) -> LuaTupleReturn<Json, JsonObject, JsonObject> {
    auto result = root->speciesDatabase()->generateHumanoid(species, seed.value_or(Random::randu64()), gender.has_value() ? GenderNames.getLeft(*gender) : std::optional<Gender>());
    return LuaTupleReturn<Json, JsonObject, JsonObject>{result.identity.toJson(), result.humanoidParameters, result.armor};
  });
  callbacks.copyCallback("generateHumanoidIdentity", "generateHumanoid");
  callbacks.registerCallback("createHumanoid", [root](String name, String speciesChoice, size_t genderChoice, size_t bodyColorChoice, size_t alty, size_t hairChoice, size_t heady, size_t shirtChoice, size_t shirtColor, size_t pantsChoice, size_t pantsColor, size_t personality, LuaVariadic<LuaValue> ext) -> LuaTupleReturn<Json, JsonObject, JsonObject> {
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
      ext);
    return LuaTupleReturn<Json, JsonObject, JsonObject>{result.identity.toJson(), result.humanoidParameters, result.armor};
  });

  callbacks.registerCallback("effectConfig", [root](String const& effect) -> Json {
    return root->statusEffectDatabase()->uniqueEffectConfig(effect).toJson();
  });

  callbacks.registerCallback("monsterConfig", [root](String const& typeName) -> Json {
    return root->monsterDatabase()->monsterConfig(typeName);
  });

  return callbacks;
}

auto LuaBindings::RootCallbacks::assetData(Root* root, String const& path) -> String {
  auto bytes = root->assets()->bytes(path);
  return {bytes->ptr(), bytes->size()};
}

auto LuaBindings::RootCallbacks::assetImage(Root* root, String const& path) -> Image {
  return *root->assets()->image(path);
}

auto LuaBindings::RootCallbacks::assetFrames(Root* root, String const& path) -> Json {
  if (auto frames = root->assets()->imageFrames(path))
    return frames->toJson();
  return {};
}

auto LuaBindings::RootCallbacks::assetJson(Root* root, String const& path) -> Json {
  return root->assets()->json(path);
}

auto LuaBindings::RootCallbacks::makeCurrentVersionedJson(Root* root, String const& identifier, Json const& content) -> Json {
  return root->versioningDatabase()->makeCurrentVersionedJson(identifier, content).toJson();
}

auto LuaBindings::RootCallbacks::loadVersionedJson(Root* root, Json const& versionedJson, String const& identifier) -> Json {
  return root->versioningDatabase()->loadVersionedJson(VersionedJson::fromJson(versionedJson), identifier);
}

auto LuaBindings::RootCallbacks::evalFunction(Root* root, String const& arg1, double arg2) -> double {
  return root->functionDatabase()->function(arg1)->evaluate(arg2);
}

auto LuaBindings::RootCallbacks::evalFunction2(Root* root, String const& arg1, double arg2, double arg3) -> double {
  return root->functionDatabase()->function2(arg1)->evaluate(arg2, arg3);
}

auto LuaBindings::RootCallbacks::imageSize(Root* root, String const& arg1) -> Vec2U {
  return root->imageMetadataDatabase()->imageSize(arg1);
}

auto LuaBindings::RootCallbacks::imageSpaces(
  Root* root, String const& arg1, Vec2F const& arg2, float arg3, bool arg4) -> List<Vec2I> {
  return root->imageMetadataDatabase()->imageSpaces(arg1, arg2, arg3, arg4);
}

auto LuaBindings::RootCallbacks::nonEmptyRegion(Root* root, String const& arg1) -> RectU {
  return root->imageMetadataDatabase()->nonEmptyRegion(arg1);
}

auto LuaBindings::RootCallbacks::npcConfig(Root* root, String const& arg1) -> Json {
  return root->npcDatabase()->buildConfig(arg1);
}

auto LuaBindings::RootCallbacks::projectileGravityMultiplier(Root* root, String const& arg1) -> float {
  auto projectileDatabase = root->projectileDatabase();
  return projectileDatabase->gravityMultiplier(arg1);
}

auto LuaBindings::RootCallbacks::projectileConfig(Root* root, String const& arg1) -> Json {
  auto projectileDatabase = root->projectileDatabase();
  return projectileDatabase->projectileConfig(arg1);
}

auto LuaBindings::RootCallbacks::recipesForItem(Root* root, String const& arg1) -> JsonArray {
  auto recipes = root->itemDatabase()->recipesForOutputItem(arg1);
  JsonArray result;
  result.reserve(recipes.size());
  for (auto& recipe : recipes)
    result.append(recipe.toJson());
  return result;
}

auto LuaBindings::RootCallbacks::allRecipes(Root* root) -> JsonArray {
  auto& recipes = root->itemDatabase()->allRecipes();
  JsonArray result;
  result.reserve(recipes.size());
  for (auto& recipe : recipes)
    result.append(recipe.toJson());
  return result;
}

auto LuaBindings::RootCallbacks::itemType(Root* root, String const& itemName) -> String {
  return ItemTypeNames.getRight(root->itemDatabase()->itemType(itemName));
}

auto LuaBindings::RootCallbacks::itemTags(Root* root, String const& itemName) -> Json {
  return jsonFromStringSet(root->itemDatabase()->itemTags(itemName));
}

auto LuaBindings::RootCallbacks::itemHasTag(Root* root, String const& itemName, String const& itemTag) -> bool {
  return root->itemDatabase()->itemTags(itemName).contains(itemTag);
}

auto LuaBindings::RootCallbacks::itemConfig(Root* root, Json const& descJson, std::optional<float> const& level, std::optional<std::uint64_t> const& seed) -> Json {
  ItemDescriptor descriptor(descJson);
  if (!root->itemDatabase()->hasItem(descriptor.name()))
    return {};
  auto config = root->itemDatabase()->itemConfig(descriptor.name(), descriptor.parameters(), level, seed);
  return JsonObject{{"directory", config.directory}, {"config", config.config}, {"parameters", config.parameters}};
}

auto LuaBindings::RootCallbacks::createItem(Root* root, Json const& descriptor, std::optional<float> const& level, std::optional<std::uint64_t> const& seed) -> Json {
  auto item = root->itemDatabase()->item(ItemDescriptor(descriptor), level, seed);
  return item->descriptor().toJson();
}

auto LuaBindings::RootCallbacks::tenantConfig(Root* root, String const& tenantName) -> Json {
  return root->tenantDatabase()->getTenant(tenantName)->config;
}

auto LuaBindings::RootCallbacks::getMatchingTenants(Root* root, StringMap<unsigned> const& colonyTags) -> JsonArray {
  return root->tenantDatabase()
    ->getMatchingTenants(colonyTags)
    .transformed([](Ptr<Tenant> const& tenant) -> Json { return tenant->config; });
}

auto LuaBindings::RootCallbacks::liquidStatusEffects(Root* root, LiquidId arg1) -> Json {
  if (auto liquidSettings = root->liquidsDatabase()->liquidSettings(arg1))
    return liquidSettings->statusEffects;
  return {};
}

auto LuaBindings::RootCallbacks::generateName(Root* root, String const& rulesAsset, std::optional<std::uint64_t> seed) -> String {
  return root->nameGenerator()->generateName(rulesAsset, seed.value_or(Random::randu64()));
}

auto LuaBindings::RootCallbacks::questConfig(Root* root, String const& templateId) -> Json {
  return root->questTemplateDatabase()->questTemplate(templateId)->config;
}

auto LuaBindings::RootCallbacks::npcPortrait(Root* root,
                                             String const& portraitMode,
                                             String const& species,
                                             String const& typeName,
                                             float level,
                                             std::optional<std::uint64_t> seed,
                                             std::optional<JsonObject> const& parameters) -> JsonArray {
  auto npcDatabase = root->npcDatabase();
  auto npcVariant = npcDatabase->generateNpcVariant(species, typeName, level, seed.value_or(Random::randu64()), parameters.value_or(JsonObject{}));

  auto drawables = npcDatabase->npcPortrait(npcVariant, PortraitModeNames.getLeft(portraitMode));
  return drawables.transformed(std::mem_fn(&Drawable::toJson));
}

auto LuaBindings::RootCallbacks::npcVariant(Root* root, String const& species, String const& typeName, float level, std::optional<std::uint64_t> seed, std::optional<JsonObject> const& parameters) -> Json {
  auto npcDatabase = root->npcDatabase();
  auto npcVariant = npcDatabase->generateNpcVariant(
    species, typeName, level, seed.value_or(Random::randu64()), parameters.value_or(JsonObject{}));
  return npcDatabase->writeNpcVariantToJson(npcVariant);
}

auto LuaBindings::RootCallbacks::monsterPortrait(Root* root, String const& typeName, std::optional<JsonObject> const& parameters) -> JsonArray {
  auto monsterDatabase = root->monsterDatabase();
  auto seed = 0;// use a static seed to utilize caching
  auto monsterVariant = monsterDatabase->monsterVariant(typeName, seed, parameters.value_or(JsonObject{}));
  auto drawables = monsterDatabase->monsterPortrait(monsterVariant);
  return drawables.transformed(std::mem_fn(&Drawable::toJson));
}

auto LuaBindings::RootCallbacks::isTreasurePool(Root* root, String const& pool) -> bool {
  return root->treasureDatabase()->isTreasurePool(pool);
}

auto LuaBindings::RootCallbacks::createTreasure(
  Root* root, String const& pool, float level, std::optional<std::uint64_t> seed) -> JsonArray {
  auto treasure = root->treasureDatabase()->createTreasure(pool, level, seed.value_or(Random::randu64()));
  return treasure.transformed([](Ptr<Item> const& item) -> Json { return item->descriptor().toJson(); });
}

auto LuaBindings::RootCallbacks::materialMiningSound(
  Root* root, String const& materialName, std::optional<String> const& modName) -> std::optional<String> {
  auto materialDatabase = root->materialDatabase();
  auto materialId = materialDatabase->materialId(materialName);
  auto modId = modName.transform([materialDatabase](auto&& PH1) -> auto { return materialDatabase->modId(std::forward<decltype(PH1)>(PH1)); }).value_or(NoModId);
  auto sound = materialDatabase->miningSound(materialId, modId);
  if (sound.empty())
    return {};
  return sound;
}

auto LuaBindings::RootCallbacks::materialFootstepSound(
  Root* root, String const& materialName, std::optional<String> const& modName) -> std::optional<String> {
  auto materialDatabase = root->materialDatabase();
  auto materialId = materialDatabase->materialId(materialName);
  auto modId = modName.transform([materialDatabase](auto&& PH1) -> auto { return materialDatabase->modId(std::forward<decltype(PH1)>(PH1)); }).value_or(NoModId);
  auto sound = materialDatabase->footstepSound(materialId, modId);
  if (sound.empty())
    return {};
  return sound;
}

}// namespace Star

#pragma once

#include "StarLiquidTypes.hpp"
#include "StarLua.hpp"
#include "StarRect.hpp"

import std;

namespace Star {

class Root;

namespace LuaBindings {
auto makeRootCallbacks() -> LuaCallbacks;

namespace RootCallbacks {
auto assetData(Root* root, String const& path) -> String;
auto assetImage(Root* root, String const& path) -> Image;
auto assetFrames(Root* root, String const& path) -> Json;
auto assetJson(Root* root, String const& path) -> Json;
auto makeCurrentVersionedJson(Root* root, String const& identifier, Json const& content) -> Json;
auto loadVersionedJson(Root* root, Json const& versionedJson, String const& expectedIdentifier) -> Json;
auto evalFunction(Root* root, String const& arg1, double arg2) -> double;
auto evalFunction2(Root* root, String const& arg1, double arg2, double arg3) -> double;
auto imageSize(Root* root, String const& arg1) -> Vec2U;
auto imageSpaces(Root* root, String const& arg1, Vec2F const& arg2, float arg3, bool arg4) -> List<Vec2I>;
auto nonEmptyRegion(Root* root, String const& arg1) -> RectU;
auto npcConfig(Root* root, String const& arg1) -> Json;
auto projectileGravityMultiplier(Root* root, String const& arg1) -> float;
auto projectileConfig(Root* root, String const& arg1) -> Json;
auto recipesForItem(Root* root, String const& arg1) -> JsonArray;
auto allRecipes(Root* root) -> JsonArray;
auto itemType(Root* root, String const& itemName) -> String;
auto itemTags(Root* root, String const& itemName) -> Json;
auto itemHasTag(Root* root, String const& itemName, String const& itemTag) -> bool;
auto itemConfig(Root* root, Json const& descriptor, std::optional<float> const& level, std::optional<std::uint64_t> const& seed) -> Json;
auto createItem(Root* root, Json const& descriptor, std::optional<float> const& level, std::optional<std::uint64_t> const& seed) -> Json;
auto tenantConfig(Root* root, String const& tenantName) -> Json;
auto getMatchingTenants(Root* root, StringMap<unsigned> const& colonyTags) -> JsonArray;
auto liquidStatusEffects(Root* root, LiquidId arg1) -> Json;
auto generateName(Root* root, String const& rulesAsset, std::optional<std::uint64_t> seed) -> String;
auto questConfig(Root* root, String const& templateId) -> Json;
auto npcPortrait(Root* root,
                 String const& portraitMode,
                 String const& species,
                 String const& typeName,
                 float level,
                 std::optional<std::uint64_t> seed,
                 std::optional<JsonObject> const& parameters) -> JsonArray;
auto npcVariant(Root* root,
                String const& species,
                String const& typeName,
                float level,
                std::optional<std::uint64_t> seed,
                std::optional<JsonObject> const& parameters) -> Json;
auto monsterPortrait(Root* root, String const& typeName, std::optional<JsonObject> const& parameters) -> JsonArray;
auto isTreasurePool(Root* root, String const& pool) -> bool;
auto createTreasure(Root* root, String const& pool, float level, std::optional<std::uint64_t> seed) -> JsonArray;
auto materialMiningSound(Root* root, String const& materialName, std::optional<String> const& modName) -> std::optional<String>;
auto materialFootstepSound(Root* root, String const& materialName, std::optional<String> const& modName) -> std::optional<String>;
}// namespace RootCallbacks
}// namespace LuaBindings
}// namespace Star

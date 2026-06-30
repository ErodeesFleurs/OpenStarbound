#pragma once

#include "StarRect.hpp"
#include "StarGameTypes.hpp"
#include "StarLua.hpp"

namespace Star {

class Root;
class Image;

namespace LuaBindings {
  [[nodiscard]] LuaCallbacks makeRootCallbacks(Root& root);

  namespace RootCallbacks {
    [[nodiscard]] String assetData(Root& root, String const& path);
    [[nodiscard]] Image assetImage(Root& root, String const& path);
    [[nodiscard]] Json assetFrames(Root& root, String const& path);
    [[nodiscard]] Json assetJson(Root& root, String const& path);
    [[nodiscard]] Json makeCurrentVersionedJson(Root& root, String const& identifier, Json const& content);
    [[nodiscard]] Json loadVersionedJson(Root& root, Json const& versionedJson, String const& expectedIdentifier);
    [[nodiscard]] double evalFunction(Root& root, String const& arg1, double arg2);
    [[nodiscard]] double evalFunction2(Root& root, String const& arg1, double arg2, double arg3);
    [[nodiscard]] Vec2U imageSize(Root& root, String const& arg1);
    [[nodiscard]] List<Vec2I> imageSpaces(Root& root, String const& arg1, Vec2F const& arg2, float arg3, bool arg4);
    [[nodiscard]] RectU nonEmptyRegion(Root& root, String const& arg1);
    [[nodiscard]] Json npcConfig(Root& root, String const& arg1);
    [[nodiscard]] float projectileGravityMultiplier(Root& root, String const& arg1);
    [[nodiscard]] Json projectileConfig(Root& root, String const& arg1);
    [[nodiscard]] JsonArray recipesForItem(Root& root, String const& arg1);
    [[nodiscard]] JsonArray allRecipes(Root& root, Maybe<StringSet> filter);
    [[nodiscard]] String itemType(Root& root, String const& itemName);
    [[nodiscard]] Json itemTags(Root& root, String const& itemName);
    [[nodiscard]] bool itemHasTag(Root& root, String const& itemName, String const& itemTag);
    [[nodiscard]] Json itemConfig(Root& root, Json const& descriptor, Maybe<float> const& level, Maybe<uint64_t> const& seed);
    [[nodiscard]] Json createItem(Root& root, Json const& descriptor, Maybe<float> const& level, Maybe<uint64_t> const& seed);
    [[nodiscard]] Json tenantConfig(Root& root, String const& tenantName);
    [[nodiscard]] JsonArray getMatchingTenants(Root& root, StringMap<unsigned> const& colonyTags);
    [[nodiscard]] Json liquidStatusEffects(Root& root, LiquidId arg1);
    [[nodiscard]] String generateName(Root& root, String const& rulesAsset, Maybe<uint64_t> seed);
    [[nodiscard]] Json questConfig(Root& root, String const& templateId);
    JsonArray npcPortrait(Root& root,
        String const& portraitMode,
        String const& species,
        String const& typeName,
        float level,
        Maybe<uint64_t> seed,
        Maybe<JsonObject> const& parameters);
    Json npcVariant(Root& root,
        String const& species,
        String const& typeName,
        float level,
        Maybe<uint64_t> seed,
        Maybe<JsonObject> const& parameters);
    [[nodiscard]] JsonArray monsterPortrait(Root& root, String const& typeName, Maybe<JsonObject> const& parameters);
    [[nodiscard]] bool isTreasurePool(Root& root, String const& pool);
    [[nodiscard]] JsonArray createTreasure(Root& root, String const& pool, float level, Maybe<uint64_t> seed);
    [[nodiscard]] Maybe<String> materialMiningSound(Root& root, String const& materialName, Maybe<String> const& modName);
    [[nodiscard]] Maybe<String> materialFootstepSound(Root& root, String const& materialName, Maybe<String> const& modName);
  }
}
}

#include "StarStatusEffectDatabase.hpp"

#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

StatusEffectDatabase::StatusEffectDatabase() {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("statuseffect");
  assets->queueJsons(files);
  for (auto& file : files) {
    auto uniqueEffect = parseUniqueEffect(assets->json(file), file);

    if (m_uniqueEffects.contains(uniqueEffect.name))
      throw StatusEffectDatabaseException::format(
        "Duplicate stat effect named '{}', config file '{}'", uniqueEffect.name, file);
    m_uniqueEffects[uniqueEffect.name] = uniqueEffect;
  }
}

auto StatusEffectDatabase::isUniqueEffect(UniqueStatusEffect const& effect) const -> bool {
  return m_uniqueEffects.contains(effect);
}

auto StatusEffectDatabase::uniqueEffectConfig(UniqueStatusEffect const& effect) const -> UniqueStatusEffectConfig {
  if (auto uniqueEffect = m_uniqueEffects.maybe(effect))
    return std::move(*uniqueEffect);
  throw StatusEffectDatabaseException::format("No such unique stat effect '{}'", effect);
}

auto StatusEffectDatabase::parseUniqueEffect(Json const& config, String const& path) const -> UniqueStatusEffectConfig {
  try {
    auto assets = Root::singleton().assets();

    UniqueStatusEffectConfig effect;
    effect.name = config.getString("name");
    effect.blockingStat = config.optString("blockingStat");
    effect.effectConfig = config.get("effectConfig", JsonObject());
    effect.defaultDuration = config.getFloat("defaultDuration", 0.0f);
    effect.scripts =
      jsonToStringList(config.get("scripts", JsonArray{})).transformed([path](auto&& PH1) -> auto { return AssetPath::relativeTo(path, std::forward<decltype(PH1)>(PH1)); });
    effect.scriptDelta = config.getUInt("scriptDelta", 1);
    effect.animationConfig = config.optString("animationConfig").transform([path](auto&& PH1) -> auto { return AssetPath::relativeTo(path, std::forward<decltype(PH1)>(PH1)); });
    effect.label = config.getString("label", "");
    effect.description = config.getString("description", "");
    effect.icon = config.optString("icon").transform([path](auto&& PH1) -> auto { return AssetPath::relativeTo(path, std::forward<decltype(PH1)>(PH1)); });
    return effect;
  } catch (std::exception const& e) {
    throw StatusEffectDatabaseException("Error reading StatusEffect config", e);
  }
}

auto UniqueStatusEffectConfig::toJson() -> JsonObject {
  return {
    {"name", name},
    {"blockingStat", blockingStat.has_value() ? blockingStat.value() : Json()},
    {"effectConfig", effectConfig},
    {"defaultDuration", defaultDuration},
    {"scripts", jsonFromStringList(scripts)},
    {"scriptDelta", scriptDelta},
    {"animationConfig", animationConfig.has_value() ? animationConfig.value() : Json()},
    {"label", label},
    {"description", description},
    {"icon", icon.has_value() ? icon.value() : Json()}};
}

}// namespace Star

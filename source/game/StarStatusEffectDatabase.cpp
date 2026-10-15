import star.json;
import star.json_extra;
import star.strong_typedef;
import star.data_stream;
import star.id_map;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.version;

import star.status_types;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.status_effect_database;

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

bool StatusEffectDatabase::isUniqueEffect(UniqueStatusEffect const& effect) const {
  return m_uniqueEffects.contains(effect);
}

UniqueStatusEffectConfig StatusEffectDatabase::uniqueEffectConfig(UniqueStatusEffect const& effect) const {
  if (auto uniqueEffect = m_uniqueEffects.maybe(effect))
    return uniqueEffect.take();
  throw StatusEffectDatabaseException::format("No such unique stat effect '{}'", effect);
}

UniqueStatusEffectConfig StatusEffectDatabase::parseUniqueEffect(Json const& config, String const& path) const {
  try {
    auto assets = Root::singleton().assets();

    UniqueStatusEffectConfig effect;
    effect.name = config.getString("name");
    effect.blockingStat = config.optString("blockingStat");
    effect.effectConfig = config.get("effectConfig", JsonObject());
    effect.defaultDuration = config.getFloat("defaultDuration", 0.0f);
    effect.scripts =
        jsonToStringList(config.get("scripts", JsonArray{})).transformed(std::bind(&AssetPath::relativeTo, path, std::placeholders::_1));
    effect.scriptDelta = config.getUInt("scriptDelta", 1);
    effect.animationConfig = config.optString("animationConfig").apply(std::bind(&AssetPath::relativeTo, path, std::placeholders::_1));
    effect.label = config.getString("label", "");
    effect.description = config.getString("description", "");
    effect.icon = config.optString("icon").apply(std::bind(&AssetPath::relativeTo, path, std::placeholders::_1));
    return effect;
  } catch (std::exception const& e) {
    throw StatusEffectDatabaseException("Error reading StatusEffect config", e);
  }
}

JsonObject UniqueStatusEffectConfig::toJson() {
  return {
    {"name", name},
    {"blockingStat", blockingStat.isValid() ? blockingStat.value() : Json()},
    {"effectConfig", effectConfig},
    {"defaultDuration", defaultDuration},
    {"scripts", jsonFromStringList(scripts)},
    {"scriptDelta", scriptDelta},
    {"animationConfig", animationConfig.isValid() ? animationConfig.value() : Json()},
    {"label", label},
    {"description", description},
    {"icon", icon.isValid() ? icon.value() : Json()}
  };
}

}// namespace Star

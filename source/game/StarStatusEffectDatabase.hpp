#pragma once

#include "StarStatusTypes.hpp"

namespace Star {

STAR_EXCEPTION(StatusEffectDatabaseException, StarException);

STAR_CLASS(StatusEffectDatabase);

// Named, unique, unstackable scripted effects.
struct UniqueStatusEffectConfig {
  String name;
  std::optional<String> blockingStat;
  Json effectConfig;
  float defaultDuration;
  StringList scripts;
  unsigned scriptDelta;
  std::optional<String> animationConfig;

  String label;
  String description;
  std::optional<String> icon;

  JsonObject toJson();
};

class StatusEffectDatabase {
public:
  StatusEffectDatabase();

  bool isUniqueEffect(UniqueStatusEffect const& effect) const;

  UniqueStatusEffectConfig uniqueEffectConfig(UniqueStatusEffect const& effect) const;

private:
  UniqueStatusEffectConfig parseUniqueEffect(Json const& config, String const& path) const;

  HashMap<UniqueStatusEffect, UniqueStatusEffectConfig> m_uniqueEffects;
};

}

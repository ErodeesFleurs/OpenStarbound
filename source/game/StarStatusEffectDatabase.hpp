#pragma once

#include "StarException.hpp"
#include "StarStatusTypes.hpp"

import std;

namespace Star {

using StatusEffectDatabaseException = ExceptionDerived<"StatusEffectDatabaseException">;

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

  auto toJson() -> JsonObject;
};

class StatusEffectDatabase {
public:
  StatusEffectDatabase();

  [[nodiscard]] auto isUniqueEffect(UniqueStatusEffect const& effect) const -> bool;

  [[nodiscard]] auto uniqueEffectConfig(UniqueStatusEffect const& effect) const -> UniqueStatusEffectConfig;

private:
  [[nodiscard]] auto parseUniqueEffect(Json const& config, String const& path) const -> UniqueStatusEffectConfig;

  HashMap<UniqueStatusEffect, UniqueStatusEffectConfig> m_uniqueEffects;
};

}// namespace Star

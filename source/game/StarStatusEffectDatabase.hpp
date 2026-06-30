#pragma once

#include "StarThread.hpp"
#include "StarStatusTypes.hpp"

namespace Star {

struct StatusEffectDatabaseExceptionTag { static constexpr char const* typeName = "StatusEffectDatabaseException"; };
using StatusEffectDatabaseException = TypedException<StarException, StatusEffectDatabaseExceptionTag>;

class StatusEffectDatabase;
using StatusEffectDatabasePtr = SharedPtr<StatusEffectDatabase>;
using StatusEffectDatabaseConstPtr = SharedPtr<StatusEffectDatabase const>;

// Named, unique, unstackable scripted effects.
struct UniqueStatusEffectConfig {
  String name;
  Maybe<String> blockingStat;
  Json effectConfig;
  float defaultDuration;
  StringList scripts;
  unsigned scriptDelta;
  Maybe<String> animationConfig;

  String label;
  String description;
  Maybe<String> icon;

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

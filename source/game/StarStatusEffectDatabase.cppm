module;

#include "StarStatusTypes.hpp"

namespace Star {

struct StatusEffectDatabaseExceptionTag {
  static constexpr char const* name() { return "StatusEffectDatabaseException"; }
};
using StatusEffectDatabaseException = StarError<StatusEffectDatabaseExceptionTag, StarException>;

STAR_CLASS(StatusEffectDatabase);

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

export module star.status_effect_database;

export namespace Star {
  using ::Star::StatusEffectDatabaseExceptionTag;
  using ::Star::StatusEffectDatabaseException;
  using ::Star::StatusEffectDatabase;
  using ::Star::StatusEffectDatabasePtr;
  using ::Star::StatusEffectDatabaseConstPtr;
  using ::Star::UniqueStatusEffectConfig;
}

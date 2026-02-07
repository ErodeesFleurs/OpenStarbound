#pragma once

#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarDataStream.hpp"
#include "StarIdMap.hpp"

import std;

namespace Star {

using StatusException = ExceptionDerived<"StatusException">;

// Multipliers act exactly the way you'd expect: 0.0 is a 100% reduction of the
// base stat, while 2.0 is a 100% increase. Since these are *base* multipliers
// they do not interact with each other, thus stacking a 0.0 and a 2.0 leaves
// the stat unmodified
struct StatBaseMultiplier {
  String statName;
  float baseMultiplier;

  auto operator==(StatBaseMultiplier const& rhs) const -> bool;
};

auto operator>>(DataStream& ds, StatBaseMultiplier& baseMultiplier) -> DataStream&;
auto operator<<(DataStream& ds, StatBaseMultiplier const& baseMultiplier) -> DataStream&;

struct StatValueModifier {
  String statName;
  float value;

  auto operator==(StatValueModifier const& rhs) const -> bool;
};

auto operator>>(DataStream& ds, StatValueModifier& valueModifier) -> DataStream&;
auto operator<<(DataStream& ds, StatValueModifier const& valueModifier) -> DataStream&;

// Unlike base multipliers, these all stack multiplicatively with the final
// stat value (including all base and value modifiers) such that an effective
// multiplier of 0.0 will ALWAYS reduce the stat to 0 regardless of other
// effects
struct StatEffectiveMultiplier {
  String statName;
  float effectiveMultiplier;

  auto operator==(StatEffectiveMultiplier const& rhs) const -> bool;
};

auto operator>>(DataStream& ds, StatEffectiveMultiplier& effectiveMultiplier) -> DataStream&;
auto operator<<(DataStream& ds, StatEffectiveMultiplier const& effectiveMultiplier) -> DataStream&;

using StatModifier = MVariant<StatValueModifier, StatBaseMultiplier, StatEffectiveMultiplier>;

auto jsonToStatModifier(Json const& config) -> StatModifier;
auto jsonFromStatModifier(StatModifier const& modifier) -> Json;

using StatModifierGroupId = std::uint32_t;
using StatModifierGroupMap = IdMap<StatModifierGroupId, List<StatModifier>>;

// Unique stat effects are identified uniquely by name.
using UniqueStatusEffect = String;

// Second element here is *percentage* of duration remaining, based on the
// highest duration that the effect has had
using ActiveUniqueStatusEffectSummary = List<std::pair<UniqueStatusEffect, std::optional<float>>>;

// Persistent status effects can either be a modifier effect or unique effect
using PersistentStatusEffect = MVariant<StatModifier, UniqueStatusEffect>;

// Reads either a name of a unique stat effect or a stat modifier object
auto jsonToPersistentStatusEffect(Json const& config) -> PersistentStatusEffect;
auto jsonFromPersistentStatusEffect(PersistentStatusEffect const& effect) -> Json;

// Ephemeral effects are always unique effects and either use the default
// duration in their config or optionally the default
struct EphemeralStatusEffect {
  UniqueStatusEffect uniqueEffect;
  std::optional<float> duration;

  auto operator==(EphemeralStatusEffect const& rhs) const -> bool;
};

auto operator>>(DataStream& ds, EphemeralStatusEffect& ephemeralStatusEffect) -> DataStream&;
auto operator<<(DataStream& ds, EphemeralStatusEffect const& ephemeralStatusEffect) -> DataStream&;

// Reads either a name of a unique stat effect or an object containing the
// type name and optionally the duration.
auto jsonToEphemeralStatusEffect(Json const& config) -> EphemeralStatusEffect;
auto jsonFromEphemeralStatusEffect(EphemeralStatusEffect const& effect) -> Json;

}

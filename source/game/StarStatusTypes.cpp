#include "StarStatusTypes.hpp"

#include "StarDataStreamExtra.hpp"
#include "StarJsonExtra.hpp"

import std;

namespace Star {

auto StatBaseMultiplier::operator==(StatBaseMultiplier const& rhs) const -> bool {
  return std::tie(statName, baseMultiplier) == std::tie(rhs.statName, rhs.baseMultiplier);
}

auto operator>>(DataStream& ds, StatBaseMultiplier& baseMultiplier) -> DataStream& {
  ds >> baseMultiplier.statName;
  ds >> baseMultiplier.baseMultiplier;
  return ds;
}

auto operator<<(DataStream& ds, StatBaseMultiplier const& baseMultiplier) -> DataStream& {
  ds << baseMultiplier.statName;
  ds << baseMultiplier.baseMultiplier;
  return ds;
}

auto StatValueModifier::operator==(StatValueModifier const& rhs) const -> bool {
  return std::tie(statName, value) == std::tie(rhs.statName, rhs.value);
}

auto operator>>(DataStream& ds, StatValueModifier& valueModifier) -> DataStream& {
  ds >> valueModifier.statName;
  ds >> valueModifier.value;
  return ds;
}

auto operator<<(DataStream& ds, StatValueModifier const& valueModifier) -> DataStream& {
  ds << valueModifier.statName;
  ds << valueModifier.value;
  return ds;
}

auto StatEffectiveMultiplier::operator==(StatEffectiveMultiplier const& rhs) const -> bool {
  return std::tie(statName, effectiveMultiplier) == std::tie(rhs.statName, rhs.effectiveMultiplier);
}

auto operator>>(DataStream& ds, StatEffectiveMultiplier& effectiveMultiplier) -> DataStream& {
  ds >> effectiveMultiplier.statName;
  ds >> effectiveMultiplier.effectiveMultiplier;
  return ds;
}

auto operator<<(DataStream& ds, StatEffectiveMultiplier const& effectiveMultiplier) -> DataStream& {
  ds << effectiveMultiplier.statName;
  ds << effectiveMultiplier.effectiveMultiplier;
  return ds;
}

auto jsonToStatModifier(Json const& config) -> StatModifier {
  String statName = config.getString("stat");
  if (auto baseMultiplier = config.optFloat("baseMultiplier")) {
    return StatModifier(StatBaseMultiplier{.statName = statName, .baseMultiplier = *baseMultiplier});
  } else if (auto amount = config.optFloat("amount")) {
    return StatModifier(StatValueModifier{.statName = statName, .value = *amount});
  } else if (auto effectiveMultiplier = config.optFloat("effectiveMultiplier")) {
    return StatModifier(StatEffectiveMultiplier{.statName = statName, .effectiveMultiplier = *effectiveMultiplier});
  } else {
    throw JsonException("Could not find 'baseMultiplier' or 'effectiveMultiplier' or 'amount' element in stat effect config");
  }
}

auto jsonFromStatModifier(StatModifier const& modifier) -> Json {
  if (auto baseMultiplier = modifier.ptr<StatBaseMultiplier>()) {
    return JsonObject{{"stat", baseMultiplier->statName}, {"baseMultiplier", baseMultiplier->baseMultiplier}};
  } else if (auto valueModifier = modifier.ptr<StatValueModifier>()) {
    return JsonObject{{"stat", valueModifier->statName}, {"amount", valueModifier->value}};
  } else if (auto effectiveMultiplier = modifier.ptr<StatEffectiveMultiplier>()) {
    return JsonObject{{"stat", effectiveMultiplier->statName}, {"effectiveMultiplier", effectiveMultiplier->effectiveMultiplier}};
  } else {
    throw JsonException("No 'baseMultiplier', 'amount', or 'effectiveMultiplier' member found in json");
  }
}

auto jsonToPersistentStatusEffect(Json const& config) -> PersistentStatusEffect {
  if (config.isType(Json::Type::String)) {
    return UniqueStatusEffect(config.toString());
  } else if (config.isType(Json::Type::Object)) {
    return jsonToStatModifier(config);
  } else {
    throw JsonException("Json is wrong type for persistent stat effect config");
  }
}

auto jsonFromPersistentStatusEffect(PersistentStatusEffect const& effect) -> Json {
  if (auto uniqueStatusEffect = effect.ptr<UniqueStatusEffect>())
    return {*uniqueStatusEffect};
  else if (auto statModifier = effect.ptr<StatModifier>())
    return jsonFromStatModifier(*statModifier);

  return {};
}

auto EphemeralStatusEffect::operator==(EphemeralStatusEffect const& rhs) const -> bool {
  return tie(uniqueEffect, duration) == tie(rhs.uniqueEffect, rhs.duration);
}

auto operator>>(DataStream& ds, EphemeralStatusEffect& ephemeralStatusEffect) -> DataStream& {
  ds >> ephemeralStatusEffect.uniqueEffect;
  ds >> ephemeralStatusEffect.duration;
  return ds;
}

auto operator<<(DataStream& ds, EphemeralStatusEffect const& ephemeralStatusEffect) -> DataStream& {
  ds << ephemeralStatusEffect.uniqueEffect;
  ds << ephemeralStatusEffect.duration;
  return ds;
}

auto jsonToEphemeralStatusEffect(Json const& config) -> EphemeralStatusEffect {
  if (config.isType(Json::Type::String)) {
    return EphemeralStatusEffect{.uniqueEffect = UniqueStatusEffect(config.toString()), .duration = {}};
  } else if (config.isType(Json::Type::Object)) {
    String effectName = config.getString("effect");
    std::optional<float> duration = config.optFloat("duration");
    return EphemeralStatusEffect{.uniqueEffect = UniqueStatusEffect(effectName), .duration = duration};
  } else {
    throw JsonException("Json is wrong type for ephemeral stat effect config");
  }
}

auto jsonFromEphemeralStatusEffect(EphemeralStatusEffect const& effect) -> Json {
  return JsonObject{{"effect", effect.uniqueEffect}, {"duration", jsonFromMaybe(effect.duration)}};
}

}// namespace Star

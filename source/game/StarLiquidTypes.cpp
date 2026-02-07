#include "StarLiquidTypes.hpp"

import std;

namespace Star {

auto LiquidLevel::take(float amount) -> LiquidLevel {
  if (liquid == EmptyLiquidId)
    return {};
  amount = std::min(amount, level);

  LiquidLevel taken = {liquid, amount};

  level -= amount;
  if (level <= 0.0f)
    liquid = EmptyLiquidId;

  return taken;
}

auto LiquidStore::filled(LiquidId liquid, float level, std::optional<float> pressure) -> LiquidStore {
  if (liquid == EmptyLiquidId)
    return {};
  return {liquid, level, pressure.value_or(level), false};
}

auto LiquidStore::endless(LiquidId liquid, float pressure) -> LiquidStore {
  if (liquid == EmptyLiquidId)
    return {};
  return {liquid, 1.0f, pressure, true};
}

LiquidStore::LiquidStore() : LiquidLevel(), pressure(0), source(false) {}

LiquidStore::LiquidStore(LiquidId liquid, float level, float pressure, bool source)
    : LiquidLevel(liquid, level), pressure(pressure), source(source) {}

auto LiquidStore::netUpdate() const -> LiquidNetUpdate {
  return LiquidNetUpdate{.liquid = liquid, .level = floatToByte(level, true)};
}

auto LiquidStore::update(LiquidId liquid, float level, float pressure) -> std::optional<LiquidNetUpdate> {
  if (source) {
    if (this->liquid != liquid)
      return {};
    level = std::max(level, this->level);
    pressure = std::max(pressure, this->pressure);
  }

  if (level <= 0.0f) {
    liquid = EmptyLiquidId;
    pressure = 0.0f;
  }

  bool netUpdate = this->liquid != liquid || floatToByte(this->level, true) != floatToByte(level, true);

  this->liquid = liquid;
  this->level = level;
  this->pressure = pressure;

  if (netUpdate)
    return LiquidNetUpdate{.liquid = liquid, .level = floatToByte(level)};
  else
    return {};
}

auto LiquidStore::take(float amount) -> LiquidLevel {
  if (source)
    return {liquid, amount};

  auto taken = LiquidLevel::take(amount);
  if (level == 0.0f)
    pressure = 0.0f;
  return taken;
}

}// namespace Star

#pragma once

#include "StarMathCommon.hpp"

import std;

namespace Star {

using LiquidId = std::uint8_t;
LiquidId const EmptyLiquidId = 0;

struct LiquidLevel {
  LiquidLevel();
  LiquidLevel(LiquidId liquid, float level);

  auto take(float amount) -> LiquidLevel;

  LiquidId liquid;
  float level;
};

struct LiquidNetUpdate {
  LiquidId liquid;
  std::uint8_t level;

  [[nodiscard]] auto liquidLevel() const -> LiquidLevel;
};

struct LiquidStore : LiquidLevel {
  // Returns a LiquidStore of the given liquid
  static auto filled(LiquidId liquid, float level, std::optional<float> pressure = {}) -> LiquidStore;
  // Returns a LiquidStore source liquid block
  static auto endless(LiquidId liquid, float pressure) -> LiquidStore;

  LiquidStore();
  LiquidStore(LiquidId liquid, float level, float pressure, bool source);

  [[nodiscard]] auto netUpdate() const -> LiquidNetUpdate;

  auto update(LiquidId liquid, float level, float pressure) -> std::optional<LiquidNetUpdate>;

  auto take(float amount) -> LiquidLevel;

  float pressure;
  bool source;
};

inline LiquidLevel::LiquidLevel()
  : liquid(EmptyLiquidId), level(0.0f) {}

inline LiquidLevel::LiquidLevel(LiquidId liquid, float level)
  : liquid(liquid), level(level) {}

inline auto LiquidNetUpdate::liquidLevel() const -> LiquidLevel {
  return LiquidLevel{liquid, byteToFloat(level)};
}

}

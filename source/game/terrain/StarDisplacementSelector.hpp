#pragma once

#include "StarConfig.hpp"
#include "StarPerlin.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarVector.hpp"

namespace Star {

struct DisplacementSelector : TerrainSelector {
  static char const* const Name;

  DisplacementSelector(
    Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] auto get(int x, int y) const -> float override;

  PerlinF xDisplacementFunction;
  PerlinF yDisplacementFunction;

  float xXInfluence;
  float xYInfluence;
  float yXInfluence;
  float yYInfluence;

  bool yClamp;
  Vec2F yClampRange;
  float yClampSmoothing;

  auto clampY(float v) const -> float;

  ConstPtr<TerrainSelector> m_source;
};

}// namespace Star

#pragma once

#include "StarConfig.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarVector.hpp"

namespace Star {

struct RotateSelector : TerrainSelector {
  static char const* const Name;

  RotateSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] auto get(int x, int y) const -> float override;

  float rotation;
  Vec2F rotationCenter;

  ConstPtr<TerrainSelector> m_source;
};

}// namespace Star

#pragma once

#include "StarConfig.hpp"
#include "StarTerrainDatabase.hpp"

namespace Star {

struct MixSelector : TerrainSelector {
  static char const* const Name;

  MixSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] auto get(int x, int y) const -> float override;

  ConstPtr<TerrainSelector> m_mixSource;
  ConstPtr<TerrainSelector> m_aSource;
  ConstPtr<TerrainSelector> m_bSource;
};

}// namespace Star

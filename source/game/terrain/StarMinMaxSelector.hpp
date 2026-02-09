#pragma once

#include "StarConfig.hpp"
#include "StarTerrainDatabase.hpp"

namespace Star {

struct MinMaxSelector : TerrainSelector {
  static char const* const Name;

  MinMaxSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] auto get(int x, int y) const -> float override;

  List<ConstPtr<TerrainSelector>> m_sources;
};

}

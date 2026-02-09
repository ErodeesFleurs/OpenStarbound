#pragma once

#include "StarConfig.hpp"
#include "StarTerrainDatabase.hpp"

namespace Star {

struct MaxSelector : TerrainSelector {
  static char const* const Name;

  MaxSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  [[nodiscard]] auto get(int x, int y) const -> float override;

  List<ConstPtr<TerrainSelector>> m_sources;
};

}// namespace Star

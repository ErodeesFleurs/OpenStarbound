module;

#include "StarTerrainDatabase.hpp"

export module star.terrain_basic;

export namespace Star {

struct ConstantSelector : TerrainSelector {
  static char const* const Name;

  ConstantSelector(Json const& config, TerrainSelectorParameters const& parameters);

  float get(int x, int y) const override;

  float m_value;
};



struct FlatSurfaceSelector : TerrainSelector {
  static char const* const Name;

  FlatSurfaceSelector(Json const& config, TerrainSelectorParameters const& parameters);

  float get(int x, int y) const override;

  float surfaceLevel;
  float adjustment;
  float flip;
};

}

namespace Star {

char const* const ConstantSelector::Name = "constant";

ConstantSelector::ConstantSelector(Json const& config, TerrainSelectorParameters const& parameters)
  : TerrainSelector(Name, config, parameters) {
  m_value = config.getFloat("value");
}

float ConstantSelector::get(int, int) const {
  return m_value;
}



char const* const FlatSurfaceSelector::Name = "flatSurface";

FlatSurfaceSelector::FlatSurfaceSelector(Json const& config, TerrainSelectorParameters const& parameters)
  : TerrainSelector(Name, config, parameters) {
  surfaceLevel = parameters.baseHeight;
  adjustment = config.getFloat("adjustment", 0);
  flip = config.getBool("flip", false) ? -1 : 1;
}

float FlatSurfaceSelector::get(int, int y) const {
  return flip * (surfaceLevel - (y - adjustment));
}

}

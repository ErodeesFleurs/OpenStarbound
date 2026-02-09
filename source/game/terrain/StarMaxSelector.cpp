#include "StarMaxSelector.hpp"

#include "StarMathCommon.hpp"

import std;

namespace Star {

char const* const MaxSelector::Name = "max";

MaxSelector::MaxSelector(
  Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database)
    : TerrainSelector(Name, config, parameters) {
  for (auto const& sourceConfig : config.getArray("sources")) {
    String sourceType = sourceConfig.getString("type");
    std::uint64_t seedBias = sourceConfig.getUInt("seedBias", 0);
    TerrainSelectorParameters sourceParameters = parameters;
    sourceParameters.seed += seedBias;
    m_sources.append(database->createSelectorType(sourceType, sourceConfig, sourceParameters));
  }
}

auto MaxSelector::get(int x, int y) const -> float {
  auto value = lowest<float>();
  for (auto const& source : m_sources)
    value = std::max(value, source->get(x, y));
  return value;
}

}// namespace Star

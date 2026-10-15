module;
import std;
import star.string;
import star.list;
import star.json;

import star.math_common;
import star.interpolation;

export module star.terrain_composition;
import star.terrain_database;

export namespace Star {

struct MaxSelector : TerrainSelector {
  static char const* const Name;

  MaxSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  float get(int x, int y) const override;

  List<std::shared_ptr<TerrainSelector const>> m_sources;
};



struct MinMaxSelector : TerrainSelector {
  static char const* const Name;

  MinMaxSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  float get(int x, int y) const override;

  List<std::shared_ptr<TerrainSelector const>> m_sources;
};



struct MixSelector : TerrainSelector {
  static char const* const Name;

  MixSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  float get(int x, int y) const override;

  std::shared_ptr<TerrainSelector const> m_mixSource;
  std::shared_ptr<TerrainSelector const> m_aSource;
  std::shared_ptr<TerrainSelector const> m_bSource;
};

}

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

float MaxSelector::get(int x, int y) const {
  float value = lowest<float>();
  for (auto const& source : m_sources)
    value = std::max(value, source->get(x, y));
  return value;
}



char const* const MinMaxSelector::Name = "minmax";

MinMaxSelector::MinMaxSelector(
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

float MinMaxSelector::get(int x, int y) const {
  float value = 0.0f;
  for (auto const& source : m_sources) {
    float srcVal = source->get(x, y);
    if (value > 0 || srcVal > 0)
      value = std::max(value, srcVal);
    else
      value = std::min(value, srcVal);
  }
  return value;
}



char const* const MixSelector::Name = "mix";

MixSelector::MixSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database)
  : TerrainSelector(Name, config, parameters) {
  auto readSource = [&](Json const& sourceConfig) {
    String type = sourceConfig.getString("type");
    return database->createSelectorType(type, sourceConfig, parameters);
  };

  m_mixSource = readSource(config.get("mixSource"));
  m_aSource = readSource(config.get("aSource"));
  m_bSource = readSource(config.get("bSource"));
}

float MixSelector::get(int x, int y) const {
  auto f = clamp(m_mixSource->get(x, y), -1.0f, 1.0f);
  if (f == -1)
    return m_aSource->get(x, y);
  if (f == 1)
    return m_bSource->get(x, y);
  return lerp(f * 0.5f + 0.5f, m_aSource->get(x, y), m_bSource->get(x, y));
}

}

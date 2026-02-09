#pragma once

#include "StarConfig.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

using TerrainException = ExceptionDerived<"TerrainException">;

struct TerrainSelectorParameters {
  TerrainSelectorParameters();
  explicit TerrainSelectorParameters(Json const& v);

  [[nodiscard]] auto toJson() const -> Json;

  [[nodiscard]] auto withSeed(std::uint64_t seed) const -> TerrainSelectorParameters;
  [[nodiscard]] auto withCommonality(float commonality) const -> TerrainSelectorParameters;

  unsigned worldWidth;
  float baseHeight;
  std::uint64_t seed;
  float commonality;
};

struct TerrainSelector {
  TerrainSelector(String type, Json config, TerrainSelectorParameters parameters);
  virtual ~TerrainSelector();

  // Returns a float signifying the "solid-ness" of a block, >= 0.0 should be
  // considered solid, < 0.0 should be considered open space.
  [[nodiscard]] virtual auto get(int x, int y) const -> float = 0;

  String type;
  Json config;
  TerrainSelectorParameters parameters;
};

class TerrainDatabase {
public:
  struct Config {
    String type;
    Json parameters;
  };

  [[nodiscard]] auto selectorConfig(String const& name) const -> Config;
  [[nodiscard]] auto createSelectorType(String const& type, Json const& config, TerrainSelectorParameters const& parameters) const -> ConstPtr<TerrainSelector>;

  TerrainDatabase();

  [[nodiscard]] auto createNamedSelector(String const& name, TerrainSelectorParameters const& parameters) const -> ConstPtr<TerrainSelector>;
  auto constantSelector(float value) -> ConstPtr<TerrainSelector>;

  [[nodiscard]] auto storeSelector(ConstPtr<TerrainSelector> const& selector) const -> Json;
  [[nodiscard]] auto loadSelector(Json const& store) const -> ConstPtr<TerrainSelector>;

private:
  StringMap<Config> m_terrainSelectors;
};

}// namespace Star

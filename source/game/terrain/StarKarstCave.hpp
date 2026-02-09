#pragma once

#include "StarLruCache.hpp"
#include "StarPerlin.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

class KarstCaveSelector : public TerrainSelector {
public:
  static char const* const Name;

  KarstCaveSelector(Json const& config, TerrainSelectorParameters const& parameters);

  auto get(int x, int y) const -> float override;

private:
  struct LayerPerlins {
    PerlinF caveDecision;
    PerlinF layerHeightVariation;
    PerlinF caveHeightVariation;
    PerlinF caveFloorVariation;
  };

  struct Sector {
    Sector(KarstCaveSelector const* parent, Vec2I sector);

    auto get(int x, int y) -> float;

    auto inside(int x, int y) -> bool;
    void set(int x, int y, float value);

    KarstCaveSelector const* parent;
    Vec2I sector;
    List<float> values;

    float m_maxValue;
  };

  auto layerPerlins(int y) const -> LayerPerlins const&;

  int m_sectorSize;
  int m_layerResolution;
  float m_layerDensity;
  int m_bufferHeight;
  float m_caveTaperPoint;

  Json m_caveDecisionPerlinConfig;
  Json m_layerHeightVariationPerlinConfig;
  Json m_caveHeightVariationPerlinConfig;
  Json m_caveFloorVariationPerlinConfig;

  int m_worldWidth;
  std::uint64_t m_seed;

  mutable HashLruCache<int, LayerPerlins> m_layerPerlinsCache;
  mutable HashLruCache<Vec2I, Sector> m_sectorCache;
};

}// namespace Star

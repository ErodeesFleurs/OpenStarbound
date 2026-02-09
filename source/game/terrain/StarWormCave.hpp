#pragma once

#include "StarTerrainDatabase.hpp"
#include "StarLruCache.hpp"
#include "StarVector.hpp"

import std;

namespace Star {

class WormCaveSector {
public:
  WormCaveSector(int sectorSize, Vec2I sector, Json const& config, std::size_t seed, float commonality);

  auto get(int x, int y) -> float;

private:
  auto inside(int x, int y) -> bool;
  void set(int x, int y, float value);

  int m_sectorSize;
  Vec2I m_sector;
  List<float> m_values;

  float m_maxValue;
};

class WormCaveSelector : public TerrainSelector {
public:
  static char const* const Name;

  WormCaveSelector(Json const& config, TerrainSelectorParameters const& parameters);

  auto get(int x, int y) const -> float override;

private:
  int m_sectorSize;
  mutable HashLruCache<Vec2I, WormCaveSector> m_cache;
};

}

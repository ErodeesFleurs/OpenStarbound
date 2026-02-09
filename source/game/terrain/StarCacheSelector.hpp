#pragma once

#include "StarConfig.hpp"
#include "StarLruCache.hpp"
#include "StarTerrainDatabase.hpp"
#include "StarVector.hpp"

namespace Star {

struct CacheSelector : TerrainSelector {
  static char const* const Name;

  CacheSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  auto get(int x, int y) const -> float override;

  ConstPtr<TerrainSelector> m_source;
  mutable HashLruCache<Vec2I, float> m_cache;
};

}// namespace Star

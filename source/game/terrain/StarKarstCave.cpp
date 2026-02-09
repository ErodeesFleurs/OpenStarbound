#include "StarKarstCave.hpp"

import std;

namespace Star {

char const* const KarstCaveSelector::Name = "karstcave";

KarstCaveSelector::KarstCaveSelector(Json const& config, TerrainSelectorParameters const& parameters)
    : TerrainSelector(Name, config, parameters) {
  m_sectorSize = config.getUInt("sectorSize", 64);
  m_layerResolution = config.getInt("layerResolution");
  m_layerDensity = config.getFloat("layerDensity");
  m_bufferHeight = config.getInt("bufferHeight");
  m_caveTaperPoint = config.getFloat("caveTaperPoint");

  m_caveDecisionPerlinConfig = config.get("caveDecision");
  m_layerHeightVariationPerlinConfig = config.get("layerHeightVariation");
  m_caveHeightVariationPerlinConfig = config.get("caveHeightVariation");
  m_caveFloorVariationPerlinConfig = config.get("caveFloorVariation");

  m_worldWidth = parameters.worldWidth;
  m_seed = parameters.seed;

  m_layerPerlinsCache.setMaxSize(config.getUInt("layerPerlinsCacheSize", 16));
  m_sectorCache.setMaxSize(config.getUInt("sectorCacheSize", 16));
}

auto KarstCaveSelector::get(int x, int y) const -> float {
  Vec2I key = Vec2I(x - pmod(x, m_sectorSize), y - pmod(y, m_sectorSize));
  return m_sectorCache.get(key, [=, this](Vec2I const& key) -> Sector {
                        return {this, key};
                      })
    .get(x, y);
}

KarstCaveSelector::Sector::Sector(KarstCaveSelector const* parent, Vec2I sector)
    : parent(parent), sector(sector), values(square(parent->m_sectorSize)) {

  m_maxValue = 0;

  for (int y = sector[1] - parent->m_bufferHeight; y < sector[1] + parent->m_sectorSize + parent->m_bufferHeight; y++) {
    float layerChance = parent->m_layerDensity * parent->m_layerResolution;
    // determine whether this layer has caves
    if (y % parent->m_layerResolution == 0 && staticRandomFloat(parent->m_seed, y) <= layerChance) {
      LayerPerlins const& layerPerlins = parent->layerPerlins(y);

      // carve out cave layer
      for (int x = sector[0]; x < sector[0] + parent->m_sectorSize; x++) {
        // use wrapping noise
        float noiseAngle = 2 * Constants::pi * x / parent->m_worldWidth;
        float noiseX = (std::cos(noiseAngle) * parent->m_worldWidth) / (2 * Constants::pi);
        float noiseY = (std::sin(noiseAngle) * parent->m_worldWidth) / (2 * Constants::pi);

        // determine where caves be at
        float isThereACaveHere = layerPerlins.caveDecision.get(noiseX, noiseY);
        if (isThereACaveHere > 0) {
          float taperFactor = isThereACaveHere < parent->m_caveTaperPoint
            ? std::sin((0.5 * Constants::pi * isThereACaveHere) / parent->m_caveTaperPoint)
            : 1.0f;

          int baseY = y + layerPerlins.layerHeightVariation.get(noiseX, noiseY);
          int ceilingY = baseY + layerPerlins.caveHeightVariation.get(noiseX, noiseY) * taperFactor;
          int floorY = baseY + layerPerlins.caveFloorVariation.get(noiseX, noiseY) * taperFactor;
          float halfHeight = std::abs(ceilingY - floorY + 1) / 2.0f;
          float midpointY = (floorY + ceilingY) / 2.0f;

          m_maxValue = std::max(m_maxValue, halfHeight);

          for (int pointY = floorY; pointY < ceilingY; pointY++)
            if (inside(x, pointY))
              set(x, pointY, std::max(get(x, pointY), halfHeight - std::abs(midpointY - pointY)));
        }
      }
    }
  }
}

auto KarstCaveSelector::Sector::get(int x, int y) -> float {
  auto val = values[(x - sector[0]) + parent->m_sectorSize * (y - sector[1])];
  if (val > 0)
    return val;
  else
    return -m_maxValue;
}

auto KarstCaveSelector::Sector::inside(int x, int y) -> bool {
  int x_ = x - sector[0];
  if (x_ < 0 || x_ >= parent->m_sectorSize)
    return false;
  int y_ = y - sector[1];
  if (y_ < 0 || y_ >= parent->m_sectorSize)
    return false;
  return true;
}

void KarstCaveSelector::Sector::set(int x, int y, float value) {
  values[(x - sector[0]) + parent->m_sectorSize * (y - sector[1])] = value;
}

auto KarstCaveSelector::layerPerlins(int y) const -> KarstCaveSelector::LayerPerlins const& {
  return m_layerPerlinsCache.get(y, [this](int y) -> LayerPerlins {
    return LayerPerlins{
      .caveDecision = PerlinF(m_caveDecisionPerlinConfig, staticRandomU64(y, m_seed, "CaveDecision")),
      .layerHeightVariation = PerlinF(m_layerHeightVariationPerlinConfig, staticRandomU64(y, m_seed, "LayerHeightVariation")),
      .caveHeightVariation = PerlinF(m_caveHeightVariationPerlinConfig, staticRandomU64(y, m_seed, "CaveHeightVariation")),
      .caveFloorVariation = PerlinF(m_caveFloorVariationPerlinConfig, staticRandomU64(y, m_seed, "CaveFloorVariation"))};
  });
}

}// namespace Star

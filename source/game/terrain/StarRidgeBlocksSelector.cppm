module;

#include "StarTerrainDatabase.hpp"
#include "StarPerlin.hpp"
#include "StarRandom.hpp"

export module star.terrain_ridge_blocks;

export namespace Star {

struct RidgeBlocksSelector : TerrainSelector {
  static char const* const Name;

  RidgeBlocksSelector(Json const& config, TerrainSelectorParameters const& parameters);

  float get(int x, int y) const override;

  float commonality;

  float amplitude;
  float frequency;
  float bias;

  float noiseAmplitude;
  float noiseFrequency;

  PerlinF ridgePerlin1;
  PerlinF ridgePerlin2;
  PerlinF noisePerlin;
};

}

namespace Star {

char const* const RidgeBlocksSelector::Name = "ridgeblocks";

RidgeBlocksSelector::RidgeBlocksSelector(Json const& config, TerrainSelectorParameters const& parameters)
  : TerrainSelector(Name, config, parameters) {
  commonality = parameters.commonality;

  amplitude = config.getFloat("amplitude");
  frequency = config.getFloat("frequency");
  bias = config.getFloat("bias");

  noiseAmplitude = config.getFloat("noiseAmplitude");
  noiseFrequency = config.getFloat("noiseFrequency");

  RandomSource random(parameters.seed);
  ridgePerlin1 = PerlinF(PerlinType::RidgedMulti, 2, frequency, amplitude, 0, 2.0f, 2.0f, random.randu64());
  ridgePerlin2 = PerlinF(PerlinType::RidgedMulti, 2, frequency, amplitude, 0, 2.0f, 2.0f, random.randu64());
  noisePerlin = PerlinF(1, noiseFrequency, noiseAmplitude, 0, 1.0f, 2.0f, random.randu64());
}

float RidgeBlocksSelector::get(int x, int y) const {
  if (commonality <= 0.0f) {
    return 0.0f;
  } else {
    x += noisePerlin.get(x, y);
    y += noisePerlin.get(y, x);
    return (ridgePerlin1.get(x, y) - ridgePerlin2.get(x, y)) * commonality + bias;
  }
}

}

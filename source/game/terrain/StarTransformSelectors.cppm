module;

#include "StarTerrainDatabase.hpp"
#include "StarPerlin.hpp"
#include "StarVector.hpp"
#include "StarRandom.hpp"
#include "StarJsonExtra.hpp"

export module star.terrain_transform;

export namespace Star {

struct RotateSelector : TerrainSelector {
  static char const* const Name;

  RotateSelector(Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  float get(int x, int y) const override;

  float rotation;
  Vec2F rotationCenter;

  TerrainSelectorConstPtr m_source;
};



struct DisplacementSelector : TerrainSelector {
  static char const* const Name;

  DisplacementSelector(
      Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database);

  float get(int x, int y) const override;

  PerlinF xDisplacementFunction;
  PerlinF yDisplacementFunction;

  float xXInfluence;
  float xYInfluence;
  float yXInfluence;
  float yYInfluence;

  bool yClamp;
  Vec2F yClampRange;
  float yClampSmoothing;

  float clampY(float v) const;

  TerrainSelectorConstPtr m_source;
};

}

namespace Star {

char const* const RotateSelector::Name = "rotate";

RotateSelector::RotateSelector(
    Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database)
  : TerrainSelector(Name, config, parameters) {
  RandomSource random(parameters.seed);

  rotation = config.getFloat("rotation");
  rotationCenter = Vec2F(parameters.worldWidth / 2, 0);

  auto sourceConfig = config.get("source");
  String sourceType = sourceConfig.getString("type");
  m_source = database->createSelectorType(sourceType, sourceConfig, parameters);
}

float RotateSelector::get(int x, int y) const {
  auto pos = (Vec2F(x, y) - rotationCenter).rotate(rotation) + rotationCenter;
  return m_source->get(pos[0], pos[1]);
}



char const* const DisplacementSelector::Name = "displacement";

DisplacementSelector::DisplacementSelector(
    Json const& config, TerrainSelectorParameters const& parameters, TerrainDatabase const* database)
  : TerrainSelector(Name, config, parameters) {
  RandomSource random(parameters.seed);

  auto xType = PerlinTypeNames.getLeft(config.getString("xType"));
  auto xOctaves = config.getFloat("xOctaves");
  auto xFreq = config.getFloat("xFreq");
  auto xAmp = config.getFloat("xAmp");
  auto xBias = config.getFloat("xBias", 0.0f);
  auto xAlpha = config.getFloat("xAlpha", 2.0f);
  auto xBeta = config.getFloat("xBeta", 2.0f);

  xDisplacementFunction = PerlinF(xType, xOctaves, xFreq, xAmp, xBias, xAlpha, xBeta, random.randu64());

  auto yType = PerlinTypeNames.getLeft(config.getString("yType"));
  auto yOctaves = config.getFloat("yOctaves");
  auto yFreq = config.getFloat("yFreq");
  auto yAmp = config.getFloat("yAmp");
  auto yBias = config.getFloat("yBias", 0.0f);
  auto yAlpha = config.getFloat("yAlpha", 2.0f);
  auto yBeta = config.getFloat("yBeta", 2.0f);

  yDisplacementFunction = PerlinF(yType, yOctaves, yFreq, yAmp, yBias, yAlpha, yBeta, random.randu64());

  xXInfluence = config.getFloat("xXInfluence", 1.0f);
  xYInfluence = config.getFloat("xYInfluence", 1.0f);
  yXInfluence = config.getFloat("yXInfluence", 1.0f);
  yYInfluence = config.getFloat("yYInfluence", 1.0f);

  yClamp = config.contains("yClamp");
  if (yClamp) {
    yClampRange = jsonToVec2F(config.get("yClamp"));
    yClampSmoothing = config.getFloat("yClampSmoothing", 0);
  }

  auto sourceConfig = config.get("source");
  String sourceType = sourceConfig.getString("type");
  m_source = database->createSelectorType(sourceType, sourceConfig, parameters);
}

float DisplacementSelector::get(int x, int y) const {
  auto x_ = x + xDisplacementFunction.get(x * xXInfluence, y * xYInfluence);
  auto y_ = y + clampY(yDisplacementFunction.get(x * yXInfluence, y * yYInfluence));
  return m_source->get(x_, y_);
}

float DisplacementSelector::clampY(float v) const {
  if (!yClamp)
    return v;
  if (yClampSmoothing == 0)
    return clamp(v, yClampRange[0], yClampRange[1]);

  return 0.2f * (clamp(v - yClampSmoothing, yClampRange[0], yClampRange[1])
      + clamp(v - 0.5f * yClampSmoothing, yClampRange[0], yClampRange[1])
      + clamp(v, yClampRange[0], yClampRange[1])
      + clamp(v + 0.5f * yClampSmoothing, yClampRange[0], yClampRange[1])
      + clamp(v + yClampSmoothing, yClampRange[0], yClampRange[1]));
}

}

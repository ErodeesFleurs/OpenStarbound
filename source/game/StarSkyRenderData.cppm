module;

#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"
import star.sky_types;
#include "StarEither.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
import star.sky_parameters;

namespace Star {

struct SkyRenderData {
  friend DataStream& operator>>(DataStream& ds, SkyRenderData& value);
  friend DataStream& operator<<(DataStream& ds, SkyRenderData const& value);
  Json settings;
  SkyParameters skyParameters;

  SkyType type;
  float dayLevel;
  float skyAlpha;

  float dayLength;
  float timeOfDay;
  double epochTime;

  Vec2F starOffset;
  float starRotation;
  Vec2F worldOffset;
  float worldRotation;
  float orbitAngle;

  size_t starFrames;
  StringList starList;
  StringList hyperStarList;

  Color environmentLight;
  Color mainSkyColor;
  Color topRectColor;
  Color bottomRectColor;
  Color flashColor;

  StringList const& starTypes() const;

  // Star and orbiter positions here are in view space, from (0, 0) to viewSize

  List<SkyOrbiter> backOrbiters(Vec2F const& viewSize) const;
  SkyWorldHorizon worldHorizon(Vec2F const& viewSize) const;
  List<SkyOrbiter> frontOrbiters(Vec2F const& viewSize) const;
};

DataStream& operator>>(DataStream& ds, SkyRenderData& skyRenderData);
DataStream& operator<<(DataStream& ds, SkyRenderData const& skyRenderData);
}

export module star.sky_render_data;

export namespace Star {
  using ::Star::SkyRenderData;
}

#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarConfig.hpp"
#include "StarEither.hpp"
#include "StarSkyTypes.hpp"

import std;

namespace Star {

class CelestialDatabase;
class VisitableWorldParameters;

// This struct is a stripped down version of CelestialParameters that only
// contains the required inforamtion to generate a sky.  It's constructable
// from a CelestialParameters or importantly from Json.  This allows places
// without a coordinate (and therefore without CelestialParameters) to have a
// valid sky. (Instances, outposts and the like.)
// Additionally, a copy-ish constructor is provided to allow changing elements
// derived from the visitableworldparameters without reconstructing all sky
// parameters, e.g. for terraforming
struct SkyParameters {
  SkyParameters();
  SkyParameters(CelestialCoordinate const& coordinate, Ptr<CelestialDatabase> const& celestialDatabase);
  SkyParameters(SkyParameters const& oldSkyParameters, ConstPtr<VisitableWorldParameters> newVisitableParameters);
  explicit SkyParameters(Json const& config);

  [[nodiscard]] auto toJson() const -> Json;

  void read(DataStream& ds);
  void write(DataStream& ds) const;

  void readVisitableParameters(ConstPtr<VisitableWorldParameters> visitableParameters);

  std::uint64_t seed;
  std::optional<float> dayLength;
  std::optional<std::pair<List<std::pair<String, float>>, Vec2F>> nearbyPlanet;
  List<std::pair<List<std::pair<String, float>>, Vec2F>> nearbyMoons;
  List<std::pair<String, String>> horizonImages;
  bool horizonClouds;
  SkyType skyType;
  Either<SkyColoring, Color> skyColoring;
  std::optional<float> spaceLevel;
  std::optional<float> surfaceLevel;
  String sunType;
  Json settings;
};

auto operator>>(DataStream& ds, SkyParameters& sky) -> DataStream&;
auto operator<<(DataStream& ds, SkyParameters const& sky) -> DataStream&;
}// namespace Star

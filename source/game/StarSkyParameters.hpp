#pragma once

#include "StarAssets.hpp"
#include "StarCelestialCoordinate.hpp"
#include "StarEither.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarSkyTypes.hpp"

namespace Star {

class CelestialParameters;
class CelestialDatabase;
using CelestialDatabasePtr = SharedPtr<CelestialDatabase>;

struct SkyParameters;

struct VisitableWorldParameters;
using VisitableWorldParametersConstPtr = SharedPtr<VisitableWorldParameters const>;

// This struct is a stripped down version of CelestialParameters that only
// contains the required inforamtion to generate a sky.  It's constructable
// from a CelestialParameters or importantly from Json.  This allows places
// without a coordinate (and therefore without CelestialParameters) to have a
// valid sky. (Instances, outposts and the like.)
// Additionally, a copy-ish constructor is provided to allow changing elements
// derived from the visitableworldparameters without reconstructing all sky
// parameters, e.g. for terraforming
struct SkyParameters {
  SkyParameters() = default;
  SkyParameters(CelestialCoordinate const& coordinate, CelestialDatabasePtr const& celestialDatabase, AssetsConstPtr assets, LiquidsDatabaseConstPtr liquidsDatabase);
  SkyParameters(SkyParameters const& oldSkyParameters, VisitableWorldParametersConstPtr newVisitableParameters);
  explicit SkyParameters(Json const& config);

  Json toJson() const;

  void read(DataStream& ds);
  void write(DataStream& ds) const;

  void readVisitableParameters(VisitableWorldParametersConstPtr visitableParameters);

  uint64_t seed = 0;
  Maybe<float> dayLength;
  Maybe<pair<List<pair<String, float>>, Vec2F>> nearbyPlanet;
  List<pair<List<pair<String, float>>, Vec2F>> nearbyMoons;
  List<pair<String, String>> horizonImages;
  bool horizonClouds = false;
  SkyType skyType = SkyType::Barren;
  Either<SkyColoring, Color> skyColoring = makeRight(Color::Black);
  Maybe<float> spaceLevel;
  Maybe<float> surfaceLevel;
  String sunType;
  Json settings = JsonObject();
};

DataStream& operator>>(DataStream& ds, SkyParameters& sky);
DataStream& operator<<(DataStream& ds, SkyParameters const& sky);
}// namespace Star

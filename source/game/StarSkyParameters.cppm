module;

#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarJson.hpp"
import star.sky_types;
#include "StarEither.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;

namespace Star {

STAR_CLASS(CelestialParameters);
STAR_CLASS(CelestialDatabase);

STAR_STRUCT(SkyParameters);

STAR_STRUCT(VisitableWorldParameters);

// This struct is a stripped down version of CelestialParameters that only
// contains the required inforamtion to generate a sky.  It's constructable
// from a CelestialParameters or importantly from Json.  This allows places
// without a coordinate (and therefore without CelestialParameters) to have a
// valid sky. (Instances, outposts and the like.)
// Additionally, a copy-ish constructor is provided to allow changing elements
// derived from the visitableworldparameters without reconstructing all sky
// parameters, e.g. for terraforming
struct SkyParameters {
  friend DataStream& operator>>(DataStream& ds, SkyParameters& value);
  friend DataStream& operator<<(DataStream& ds, SkyParameters const& value);
  SkyParameters();
  SkyParameters(CelestialCoordinate const& coordinate, CelestialDatabasePtr const& celestialDatabase);
  SkyParameters(SkyParameters const& oldSkyParameters, VisitableWorldParametersConstPtr newVisitableParameters);
  explicit SkyParameters(Json const& config);

  Json toJson() const;

  void read(DataStream& ds);
  void write(DataStream& ds) const;

  void readVisitableParameters(VisitableWorldParametersConstPtr visitableParameters);

  uint64_t seed;
  Maybe<float> dayLength;
  Maybe<pair<List<pair<String, float>>, Vec2F>> nearbyPlanet;
  List<pair<List<pair<String, float>>, Vec2F>> nearbyMoons;
  List<pair<String, String>> horizonImages;
  bool horizonClouds = true;
  SkyType skyType = SkyType::Barren;
  Either<SkyColoring, Color> skyColoring;
  Maybe<float> spaceLevel;
  Maybe<float> surfaceLevel;
  String sunType;
  Json settings;
};

DataStream& operator>>(DataStream& ds, SkyParameters& sky);
DataStream& operator<<(DataStream& ds, SkyParameters const& sky);
}

export module star.sky_parameters;

export namespace Star {
  using ::Star::SkyParameters;
  using ::Star::CelestialParameters;
  using ::Star::CelestialParametersPtr;
  using ::Star::CelestialParametersConstPtr;
  using ::Star::CelestialParametersWeakPtr;
  using ::Star::CelestialParametersConstWeakPtr;
  using ::Star::CelestialParametersUPtr;
  using ::Star::CelestialParametersConstUPtr;
  using ::Star::CelestialDatabase;
  using ::Star::CelestialDatabasePtr;
  using ::Star::CelestialDatabaseConstPtr;
  using ::Star::CelestialDatabaseWeakPtr;
  using ::Star::CelestialDatabaseConstWeakPtr;
  using ::Star::CelestialDatabaseUPtr;
  using ::Star::CelestialDatabaseConstUPtr;
  using ::Star::SkyParametersPtr;
  using ::Star::SkyParametersConstPtr;
  using ::Star::SkyParametersWeakPtr;
  using ::Star::SkyParametersConstWeakPtr;
  using ::Star::SkyParametersUPtr;
  using ::Star::SkyParametersConstUPtr;
  using ::Star::VisitableWorldParameters;
  using ::Star::VisitableWorldParametersPtr;
  using ::Star::VisitableWorldParametersConstPtr;
  using ::Star::VisitableWorldParametersWeakPtr;
  using ::Star::VisitableWorldParametersConstWeakPtr;
  using ::Star::VisitableWorldParametersUPtr;
  using ::Star::VisitableWorldParametersConstUPtr;
}

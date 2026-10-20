module;
import std;
import star.string;
import star.list;
import star.data_stream;
import star.json;

import star.color;
import star.bi_map;
import star.sky_types;
import star.either;
import star.vector;
import star.celestial_coordinate;

namespace Star {

class CelestialParameters;
class CelestialDatabase;
class Root;

struct VisitableWorldParameters;

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
  SkyParameters(Root& root, CelestialCoordinate const& coordinate, std::shared_ptr<CelestialDatabase> const& celestialDatabase);
  SkyParameters(SkyParameters const& oldSkyParameters, std::shared_ptr<VisitableWorldParameters const> newVisitableParameters);
  explicit SkyParameters(Json const& config);

  Json toJson() const;

  void read(DataStream& ds);
  void write(DataStream& ds) const;

  void readVisitableParameters(std::shared_ptr<VisitableWorldParameters const> visitableParameters);

  std::uint64_t seed;
  std::optional<float> dayLength;
  std::optional<std::pair<List<std::pair<String, float>>, Vec2F>> nearbyPlanet;
  List<std::pair<List<std::pair<String, float>>, Vec2F>> nearbyMoons;
  List<std::pair<String, String>> horizonImages;
  bool horizonClouds = true;
  SkyType skyType = SkyType::Barren;
  Either<SkyColoring, Color> skyColoring;
  std::optional<float> spaceLevel;
  std::optional<float> surfaceLevel;
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
  using ::Star::CelestialDatabase;
  using ::Star::VisitableWorldParameters;
}

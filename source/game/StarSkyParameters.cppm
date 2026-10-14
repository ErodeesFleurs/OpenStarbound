module;
import std;
import star.string;
import star.list;
import star.data_stream;
import star.maybe;
import star.json;

import star.color;
import star.bi_map;
import star.sky_types;
import star.either;
import star.vector;
import star.celestial_coordinate;

namespace Star {

class CelestialParameters;
using CelestialParametersPtr = std::shared_ptr<CelestialParameters>;
using CelestialParametersConstPtr = std::shared_ptr<const CelestialParameters>;
using CelestialParametersWeakPtr = std::weak_ptr<CelestialParameters>;
using CelestialParametersConstWeakPtr = std::weak_ptr<const CelestialParameters>;
using CelestialParametersUPtr = std::unique_ptr<CelestialParameters>;
using CelestialParametersConstUPtr = std::unique_ptr<const CelestialParameters>;
class CelestialDatabase;
using CelestialDatabasePtr = std::shared_ptr<CelestialDatabase>;
using CelestialDatabaseConstPtr = std::shared_ptr<const CelestialDatabase>;
using CelestialDatabaseWeakPtr = std::weak_ptr<CelestialDatabase>;
using CelestialDatabaseConstWeakPtr = std::weak_ptr<const CelestialDatabase>;
using CelestialDatabaseUPtr = std::unique_ptr<CelestialDatabase>;
using CelestialDatabaseConstUPtr = std::unique_ptr<const CelestialDatabase>;

struct SkyParameters;
using SkyParametersPtr = std::shared_ptr<SkyParameters>;
using SkyParametersConstPtr = std::shared_ptr<const SkyParameters>;
using SkyParametersWeakPtr = std::weak_ptr<SkyParameters>;
using SkyParametersConstWeakPtr = std::weak_ptr<const SkyParameters>;
using SkyParametersUPtr = std::unique_ptr<SkyParameters>;
using SkyParametersConstUPtr = std::unique_ptr<const SkyParameters>;

struct VisitableWorldParameters;
using VisitableWorldParametersPtr = std::shared_ptr<VisitableWorldParameters>;
using VisitableWorldParametersConstPtr = std::shared_ptr<const VisitableWorldParameters>;
using VisitableWorldParametersWeakPtr = std::weak_ptr<VisitableWorldParameters>;
using VisitableWorldParametersConstWeakPtr = std::weak_ptr<const VisitableWorldParameters>;
using VisitableWorldParametersUPtr = std::unique_ptr<VisitableWorldParameters>;
using VisitableWorldParametersConstUPtr = std::unique_ptr<const VisitableWorldParameters>;

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

  std::uint64_t seed;
  Maybe<float> dayLength;
  Maybe<std::pair<List<std::pair<String, float>>, Vec2F>> nearbyPlanet;
  List<std::pair<List<std::pair<String, float>>, Vec2F>> nearbyMoons;
  List<std::pair<String, String>> horizonImages;
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

module;
import std;
import star.string;
import star.json;

import star.celestial_coordinate;
import star.variant;
import star.game_types;
import star.color;
import star.bi_map;
import star.sky_types;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;

import star.byte_array;
namespace Star {

class CelestialParameters;
using CelestialParametersPtr = std::shared_ptr<CelestialParameters>;
using CelestialParametersConstPtr = std::shared_ptr<const CelestialParameters>;
using CelestialParametersWeakPtr = std::weak_ptr<CelestialParameters>;
using CelestialParametersConstWeakPtr = std::weak_ptr<const CelestialParameters>;
using CelestialParametersUPtr = std::unique_ptr<CelestialParameters>;
using CelestialParametersConstUPtr = std::unique_ptr<const CelestialParameters>;

class CelestialParameters {
public:
  CelestialParameters();
  CelestialParameters(CelestialCoordinate coordinate, std::uint64_t seed, String name, Json parameters);
  explicit CelestialParameters(Json const& diskStore);
  explicit CelestialParameters(ByteArray netStore);

  Json diskStore() const;
  ByteArray netStore() const;

  CelestialCoordinate coordinate() const;
  String name() const;
  std::uint64_t seed() const;

  Json parameters() const;
  Json getParameter(String const& name, Json def = Json()) const;
  // Predictably select from a json array, given by the named parameter.
  // Selects based on the name hash and the system seed.
  Json randomizeParameterList(String const& name, std::int32_t mix = 0) const;
  // Predictably select from a range, given by the named parameter.  Works for
  // either floating or integral ranges.
  Json randomizeParameterRange(String const& name, std::int32_t mix = 0) const;
  // Same function, but if you want to specify the range from an external source
  Json randomizeParameterRange(JsonArray const& range, std::int32_t mix = 0, Maybe<String> const& name = {}) const;

  // Not all worlds are visitable, if the world is not visitable its
  // visitableParameters will be empty.
  bool isVisitable() const;
  VisitableWorldParametersConstPtr visitableParameters() const;
  void setVisitableParameters(VisitableWorldParametersPtr const& newVisitableParameters);

private:
  CelestialCoordinate m_coordinate;
  std::uint64_t m_seed;
  String m_name;
  Json m_parameters;
  VisitableWorldParametersConstPtr m_visitableParameters;
};

}

export module star.celestial_parameters;

export namespace Star {
  using ::Star::CelestialParameters;
  using ::Star::CelestialParametersPtr;
  using ::Star::CelestialParametersConstPtr;
  using ::Star::CelestialParametersWeakPtr;
  using ::Star::CelestialParametersConstWeakPtr;
  using ::Star::CelestialParametersUPtr;
  using ::Star::CelestialParametersConstUPtr;
}

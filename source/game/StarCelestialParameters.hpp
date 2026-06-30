#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarWorldParameters.hpp"

namespace Star {

class CelestialParameters {
public:
  CelestialParameters() = default;
  CelestialParameters(CelestialCoordinate coordinate, uint64_t seed, String name, Json parameters, AssetsConstPtr assets, LiquidsDatabaseConstPtr liquidsDatabase, BiomeDatabaseConstPtr biomeDatabase);
  explicit CelestialParameters(Json const& diskStore);
  explicit CelestialParameters(ByteArray netStore);

  [[nodiscard]] Json diskStore() const;
  [[nodiscard]] ByteArray netStore() const;

  [[nodiscard]] CelestialCoordinate coordinate() const;
  [[nodiscard]] String name() const;
  [[nodiscard]] uint64_t seed() const;

  [[nodiscard]] Json parameters() const;
  [[nodiscard]] Json getParameter(String const& name, Json def = Json()) const;
  // Predictably select from a json array, given by the named parameter.
  // Selects based on the name hash and the system seed.
  [[nodiscard]] Json randomizeParameterList(String const& name, int32_t mix = 0) const;
  // Predictably select from a range, given by the named parameter.  Works for
  // either floating or integral ranges.
  [[nodiscard]] Json randomizeParameterRange(String const& name, int32_t mix = 0) const;
  // Same function, but if you want to specify the range from an external source
  [[nodiscard]] Json randomizeParameterRange(JsonArray const& range, int32_t mix = 0, Maybe<String> const& name = {}) const;

  // Not all worlds are visitable, if the world is not visitable its
  // visitableParameters will be empty.
  [[nodiscard]] bool isVisitable() const;
  [[nodiscard]] VisitableWorldParametersConstPtr visitableParameters() const;
  void setVisitableParameters(VisitableWorldParametersPtr const& newVisitableParameters);

private:
  CelestialCoordinate m_coordinate;
  uint64_t m_seed = 0;
  String m_name;
  Json m_parameters;
  VisitableWorldParametersConstPtr m_visitableParameters;
};

}// namespace Star

#pragma once

#include "StarCelestialCoordinate.hpp"
#include "StarConfig.hpp"
#include "StarWorldParameters.hpp"

namespace Star {

class CelestialParameters {
public:
  CelestialParameters();
  CelestialParameters(CelestialCoordinate coordinate, uint64_t seed, String name, Json parameters);
  explicit CelestialParameters(Json const& diskStore);
  explicit CelestialParameters(ByteArray netStore);

  [[nodiscard]] auto diskStore() const -> Json;
  [[nodiscard]] auto netStore() const -> ByteArray;

  [[nodiscard]] auto coordinate() const -> CelestialCoordinate;
  [[nodiscard]] auto name() const -> String;
  [[nodiscard]] auto seed() const -> uint64_t;

  [[nodiscard]] auto parameters() const -> Json;
  [[nodiscard]] auto getParameter(String const& name, Json def = Json()) const -> Json;
  // Predictably select from a json array, given by the named parameter.
  // Selects based on the name hash and the system seed.
  [[nodiscard]] auto randomizeParameterList(String const& name, int32_t mix = 0) const -> Json;
  // Predictably select from a range, given by the named parameter.  Works for
  // either floating or integral ranges.
  [[nodiscard]] auto randomizeParameterRange(String const& name, int32_t mix = 0) const -> Json;
  // Same function, but if you want to specify the range from an external source
  [[nodiscard]] auto randomizeParameterRange(JsonArray const& range, int32_t mix = 0, std::optional<String> const& name = {}) const -> Json;

  // Not all worlds are visitable, if the world is not visitable its
  // visitableParameters will be empty.
  [[nodiscard]] auto isVisitable() const -> bool;
  [[nodiscard]] auto visitableParameters() const -> ConstPtr<VisitableWorldParameters>;
  void setVisitableParameters(Ptr<VisitableWorldParameters> const& newVisitableParameters);

private:
  CelestialCoordinate m_coordinate;
  uint64_t m_seed;
  String m_name;
  Json m_parameters;
  ConstPtr<VisitableWorldParameters> m_visitableParameters;
};

}// namespace Star

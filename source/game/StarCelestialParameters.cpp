#include "StarCelestialParameters.hpp"
#include "StarConfig.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarStaticRandom.hpp"

import std;

namespace Star {

CelestialParameters::CelestialParameters() : m_seed(0) {}

CelestialParameters::CelestialParameters(CelestialCoordinate coordinate, std::uint64_t seed, String name, Json parameters)
    : m_coordinate(std::move(coordinate)), m_seed(seed), m_name(std::move(name)), m_parameters(std::move(parameters)) {
  if (auto worldType = getParameter("worldType").optString()) {
    if (worldType->equalsIgnoreCase("Terrestrial")) {
      auto worldSize = getParameter("worldSize").toString();
      auto type = randomizeParameterList("terrestrialType").toString();
      m_visitableParameters = generateTerrestrialWorldParameters(type, worldSize, m_seed);
    } else if (worldType->equalsIgnoreCase("Asteroids")) {
      m_visitableParameters = generateAsteroidsWorldParameters(m_seed);
    } else if (worldType->equalsIgnoreCase("FloatingDungeon")) {
      m_visitableParameters = generateFloatingDungeonWorldParameters(getParameter("dungeonWorld").toString());
    }
  }
}

CelestialParameters::CelestialParameters(ByteArray netStore) {
  DataStreamBuffer ds(std::move(netStore));
  ds >> m_coordinate;
  ds >> m_seed;
  ds >> m_name;
  ds >> m_parameters;
  m_visitableParameters = netLoadVisitableWorldParameters(ds.read<ByteArray>());
}

CelestialParameters::CelestialParameters(Json const& variant) {
  m_coordinate = CelestialCoordinate(variant.get("coordinate"));
  m_seed = variant.getUInt("seed");
  m_name = variant.getString("name");
  m_parameters = variant.get("parameters");
  m_visitableParameters = diskLoadVisitableWorldParameters(variant.get("visitableParameters"));
}

auto CelestialParameters::diskStore() const -> Json {
  return JsonObject{{"coordinate", m_coordinate.toJson()},
                    {"seed", m_seed},
                    {"name", m_name},
                    {"parameters", m_parameters},
                    {"visitableParameters", diskStoreVisitableWorldParameters(m_visitableParameters)}};
}

auto CelestialParameters::netStore() const -> ByteArray {
  DataStreamBuffer ds;
  ds << m_coordinate;
  ds << m_seed;
  ds << m_name;
  ds << m_parameters;
  ds.write(netStoreVisitableWorldParameters(m_visitableParameters));

  return ds.takeData();
}

auto CelestialParameters::coordinate() const -> CelestialCoordinate {
  return m_coordinate;
}

auto CelestialParameters::name() const -> String {
  return m_name;
}

auto CelestialParameters::seed() const -> std::uint64_t {
  return m_seed;
}

auto CelestialParameters::parameters() const -> Json {
  return m_parameters;
}

auto CelestialParameters::getParameter(String const& name, Json def) const -> Json {
  return m_parameters.get(name, std::move(def));
}

auto CelestialParameters::randomizeParameterList(String const& name, std::int32_t mix) const -> Json {
  auto parameter = getParameter(name);
  if (parameter.isNull())
    return {};
  return staticRandomFrom(parameter.toArray(), mix, m_seed, name);
}

auto CelestialParameters::randomizeParameterRange(String const& name, std::int32_t mix) const -> Json {
  auto parameter = getParameter(name);
  if (parameter.isNull()) {
    return {};
  } else {
    JsonArray list = parameter.toArray();
    if (list.size() != 2)
      throw CelestialException(
        strf("Parameter '{}' does not appear to be a range in CelestialParameters::randomizeRange", name));

    return randomizeParameterRange(list, mix, name);
  }
}

auto CelestialParameters::randomizeParameterRange(
  JsonArray const& range, std::int32_t mix, std::optional<String> const& name) const -> Json {
  if (range[0].type() == Json::Type::Int) {
    std::int64_t min = range[0].toInt();
    std::int64_t max = range[1].toInt();
    return staticRandomU64(mix, m_seed, name.value_or("")) % (max - min + 1) + min;
  } else {
    double min = range[0].toDouble();
    double max = range[1].toDouble();
    return staticRandomDouble(mix, m_seed, name.value_or("")) * (max - min) + min;
  }
}

auto CelestialParameters::isVisitable() const -> bool {
  return (bool)m_visitableParameters;
}

auto CelestialParameters::visitableParameters() const -> ConstPtr<VisitableWorldParameters> {
  return m_visitableParameters;
}

void CelestialParameters::setVisitableParameters(Ptr<VisitableWorldParameters> const& newVisitableParameters) {
  m_visitableParameters = newVisitableParameters;
}

}// namespace Star

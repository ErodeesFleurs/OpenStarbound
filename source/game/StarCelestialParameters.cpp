import star.json;
import star.json_extra;
import star.variant;
import star.game_types;
import star.color;
import star.bi_map;
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
import star.static_random;
import star.data_stream_devices;
import star.data_stream_extra;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.format;
import star.version;

import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.animation;
import star.particle;
import star.weather_types;

import star.byte_array;
import star.pythonic;
namespace Star {

CelestialParameters::CelestialParameters() : m_seed(0) {}

CelestialParameters::CelestialParameters(Root& root, CelestialCoordinate coordinate, std::uint64_t seed, String name, Json parameters)
  : m_coordinate(std::move(coordinate)), m_seed(seed), m_name(std::move(name)), m_parameters(std::move(parameters)) {
  if (auto worldType = getParameter("worldType").optString()) {
    if (worldType->equalsIgnoreCase("Terrestrial")) {
      auto worldSize = getParameter("worldSize").toString();
      auto type = randomizeParameterList("terrestrialType").toString();
      m_visitableParameters = generateTerrestrialWorldParameters(root, type, worldSize, m_seed);
    } else if (worldType->equalsIgnoreCase("Asteroids")) {
      m_visitableParameters = generateAsteroidsWorldParameters(root, m_seed);
    } else if (worldType->equalsIgnoreCase("FloatingDungeon")) {
      m_visitableParameters = generateFloatingDungeonWorldParameters(root, getParameter("dungeonWorld").toString());
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

Json CelestialParameters::diskStore() const {
  return JsonObject{{"coordinate", m_coordinate.toJson()},
      {"seed", m_seed},
      {"name", m_name},
      {"parameters", m_parameters},
      {"visitableParameters", diskStoreVisitableWorldParameters(m_visitableParameters)}};
}

ByteArray CelestialParameters::netStore() const {
  DataStreamBuffer ds;
  ds << m_coordinate;
  ds << m_seed;
  ds << m_name;
  ds << m_parameters;
  ds.write(netStoreVisitableWorldParameters(m_visitableParameters));

  return ds.takeData();
}

CelestialCoordinate CelestialParameters::coordinate() const {
  return m_coordinate;
}

String CelestialParameters::name() const {
  return m_name;
}

std::uint64_t CelestialParameters::seed() const {
  return m_seed;
}

Json CelestialParameters::parameters() const {
  return m_parameters;
}

Json CelestialParameters::getParameter(String const& name, Json def) const {
  return m_parameters.get(name, std::move(def));
}

Json CelestialParameters::randomizeParameterList(String const& name, std::int32_t mix) const {
  auto parameter = getParameter(name, Json());
  if (parameter.isNull())
    return Json();
  return staticRandomFrom(parameter.toArray(), mix, m_seed, name);
}

Json CelestialParameters::randomizeParameterRange(String const& name, std::int32_t mix) const {
  auto parameter = getParameter(name, Json());
  if (parameter.isNull()) {
    return Json();
  } else {
    JsonArray list = parameter.toArray();
    if (list.size() != 2)
      throw CelestialException(
          strf("Parameter '{}' does not appear to be a range in CelestialParameters::randomizeRange", name));

    return randomizeParameterRange(list, mix, name);
  }
}

Json CelestialParameters::randomizeParameterRange(
    JsonArray const& range, std::int32_t mix, Maybe<String> const& name) const {
  if (range[0].type() == Json::Type::Int) {
    std::int64_t min = range[0].toInt();
    std::int64_t max = range[1].toInt();
    return staticRandomU64(mix, m_seed, name.value("")) % (max - min + 1) + min;
  } else {
    double min = range[0].toDouble();
    double max = range[1].toDouble();
    return staticRandomDouble(mix, m_seed, name.value("")) * (max - min) + min;
  }
}

bool CelestialParameters::isVisitable() const {
  return (bool)m_visitableParameters;
}

std::shared_ptr<VisitableWorldParameters const> CelestialParameters::visitableParameters() const {
  return m_visitableParameters;
}

void CelestialParameters::setVisitableParameters(std::shared_ptr<VisitableWorldParameters> const& newVisitableParameters) {
  m_visitableParameters = newVisitableParameters;
}

}

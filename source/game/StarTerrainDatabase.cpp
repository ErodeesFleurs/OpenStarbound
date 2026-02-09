#include "StarTerrainDatabase.hpp"

#include "StarCacheSelector.hpp"
#include "StarConfig.hpp"
#include "StarConstantSelector.hpp"
#include "StarDisplacementSelector.hpp"
#include "StarFlatSurfaceSelector.hpp"
#include "StarIslandSurfaceSelector.hpp"
#include "StarKarstCave.hpp"
#include "StarMaxSelector.hpp"
#include "StarMinMaxSelector.hpp"
#include "StarMixSelector.hpp"
#include "StarPerlinSelector.hpp"
#include "StarRandom.hpp"
#include "StarRidgeBlocksSelector.hpp"
#include "StarRoot.hpp"
#include "StarRotateSelector.hpp"
#include "StarWormCave.hpp"

import std;

namespace Star {

TerrainSelectorParameters::TerrainSelectorParameters() {
  seed = Random::randu64();
  worldWidth = 0;
  commonality = 1.0f;
}

TerrainSelectorParameters::TerrainSelectorParameters(Json const& v) {
  worldWidth = v.getUInt("worldWidth");
  baseHeight = v.getFloat("baseHeight");
  seed = v.getUInt("seed");
  commonality = v.getFloat("commonality");
}

auto TerrainSelectorParameters::toJson() const -> Json {
  return JsonObject{
    {"worldWidth", worldWidth},
    {"baseHeight", baseHeight},
    {"seed", seed},
    {"commonality", commonality}};
}

auto TerrainSelectorParameters::withSeed(std::uint64_t seed) const -> TerrainSelectorParameters {
  TerrainSelectorParameters copy = *this;
  copy.seed = seed;
  return copy;
}

auto TerrainSelectorParameters::withCommonality(float commonality) const -> TerrainSelectorParameters {
  TerrainSelectorParameters copy = *this;
  copy.commonality = commonality;
  return copy;
}

TerrainSelector::TerrainSelector(String type, Json config, TerrainSelectorParameters parameters)
    : type(std::move(type)), config(std::move(config)), parameters(std::move(parameters)) {}

TerrainSelector::~TerrainSelector() = default;

TerrainDatabase::TerrainDatabase() {
  auto assets = Root::singleton().assets();

  // 'type' here is the extension of the file, and determines the selector type
  auto scanFiles = [this, assets](String const& type) -> void {
    auto& files = assets->scanExtension(type);
    assets->queueJsons(files);
    for (auto& path : files) {
      auto parameters = assets->json(path);
      auto name = parameters.getString("name");
      if (m_terrainSelectors.contains(name))
        throw TerrainException(strf("Duplicate terrain generator name '{}'", name));
      m_terrainSelectors[name] = {.type = type, .parameters = parameters};
    }
  };

  scanFiles(KarstCaveSelector::Name);
  scanFiles(WormCaveSelector::Name);
  scanFiles(RidgeBlocksSelector::Name);

  auto& files = assets->scanExtension("terrain");
  assets->queueJsons(files);
  for (auto& path : files) {
    auto parameters = assets->json(path);
    auto name = parameters.getString("name");
    auto type = parameters.getString("type");
    if (m_terrainSelectors.contains(name))
      throw TerrainException(strf("Duplicate composed terrain generator name '{}'", name));
    m_terrainSelectors[name] = {.type = type, .parameters = parameters};
  }
}

auto TerrainDatabase::selectorConfig(String const& name) const -> TerrainDatabase::Config {
  if (auto config = m_terrainSelectors.maybe(name))
    return std::move(*config);
  else
    throw TerrainException(strf("No such terrain selector '{}'", name));
}

auto TerrainDatabase::createNamedSelector(String const& name, TerrainSelectorParameters const& parameters) const -> ConstPtr<TerrainSelector> {
  auto config = selectorConfig(name);

  return createSelectorType(config.type, config.parameters, parameters);
}

auto TerrainDatabase::constantSelector(float value) -> ConstPtr<TerrainSelector> {
  return createSelectorType(ConstantSelector::Name, JsonObject{{"value", value}}, TerrainSelectorParameters());
}

auto TerrainDatabase::storeSelector(ConstPtr<TerrainSelector> const& selector) const -> Json {
  if (!selector)
    return {};

  return JsonObject{
    {"type", selector->type},
    {"config", selector->config},
    {"parameters", selector->parameters.toJson()}};
}

auto TerrainDatabase::loadSelector(Json const& store) const -> ConstPtr<TerrainSelector> {
  if (store.isNull())
    return {};
  return createSelectorType(store.getString("type"), store.get("config"),
                            TerrainSelectorParameters(store.get("parameters")));
}

auto TerrainDatabase::createSelectorType(String const& type, Json const& config, TerrainSelectorParameters const& parameters) const -> ConstPtr<TerrainSelector> {
  if (type == WormCaveSelector::Name)
    return std::make_shared<WormCaveSelector>(config, parameters);
  else if (type == KarstCaveSelector::Name)
    return std::make_shared<KarstCaveSelector>(config, parameters);
  else if (type == ConstantSelector::Name)
    return std::make_shared<ConstantSelector>(config, parameters);
  else if (type == MaxSelector::Name)
    return std::make_shared<MaxSelector>(config, parameters, this);
  else if (type == MinMaxSelector::Name)
    return std::make_shared<MinMaxSelector>(config, parameters, this);
  else if (type == IslandSurfaceSelector::Name)
    return std::make_shared<IslandSurfaceSelector>(config, parameters);
  else if (type == FlatSurfaceSelector::Name)
    return std::make_shared<FlatSurfaceSelector>(config, parameters);
  else if (type == DisplacementSelector::Name)
    return std::make_shared<DisplacementSelector>(config, parameters, this);
  else if (type == RotateSelector::Name)
    return std::make_shared<RotateSelector>(config, parameters, this);
  else if (type == MixSelector::Name)
    return std::make_shared<MixSelector>(config, parameters, this);
  else if (type == PerlinSelector::Name)
    return std::make_shared<PerlinSelector>(config, parameters);
  else if (type == RidgeBlocksSelector::Name)
    return std::make_shared<RidgeBlocksSelector>(config, parameters);
  else if (type == CacheSelector::Name)
    return std::make_shared<CacheSelector>(config, parameters, this);
  else
    throw TerrainException(strf("Unknown terrain selector type '{}'", type));
}

}// namespace Star

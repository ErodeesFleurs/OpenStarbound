#pragma once

#include "StarBiome.hpp"
#include "StarConfig.hpp"
#include "StarSkyTypes.hpp"
#include "StarWeatherTypes.hpp"

import std;

namespace Star {

class BiomeDatabase {
public:
  BiomeDatabase();

  [[nodiscard]] auto biomeNames() const -> StringList;

  [[nodiscard]] auto biomeHueShift(String const& biomeName, std::uint64_t seed) const -> float;
  [[nodiscard]] auto biomeWeathers(String const& biomeName, std::uint64_t seed, float threatLevel) const -> WeatherPool;
  [[nodiscard]] auto biomeIsAirless(String const& biomeName) const -> bool;
  [[nodiscard]] auto biomeSkyColoring(String const& biomeName, std::uint64_t seed) const -> SkyColoring;
  [[nodiscard]] auto biomeFriendlyName(String const& biomeName) const -> String;
  [[nodiscard]] auto biomeStatusEffects(String const& biomeName) const -> StringList;
  [[nodiscard]] auto biomeOres(String const& biomeName, float threatLevel) const -> StringList;

  [[nodiscard]] auto weatherNames() const -> StringList;
  [[nodiscard]] auto weatherType(String const& weatherName) const -> WeatherType;

  [[nodiscard]] auto createBiome(String const& biomeName, std::uint64_t seed, float verticalMidPoint, float threatLevel) const -> Ptr<Biome>;

private:
  struct Config {
    String path;
    String name;
    Json parameters;
  };
  using ConfigMap = StringMap<Config>;

  static auto pickHueShiftFromJson(Json source, std::uint64_t seed, String const& key) -> float;

  [[nodiscard]] auto readBiomePlaceables(Json const& config, std::uint64_t seed, float biomeHueShift) const -> BiomePlaceables;
  [[nodiscard]] auto readOres(Json const& oreDistribution, float threatLevel) const -> List<std::pair<ModId, float>>;

  ConfigMap m_biomes;
  ConfigMap m_weathers;
};

}// namespace Star

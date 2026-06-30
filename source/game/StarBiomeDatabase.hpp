#pragma once

#include "StarAssets.hpp"
#include "StarBiome.hpp"
#include "StarWeatherTypes.hpp"
#include "StarSkyTypes.hpp"

namespace Star {

class BiomeDatabase;
using BiomeDatabasePtr = SharedPtr<BiomeDatabase>;
using BiomeDatabaseConstPtr = SharedPtr<BiomeDatabase const>;
class FunctionDatabase;
using FunctionDatabaseConstPtr = SharedPtr<FunctionDatabase const>;
class MaterialDatabase;
using MaterialDatabaseConstPtr = SharedPtr<MaterialDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;
class PlantDatabase;
using PlantDatabaseConstPtr = SharedPtr<PlantDatabase const>;

class BiomeDatabase {
public:
  BiomeDatabase(AssetsConstPtr assets, MaterialDatabaseConstPtr materialDatabase, FunctionDatabaseConstPtr functionDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, PlantDatabaseConstPtr plantDatabase);

  StringList biomeNames() const;

  float biomeHueShift(String const& biomeName, uint64_t seed) const;
  WeatherPool biomeWeathers(String const& biomeName, uint64_t seed, float threatLevel) const;
  bool biomeIsAirless(String const& biomeName) const;
  SkyColoring biomeSkyColoring(String const& biomeName, uint64_t seed) const;
  String biomeFriendlyName(String const& biomeName) const;
  StringList biomeStatusEffects(String const& biomeName) const;
  StringList biomeOres(String const& biomeName, float threatLevel) const;

  StringList weatherNames() const;
  WeatherType weatherType(String const& weatherName) const;

  BiomePtr createBiome(String const& biomeName, uint64_t seed, float verticalMidPoint, float threatLevel) const;

  Json biomeConfig(String const& biomeName) const;

private:
  struct Config {
    String path;
    String name;
    Json parameters;
    Json toJson() const;
  };
  using ConfigMap = StringMap<Config>;

  static float pickHueShiftFromJson(Json source, uint64_t seed, String const& key);

  BiomePlaceables readBiomePlaceables(Json const& config, uint64_t seed, float biomeHueShift) const;
  List<pair<ModId, float>> readOres(Json const& oreDistribution, float threatLevel) const;

  ConfigMap m_biomes;
  ConfigMap m_weathers;
  AssetsConstPtr m_assets;
  MaterialDatabaseConstPtr m_materialDatabase;
  FunctionDatabaseConstPtr m_functionDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  PlantDatabaseConstPtr m_plantDatabase;
  Json m_spawnGroups;
};

}

module;
import std;
import star.string;
import star.json;

import star.game_types;
import star.material_types;
import star.maybe;
import star.random;
import star.weighted_pool;
import star.color;
import star.bi_map;
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
import star.sky_types;

namespace Star {
class Root;

struct Biome;
struct BiomePlaceables;

class BiomeDatabase {
public:
  explicit BiomeDatabase(Root& root);

  StringList biomeNames() const;

  float biomeHueShift(String const& biomeName, std::uint64_t seed) const;
  bool biomeHasWeather(String const& biomeName) const;
  WeatherPool biomeWeathers(String const& biomeName, std::uint64_t seed, float threatLevel) const;
  bool biomeIsAirless(String const& biomeName) const;
  SkyColoring biomeSkyColoring(String const& biomeName, std::uint64_t seed) const;
  String biomeFriendlyName(String const& biomeName) const;
  StringList biomeStatusEffects(String const& biomeName) const;
  StringList biomeOres(String const& biomeName, float threatLevel) const;

  StringList weatherNames() const;
  WeatherType weatherType(String const& weatherName) const;

  std::shared_ptr<Biome> createBiome(String const& biomeName, std::uint64_t seed, float verticalMidPoint, float threatLevel) const;

  Json biomeConfig(String const& biomeName) const;

private:
  Root& m_root;
  struct Config {
    String path;
    String name;
    Json parameters;
    Json toJson() const;
  };
  typedef StringMap<Config> ConfigMap;

  static float pickHueShiftFromJson(Json source, std::uint64_t seed, String const& key);

  BiomePlaceables readBiomePlaceables(Json const& config, std::uint64_t seed, float biomeHueShift) const;
  List<std::pair<ModId, float>> readOres(Json const& oreDistribution, float threatLevel) const;

  ConfigMap m_biomes;
  ConfigMap m_weathers;
};

}

export module star.biome_database;

export namespace Star {
  using ::Star::Biome;
  using ::Star::BiomeDatabase;
}

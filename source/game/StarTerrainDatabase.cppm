module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {

struct TerrainExceptionTag {
  static constexpr char const* name() { return "TerrainException"; }
};
using TerrainException = StarError<TerrainExceptionTag, StarException>;

struct TerrainSelectorParameters {
  TerrainSelectorParameters();
  explicit TerrainSelectorParameters(Json const& v);

  Json toJson() const;

  TerrainSelectorParameters withSeed(std::uint64_t seed) const;
  TerrainSelectorParameters withCommonality(float commonality) const;

  unsigned worldWidth;
  float baseHeight;
  std::uint64_t seed;
  float commonality;
};

struct TerrainSelector {
  TerrainSelector(String type, Json config, TerrainSelectorParameters parameters);
  virtual ~TerrainSelector();

  // Returns a float signifying the "solid-ness" of a block, >= 0.0 should be
  // considered solid, < 0.0 should be considered open space.
  virtual float get(int x, int y) const = 0;

  String type;
  Json config;
  TerrainSelectorParameters parameters;
};

class TerrainDatabase {
public:
  struct Config {
    String type;
    Json parameters;
  };

  Config selectorConfig(String const& name) const;
  std::shared_ptr<TerrainSelector const> createSelectorType(String const& type, Json const& config, TerrainSelectorParameters const& parameters) const;

  TerrainDatabase();

  std::shared_ptr<TerrainSelector const> createNamedSelector(String const& name, TerrainSelectorParameters const& parameters) const;
  std::shared_ptr<TerrainSelector const> constantSelector(float value);

  Json storeSelector(std::shared_ptr<TerrainSelector const> const& selector) const;
  std::shared_ptr<TerrainSelector const> loadSelector(Json const& store) const;

private:
  StringMap<Config> m_terrainSelectors;
};

}

export module star.terrain_database;

export namespace Star {
  using ::Star::TerrainExceptionTag;
  using ::Star::TerrainException;
  using ::Star::TerrainSelectorParameters;
  using ::Star::TerrainSelector;
  using ::Star::TerrainDatabase;
}

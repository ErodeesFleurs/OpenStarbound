module;

import std;
import star.string;
import star.format;
import star.json;

namespace Star {

struct TerrainSelector;
using TerrainSelectorPtr = std::shared_ptr<TerrainSelector>;
using TerrainSelectorConstPtr = std::shared_ptr<const TerrainSelector>;
using TerrainSelectorWeakPtr = std::weak_ptr<TerrainSelector>;
using TerrainSelectorConstWeakPtr = std::weak_ptr<const TerrainSelector>;
using TerrainSelectorUPtr = std::unique_ptr<TerrainSelector>;
using TerrainSelectorConstUPtr = std::unique_ptr<const TerrainSelector>;
class TerrainDatabase;
using TerrainDatabasePtr = std::shared_ptr<TerrainDatabase>;
using TerrainDatabaseConstPtr = std::shared_ptr<const TerrainDatabase>;
using TerrainDatabaseWeakPtr = std::weak_ptr<TerrainDatabase>;
using TerrainDatabaseConstWeakPtr = std::weak_ptr<const TerrainDatabase>;
using TerrainDatabaseUPtr = std::unique_ptr<TerrainDatabase>;
using TerrainDatabaseConstUPtr = std::unique_ptr<const TerrainDatabase>;

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
  TerrainSelectorConstPtr createSelectorType(String const& type, Json const& config, TerrainSelectorParameters const& parameters) const;

  TerrainDatabase();

  TerrainSelectorConstPtr createNamedSelector(String const& name, TerrainSelectorParameters const& parameters) const;
  TerrainSelectorConstPtr constantSelector(float value);

  Json storeSelector(TerrainSelectorConstPtr const& selector) const;
  TerrainSelectorConstPtr loadSelector(Json const& store) const;

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
  using ::Star::TerrainSelectorPtr;
  using ::Star::TerrainSelectorConstPtr;
  using ::Star::TerrainSelectorWeakPtr;
  using ::Star::TerrainSelectorConstWeakPtr;
  using ::Star::TerrainSelectorUPtr;
  using ::Star::TerrainSelectorConstUPtr;
  using ::Star::TerrainDatabase;
  using ::Star::TerrainDatabasePtr;
  using ::Star::TerrainDatabaseConstPtr;
  using ::Star::TerrainDatabaseWeakPtr;
  using ::Star::TerrainDatabaseConstWeakPtr;
  using ::Star::TerrainDatabaseUPtr;
  using ::Star::TerrainDatabaseConstUPtr;
}

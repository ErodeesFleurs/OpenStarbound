module;
import std;
import star.string;
import star.set;
import star.variant;
import star.format;
import star.json;

import star.vector;
import star.bi_map;
import star.random;
import star.weighted_pool;

namespace Star {

struct SpawnTypeDatabaseExceptionTag {
  static constexpr char const* name() { return "SpawnTypeDatabaseException"; }
};
using SpawnTypeDatabaseException = StarError<SpawnTypeDatabaseExceptionTag, StarException>;

class SpawnTypeDatabase;
using SpawnTypeDatabasePtr = std::shared_ptr<SpawnTypeDatabase>;
using SpawnTypeDatabaseConstPtr = std::shared_ptr<const SpawnTypeDatabase>;
using SpawnTypeDatabaseWeakPtr = std::weak_ptr<SpawnTypeDatabase>;
using SpawnTypeDatabaseConstWeakPtr = std::weak_ptr<const SpawnTypeDatabase>;
using SpawnTypeDatabaseUPtr = std::unique_ptr<SpawnTypeDatabase>;
using SpawnTypeDatabaseConstUPtr = std::unique_ptr<const SpawnTypeDatabase>;

struct SpawnParameters {
  enum class Area : std::uint8_t {
    Surface,
    Ceiling,
    Air,
    Liquid,
    Solid
  };

  enum class Region : std::uint8_t {
    All,
    Enclosed,
    Exposed
  };

  enum class Time : std::uint8_t {
    All,
    Day,
    Night
  };

  static EnumMap<Area> const AreaNames;
  static EnumMap<Region> const RegionNames;
  static EnumMap<Time> const TimeNames;

  // Null config constructs SpawnParameters filled with All
  SpawnParameters();
  SpawnParameters(Json const& config);
  SpawnParameters(Set<Area> areas, Region region, Time time);

  bool compatible(SpawnParameters const& parameters) const;

  Set<Area> areas;
  Region region;
  Time time;
};

struct SpawnType {
  String typeName;

  Vec2F dayLevelAdjustment;
  Vec2F nightLevelAdjustment;

  Variant<String, WeightedPool<String>> monsterType;
  Json monsterParameters;

  Vec2I groupSize;
  float spawnChance;

  SpawnParameters spawnParameters;
  std::uint64_t seedMix;
};

SpawnType spawnTypeFromJson(Json const& config);

struct SpawnProfile {
  SpawnProfile();
  SpawnProfile(Json const& config);
  SpawnProfile(StringSet spawnTypes, Json monsterParameters);

  Json toJson() const;

  StringSet spawnTypes;
  Json monsterParameters;
};

SpawnProfile constructSpawnProfile(Json const& config, std::uint64_t seed);

class SpawnTypeDatabase {
public:
  SpawnTypeDatabase();

  SpawnType spawnType(String const& typeName) const;

private:
  StringMap<SpawnType> m_spawnTypes;
};

}

export module star.spawn_type_database;

export namespace Star {
  using ::Star::SpawnTypeDatabaseExceptionTag;
  using ::Star::SpawnTypeDatabaseException;
  using ::Star::SpawnTypeDatabase;
  using ::Star::SpawnTypeDatabasePtr;
  using ::Star::SpawnTypeDatabaseConstPtr;
  using ::Star::SpawnParameters;
  using ::Star::SpawnType;
  using ::Star::SpawnProfile;
  using ::Star::spawnTypeFromJson;
  using ::Star::constructSpawnProfile;
}

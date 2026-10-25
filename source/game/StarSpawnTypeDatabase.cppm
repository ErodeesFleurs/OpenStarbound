module;
import std;
import star.string;
import star.set;
import star.format;
import star.json;

import star.vector;
import star.bi_map;
import star.random;
import star.weighted_pool;

namespace Star {
class Assets;

struct SpawnTypeDatabaseExceptionTag {
  static constexpr std::string_view name() { return "SpawnTypeDatabaseException"; }
};
using SpawnTypeDatabaseException = StarError<SpawnTypeDatabaseExceptionTag, StarException>;

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

  static std::expected<EnumMap<Area>, StarException> const AreaNames;
  static std::expected<EnumMap<Region>, StarException> const RegionNames;
  static std::expected<EnumMap<Time>, StarException> const TimeNames;

  // Null config constructs SpawnParameters filled with All
  SpawnParameters();
  [[nodiscard]]
  static std::expected<SpawnParameters, StarException> create(Json const& config);
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

  std::variant<String, WeightedPool<String>> monsterType;
  Json monsterParameters;

  Vec2I groupSize;
  float spawnChance;

  SpawnParameters spawnParameters;
  std::uint64_t seedMix;
};

[[nodiscard]]
std::expected<SpawnType, StarException> spawnTypeFromJson(Json const& config);

struct SpawnProfile {
  SpawnProfile();
  [[nodiscard]]
  static std::expected<SpawnProfile, StarException> create(Json const& config);
  SpawnProfile(StringSet spawnTypes, Json monsterParameters);

  Json toJson() const;

  StringSet spawnTypes;
  Json monsterParameters;
};

[[nodiscard]]
std::expected<SpawnProfile, StarException> constructSpawnProfile(Assets const& assets, Json const& config, std::uint64_t seed);

class SpawnTypeDatabase {
public:
  [[nodiscard]]
  static std::expected<std::shared_ptr<SpawnTypeDatabase>, StarException> create(std::shared_ptr<Assets const> const& assets);

  [[nodiscard]]
  std::expected<SpawnType, StarException> spawnType(String const& typeName) const;

private:
  SpawnTypeDatabase() = default;
  StringMap<SpawnType> m_spawnTypes;
};

}

export module star.spawn_type_database;

export namespace Star {
  using ::Star::SpawnTypeDatabaseExceptionTag;
  using ::Star::SpawnTypeDatabaseException;
  using ::Star::SpawnTypeDatabase;
  using ::Star::SpawnParameters;
  using ::Star::SpawnType;
  using ::Star::SpawnProfile;
  using ::Star::spawnTypeFromJson;
  using ::Star::constructSpawnProfile;
}

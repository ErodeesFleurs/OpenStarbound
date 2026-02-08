#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarWeightedPool.hpp"

import std;

namespace Star {

using SpawnTypeDatabaseException = ExceptionDerived<"SpawnTypeDatabaseException">;

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
  SpawnParameters(Json const& config = {});
  SpawnParameters(Set<Area> areas, Region region, Time time);

  [[nodiscard]] auto compatible(SpawnParameters const& parameters) const -> bool;

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

auto spawnTypeFromJson(Json const& config) -> SpawnType;

struct SpawnProfile {
  SpawnProfile();
  SpawnProfile(Json const& config);
  SpawnProfile(StringSet spawnTypes, Json monsterParameters);

  [[nodiscard]] auto toJson() const -> Json;

  StringSet spawnTypes;
  Json monsterParameters;
};

auto constructSpawnProfile(Json const& config, std::uint64_t seed) -> SpawnProfile;

class SpawnTypeDatabase {
public:
  SpawnTypeDatabase();

  [[nodiscard]] auto spawnType(String const& typeName) const -> SpawnType;

private:
  StringMap<SpawnType> m_spawnTypes;
};

}// namespace Star

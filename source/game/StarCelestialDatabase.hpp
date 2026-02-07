#pragma once

#include "StarBTreeDatabase.hpp"
#include "StarCelestialTypes.hpp"
#include "StarPerlin.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarTtlCache.hpp"
#include "StarWeightedPool.hpp"

import std;

namespace Star {

// STAR_CLASS(CelestialDatabase);
// STAR_CLASS(CelestialMasterDatabase);
// STAR_CLASS(CelestialSlaveDatabase);

class CelestialDatabase {
public:
  virtual ~CelestialDatabase();

  // The x/y region of usable worlds.
  [[nodiscard]] auto xyRange() const -> RectI;

  // The maximum number of bodies that can orbit a single system center /
  // planetary body.  Orbital numbers are up to this number of levels
  // *inclusive*, so planetary orbit numbers would be 1-N, and planetary orbit
  // "0", in this system, would refer to the center of the planetary system
  // itself, e.g. a star.  In the same way, satellites around a planetary
  // object are numbered 1-N, and 0 refers to the planetary object itself.
  [[nodiscard]] auto planetOrbitalLevels() const -> int;
  [[nodiscard]] auto satelliteOrbitalLevels() const -> int;

  // The following methods are allowed to return no information even in the
  // case of valid coordinates, due to delayed loading.

  virtual auto parameters(CelestialCoordinate const& coordinate) -> std::optional<CelestialParameters> = 0;
  virtual auto name(CelestialCoordinate const& coordinate) -> std::optional<String> = 0;

  virtual auto hasChildren(CelestialCoordinate const& coordinate) -> std::optional<bool> = 0;
  virtual auto children(CelestialCoordinate const& coordinate) -> List<CelestialCoordinate> = 0;
  virtual auto childOrbits(CelestialCoordinate const& coordinate) -> List<int> = 0;

  // Return all valid system coordinates in the given x/y range.  All systems
  // are guaranteed to have unique x/y coordinates, and are meant to be viewed
  // from the top in 2d.  The z-coordinate is there simpy as a validation
  // parameter.
  virtual auto scanSystems(RectI const& region, std::optional<StringSet> const& includedTypes = {}) -> List<CelestialCoordinate> = 0;
  virtual auto scanConstellationLines(RectI const& region) -> List<std::pair<Vec2I, Vec2I>> = 0;

  // Returns false if part or all of the specified region is not loaded.  This
  // is only relevant for calls to scanSystems and scanConstellationLines, and
  // does not imply that each individual system in the given region is fully
  // loaded with all planets moons etc, only that scanSystem and
  // scanConstellationLines are not waiting on missing data.
  virtual auto scanRegionFullyLoaded(RectI const& region) -> bool = 0;

protected:
  [[nodiscard]] auto chunkIndexFor(CelestialCoordinate const& coordinate) const -> Vec2I;
  [[nodiscard]] auto chunkIndexFor(Vec2I const& systemXY) const -> Vec2I;

  // Returns the chunk indexes for the given region.
  [[nodiscard]] auto chunkIndexesFor(RectI const& region) const -> List<Vec2I>;

  // Returns the region of the given chunk.
  [[nodiscard]] auto chunkRegion(Vec2I const& chunkIndex) const -> RectI;

  // m_baseInformation should only be modified in the constructor, as it is not
  // thread protected.
  CelestialBaseInformation m_baseInformation;
};

class CelestialMasterDatabase : public CelestialDatabase {
public:
  CelestialMasterDatabase(std::optional<String> databaseFile = {});

  auto baseInformation() const -> CelestialBaseInformation;
  auto respondToRequest(CelestialRequest const& requests) -> CelestialResponse;

  // Unload data that has not been used in the configured TTL time, and
  // periodically commit to the underlying database if it is in use.
  void cleanupAndCommit();

  // Does this coordinate point to a valid existing object?
  auto coordinateValid(CelestialCoordinate const& coordinate) -> bool;

  // Find a planetary or satellite object randomly throughout the entire
  // celestial space that satisfies the given parameters.  May fail to find
  // anything, though with the defaults this is vanishingly unlikely.
  auto findRandomWorld(unsigned tries = 10, unsigned trySpatialRange = 50,
                       std::function<bool(CelestialCoordinate)> filter = {}, std::optional<uint64_t> seed = {}) -> std::optional<CelestialCoordinate>;

  // CelestialMasterDatabase always returns actual data, as it does just in
  // time generation.

  auto parameters(CelestialCoordinate const& coordinate) -> std::optional<CelestialParameters> override;
  auto name(CelestialCoordinate const& coordinate) -> std::optional<String> override;

  auto hasChildren(CelestialCoordinate const& coordinate) -> std::optional<bool> override;
  auto children(CelestialCoordinate const& coordinate) -> List<CelestialCoordinate> override;
  auto childOrbits(CelestialCoordinate const& coordinate) -> List<int> override;

  auto scanSystems(RectI const& region, std::optional<StringSet> const& includedTypes = {}) -> List<CelestialCoordinate> override;
  auto scanConstellationLines(RectI const& region) -> List<std::pair<Vec2I, Vec2I>> override;

  auto scanRegionFullyLoaded(RectI const& region) -> bool override;

  // overwrite the celestial parameters for the world at the given celestial coordinate
  void updateParameters(CelestialCoordinate const& coordinate, CelestialParameters const& parameters);

protected:
  struct SatelliteType {
    String typeName;
    Json baseParameters;
    JsonArray variationParameters;
    JsonObject orbitParameters;
  };

  struct PlanetaryType {
    String typeName;
    float satelliteProbability;
    size_t maxSatelliteCount;
    Json baseParameters;
    JsonArray variationParameters;
    JsonObject orbitParameters;
  };

  struct SystemType {
    String typeName;
    bool constellationCapable;
    Json baseParameters;
    JsonArray variationParameters;
    List<CelestialOrbitRegion> orbitRegions;
  };

  struct GenerationInformation {
    float systemProbability;
    float constellationProbability;
    Vec2U constellationLineCountRange;
    unsigned constellationMaxTries;
    float maximumConstellationLineLength;
    float minimumConstellationLineLength;
    float minimumConstellationMagnitude;
    float minimumConstellationLineCloseness;

    Map<String, SystemType> systemTypes;

    PerlinD systemTypePerlin;
    Json systemTypeBins;

    StringMap<PlanetaryType> planetaryTypes;
    StringMap<SatelliteType> satelliteTypes;

    StringList planetarySuffixes;
    StringList satelliteSuffixes;

    WeightedPool<String> systemPrefixNames;
    WeightedPool<String> systemNames;
    WeightedPool<String> systemSuffixNames;
  };

  static auto orbitRegion(
    List<CelestialOrbitRegion> const& orbitRegions, int planetaryOrbitNumber) -> std::optional<CelestialOrbitRegion>;

  using UnlockDuringFunction = std::function<void(std::function<void()>&&)>&&;
  auto getChunk(Vec2I const& chunkLocation, UnlockDuringFunction unlockDuring = {}) -> CelestialChunk const&;

  auto produceChunk(Vec2I const& chunkLocation) const -> CelestialChunk;
  auto produceSystem(
    RandomSource& random, Vec3I const& location) const -> std::optional<std::pair<CelestialParameters, HashMap<int, CelestialPlanet>>>;
  auto produceConstellations(
    RandomSource& random, List<Vec2I> const& constellationCandidates) const -> List<CelestialConstellation>;

  GenerationInformation m_generationInformation;

  mutable RecursiveMutex m_mutex;

  HashTtlCache<Vec2I, CelestialChunk> m_chunkCache;
  BTreeSha256Database m_database;

  float m_commitInterval;
  Timer m_commitTimer;
};

class CelestialSlaveDatabase : public CelestialDatabase {
public:
  CelestialSlaveDatabase(CelestialBaseInformation baseInformation);

  // Signal that the given region should be requested from the master database.
  void signalRegion(RectI const& region);

  // Signal that the given system should be fully requested from the master
  // database.
  void signalSystem(CelestialCoordinate const& system);

  // There is an internal activity time for chunk requests to live to prevent
  // repeatedly requesting the same set of chunks.
  auto pullRequests() -> List<CelestialRequest>;
  void pushResponses(List<CelestialResponse> responses);

  // Unload data that has not been used in the configured TTL time.
  void cleanup();

  auto parameters(CelestialCoordinate const& coordinate) -> std::optional<CelestialParameters> override;
  auto name(CelestialCoordinate const& coordinate) -> std::optional<String> override;

  auto hasChildren(CelestialCoordinate const& coordinate) -> std::optional<bool> override;
  auto children(CelestialCoordinate const& coordinate) -> List<CelestialCoordinate> override;
  auto childOrbits(CelestialCoordinate const& coordinate) -> List<int> override;

  auto scanSystems(RectI const& region, std::optional<StringSet> const& includedTypes = {}) -> List<CelestialCoordinate> override;
  auto scanConstellationLines(RectI const& region) -> List<std::pair<Vec2I, Vec2I>> override;

  auto scanRegionFullyLoaded(RectI const& region) -> bool override;

  void invalidateCacheFor(CelestialCoordinate const& coordinate);

private:
  float m_requestTimeout;

  mutable RecursiveMutex m_mutex;
  HashTtlCache<Vec2I, CelestialChunk> m_chunkCache;
  HashMap<Vec2I, Timer> m_pendingChunkRequests;
  HashMap<Vec3I, Timer> m_pendingSystemRequests;
};

}// namespace Star

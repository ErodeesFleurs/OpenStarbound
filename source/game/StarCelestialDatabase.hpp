#pragma once

#include "StarAssets.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarBiomeDatabase.hpp"
#include "StarCelestialTypes.hpp"
#include "StarLiquidsDatabase.hpp"
#include "StarPerlin.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarTtlCache.hpp"
#include "StarWeightedPool.hpp"

namespace Star {

class CelestialDatabase;
using CelestialDatabasePtr = SharedPtr<CelestialDatabase>;
class CelestialMasterDatabase;
using CelestialMasterDatabasePtr = SharedPtr<CelestialMasterDatabase>;
class CelestialSlaveDatabase;
using CelestialSlaveDatabasePtr = SharedPtr<CelestialSlaveDatabase>;
class VersioningDatabase;
using VersioningDatabaseConstPtr = SharedPtr<VersioningDatabase const>;

class CelestialDatabase {
public:
  virtual ~CelestialDatabase();

  // The x/y region of usable worlds.
  [[nodiscard]] RectI xyRange() const;

  // The maximum number of bodies that can orbit a single system center /
  // planetary body.  Orbital numbers are up to this number of levels
  // *inclusive*, so planetary orbit numbers would be 1-N, and planetary orbit
  // "0", in this system, would refer to the center of the planetary system
  // itself, e.g. a star.  In the same way, satellites around a planetary
  // object are numbered 1-N, and 0 refers to the planetary object itself.
  [[nodiscard]] int planetOrbitalLevels() const;
  [[nodiscard]] int satelliteOrbitalLevels() const;

  // The following methods are allowed to return no information even in the
  // case of valid coordinates, due to delayed loading.

  [[nodiscard]] virtual Maybe<CelestialParameters> parameters(CelestialCoordinate const& coordinate) = 0;
  [[nodiscard]] virtual Maybe<String> name(CelestialCoordinate const& coordinate) = 0;

  [[nodiscard]] virtual Maybe<bool> hasChildren(CelestialCoordinate const& coordinate) = 0;
  [[nodiscard]] virtual List<CelestialCoordinate> children(CelestialCoordinate const& coordinate) = 0;
  [[nodiscard]] virtual List<int> childOrbits(CelestialCoordinate const& coordinate) = 0;

  // Return all valid system coordinates in the given x/y range.  All systems
  // are guaranteed to have unique x/y coordinates, and are meant to be viewed
  // from the top in 2d.  The z-coordinate is there simpy as a validation
  // parameter.
  [[nodiscard]] virtual List<CelestialCoordinate> scanSystems(RectI const& region, Maybe<StringSet> const& includedTypes = {}) = 0;
  [[nodiscard]] virtual List<pair<Vec2I, Vec2I>> scanConstellationLines(RectI const& region) = 0;

  // Returns false if part or all of the specified region is not loaded.  This
  // is only relevant for calls to scanSystems and scanConstellationLines, and
  // does not imply that each individual system in the given region is fully
  // loaded with all planets moons etc, only that scanSystem and
  // scanConstellationLines are not waiting on missing data.
  [[nodiscard]] virtual bool scanRegionFullyLoaded(RectI const& region) = 0;

protected:
  [[nodiscard]] Vec2I chunkIndexFor(CelestialCoordinate const& coordinate) const;
  [[nodiscard]] Vec2I chunkIndexFor(Vec2I const& systemXY) const;

  // Returns the chunk indexes for the given region.
  [[nodiscard]] List<Vec2I> chunkIndexesFor(RectI const& region) const;

  // Returns the region of the given chunk.
  [[nodiscard]] RectI chunkRegion(Vec2I const& chunkIndex) const;

  // m_baseInformation should only be modified in the constructor, as it is not
  // thread protected.
  CelestialBaseInformation m_baseInformation;
};

class CelestialMasterDatabase : public CelestialDatabase {
public:
  CelestialMasterDatabase(AssetsConstPtr assets, LiquidsDatabaseConstPtr liquidsDatabase, BiomeDatabaseConstPtr biomeDatabase, Maybe<VersioningDatabaseConstPtr> versioningDatabase = {}, Maybe<String> databaseFile = {});

  [[nodiscard]] CelestialBaseInformation baseInformation() const;
  [[nodiscard]] CelestialResponse respondToRequest(CelestialRequest const& requests);

  // Unload data that has not been used in the configured TTL time, and
  // periodically commit to the underlying database if it is in use.
  void cleanupAndCommit();

  // Does this coordinate point to a valid existing object?
  [[nodiscard]] bool coordinateValid(CelestialCoordinate const& coordinate);

  // Find a planetary or satellite object randomly throughout the entire
  // celestial space that satisfies the given parameters.  May fail to find
  // anything, though with the defaults this is vanishingly unlikely.
  [[nodiscard]] Maybe<CelestialCoordinate> findRandomWorld(unsigned tries = 10, unsigned trySpatialRange = 50,
                                             function<bool(CelestialCoordinate)> filter = {}, Maybe<uint64_t> seed = {});

  // CelestialMasterDatabase always returns actual data, as it does just in
  // time generation.

  [[nodiscard]] Maybe<CelestialParameters> parameters(CelestialCoordinate const& coordinate) override;
  [[nodiscard]] Maybe<String> name(CelestialCoordinate const& coordinate) override;

  [[nodiscard]] Maybe<bool> hasChildren(CelestialCoordinate const& coordinate) override;
  [[nodiscard]] List<CelestialCoordinate> children(CelestialCoordinate const& coordinate) override;
  [[nodiscard]] List<int> childOrbits(CelestialCoordinate const& coordinate) override;

  [[nodiscard]] List<CelestialCoordinate> scanSystems(RectI const& region, Maybe<StringSet> const& includedTypes = {}) override;
  [[nodiscard]] List<pair<Vec2I, Vec2I>> scanConstellationLines(RectI const& region) override;

  [[nodiscard]] bool scanRegionFullyLoaded(RectI const& region) override;

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

  static Maybe<CelestialOrbitRegion> orbitRegion(
    List<CelestialOrbitRegion> const& orbitRegions, int planetaryOrbitNumber);

  using UnlockDuringFunction = std::function<void(std::function<void()>&&)>&&;
  [[nodiscard]] CelestialChunk const& getChunk(Vec2I const& chunkLocation, UnlockDuringFunction unlockDuring = {});

  [[nodiscard]] CelestialChunk produceChunk(Vec2I const& chunkLocation) const;
  Maybe<pair<CelestialParameters, HashMap<int, CelestialPlanet>>> produceSystem(
    RandomSource& random, Vec3I const& location) const;
  List<CelestialConstellation> produceConstellations(
    RandomSource& random, List<Vec2I> const& constellationCandidates) const;

  GenerationInformation m_generationInformation;
  AssetsConstPtr m_assets;
  LiquidsDatabaseConstPtr m_liquidsDatabase;
  BiomeDatabaseConstPtr m_biomeDatabase;
  VersioningDatabaseConstPtr m_versioningDatabase;

  mutable RecursiveMutex m_mutex;

  HashTtlCache<Vec2I, CelestialChunk> m_chunkCache;
  BTreeSha256Database m_database;

  float m_commitInterval;
  Timer m_commitTimer;
};

class CelestialSlaveDatabase : public CelestialDatabase {
public:
  CelestialSlaveDatabase(AssetsConstPtr assets, CelestialBaseInformation baseInformation);

  // Signal that the given region should be requested from the master database.
  void signalRegion(RectI const& region);

  // Signal that the given system should be fully requested from the master
  // database.
  void signalSystem(CelestialCoordinate const& system);

  // There is an internal activity time for chunk requests to live to prevent
  // repeatedly requesting the same set of chunks.
  [[nodiscard]] List<CelestialRequest> pullRequests();
  void pushResponses(List<CelestialResponse> responses);

  // Unload data that has not been used in the configured TTL time.
  void cleanup();

  [[nodiscard]] Maybe<CelestialParameters> parameters(CelestialCoordinate const& coordinate) override;
  [[nodiscard]] Maybe<String> name(CelestialCoordinate const& coordinate) override;

  [[nodiscard]] Maybe<bool> hasChildren(CelestialCoordinate const& coordinate) override;
  [[nodiscard]] List<CelestialCoordinate> children(CelestialCoordinate const& coordinate) override;
  [[nodiscard]] List<int> childOrbits(CelestialCoordinate const& coordinate) override;

  [[nodiscard]] List<CelestialCoordinate> scanSystems(RectI const& region, Maybe<StringSet> const& includedTypes = {}) override;
  [[nodiscard]] List<pair<Vec2I, Vec2I>> scanConstellationLines(RectI const& region) override;

  [[nodiscard]] bool scanRegionFullyLoaded(RectI const& region) override;

  void invalidateCacheFor(CelestialCoordinate const& coordinate);

private:
  float m_requestTimeout;

  mutable RecursiveMutex m_mutex;
  HashTtlCache<Vec2I, CelestialChunk> m_chunkCache;
  HashMap<Vec2I, Timer> m_pendingChunkRequests;
  HashMap<Vec3I, Timer> m_pendingSystemRequests;
};

}// namespace Star

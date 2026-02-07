#pragma once

#include "StarCelestialParameters.hpp"
#include "StarEither.hpp"

import std;

namespace Star {

using CelestialConstellation = List<std::pair<Vec2I, Vec2I>>;

struct CelestialOrbitRegion {
  String regionName;
  Vec2I orbitRange;
  float bodyProbability;
  WeightedPool<String> planetaryTypes;
  WeightedPool<String> satelliteTypes;
};

struct CelestialPlanet {
  CelestialParameters planetParameters;
  HashMap<int, CelestialParameters> satelliteParameters;
};
auto operator>>(DataStream& ds, CelestialPlanet& planet) -> DataStream&;
auto operator<<(DataStream& ds, CelestialPlanet const& planet) -> DataStream&;

struct CelestialSystemObjects {
  Vec3I systemLocation;
  HashMap<int, CelestialPlanet> planets;
};
auto operator>>(DataStream& ds, CelestialSystemObjects& systemObjects) -> DataStream&;
auto operator<<(DataStream& ds, CelestialSystemObjects const& systemObjects) -> DataStream&;

struct CelestialChunk {
  CelestialChunk();
  CelestialChunk(Json const& store);

  [[nodiscard]] auto toJson() const -> Json;

  Vec2I chunkIndex;
  List<CelestialConstellation> constellations;
  HashMap<Vec3I, CelestialParameters> systemParameters;

  // System objects are kept separate from systemParameters here so that there
  // can be two phases of loading, one for basic system-level parameters for an
  // entire chunk the other for each set of sub objects for each system.
  HashMap<Vec3I, HashMap<int, CelestialPlanet>> systemObjects;
};
auto operator>>(DataStream& ds, CelestialChunk& chunk) -> DataStream&;
auto operator<<(DataStream& ds, CelestialChunk const& chunk) -> DataStream&;

using CelestialRequest = Either<Vec2I, Vec3I>;
using CelestialResponse = Either<CelestialChunk, CelestialSystemObjects>;

struct CelestialBaseInformation {
  int planetOrbitalLevels;
  int satelliteOrbitalLevels;
  int chunkSize;
  Vec2I xyCoordRange;
  Vec2I zCoordRange;
  bool enforceCoordRange;
};
auto operator>>(DataStream& ds, CelestialBaseInformation& celestialInformation) -> DataStream&;
auto operator<<(DataStream& ds, CelestialBaseInformation const& celestialInformation) -> DataStream&;
}// namespace Star

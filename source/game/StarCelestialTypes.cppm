module;

#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarEither.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
import star.celestial_coordinate;
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarJson.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
import star.sky_types;
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;

namespace Star {

STAR_STRUCT(CelestialSystemObjects);
STAR_STRUCT(CelestialChunk);
STAR_STRUCT(CelestialBaseInformation);

typedef List<pair<Vec2I, Vec2I>> CelestialConstellation;

struct CelestialOrbitRegion {
  String regionName;
  Vec2I orbitRange;
  float bodyProbability;
  WeightedPool<String> planetaryTypes;
  WeightedPool<String> satelliteTypes;
};

struct CelestialPlanet {
  friend DataStream& operator>>(DataStream& ds, CelestialPlanet& value);
  friend DataStream& operator<<(DataStream& ds, CelestialPlanet const& value);
  CelestialParameters planetParameters;
  HashMap<int, CelestialParameters> satelliteParameters;
};
DataStream& operator>>(DataStream& ds, CelestialPlanet& planet);
DataStream& operator<<(DataStream& ds, CelestialPlanet const& planet);

struct CelestialSystemObjects {
  friend DataStream& operator>>(DataStream& ds, CelestialSystemObjects& value);
  friend DataStream& operator<<(DataStream& ds, CelestialSystemObjects const& value);
  Vec3I systemLocation;
  HashMap<int, CelestialPlanet> planets;
};
DataStream& operator>>(DataStream& ds, CelestialSystemObjects& systemObjects);
DataStream& operator<<(DataStream& ds, CelestialSystemObjects const& systemObjects);

struct CelestialChunk {
  friend DataStream& operator>>(DataStream& ds, CelestialChunk& value);
  friend DataStream& operator<<(DataStream& ds, CelestialChunk const& value);
  CelestialChunk();
  CelestialChunk(Json const& store);

  Json toJson() const;

  Vec2I chunkIndex;
  List<CelestialConstellation> constellations;
  HashMap<Vec3I, CelestialParameters> systemParameters;

  // System objects are kept separate from systemParameters here so that there
  // can be two phases of loading, one for basic system-level parameters for an
  // entire chunk the other for each set of sub objects for each system.
  HashMap<Vec3I, HashMap<int, CelestialPlanet>> systemObjects;
  
  // only persistent chunks are saved to storage
  bool persistent = false;
};
DataStream& operator>>(DataStream& ds, CelestialChunk& chunk);
DataStream& operator<<(DataStream& ds, CelestialChunk const& chunk);

typedef Either<Vec2I, Vec3I> CelestialRequest;
typedef Either<CelestialChunk, CelestialSystemObjects> CelestialResponse;

struct CelestialBaseInformation {
  friend DataStream& operator>>(DataStream& ds, CelestialBaseInformation& value);
  friend DataStream& operator<<(DataStream& ds, CelestialBaseInformation const& value);
  int planetOrbitalLevels;
  int satelliteOrbitalLevels;
  int chunkSize;
  Vec2I xyCoordRange;
  Vec2I zCoordRange;
  bool enforceCoordRange;
};
DataStream& operator>>(DataStream& ds, CelestialBaseInformation& celestialInformation);
DataStream& operator<<(DataStream& ds, CelestialBaseInformation const& celestialInformation);
}

export module star.celestial_types;

export namespace Star {
  using ::Star::CelestialSystemObjects;
  using ::Star::CelestialChunk;
  using ::Star::CelestialBaseInformation;
  using ::Star::CelestialConstellation;
  using ::Star::CelestialOrbitRegion;
  using ::Star::CelestialPlanet;
  using ::Star::CelestialRequest;
  using ::Star::CelestialResponse;
  using ::Star::CelestialSystemObjectsPtr;
  using ::Star::CelestialSystemObjectsConstPtr;
  using ::Star::CelestialSystemObjectsWeakPtr;
  using ::Star::CelestialSystemObjectsConstWeakPtr;
  using ::Star::CelestialSystemObjectsUPtr;
  using ::Star::CelestialSystemObjectsConstUPtr;
  using ::Star::CelestialChunkPtr;
  using ::Star::CelestialChunkConstPtr;
  using ::Star::CelestialChunkWeakPtr;
  using ::Star::CelestialChunkConstWeakPtr;
  using ::Star::CelestialChunkUPtr;
  using ::Star::CelestialChunkConstUPtr;
  using ::Star::CelestialBaseInformationPtr;
  using ::Star::CelestialBaseInformationConstPtr;
  using ::Star::CelestialBaseInformationWeakPtr;
  using ::Star::CelestialBaseInformationConstWeakPtr;
  using ::Star::CelestialBaseInformationUPtr;
  using ::Star::CelestialBaseInformationConstUPtr;
}

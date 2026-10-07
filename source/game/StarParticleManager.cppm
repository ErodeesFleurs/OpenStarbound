module;
#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
#include "StarMultiArray.hpp"
#include <functional>
#include "StarGameTypes.hpp"
#include "StarMathCommon.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
#include "StarRect.hpp"
#include "StarSet.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarThread.hpp"
#include "StarInterpolation.hpp"
#include "StarRandom.hpp"
import star.perlin;
#include "StarMaybe.hpp"
import star.weighted_pool;
#include "StarVariant.hpp"
#include "StarConfig.hpp"
import star.version;


import star.world_geometry;
import star.animation;
import star.particle;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.weather_types;
import star.celestial_coordinate;
import star.sky_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;

namespace Star {

STAR_CLASS(ParticleManager);

class ParticleManager {
public:
  ParticleManager(WorldGeometry const& worldGeometry, ClientTileSectorArrayPtr const& tileSectorArray);

  void add(Particle particle);
  void addParticles(List<Particle> particles);

  size_t count() const;
  void clear();

  void setUndergroundLevel(float undergroundLevel);

  // Updates current particles and spawns new weather particles
  void update(float dt, RectF const& cullRegion, float wind);

  List<Particle> const& particles() const;
  List<pair<Vec2F, Vec3F>> lightSources() const;

private:
  enum class TileType { Colliding, Water, Empty };

  List<Particle> m_particles;
  List<Particle> m_nextParticles;

  WorldGeometry m_worldGeometry;
  float m_undergroundLevel;
  ClientTileSectorArrayPtr m_tileSectorArray;
};

}

export module star.particle_manager;

export namespace Star {
using ::Star::ParticleManager;
using ::Star::ParticleManagerPtr;
using ::Star::ParticleManagerConstPtr;
using ::Star::ParticleManagerWeakPtr;
using ::Star::ParticleManagerConstWeakPtr;
using ::Star::ParticleManagerUPtr;
using ::Star::ParticleManagerConstUPtr;
}

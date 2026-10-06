module;

#include "StarPoly.hpp"
import star.world_geometry;
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
import star.animation;
import star.particle;
#include "StarPoly.hpp"
#include "StarList.hpp"
#include "StarBiMap.hpp"
import star.collision_block;
#include "StarMultiArray.hpp"
#include <functional>
#include "StarGameTypes.hpp"
#include "StarXXHash.hpp"
#include "StarMathCommon.hpp"
import star.liquid_types;
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarNetElementSystem.hpp"
import star.tile_damage;
#include "StarTileSectorArray.hpp"
#include "StarWorldLayout.hpp"
#include "StarVersion.hpp"
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

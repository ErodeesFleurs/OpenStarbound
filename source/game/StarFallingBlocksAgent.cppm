module;
#include "StarXXHash.hpp"
#include "StarJson.hpp"
#include "StarVector.hpp"
#include "StarSet.hpp"
#include "StarMap.hpp"
#include "StarRandom.hpp"
#include "StarGameTypes.hpp"
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
#include "StarThread.hpp"
import star.worker_pool;

#include "thread"
import star.sector_array_2d;
#include "StarThread.hpp"
#include "StarInterpolation.hpp"
import star.perlin;
#include "StarMaybe.hpp"
import star.weighted_pool;
#include "StarColor.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarVariant.hpp"
#include "StarConfig.hpp"
import star.version;


import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.animation;
import star.particle;
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

STAR_CLASS(MaterialDatabase);
STAR_CLASS(FallingBlocksFacade);
STAR_CLASS(FallingBlocksAgent);

enum class FallingBlockType {
  Immovable,
  Falling,
  Cascading,
  Open
};

class FallingBlocksFacade {
public:
  virtual ~FallingBlocksFacade() = default;

  virtual FallingBlockType blockType(Vec2I const& pos) = 0;
  virtual void moveBlock(Vec2I const& from, Vec2I const& to) = 0;
};

class FallingBlocksAgent {
public:
  FallingBlocksAgent(FallingBlocksFacadePtr worldFacade);

  void update();

  void visitLocation(Vec2I const& location);
  void visitRegion(RectI const& region);

private:
  FallingBlocksFacadePtr m_facade;
  float m_immediateUpwardPropagateProbability;
  HashSet<Vec2I> m_pending;
  RandomSource m_random;
};

}

export module star.falling_blocks_agent;

export namespace Star {
  using ::Star::FallingBlockType;
  using ::Star::MaterialDatabase;
  using ::Star::MaterialDatabasePtr;
  using ::Star::MaterialDatabaseConstPtr;
  using ::Star::MaterialDatabaseWeakPtr;
  using ::Star::MaterialDatabaseConstWeakPtr;
  using ::Star::MaterialDatabaseUPtr;
  using ::Star::MaterialDatabaseConstUPtr;
  using ::Star::FallingBlocksFacade;
  using ::Star::FallingBlocksFacadePtr;
  using ::Star::FallingBlocksFacadeConstPtr;
  using ::Star::FallingBlocksFacadeWeakPtr;
  using ::Star::FallingBlocksFacadeConstWeakPtr;
  using ::Star::FallingBlocksFacadeUPtr;
  using ::Star::FallingBlocksFacadeConstUPtr;
  using ::Star::FallingBlocksAgent;
  using ::Star::FallingBlocksAgentPtr;
  using ::Star::FallingBlocksAgentConstPtr;
  using ::Star::FallingBlocksAgentWeakPtr;
  using ::Star::FallingBlocksAgentConstWeakPtr;
  using ::Star::FallingBlocksAgentUPtr;
  using ::Star::FallingBlocksAgentConstUPtr;
}

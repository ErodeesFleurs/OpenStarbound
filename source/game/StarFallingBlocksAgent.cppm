module;
import star.xx_hash;
import star.json;
import star.vector;
import star.set;
import star.map;
import star.random;
import star.game_types;
import star.list;
import star.bi_map;
import std;
import star.rect;
import star.thread;
import star.worker_pool;

import star.sector_array_2d;
import star.perlin;
import star.maybe;
import star.weighted_pool;
import star.color;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.variant;
import star.version;


import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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

class MaterialDatabase;
using MaterialDatabasePtr = std::shared_ptr<MaterialDatabase>;
using MaterialDatabaseConstPtr = std::shared_ptr<const MaterialDatabase>;
using MaterialDatabaseWeakPtr = std::weak_ptr<MaterialDatabase>;
using MaterialDatabaseConstWeakPtr = std::weak_ptr<const MaterialDatabase>;
using MaterialDatabaseUPtr = std::unique_ptr<MaterialDatabase>;
using MaterialDatabaseConstUPtr = std::unique_ptr<const MaterialDatabase>;
class FallingBlocksFacade;
using FallingBlocksFacadePtr = std::shared_ptr<FallingBlocksFacade>;
using FallingBlocksFacadeConstPtr = std::shared_ptr<const FallingBlocksFacade>;
using FallingBlocksFacadeWeakPtr = std::weak_ptr<FallingBlocksFacade>;
using FallingBlocksFacadeConstWeakPtr = std::weak_ptr<const FallingBlocksFacade>;
using FallingBlocksFacadeUPtr = std::unique_ptr<FallingBlocksFacade>;
using FallingBlocksFacadeConstUPtr = std::unique_ptr<const FallingBlocksFacade>;
class FallingBlocksAgent;
using FallingBlocksAgentPtr = std::shared_ptr<FallingBlocksAgent>;
using FallingBlocksAgentConstPtr = std::shared_ptr<const FallingBlocksAgent>;
using FallingBlocksAgentWeakPtr = std::weak_ptr<FallingBlocksAgent>;
using FallingBlocksAgentConstWeakPtr = std::weak_ptr<const FallingBlocksAgent>;
using FallingBlocksAgentUPtr = std::unique_ptr<FallingBlocksAgent>;
using FallingBlocksAgentConstUPtr = std::unique_ptr<const FallingBlocksAgent>;

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

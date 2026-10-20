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
class Root;

class MaterialDatabase;

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
  FallingBlocksAgent(Root& root, std::shared_ptr<FallingBlocksFacade> worldFacade);

  void update();

  void visitLocation(Vec2I const& location);
  void visitRegion(RectI const& region);

private:
  std::shared_ptr<FallingBlocksFacade> m_facade;
  float m_immediateUpwardPropagateProbability;
  HashSet<Vec2I> m_pending;
  RandomSource m_random;
};

}

export module star.falling_blocks_agent;

export namespace Star {
  using ::Star::FallingBlockType;
  using ::Star::MaterialDatabase;
  using ::Star::FallingBlocksFacade;
  using ::Star::FallingBlocksAgent;
}

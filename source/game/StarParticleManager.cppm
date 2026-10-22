module;
import star.format;
import star.json;
import star.xx_hash;
import star.color;
import star.bi_map;
import star.list;
import star.rect;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import std;
import star.game_types;
import star.vector;
import star.set;
import star.worker_pool;

import star.sector_array_2d;
import star.random;
import star.perlin;
import star.weighted_pool;
import star.version;


import star.world_geometry;
import star.animation;
import star.particle;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
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

import star.algorithm;
namespace Star {

class Root;

class ParticleManager {
public:
  ParticleManager(Root& root, WorldGeometry const& worldGeometry, std::shared_ptr<ClientTileSectorArray> const& tileSectorArray);

  void add(Particle particle);
  void addParticles(List<Particle> particles);

  std::size_t count() const;
  void clear();

  void setUndergroundLevel(float undergroundLevel);

  // Updates current particles and spawns new weather particles
  [[nodiscard]] std::expected<void, StarException> update(float dt, RectF const& cullRegion, float wind);

  List<Particle> const& particles() const;
  List<std::pair<Vec2F, Vec3F>> lightSources() const;

private:
  enum class TileType { Colliding, Water, Empty };

  Root& m_root;

  List<Particle> m_particles;
  List<Particle> m_nextParticles;

  WorldGeometry m_worldGeometry;
  float m_undergroundLevel;
  std::shared_ptr<ClientTileSectorArray> m_tileSectorArray;
};

}

export module star.particle_manager;

export namespace Star {
using ::Star::ParticleManager;
}

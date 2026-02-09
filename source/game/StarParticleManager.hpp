#pragma once

#include "StarConfig.hpp"
#include "StarParticle.hpp"
#include "StarWorldGeometry.hpp"
#include "StarWorldTiles.hpp"

import std;

namespace Star {

class ParticleManager {
public:
  ParticleManager(WorldGeometry const& worldGeometry, Ptr<ClientTileSectorArray> const& tileSectorArray);

  void add(Particle particle);
  void addParticles(List<Particle> particles);

  [[nodiscard]] auto count() const -> size_t;
  void clear();

  void setUndergroundLevel(float undergroundLevel);

  // Updates current particles and spawns new weather particles
  void update(float dt, RectF const& cullRegion, float wind);

  [[nodiscard]] auto particles() const -> List<Particle> const&;
  [[nodiscard]] auto lightSources() const -> List<std::pair<Vec2F, Vec3F>>;

private:
  enum class TileType { Colliding,
                        Water,
                        Empty };

  List<Particle> m_particles;
  List<Particle> m_nextParticles;

  WorldGeometry m_worldGeometry;
  float m_undergroundLevel;
  Ptr<ClientTileSectorArray> m_tileSectorArray;
};

}// namespace Star

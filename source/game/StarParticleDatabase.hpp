#pragma once

#include "StarConfig.hpp"
#include "StarJson.hpp"
#include "StarParticle.hpp"

namespace Star {

class ParticleConfig {
public:
  ParticleConfig(Json const& config);

  auto kind() -> String const&;
  auto instance() -> Particle;

private:
  String m_kind;
  Particle m_particle;
  Particle m_variance;
};

class ParticleDatabase {
public:
  ParticleDatabase();

  [[nodiscard]] auto config(String const& kind) const -> Ptr<ParticleConfig>;

  // If the given variant is a string, loads the particle of that kind,
  // otherwise loads the given config directly.  If the config is given
  // directly it is assumed to optionally contain the variance config in-line.
  [[nodiscard]] auto particleCreator(Json const& kindOrConfig, String const& relativePath = "") const -> ParticleVariantCreator;

  // Like particleCreator except just returns a single particle.  Probably not
  // what you want if you want to support particle variance.
  [[nodiscard]] auto particle(Json const& kindOrConfig, String const& relativePath = "") const -> Particle;

private:
  StringMap<Ptr<ParticleConfig>> m_configs;
};

}// namespace Star

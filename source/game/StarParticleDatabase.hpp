#pragma once

#include "StarJson.hpp"
#include "StarThread.hpp"
#include "StarParticle.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"

namespace Star {

class ParticleConfig;
using ParticleConfigPtr = SharedPtr<ParticleConfig>;
class ParticleDatabase;
using ParticleDatabasePtr = SharedPtr<ParticleDatabase>;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;

class ParticleConfig {
public:
  ParticleConfig(Json const& config, AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] String const& kind() const;
  [[nodiscard]] Particle instance() const;

private:
  String m_kind;
  Particle m_particle;
  Particle m_variance;
};

class ParticleDatabase {
public:
  ParticleDatabase(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase);

  [[nodiscard]] ParticleConfigPtr config(String const& kind) const;

  // If the given variant is a string, loads the particle of that kind,
  // otherwise loads the given config directly.  If the config is given
  // directly it is assumed to optionally contain the variance config in-line.
  [[nodiscard]] ParticleVariantCreator particleCreator(Json const& kindOrConfig, String const& relativePath = "") const;

  // Like particleCreator except just returns a single particle.  Probably not
  // what you want if you want to support particle variance.
  [[nodiscard]] Particle particle(Json const& kindOrConfig, String const& relativePath = "") const;

private:
  AssetsConstPtr m_assets;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  StringMap<ParticleConfigPtr> m_configs;
};

}

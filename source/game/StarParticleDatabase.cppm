module;
import std;
import star.string;
import star.json;

import star.color;
import star.bi_map;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.thread;
import star.directives;
import star.asset_path;
import star.animation;
import star.particle;

namespace Star {

class Root;

class ParticleConfig {
public:
  ParticleConfig(Root& root, Json const& config);

  String const& kind();
  Particle instance();

private:
  Root& m_root;
  String m_kind;
  Particle m_particle;
  Particle m_variance;
};

class ParticleDatabase {
public:
  explicit ParticleDatabase(Root& root);

  std::shared_ptr<ParticleConfig> config(String const& kind) const;

  // If the given variant is a string, loads the particle of that kind,
  // otherwise loads the given config directly.  If the config is given
  // directly it is assumed to optionally contain the variance config in-line.
  ParticleVariantCreator particleCreator(Json const& kindOrConfig, String const& relativePath = "") const;

  // Like particleCreator except just returns a single particle.  Probably not
  // what you want if you want to support particle variance.
  Particle particle(Json const& kindOrConfig, String const& relativePath = "") const;

private:
  Root& m_root;
  StringMap<std::shared_ptr<ParticleConfig>> m_configs;
};

}

export module star.particle_database;

export namespace Star {
  using ::Star::Particle;
  using ::Star::ParticleVariantCreator;
  using ::Star::ParticleConfig;
  using ::Star::ParticleDatabase;
}

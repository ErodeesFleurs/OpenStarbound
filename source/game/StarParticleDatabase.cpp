import star.json;
#include "StarJsonExtra.hpp"
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
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.string;
import star.format;
import star.version;

import star.animation;
import star.particle;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.particle_database;

namespace Star {

ParticleConfig::ParticleConfig(Json const& config) {
  m_kind = config.getString("kind");
  m_particle = Particle(config.queryObject("definition"));
  m_variance = Particle(config.queryObject("definition.variance", {}));
}

String const& ParticleConfig::kind() {
  return m_kind;
}

Particle ParticleConfig::instance() {
  auto particle = m_particle;
  particle.applyVariance(m_variance);
  return particle;
}

ParticleDatabase::ParticleDatabase() {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("particle");
  assets->queueJsons(files);
  for (auto& file : files) {
    auto particleConfig = std::make_shared<ParticleConfig>(assets->json(file));
    if (m_configs.contains(particleConfig->kind()))
      throw StarException(strf("Duplicate particle asset kind Name {}. configfile {}", particleConfig->kind(), file));
    m_configs[particleConfig->kind()] = particleConfig;
  }
}

ParticleConfigPtr ParticleDatabase::config(String const& kind) const {
  auto k = kind.toLower();
  if (!m_configs.contains(k))
    throw StarException(strf("Unknown particle definition with kind {}.", kind));
  return m_configs.get(k);
}

ParticleVariantCreator ParticleDatabase::particleCreator(Json const& kindOrConfig, String const& relativePath) const {
  if (kindOrConfig.isType(Json::Type::String)) {
    auto pconfig = config(kindOrConfig.toString());
    return [pconfig]() { return pconfig->instance(); };
  } else {
    Particle particle(kindOrConfig.toObject(), relativePath);
    Particle variance(kindOrConfig.getObject("variance", {}), relativePath);
    return makeParticleVariantCreator(std::move(particle), std::move(variance));
  }
}

Particle ParticleDatabase::particle(Json const& kindOrConfig, String const& relativePath) const {
  return particleCreator(kindOrConfig, relativePath)();
}

}

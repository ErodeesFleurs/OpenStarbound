#include "StarParticleDatabase.hpp"

#include "StarConfig.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

ParticleConfig::ParticleConfig(Json const& config) {
  m_kind = config.getString("kind");
  m_particle = Particle(config.queryObject("definition"));
  m_variance = Particle(config.queryObject("definition.variance", {}));
}

auto ParticleConfig::kind() -> String const& {
  return m_kind;
}

auto ParticleConfig::instance() -> Particle {
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

auto ParticleDatabase::config(String const& kind) const -> Ptr<ParticleConfig> {
  auto k = kind.toLower();
  if (!m_configs.contains(k))
    throw StarException(strf("Unknown particle definition with kind {}.", kind));
  return m_configs.get(k);
}

auto ParticleDatabase::particleCreator(Json const& kindOrConfig, String const& relativePath) const -> ParticleVariantCreator {
  if (kindOrConfig.isType(Json::Type::String)) {
    auto pconfig = config(kindOrConfig.toString());
    return [pconfig]() { return pconfig->instance(); };
  } else {
    Particle particle(kindOrConfig.toObject(), relativePath);
    Particle variance(kindOrConfig.getObject("variance", {}), relativePath);
    return makeParticleVariantCreator(std::move(particle), std::move(variance));
  }
}

auto ParticleDatabase::particle(Json const& kindOrConfig, String const& relativePath) const -> Particle {
  return particleCreator(kindOrConfig, relativePath)();
}

}// namespace Star

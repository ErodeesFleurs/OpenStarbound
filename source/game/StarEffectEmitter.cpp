#include "StarEffectEmitter.hpp"
#include "StarJsonExtra.hpp"
#include "StarParticleDatabase.hpp"
#include "StarEntityRendering.hpp"
#include "StarDataStreamExtra.hpp"

namespace Star {

EffectEmitter::EffectEmitter() {
  addNetElement(&m_activeSources);
}

void EffectEmitter::addEffectSources(String const& position, StringSet effectSources) {
  for (auto& effectSource : effectSources)
    m_newSources.add({position, std::move(effectSource)});
}

void EffectEmitter::setSourcePosition(String name, Vec2F const& position) {
  m_positions[std::move(name)] = position;
}

void EffectEmitter::setDirection(Direction direction) {
  m_direction = direction;
}

void EffectEmitter::setBaseVelocity(Vec2F const& velocity) {
  m_baseVelocity = velocity;
}

void EffectEmitter::tick(float dt, EntityMode mode, EffectSourceDatabaseConstPtr effectSourceDatabase) {
  if (mode == EntityMode::Master) {
    m_activeSources.set(std::move(m_newSources));
    m_newSources.clear();
  } else {
    if (!m_newSources.empty())
      throw StarException("EffectEmitters can only be added to the master entity.");
  }
  if (m_renders) {
    eraseWhere(m_sources, [](EffectSourcePtr const& source) { return source->expired(); });

    for (auto& particleSource : m_sources)
      particleSource->tick(dt);

    Set<pair<String, String>> current;
    for (auto& particleSource : m_sources) {
      pair<String, String> entry = {particleSource->suggestedSpawnLocation(), particleSource->kind()};
      current.add(entry);
      if (!m_activeSources.get().contains(entry)) {
        particleSource->stop();
      }
    }
    for (auto& c : m_activeSources.get()) {
      if (!current.contains(c)) {
        auto const& [sourcePosition, sourceKind] = c;
        m_sources.append(effectSourceDatabase->effectSourceConfig(sourceKind)->instance(sourcePosition));
      }
    }
  }
}

void EffectEmitter::reset() {
  m_sources.clear();
  m_newSources.clear();
  m_activeSources.set({});
}

void EffectEmitter::render(RenderCallback* renderCallback, ParticleDatabaseConstPtr particleDatabase) {
  m_renders = true;
  if (m_sources.empty())
    return;
  for (auto& particleSource : m_sources) {
    Vec2F position = m_positions.get(particleSource->effectSpawnLocation());
    for (auto& particleName : particleSource->particles()) {
      Particle particle = particleDatabase->particle(particleName);
      if (m_direction == Direction::Left) {
        particle.flip = true;
        particle.position[0] = -particle.position[0];
        particle.velocity[0] = -particle.velocity[0];
        particle.finalVelocity[0] = -particle.finalVelocity[0];
      }
      particle.velocity += m_baseVelocity;
      particle.finalVelocity += m_baseVelocity;
      particle.position += position;
      renderCallback->addParticle(particle);
    }
    for (auto& s : particleSource->sounds(position))
      renderCallback->addAudio(s);
  }
  for (auto& particleSource : m_sources)
    particleSource->postRender();
}

Json EffectEmitter::toJson() const {
  return JsonObject{{"activeSources",
      jsonFromSet<Set<pair<String, String>>>(m_activeSources.get(),
                         [](pair<String, String> const& entry) {
                           auto const& [sourcePosition, sourceKind] = entry;
                           return JsonObject{{"position", sourcePosition}, {"source", sourceKind}};
                         })}};
}

void EffectEmitter::fromJson(Json const& diskStore) {
  m_activeSources.set(jsonToSet<Set<pair<String, String>>>(diskStore.get("activeSources"),
      [](Json const& v) {
        return pair<String, String>{v.getString("position"), v.getString("source")};
      }));
}

}

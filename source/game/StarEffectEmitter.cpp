import star.json_extra;
import star.json;
import star.game_types;
import star.ordered_map;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.thread;
import star.listener;
import std;
import star.format;
import star.version;
import star.audio;
import star.list;
import star.map;
import star.set;
import star.vector;
import star.maybe;
import star.bi_map;
import star.color;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.data_stream_extra;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;

import star.effect_emitter;
import star.effect_source_database;
import star.particle_database;

import star.algorithm;
namespace Star {

EffectEmitter::EffectEmitter(Root& root) : m_root(root) {
  m_renders = false;
  m_direction = Direction::Right;
  addNetElement(&m_activeSources);
}

void EffectEmitter::addEffectSources(String const& position, StringSet effectSources) {
  for (auto& e : effectSources)
    m_newSources.add({position, std::move(e)});
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

void EffectEmitter::tick(float dt, EntityMode mode) {
  if (mode == EntityMode::Master) {
    m_activeSources.set(std::move(m_newSources));
    m_newSources.clear();
  } else {
    if (!m_newSources.empty())
      throw StarException("EffectEmitters can only be added to the master entity.");
  }
  if (m_renders) {
    m_sources.erase(std::remove_if(m_sources.begin(), m_sources.end(),
        [](std::shared_ptr<EffectSource> const& source) { return source->expired(); }), m_sources.end());

    for (auto& ps : m_sources)
      ps->tick(dt);

    Set<std::pair<String, String>> current;
    for (auto& ps : m_sources) {
      std::pair<String, String> entry = {ps->suggestedSpawnLocation(), ps->kind()};
      current.add(entry);
      if (!m_activeSources.get().contains(entry)) {
        ps->stop();
      }
    }
    for (auto& c : m_activeSources.get()) {
      if (!current.contains(c)) {
        m_sources.append(m_root.effectSourceDatabase()->effectSourceConfig(c.second)->instance(c.first));
      }
    }
  }
}

void EffectEmitter::reset() {
  m_sources.clear();
  m_newSources.clear();
  m_activeSources.set({});
}

void EffectEmitter::render(RenderCallback* renderCallback) {
  m_renders = true;
  if (m_sources.empty())
    return;
  for (auto& ps : m_sources) {
    Vec2F position = m_positions.get(ps->effectSpawnLocation());
    for (auto& p : ps->particles()) {
      Particle particle = m_root.particleDatabase()->particle(p);
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
    for (auto& s : ps->sounds(position))
      renderCallback->addAudio(s);
  }
  for (auto& ps : m_sources)
    ps->postRender();
}

Json EffectEmitter::toJson() const {
  return JsonObject{{"activeSources",
      jsonFromSet<Set<std::pair<String, String>>>(m_activeSources.get(),
                         [](std::pair<String, String> const& entry) {
                           return JsonObject{{"position", entry.first}, {"source", entry.second}};
                         })}};
}

void EffectEmitter::fromJson(Json const& diskStore) {
  m_activeSources.set(jsonToSet<Set<std::pair<String, String>>>(diskStore.get("activeSources"),
      [](Json const& v) {
        return std::pair<String, String>{v.getString("position"), v.getString("source")};
      }));
}

}

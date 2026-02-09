#include "StarEffectSourceDatabase.hpp"

#include "StarConfig.hpp"
#include "StarJsonExtra.hpp"
#include "StarMixer.hpp"
#include "StarParticleDatabase.hpp"// IWYU pragma: export
#include "StarRandom.hpp"
#include "StarRoot.hpp"

import std;

namespace Star {

EffectSourceDatabase::EffectSourceDatabase() {
  auto assets = Root::singleton().assets();

  auto& files = assets->scanExtension("effectsource");
  assets->queueJsons(files);
  for (auto const& file : files) {
    auto sourceConfig = std::make_shared<EffectSourceConfig>(assets->json(file));
    if (m_sourceConfigs.contains(sourceConfig->kind()))
      throw StarException(
        strf("Duplicate effect source asset kind Name {}. configfile {}", sourceConfig->kind(), file));
    auto k = sourceConfig->kind().toLower();
    m_sourceConfigs[k] = sourceConfig;
  }
}

auto EffectSourceDatabase::effectSourceConfig(String const& kind) const -> Ptr<EffectSourceConfig> {
  auto k = kind.toLower();
  if (!m_sourceConfigs.contains(k))
    throw StarException(strf("Unknown effect source definition with kind '{}'.", kind));
  return m_sourceConfigs.get(k);
}

EffectSourceConfig::EffectSourceConfig(Json const& config) {
  m_kind = config.getString("kind");
  m_config = config;
}

auto EffectSourceConfig::kind() -> String const& {
  return m_kind;
}

auto EffectSourceConfig::instance(String const& suggestedSpawnLocation) -> Ptr<EffectSource> {
  return std::make_shared<EffectSource>(kind(), suggestedSpawnLocation, m_config.getObject("definition"));
}

EffectSource::EffectSource(String const& kind, String suggestedSpawnLocation, Json const& definition) {
  m_kind = kind;
  m_config = definition;
  m_expired = false;
  m_loopDuration = m_config.getFloat("duration", 0);
  m_durationVariance = m_config.getFloat("durationVariance", 0);
  m_loops = m_config.getBool("loops", m_loopDuration != 0);
  m_timer = Random::randf() * (m_loopDuration + 0.5 * m_durationVariance);
  m_stop = false;
  m_initialTick = true;
  m_loopTick = false;
  m_finalTick = false;
  m_effectSpawnLocation = m_config.getString("location", "normal");
  m_suggestedSpawnLocation = suggestedSpawnLocation;
}

auto EffectSource::kind() const -> String const& {
  return m_kind;
}

auto EffectSource::expired() const -> bool {
  return m_expired;
}

void EffectSource::stop() {
  m_stop = true;
}

void EffectSource::tick(float dt) {
  m_timer -= dt;
  if ((m_timer <= 0) && m_loops) {
    m_timer = m_loopDuration + m_durationVariance * Random::randf(-0.5f, 0.5f);
    m_loopTick = true;
  }
  if (m_stop || (m_timer <= 0))
    if (!m_expired)
      m_finalTick = true;
}

auto EffectSource::particles() -> List<String> {
  auto pickParticleSources = [](Json const& config, List<String>& particles) -> void {
    particles.appendAll(jsonToStringList(Random::randValueFrom(config.toArray(), JsonArray())));
  };
  List<String> result;
  if (m_initialTick)
    pickParticleSources(m_config.get("start", JsonObject()).get("particles", JsonArray()), result);
  if (m_loopTick)
    pickParticleSources(m_config.get("particles", JsonArray()), result);
  if (m_finalTick)
    pickParticleSources(m_config.get("stop", JsonObject()).get("particles", JsonArray()), result);
  return result;
}

auto EffectSource::sounds(Vec2F offset) -> List<Ptr<AudioInstance>> {
  List<Ptr<AudioInstance>> result;
  if (m_initialTick) {
    result.appendAll(soundsFromDefinition(m_config.get("start", JsonObject()).get("sounds", Json()), offset));

    m_mainSounds = soundsFromDefinition(m_config.get("sounds", Json()), offset);
    result.appendAll(m_mainSounds);
  }
  if (m_finalTick) {
    for (auto& s : m_mainSounds)
      s->stop();
    result.appendAll(soundsFromDefinition(m_config.get("stop", JsonObject()).get("sounds", Json()), offset));
  }
  return result;
}

void EffectSource::postRender() {
  m_initialTick = false;
  m_loopTick = false;
  if (m_finalTick) {
    m_finalTick = false;
    m_expired = true;
  }
}

auto EffectSource::effectSpawnLocation() const -> String {
  if ((m_effectSpawnLocation == "normal") && (!m_suggestedSpawnLocation.empty()))
    return m_suggestedSpawnLocation;
  return m_effectSpawnLocation;
}

auto EffectSource::suggestedSpawnLocation() const -> String {
  return m_suggestedSpawnLocation;
}

auto particlesFromDefinition(Json const& config, Vec2F const& position) -> List<Particle> {
  Json particles;
  if (config.type() == Json::Type::Array)
    particles = Random::randValueFrom(config.toArray(), Json());
  else
    particles = config;
  if (!particles.isNull()) {
    if (particles.type() != Json::Type::Array)
      particles = JsonArray{particles};
    List<Particle> result;
    for (auto entry : particles.iterateArray()) {
      if (entry.type() != Json::Type::Object) {
        result.append(Root::singleton().particleDatabase()->particle(entry.toString()));
      } else {
        Particle particle(entry.toObject());
        Particle variance(entry.getObject("variance", {}));
        particle.applyVariance(variance);
        particle.position += position;
        result.append(particle);
      }
    }
    return result;
  }
  return {};
}

auto soundsFromDefinition(Json const& config, Vec2F const& position) -> List<Ptr<AudioInstance>> {
  Json sound;
  if (config.type() == Json::Type::Array)
    sound = Random::randValueFrom(config.toArray(), Json());
  else
    sound = config;
  if (!sound.isNull()) {
    if (sound.type() != Json::Type::Array)
      sound = JsonArray{sound};
    List<Ptr<AudioInstance>> result;
    auto assets = Root::singleton().assets();
    for (auto entry : sound.iterateArray()) {
      if (entry.type() != Json::Type::Object) {
        JsonObject t;
        t["resource"] = entry.toString();
        entry = t;
      }

      auto sample = std::make_shared<AudioInstance>(*assets->audio(entry.getString("resource")));
      sample->setLoops(entry.getInt("loops", 0));
      sample->setVolume(entry.getFloat("volume", 1.0f));
      sample->setPitchMultiplier(entry.getFloat("pitch", 1.0f) + Random::randf(-1, 1) * entry.getFloat("pitchVariability", 0.0f));
      sample->setRangeMultiplier(entry.getFloat("audioRangeMultiplier", 1.0f));
      sample->setPosition(position);

      result.append(std::move(sample));
    }
    return result;
  }
  return {};
}

}// namespace Star

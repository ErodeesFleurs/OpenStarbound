import star.json;
import star.json_extra;
import star.game_types;
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
import star.random;
import star.audio;
import star.map;
import star.set;
import star.vector;
import star.maybe;

import star.animation;
import star.particle;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.mixer;

import star.effect_source_database;
import star.particle_database;

namespace Star {

EffectSourceDatabase::EffectSourceDatabase(Root& root) {
  auto assets = root.assets();

  auto& files = assets->scanExtension("effectsource");
  assets->queueJsons(files);
  for (auto const& file : files) {
    auto sourceConfig = std::make_shared<EffectSourceConfig>(root, assets->json(file));
    if (m_sourceConfigs.contains(sourceConfig->kind()))
      throw StarException(
          strf("Duplicate effect source asset kind Name {}. configfile {}", sourceConfig->kind(), file));
    auto k = sourceConfig->kind().toLower();
    m_sourceConfigs[k] = sourceConfig;
  }
}

std::shared_ptr<EffectSourceConfig> EffectSourceDatabase::effectSourceConfig(String const& kind) const {
  auto k = kind.toLower();
  if (!m_sourceConfigs.contains(k))
    throw StarException(strf("Unknown effect source definition with kind '{}'.", kind));
  return m_sourceConfigs.get(k);
}

EffectSourceConfig::EffectSourceConfig(Root& root, Json const& config) : m_root(root) {
  m_kind = config.getString("kind");
  m_config = config;
}

String const& EffectSourceConfig::kind() {
  return m_kind;
}

std::shared_ptr<EffectSource> EffectSourceConfig::instance(String const& suggestedSpawnLocation) {
  return std::make_shared<EffectSource>(m_root, kind(), suggestedSpawnLocation, m_config.getObject("definition"));
}

EffectSource::EffectSource(Root& root, String const& kind, String suggestedSpawnLocation, Json const& definition) : m_root(root) {
  m_kind = kind;
  m_config = definition;
  m_expired = false;
  m_loopDuration = m_config.getFloat("duration", 0);
  m_durationVariance = m_config.getFloat("durationVariance", 0);
  m_loops = m_config.getBool("loops", m_loopDuration != 0);
  m_timer = m_root.random().randf() * (m_loopDuration + 0.5 * m_durationVariance);
  m_stop = false;
  m_initialTick = true;
  m_loopTick = false;
  m_finalTick = false;
  m_effectSpawnLocation = m_config.getString("location", "normal");
  m_suggestedSpawnLocation = suggestedSpawnLocation;
}

String const& EffectSource::kind() const {
  return m_kind;
}

bool EffectSource::expired() const {
  return m_expired;
}

void EffectSource::stop() {
  m_stop = true;
}

void EffectSource::tick(float dt) {
  m_timer -= dt;
  if ((m_timer <= 0) && m_loops) {
    m_timer = m_loopDuration + m_durationVariance * m_root.random().randf(-0.5f, 0.5f);
    m_loopTick = true;
  }
  if (m_stop || (m_timer <= 0))
    if (!m_expired)
      m_finalTick = true;
}

List<String> EffectSource::particles() {
  auto pickParticleSources = [this](Json const& config, List<String>& particles) {
    particles.appendAll(jsonToStringList(m_root.random().randValueFrom(config.toArray(), JsonArray())));
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

List<std::shared_ptr<AudioInstance>> EffectSource::sounds(Vec2F offset) {
  List<std::shared_ptr<AudioInstance>> result;
  if (m_initialTick) {
    result.appendAll(soundsFromDefinition(m_root, m_config.get("start", JsonObject()).get("sounds", Json()), offset));

    m_mainSounds = soundsFromDefinition(m_root, m_config.get("sounds", Json()), offset);
    result.appendAll(m_mainSounds);
  }
  if (m_finalTick) {
    for (auto& s : m_mainSounds)
      s->stop();
    result.appendAll(soundsFromDefinition(m_root, m_config.get("stop", JsonObject()).get("sounds", Json()), offset));
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

String EffectSource::effectSpawnLocation() const {
  if ((m_effectSpawnLocation == "normal") && (!m_suggestedSpawnLocation.empty()))
    return m_suggestedSpawnLocation;
  return m_effectSpawnLocation;
}

String EffectSource::suggestedSpawnLocation() const {
  return m_suggestedSpawnLocation;
}

List<Particle> particlesFromDefinition(Root& root, Json const& config, Vec2F const& position) {
  Json particles;
  if (config.type() == Json::Type::Array)
    particles = root.random().randValueFrom(config.toArray(), Json());
  else
    particles = config;
  if (!particles.isNull()) {
    if (particles.type() != Json::Type::Array)
      particles = JsonArray{particles};
    List<Particle> result;
    for (auto entry : particles.iterateArray()) {
      if (entry.type() != Json::Type::Object) {
        result.append(root.particleDatabase()->particle(entry.toString()));
      } else {
        Particle particle(root, entry.toObject());
        Particle variance(root, entry.getObject("variance", {}));
        particle.applyVariance(root, variance);
        particle.position += position;
        result.append(particle);
      }
    }
    return result;
  }
  return {};
}

List<std::shared_ptr<AudioInstance>> soundsFromDefinition(Root& root, Json const& config, Vec2F const& position) {
  Json sound;
  if (config.type() == Json::Type::Array)
    sound = root.random().randValueFrom(config.toArray(), Json());
  else
    sound = config;
  if (!sound.isNull()) {
    if (sound.type() != Json::Type::Array)
      sound = JsonArray{sound};
    List<std::shared_ptr<AudioInstance>> result;
    auto assets = root.assets();
    for (auto entry : sound.iterateArray()) {
      if (entry.type() != Json::Type::Object) {
        JsonObject t;
        t["resource"] = entry.toString();
        entry = t;
      }

      auto sample = std::make_shared<AudioInstance>(*assets->audio(entry.getString("resource")));
      sample->setLoops(entry.getInt("loops", 0));
      sample->setVolume(entry.getFloat("volume", 1.0f));
      sample->setPitchMultiplier(entry.getFloat("pitch", 1.0f) + root.random().randf(-1, 1) * entry.getFloat("pitchVariability", 0.0f));
      sample->setRangeMultiplier(entry.getFloat("audioRangeMultiplier", 1.0f));
      sample->setPosition(position);

      result.append(std::move(sample));
    }
    return result;
  }
  return {};
}

}

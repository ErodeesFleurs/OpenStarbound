module;
import std;
import star.string;
import star.list;
import star.json;

import star.vector;

namespace Star {
class Root;

class AudioInstance;
struct Particle;

class EffectSource {
public:
  EffectSource(Root& root, String const& kind, String suggestedSpawnLocation, Json const& definition);
  String const& kind() const;
  void tick(float dt);
  bool expired() const;
  void stop();
  List<String> particles();
  List<std::shared_ptr<AudioInstance>> sounds(Vec2F offset);
  void postRender();
  String effectSpawnLocation() const;
  String suggestedSpawnLocation() const;

private:
  Root& m_root;
  String m_kind;
  Json m_config;
  bool m_loops;
  float m_loopDuration;
  float m_durationVariance;
  String m_effectSpawnLocation;
  String m_suggestedSpawnLocation;

  bool m_initialTick;
  bool m_loopTick;
  bool m_finalTick;
  float m_timer;
  bool m_expired;
  bool m_stop;

  List<std::shared_ptr<AudioInstance>> m_mainSounds;
};

class EffectSourceConfig {
public:
  EffectSourceConfig(Root& root, Json const& config);
  String const& kind();
  std::shared_ptr<EffectSource> instance(String const& suggestedSpawnLocation);

private:
  Root& m_root;
  String m_kind;
  Json m_config;
};

class EffectSourceDatabase {
public:
  explicit EffectSourceDatabase(Root& root);

  std::shared_ptr<EffectSourceConfig> effectSourceConfig(String const& kind) const;

private:
  StringMap<std::shared_ptr<EffectSourceConfig>> m_sourceConfigs;
};

List<Particle> particlesFromDefinition(Root& root, Json const& config, Vec2F const& position = Vec2F());
List<std::shared_ptr<AudioInstance>> soundsFromDefinition(Root& root, Json const& config, Vec2F const& position = Vec2F());

}

export module star.effect_source_database;

export namespace Star {
  using ::Star::AudioInstance;
  using ::Star::Particle;
  using ::Star::EffectSource;
  using ::Star::EffectSourceConfig;
  using ::Star::EffectSourceDatabase;
  using ::Star::particlesFromDefinition;
  using ::Star::soundsFromDefinition;
}

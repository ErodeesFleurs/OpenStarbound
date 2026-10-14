module;
import std;
import star.string;
import star.list;
import star.json;

import star.vector;

namespace Star {

class AudioInstance;
using AudioInstancePtr = std::shared_ptr<AudioInstance>;
using AudioInstanceConstPtr = std::shared_ptr<const AudioInstance>;
using AudioInstanceWeakPtr = std::weak_ptr<AudioInstance>;
using AudioInstanceConstWeakPtr = std::weak_ptr<const AudioInstance>;
using AudioInstanceUPtr = std::unique_ptr<AudioInstance>;
using AudioInstanceConstUPtr = std::unique_ptr<const AudioInstance>;
class EffectSource;
using EffectSourcePtr = std::shared_ptr<EffectSource>;
using EffectSourceConstPtr = std::shared_ptr<const EffectSource>;
using EffectSourceWeakPtr = std::weak_ptr<EffectSource>;
using EffectSourceConstWeakPtr = std::weak_ptr<const EffectSource>;
using EffectSourceUPtr = std::unique_ptr<EffectSource>;
using EffectSourceConstUPtr = std::unique_ptr<const EffectSource>;
class EffectSourceConfig;
using EffectSourceConfigPtr = std::shared_ptr<EffectSourceConfig>;
using EffectSourceConfigConstPtr = std::shared_ptr<const EffectSourceConfig>;
using EffectSourceConfigWeakPtr = std::weak_ptr<EffectSourceConfig>;
using EffectSourceConfigConstWeakPtr = std::weak_ptr<const EffectSourceConfig>;
using EffectSourceConfigUPtr = std::unique_ptr<EffectSourceConfig>;
using EffectSourceConfigConstUPtr = std::unique_ptr<const EffectSourceConfig>;
class EffectSourceDatabase;
using EffectSourceDatabasePtr = std::shared_ptr<EffectSourceDatabase>;
using EffectSourceDatabaseConstPtr = std::shared_ptr<const EffectSourceDatabase>;
using EffectSourceDatabaseWeakPtr = std::weak_ptr<EffectSourceDatabase>;
using EffectSourceDatabaseConstWeakPtr = std::weak_ptr<const EffectSourceDatabase>;
using EffectSourceDatabaseUPtr = std::unique_ptr<EffectSourceDatabase>;
using EffectSourceDatabaseConstUPtr = std::unique_ptr<const EffectSourceDatabase>;
struct Particle;

class EffectSource {
public:
  EffectSource(String const& kind, String suggestedSpawnLocation, Json const& definition);
  String const& kind() const;
  void tick(float dt);
  bool expired() const;
  void stop();
  List<String> particles();
  List<AudioInstancePtr> sounds(Vec2F offset);
  void postRender();
  String effectSpawnLocation() const;
  String suggestedSpawnLocation() const;

private:
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

  List<AudioInstancePtr> m_mainSounds;
};

class EffectSourceConfig {
public:
  EffectSourceConfig(Json const& config);
  String const& kind();
  EffectSourcePtr instance(String const& suggestedSpawnLocation);

private:
  String m_kind;
  Json m_config;
};

class EffectSourceDatabase {
public:
  EffectSourceDatabase();

  EffectSourceConfigPtr effectSourceConfig(String const& kind) const;

private:
  StringMap<EffectSourceConfigPtr> m_sourceConfigs;
};

List<Particle> particlesFromDefinition(Json const& config, Vec2F const& position = Vec2F());
List<AudioInstancePtr> soundsFromDefinition(Json const& config, Vec2F const& position = Vec2F());

}

export module star.effect_source_database;

export namespace Star {
  using ::Star::AudioInstance;
  using ::Star::AudioInstancePtr;
  using ::Star::AudioInstanceConstPtr;
  using ::Star::Particle;
  using ::Star::EffectSource;
  using ::Star::EffectSourcePtr;
  using ::Star::EffectSourceConstPtr;
  using ::Star::EffectSourceConfig;
  using ::Star::EffectSourceConfigPtr;
  using ::Star::EffectSourceConfigConstPtr;
  using ::Star::EffectSourceDatabase;
  using ::Star::EffectSourceDatabasePtr;
  using ::Star::EffectSourceDatabaseConstPtr;
  using ::Star::particlesFromDefinition;
  using ::Star::soundsFromDefinition;
}

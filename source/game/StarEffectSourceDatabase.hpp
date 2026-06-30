#pragma once

#include "StarVector.hpp"
#include "StarJson.hpp"
#include "StarThread.hpp"
#include "StarParticle.hpp"
#include "StarParticleDatabase.hpp"
#include "StarAssets.hpp"

namespace Star {

class AudioInstance;
using AudioInstancePtr = SharedPtr<AudioInstance>;
class EffectSource;
using EffectSourcePtr = SharedPtr<EffectSource>;
class EffectSourceConfig;
using EffectSourceConfigPtr = SharedPtr<EffectSourceConfig>;
class EffectSourceDatabase;
using EffectSourceDatabasePtr = SharedPtr<EffectSourceDatabase>;
using EffectSourceDatabaseConstPtr = SharedPtr<EffectSourceDatabase const>;

class EffectSource {
public:
  EffectSource(AssetsConstPtr assets, String const& kind, String suggestedSpawnLocation, Json const& definition);
  [[nodiscard]] String const& kind() const;
  void tick(float dt);
  [[nodiscard]] bool expired() const;
  void stop();
  [[nodiscard]] List<String> particles();
  [[nodiscard]] List<AudioInstancePtr> sounds(Vec2F offset);
  void postRender();
  [[nodiscard]] String effectSpawnLocation() const;
  [[nodiscard]] String suggestedSpawnLocation() const;

private:
  AssetsConstPtr m_assets;
  String m_kind;
  Json m_config;
  bool m_loops;
  float m_loopDuration;
  float m_durationVariance;
  String m_effectSpawnLocation;
  String m_suggestedSpawnLocation;

  bool m_initialTick = true;
  bool m_loopTick = false;
  bool m_finalTick = false;
  float m_timer;
  bool m_expired = false;
  bool m_stop = false;

  List<AudioInstancePtr> m_mainSounds;
};

class EffectSourceConfig {
public:
  EffectSourceConfig(AssetsConstPtr assets, Json const& config);
  [[nodiscard]] String const& kind();
  [[nodiscard]] EffectSourcePtr instance(String const& suggestedSpawnLocation);

private:
  AssetsConstPtr m_assets;
  String m_kind;
  Json m_config;
};

class EffectSourceDatabase {
public:
  EffectSourceDatabase(AssetsConstPtr assets);

  [[nodiscard]] EffectSourceConfigPtr effectSourceConfig(String const& kind) const;

private:
  StringMap<EffectSourceConfigPtr> m_sourceConfigs;
};

[[nodiscard]] List<Particle> particlesFromDefinition(Json const& config, Vec2F const& position, ParticleDatabaseConstPtr particleDatabase);
[[nodiscard]] List<AudioInstancePtr> soundsFromDefinition(AssetsConstPtr assets, Json const& config, Vec2F const& position = Vec2F());

}

#pragma once

#include "StarConfig.hpp"
#include "StarJson.hpp"
#include "StarParticle.hpp"
#include "StarVector.hpp"

namespace Star {

class AudioInstance;

class EffectSource {
public:
  EffectSource(String const& kind, String suggestedSpawnLocation, Json const& definition);
  [[nodiscard]] auto kind() const -> String const&;
  void tick(float dt);
  [[nodiscard]] auto expired() const -> bool;
  void stop();
  auto particles() -> List<String>;
  auto sounds(Vec2F offset) -> List<Ptr<AudioInstance>>;
  void postRender();
  [[nodiscard]] auto effectSpawnLocation() const -> String;
  [[nodiscard]] auto suggestedSpawnLocation() const -> String;

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

  List<Ptr<AudioInstance>> m_mainSounds;
};

class EffectSourceConfig {
public:
  EffectSourceConfig(Json const& config);
  auto kind() -> String const&;
  auto instance(String const& suggestedSpawnLocation) -> Ptr<EffectSource>;

private:
  String m_kind;
  Json m_config;
};

class EffectSourceDatabase {
public:
  EffectSourceDatabase();

  [[nodiscard]] auto effectSourceConfig(String const& kind) const -> Ptr<EffectSourceConfig>;

private:
  StringMap<Ptr<EffectSourceConfig>> m_sourceConfigs;
};

auto particlesFromDefinition(Json const& config, Vec2F const& position = Vec2F()) -> List<Particle>;
auto soundsFromDefinition(Json const& config, Vec2F const& position = Vec2F()) -> List<Ptr<AudioInstance>>;

}// namespace Star

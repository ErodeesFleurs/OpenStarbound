#pragma once

#include "StarParticle.hpp"
#include "StarWeightedPool.hpp"

namespace Star {

struct WeatherType {
  struct ParticleConfig {
    Particle particle;
    float density;
    bool autoRotate;
  };

  struct ProjectileConfig {
    String projectile;
    Json parameters;
    Vec2F velocity;
    float ratePerX;
    int spawnAboveRegion;
    int spawnHorizontalPad;
    float windAffectAmount;
  };

  WeatherType();
  WeatherType(Json config, String path = String());

  [[nodiscard]] auto toJson() const -> Json;

  String name;

  List<ParticleConfig> particles;
  List<ProjectileConfig> projectiles;
  StringList statusEffects;

  float maximumWind;
  Vec2F duration;
  StringList weatherNoises;
};

using WeatherPool = WeightedPool<String>;

auto operator>>(DataStream& ds, WeatherType& weatherType) -> DataStream&;
auto operator<<(DataStream& ds, WeatherType const& weatherType) -> DataStream&;
}// namespace Star

#pragma once

#include "StarJson.hpp"
#include "StarParticle.hpp"

namespace Star {

class World;

struct EntitySplashConfig {
  EntitySplashConfig();
  EntitySplashConfig(Json const& config);
  float splashSpeedMin;
  Vec2F splashBottomSensor;
  Vec2F splashTopSensor;
  float splashMinWaterLevel;
  int numSplashParticles;
  Particle splashParticle;
  Particle splashParticleVariance;
  float splashYVelocityFactor;

  auto doSplash(Vec2F position, Vec2F velocity, World* world) const -> List<Particle>;
};

}// namespace Star

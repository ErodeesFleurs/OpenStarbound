#pragma once

#include "StarJson.hpp"
#include "StarParticle.hpp"
#include "StarAssets.hpp"

namespace Star {

class World;
struct EntitySplashConfig;
class EntitySplashHelper;

struct EntitySplashConfig {
  EntitySplashConfig() = default;
  EntitySplashConfig(Json const& config, AssetsConstPtr assets);
  float splashSpeedMin = 0.0f;
  Vec2F splashBottomSensor;
  Vec2F splashTopSensor;
  float splashMinWaterLevel = 0.0f;
  int numSplashParticles = 0;
  Particle splashParticle;
  Particle splashParticleVariance;
  float splashYVelocityFactor = 0.0f;

  List<Particle> doSplash(Vec2F position, Vec2F velocity, World& world) const;
};

}

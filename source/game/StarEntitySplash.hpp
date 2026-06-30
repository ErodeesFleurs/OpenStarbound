#pragma once

#include "StarJson.hpp"
#include "StarParticle.hpp"
#include "StarAssets.hpp"

namespace Star {

class World;
struct EntitySplashConfig;
class EntitySplashHelper;

struct EntitySplashConfig {
  EntitySplashConfig();
  EntitySplashConfig(Json const& config, AssetsConstPtr assets);
  float splashSpeedMin;
  Vec2F splashBottomSensor;
  Vec2F splashTopSensor;
  float splashMinWaterLevel;
  int numSplashParticles;
  Particle splashParticle;
  Particle splashParticleVariance;
  float splashYVelocityFactor;

  List<Particle> doSplash(Vec2F position, Vec2F velocity, World* world) const;
};

}

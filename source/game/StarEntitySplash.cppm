module;
import std;
import star.json;
import star.format;

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
import star.animation;
import star.particle;

import star.vector;
namespace Star {

class World;
class Root;
class EntitySplashHelper;

struct EntitySplashConfig {
  EntitySplashConfig();
  [[nodiscard]] static std::expected<EntitySplashConfig, StarException> create(Root& root, Json const& config);
  float splashSpeedMin;
  Vec2F splashBottomSensor;
  Vec2F splashTopSensor;
  float splashMinWaterLevel;
  int numSplashParticles;
  Particle splashParticle;
  Particle splashParticleVariance;
  float splashYVelocityFactor;

  [[nodiscard]] std::expected<List<Particle>, StarException> doSplash(Vec2F position, Vec2F velocity, World& world) const;
};

}

export module star.entity_splash;

export namespace Star {
  using ::Star::World;
  using ::Star::EntitySplashConfig;
  using ::Star::EntitySplashHelper;
}

module;
import std;
import star.json;

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
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
struct EntitySplashConfig;
using EntitySplashConfigPtr = std::shared_ptr<EntitySplashConfig>;
using EntitySplashConfigConstPtr = std::shared_ptr<const EntitySplashConfig>;
using EntitySplashConfigWeakPtr = std::weak_ptr<EntitySplashConfig>;
using EntitySplashConfigConstWeakPtr = std::weak_ptr<const EntitySplashConfig>;
using EntitySplashConfigUPtr = std::unique_ptr<EntitySplashConfig>;
using EntitySplashConfigConstUPtr = std::unique_ptr<const EntitySplashConfig>;
class EntitySplashHelper;
using EntitySplashHelperPtr = std::shared_ptr<EntitySplashHelper>;
using EntitySplashHelperConstPtr = std::shared_ptr<const EntitySplashHelper>;
using EntitySplashHelperWeakPtr = std::weak_ptr<EntitySplashHelper>;
using EntitySplashHelperConstWeakPtr = std::weak_ptr<const EntitySplashHelper>;
using EntitySplashHelperUPtr = std::unique_ptr<EntitySplashHelper>;
using EntitySplashHelperConstUPtr = std::unique_ptr<const EntitySplashHelper>;

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

  List<Particle> doSplash(Vec2F position, Vec2F velocity, World* world) const;
};

}

export module star.entity_splash;

export namespace Star {
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::EntitySplashConfig;
  using ::Star::EntitySplashConfigPtr;
  using ::Star::EntitySplashConfigConstPtr;
  using ::Star::EntitySplashConfigWeakPtr;
  using ::Star::EntitySplashConfigConstWeakPtr;
  using ::Star::EntitySplashConfigUPtr;
  using ::Star::EntitySplashConfigConstUPtr;
  using ::Star::EntitySplashHelper;
  using ::Star::EntitySplashHelperPtr;
  using ::Star::EntitySplashHelperConstPtr;
  using ::Star::EntitySplashHelperWeakPtr;
  using ::Star::EntitySplashHelperConstWeakPtr;
  using ::Star::EntitySplashHelperUPtr;
  using ::Star::EntitySplashHelperConstUPtr;
}

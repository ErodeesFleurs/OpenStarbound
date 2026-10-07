module;

#include "StarJson.hpp"
#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
import star.asset_path;
import star.animation;
import star.particle;

namespace Star {

STAR_CLASS(World);
STAR_STRUCT(EntitySplashConfig);
STAR_CLASS(EntitySplashHelper);

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
